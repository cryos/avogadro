/**********************************************************************
  SurfaceEngine - Engine for display of isosurfaces

  Copyright (C) 2007 Geoffrey R. Hutchison
  Copyright (C) 2008 Marcus D. Hanwell
  Copyright (C) 2008 Tim Vandermeersch

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  Avogadro is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Avogadro is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#include "surfaceengine.h"

#include <config.h>
#include <avogadro/primitive.h>
#include <avogadro/atom.h>
#include <avogadro/cube.h>
#include <avogadro/mesh.h>
#include <avogadro/molecule.h>

#include <avogadro/boxcontrol.h>

#include <Eigen/Geometry>

#include <QGLWidget>
#include <QReadWriteLock>
#include <QDebug>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro {

  SurfaceEngine::SurfaceEngine(QObject *parent) : Engine(parent),
    m_settingsWidget(0), m_mesh(0), m_alpha(0.5), m_stepSize(0.33333),
    m_padding(2.5), m_renderMode(0), m_colorMode(0), m_drawBox(false)
  {
    setDescription(tr("Surface rendering"));
    m_vdwThread = new VDWGridThread;
    m_isoGen = new IsoGen;
    connect(m_vdwThread, SIGNAL(finished()), this, SLOT(vdwThreadFinished()));
    connect(m_isoGen, SIGNAL(finished()), this, SLOT(isoGenFinished()));
    m_color = Color(1.0, 0.0, 0.0, m_alpha);
    m_surfaceValid = false;

    m_boxControl = new BoxControl;
    connect(m_boxControl, SIGNAL(modified()), this, SLOT(boxModified()));

    // clipping stuff
    m_clip = false;
    m_clipEqA =1.0;
    m_clipEqB =0.0;
    m_clipEqC =0.0;
    m_clipEqD =0.0;
    // clipping stuff
  }

  SurfaceEngine::~SurfaceEngine()
  {
    //delete m_grid;
    delete m_isoGen;
    delete m_vdwThread;
    delete m_boxControl;

    // Delete the settings widget if it exists
    if(m_settingsWidget)
      m_settingsWidget->deleteLater();
  }

  Engine *SurfaceEngine::clone() const
  {
    SurfaceEngine *engine = new SurfaceEngine(parent());
    engine->setAlias(alias());
    engine->setEnabled(isEnabled());

    return engine;
  }

  //                                          //
  //     |    red    green     blue           //
  // 1.0 |...--+       +       +--...         //
  //     |      \     / \     /               //
  //     |       \   /   \   /                //
  //     |        \ /     \ /                 //
  //     |         X       X                  //
  //     |        / \     / \                 //
  //     |       /   \   /   \                //
  //     |      /     \ /     \               //
  // 0.0 +...--+-------+-------+--...-->      //
  //           a      0.0      b      energy
  //
  //  a = 20 * energy
  //  b = 20 * energy
  //
  QColor SurfaceEngine::espColor(Molecule *, const Vector3f &pos)
  {
    GLfloat red, green, blue;
    double energy = 0.0;
    Vector3f dist;

    QList<Primitive *> list;
    // Get a list of atoms and calculate the dipole moment
    list = primitives().subList(Primitive::AtomType);

    foreach(const Primitive *p, list) {
      const Atom *a = static_cast<const Atom *>(p);
      dist = a->pos()->cast<float>() - pos;
      energy += a->partialCharge() / dist.squaredNorm();
    }

    // Chemistry convention: red = negative, blue = positive
    QColor color;

    if (energy < 0.0) {
      red = -20.0*energy;
      if (red >= 1.0) {
        color.setRgbF(1.0, 0.0, 0.0, m_alpha);
        return color;
      }

      green = 1.0 - red;
      color.setRgbF(red, green, 0.0, m_alpha);
      return color;
    }

    if (energy > 0.0) {
      blue = 20.0*energy;
      if (blue >= 1.0) {
        color.setRgbF(0.0, 0.0, 1.0, m_alpha);
        return color;
      }
      green = 1.0 - blue;
      color.setRgbF(0.0, green, blue, m_alpha);
      return color;
    }

    color.setRgbF(0.0, 1.0, 0.0, m_alpha);
    return color;
  }

  bool SurfaceEngine::renderOpaque(PainterDevice *pd)
  {
    // Render the opaque surface if m_alpha is 1
    if (m_alpha >= 0.999)
    {
      if (m_mesh) {
        if (m_mesh->stable()) {
          pd->painter()->setColor(&m_color);
          pd->painter()->drawMesh(*m_mesh, m_renderMode);
        }
      }
    }
    return true;
  }

  bool SurfaceEngine::renderTransparent(PainterDevice *pd)
  {
    // Render the transparent surface if m_alpha is between 0 and 1.
    if (m_alpha > 0.001 && m_alpha < 0.999)
    {
      if (m_mesh) {
        if (m_mesh->stable()) {
          pd->painter()->setColor(&m_color);
          pd->painter()->drawMesh(*m_mesh, m_renderMode);
        }
      }
    }
    return true;
  }

  bool SurfaceEngine::renderQuick(PainterDevice *pd)
  {
    int renderMode = 1;
    if (m_renderMode == 2)
      renderMode = 2;

    if (m_mesh) {
      if (m_mesh->stable()) {
        pd->painter()->setColor(&m_color);
        pd->painter()->drawMesh(*m_mesh, renderMode);
      }
    }
    return true;
  }

  void SurfaceEngine::doWork(PainterDevice *pd, Molecule *mol)
  {
    if (m_colorMode == 1) { // ESP
      if (m_isoGen->mesh().colors().size() == 0) {
        // Generate the colours for the isosurface
        std::vector<QColor> colors;
        for(unsigned int i=0; i < m_isoGen->mesh().vertices().size(); ++i) {
          const Vector3f *v = m_isoGen->mesh().vertex(i);
          colors.push_back(espColor(mol, *v));
        }
        m_isoGen->mesh().setColors(colors);
      }
      pd->painter()->drawColorMesh(m_isoGen->mesh(), m_renderMode);
    }
    else { // RGB
      pd->painter()->drawMesh(m_isoGen->mesh(), m_renderMode);
    }

    if (m_drawBox)
      m_boxControl->addPrimitives();
  }

  inline double SurfaceEngine::radius(const Atom *a) const
  {
    return etab.GetVdwRad(a->atomicNumber());
  }

  double SurfaceEngine::radius(const PainterDevice *, const Primitive *p) const
  {
    // Atom radius
    if (p->type() == Primitive::AtomType)
    {
      if(primitives().contains(p))
      {
        return radius(static_cast<const Atom *>(p));
      }
    }
    // Something else
    return 0.;
  }

  double SurfaceEngine::transparencyDepth() const
  {
    return 1.0;
  }

  Engine::Layers SurfaceEngine::layers() const
  {
    return Engine::Opaque | Engine::Transparent;
  }

  Engine::PrimitiveTypes SurfaceEngine::primitiveTypes() const
  {
    return Engine::Atoms;
  }

  Engine::ColorTypes SurfaceEngine::colorTypes() const
  {
    return Engine::ColorGradients;
  }

  void SurfaceEngine::setOpacity(int value)
  {
    m_alpha = 0.05 * value;
    m_color.setAlpha(m_alpha);
    emit changed();
  }

  void SurfaceEngine::setRenderMode(int value)
  {
    m_renderMode = value;
    emit changed();
  }

  void SurfaceEngine::setColorMode(int value)
  {
    if (m_settingsWidget) {
      // Enable/Disable both the custom color widget and label
      if (value == 1) { // ESP
        m_settingsWidget->customColorLabel->setEnabled(false);
        m_settingsWidget->customColorButton->setEnabled(false);
      } else { // Custom color
        m_settingsWidget->customColorLabel->setEnabled(true);
        m_settingsWidget->customColorButton->setEnabled(true);
      }
    }

    m_colorMode = value;
    emit changed();
  }

  void SurfaceEngine::setColor(const QColor& color)
  {
    m_color.set(color.redF(), color.greenF(), color.blueF(), m_alpha);
    emit changed();
  }

  void SurfaceEngine::setDrawBox(int value)
  {
    if (value == 0) {
      m_drawBox = false;
      m_boxControl->removePrimitives();
    } else {
      m_drawBox = true;
      m_boxControl->addPrimitives();
    }

    emit changed();
  }

  void SurfaceEngine::boxModified()
  {
    m_surfaceValid = false;
    emit changed();
  }

  void SurfaceEngine::resetBox()
  {
    m_surfaceValid = false;
    m_boxControl->setModified(false);
    emit changed();
  }

  QWidget* SurfaceEngine::settingsWidget()
  {
    if(!m_settingsWidget)
    {
      m_settingsWidget = new SurfaceSettingsWidget();
      connect(m_settingsWidget->opacitySlider, SIGNAL(valueChanged(int)), this, SLOT(setOpacity(int)));
      connect(m_settingsWidget->renderCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setRenderMode(int)));
      connect(m_settingsWidget->colorCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setColorMode(int)));
      connect(m_settingsWidget->customColorButton, SIGNAL(colorChanged(QColor)), this, SLOT(setColor(QColor)));
      connect(m_settingsWidget, SIGNAL(destroyed()), this, SLOT(settingsWidgetDestroyed()));

      // draw box...
      connect(m_settingsWidget->drawBoxCheck, SIGNAL(stateChanged(int)),
              this, SLOT(setDrawBox(int)));

      m_settingsWidget->opacitySlider->setValue(static_cast<int>(20*m_alpha));
      m_settingsWidget->renderCombo->setCurrentIndex(m_renderMode);
      m_settingsWidget->colorCombo->setCurrentIndex(m_colorMode);
      m_settingsWidget->drawBoxCheck->setChecked(m_drawBox);
      if (m_colorMode == 1) { // ESP
        m_settingsWidget->customColorButton->setEnabled(false);
      } else { // Custom color
        m_settingsWidget->customColorButton->setEnabled(true);
      }
      QColor initial;
      initial.setRgbF(m_color.red(), m_color.green(), m_color.blue());
      m_settingsWidget->customColorButton->setColor(initial);
    }
    return m_settingsWidget;
  }

  void SurfaceEngine::vdwThreadFinished()
  {
    qDebug() << "vdwThreadFinished()";

    if (!m_boxControl->isModified())
      m_boxControl->setOppositeCorners( m_vdwThread->grid()->cube()->min(), m_vdwThread->grid()->cube()->max() );

    m_isoGen->init(m_vdwThread->grid(), 0, false, m_vdwThread->stepSize());
    m_isoGen->start();
  }

  void SurfaceEngine::isoGenFinished()
  {
    qDebug() << "isoGenFinished()";
    emit changed();
  }


  void SurfaceEngine::settingsWidgetDestroyed()
  {
    qDebug() << "Destroyed Settings Widget";
    m_settingsWidget = 0;
  }

  void SurfaceEngine::setPrimitives(const PrimitiveList &primitives)
  {
    Engine::setPrimitives(primitives);
    m_surfaceValid = false;
  }

  void SurfaceEngine::addPrimitive(Primitive *primitive)
  {
    if (primitive->type() == Primitive::AtomType) {
      m_surfaceValid = false;
    }
    Engine::addPrimitive(primitive);
  }

  void SurfaceEngine::updatePrimitive(Primitive *primitive)
  {
    if ((primitive->type() == Primitive::AtomType) || (primitive->type() == Primitive::MoleculeType)) {
      m_surfaceValid = false;
    }
    Engine::updatePrimitive(primitive);
  }

  void SurfaceEngine::removePrimitive(Primitive *primitive)
  {
    if (primitive->type() == Primitive::AtomType) {
      m_surfaceValid = false;
    }
    Engine::removePrimitive(primitive);
  }

  void SurfaceEngine::writeSettings(QSettings &settings) const
  {
    Engine::writeSettings(settings);
    settings.setValue("opacity", 20*m_alpha);
    settings.setValue("renderMode", m_renderMode);
    settings.setValue("colorMode", m_colorMode);
    settings.setValue("color", m_color.color());
    settings.setValue("drawBox", m_drawBox);
    if (m_mesh)
      settings.setValue("meshId", static_cast<int>(m_mesh->id()));
  }

  void SurfaceEngine::readSettings(QSettings &settings)
  {
    Engine::readSettings(settings);
    setOpacity(settings.value("opacity", 20).toInt());
    setRenderMode(settings.value("renderMode", 0).toInt());
    setColorMode(settings.value("colorMode", 0).toInt());
    m_color = settings.value("color").value<QColor>();
    m_color.setAlpha(m_alpha);
    setDrawBox(settings.value("drawBox").toBool());

    if (m_molecule)
      m_mesh = m_molecule->meshById(settings.value("meshId", 0).toInt());

    if(m_settingsWidget) {
      m_settingsWidget->opacitySlider->setValue(static_cast<int>(20*m_alpha));
      m_settingsWidget->renderCombo->setCurrentIndex(m_renderMode);
      m_settingsWidget->colorCombo->setCurrentIndex(m_colorMode);
      m_settingsWidget->drawBoxCheck->setChecked(m_drawBox);
      QColor initial;
      initial.setRgbF(m_color.red(), m_color.green(), m_color.blue());
      m_settingsWidget->customColorButton->setColor(initial);
    }
  }

  VDWGridThread::VDWGridThread(QObject *parent): QThread(parent), m_molecule(0),
    m_stepSize(0.0), m_padding(0.0)
  {
    m_grid = new Grid;
  }

  VDWGridThread::~VDWGridThread()
  {
    delete m_grid;
  }

  void VDWGridThread::init(Molecule *molecule, PrimitiveList &primitives, const PainterDevice* pd, double stepSize)
  {
    m_mutex.lock();

    initStepSize(pd, stepSize);

    m_molecule = molecule;
    m_primitives = primitives;
    m_boxControl = 0;

    m_mutex.unlock();
  }

  void VDWGridThread::init(Molecule *molecule, PrimitiveList &primitives, const PainterDevice* pd,
      BoxControl *boxControl, double stepSize)
  {
    m_mutex.lock();

    initStepSize(pd, stepSize);

    m_molecule = molecule;
    m_primitives = primitives;
    m_boxControl = boxControl;

    m_mutex.unlock();
  }

  void VDWGridThread::initStepSize(const PainterDevice* pd, double stepSize)
  {
    if (stepSize)
      m_stepSize = stepSize;
    else
    {
      // Work out the step size from the global quality level
      switch(pd->painter()->quality())
      {
      case 0:
        m_stepSize = 1.0;
        break;
      case 1:
        m_stepSize = 0.5;
        break;
      case 2:
        m_stepSize = 0.3;
        break;
      case 3:
        m_stepSize = 0.22;
        break;
      case 4:
        m_stepSize = 0.15;
        break;
      default:
        m_stepSize = 0.10;
      }
    }
  }


  Grid* VDWGridThread::grid()
  {
    return m_grid;
  }

  double VDWGridThread::stepSize()
  {
    return m_stepSize;
  }

  // We define a VDW surface here.
  // The isosurface finder declares values < 0 to be outside the surface
  // So values of 0.0 here equal the VDW surface of the molecule
  // + values = the distance inside the surface (i.e., closer to the atomic cente)
  // - values = the distance outside the surface (i.e., farther away)
  void VDWGridThread::run()
  {
    m_mutex.lock();

    // In order to minimise the need for locking but also reduce crashes I think
    // that making a local copy of the atoms concerned is the most efficient
    // method until we improve this function
    m_molecule->lock()->lockForRead();
    QList<Primitive*> pSurfaceAtoms = m_primitives.subList(Primitive::AtomType);
    QList<Vector3d> surfaceAtomsPos;
    QList<int> surfaceAtomsNum;
    foreach(Primitive* p, pSurfaceAtoms) {
      Atom* a = static_cast<Atom *>(p);
      surfaceAtomsPos.push_back(*a->pos());
      surfaceAtomsNum.push_back(a->atomicNumber());
    }
    m_molecule->lock()->unlock();

    Cube *cube = new Cube;
    if (!m_boxControl)
      cube->setLimits(m_molecule, m_stepSize, 2.5);
    else
      cube->setLimits(m_boxControl->min(), m_boxControl->max(), m_stepSize);
    Vector3d min = cube->min();
    Vector3d max = cube->max();
    Vector3i dim = cube->dimensions();

    Vector3d coord;
    double distance;

    for (int i = 0; i < dim.x(); ++i)
      for (int j = 0; j < dim.y(); ++j)
        for (int k = 0; k < dim.z(); ++k)
          cube->setValue(i, j, k, -1.0E+10);

    Vector3i index;
    int numBoxes = static_cast<int>(3.0 / m_stepSize);
    if (numBoxes < 4)
      numBoxes = 4;

    for (int ai=0; ai < surfaceAtomsPos.size(); ai++) {
      index = cube->indexVector(surfaceAtomsPos[ai]);

      for (int i = index[0] - numBoxes; i < index[0] + numBoxes; ++i) {
        if ((i < 0) || (i >= dim.x())) continue;
        coord(0) = min[0] + i * m_stepSize;
        for (int j = index[1] - numBoxes; j < index[1] + numBoxes; ++j) {
          if ((j < 0) || (j >= dim.y())) continue;
          coord(1) = min[1] + j * m_stepSize;
          for (int k = index[2] - numBoxes; k < index[2] + numBoxes; ++k) {
            if ((k < 0) || (k >= dim.z())) continue;
            coord(2) = min[2] + k * m_stepSize;
            distance = fabs((coord - surfaceAtomsPos[ai]).norm());
            distance -= etab.GetVdwRad(surfaceAtomsNum[ai]);
            const double value = cube->value(i, j, k);
            if ((value < -1.0E+9) || (distance < -value))
              cube->setValue(i, j, k, -distance);
          }
        }
      }
    }

    m_grid->setCube(cube);
    m_grid->setIsoValue(0.0);

    m_mutex.unlock();
  }

}

#include "surfaceengine.moc"

Q_EXPORT_PLUGIN2(surfaceengine, Avogadro::SurfaceEngineFactory)
