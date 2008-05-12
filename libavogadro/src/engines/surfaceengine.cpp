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

#include <openbabel/math/vector3.h>
#include <openbabel/griddata.h>
#include <openbabel/grid.h>

#include <eigen/projective.h>

#include <QGLWidget>
#include <QDebug>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro {

  SurfaceEngine::SurfaceEngine(QObject *parent) : Engine(parent), m_settingsWidget(0),
  m_alpha(0.5), m_stepSize(0.33333), m_padding(2.5), m_renderMode(0), m_colorMode(0)
  {
    setDescription(tr("Surface rendering"));
    //m_grid = new Grid;
    m_vdwThread = new VDWGridThread;
    m_isoGen = new IsoGen;
    connect(m_vdwThread, SIGNAL(finished()), this, SLOT(vdwThreadFinished()));
    connect(m_isoGen, SIGNAL(finished()), this, SLOT(isoGenFinished()));
    m_color = Color(1.0, 0.0, 0.0, m_alpha);
    m_surfaceValid = false;
      
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

    // Delete the settings widget if it exists
    if(m_settingsWidget)
      m_settingsWidget->deleteLater();
  }

  Engine *SurfaceEngine::clone() const
  {
    SurfaceEngine *engine = new SurfaceEngine(parent());
    engine->setName(name());
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
  Color SurfaceEngine::espColor(Molecule *mol, Vector3f &pos)
  {
    GLfloat red, green, blue;
    double energy = 0.0;
    vector3 p, dist;

    p.SetX(pos.x());
    p.SetY(pos.y());
    p.SetZ(pos.z());

    FOR_ATOMS_OF_MOL (atom, mol) {
      dist = atom->GetVector() - p;
      energy += atom->GetPartialCharge() / (dist.length()*dist.length());
    }

    // Chemistry convention: red = negative, blue = positive

    if (energy < 0.0) {
      red = -20.0*energy;
      if (red >= 1.0) {
        return Color(1.0, 0.0, 0.0, m_alpha);
      }

      green = 1.0 - red;
      return Color(red, green, 0.0, m_alpha);
    }

    if (energy > 0.0) {
      blue = 20.0*energy;
      if (blue >= 1.0) {
        return Color(0.0, 0.0, 1.0, m_alpha);
      }
      green = 1.0 - blue;
      return Color(0.0, green, blue, m_alpha);
    }

    return Color(0.0, 1.0, 0.0, m_alpha);
  }

  bool SurfaceEngine::renderOpaque(PainterDevice *pd)
  {
    // Don't try to render anything while the surface is being calculated.
    if (m_vdwThread->isRunning())
      return false;
    Molecule *mol = const_cast<Molecule *>(pd->molecule());
    if (!m_surfaceValid)
    {
      //VDWSurface(mol);

      PrimitiveList prims = primitives();
      m_vdwThread->init(mol, prims, pd);
      m_vdwThread->start();

      //m_isoGen->init(m_grid, pd);
      //m_isoGen->start();
      m_surfaceValid = true;
      return true;
    }

    qDebug() << "Number of triangles = " << m_isoGen->numTriangles();

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glEnable(GL_BLEND);
    glShadeModel(GL_SMOOTH);

    pd->painter()->setColor(1.0, 0.0, 0.0, m_alpha);
    m_color.applyAsMaterials();

    switch (m_renderMode) {
    case 0:
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      break;
    case 1:
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      break;
    case 2:
      glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
      break;
    }

    if (m_clip) 
    {

    GLdouble eq[4] = {m_clipEqA, m_clipEqB, m_clipEqC, m_clipEqD};
    glEnable(GL_CLIP_PLANE0);
    glClipPlane(GL_CLIP_PLANE0, eq);
    // Rendering the mesh's clip edge 
    glEnable(GL_STENCIL_TEST);
    glClear(GL_STENCIL_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    // first pass: increment stencil buffer value on back faces
    glStencilFunc(GL_ALWAYS, 0, 0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
    glCullFace(GL_FRONT); // render back faces only
    doWork(mol);
    // second pass: decrement stencil buffer value on front faces
    glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
    glCullFace(GL_BACK); // render front faces only
    doWork(mol);
    // drawing clip planes masked by stencil buffer content
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CLIP_PLANE0);
    glStencilFunc(GL_NOTEQUAL, 0, ~0); 
    // stencil test will pass only when stencil buffer value = 0; 
    // (~0 = 0x11...11)
    glPushMatrix();
    Vector3f normalEq(m_clipEqA, m_clipEqB, m_clipEqC);
    Vector3f point1(-m_clipEqD / normalEq.norm(),  1000.,  1000.);
    Vector3f point2(-m_clipEqD / normalEq.norm(),  1000., -1000.);
    Vector3f point3(-m_clipEqD / normalEq.norm(), -1000.,  1000.);
    Vector3f point4(-m_clipEqD / normalEq.norm(), -1000., -1000.);

    if ( (m_clipEqB == 0.0) && (m_clipEqC == 0.0) ) {
      if (m_clipEqA < 0.0 ) {
        MatrixP3f mat;
        Vector3f zAxis(0., 0., 1.);
        mat.loadRotation3(M_PI, zAxis);

        point1 = mat * point1;    
        point2 = mat * point2;    
        point3 = mat * point3;    
        point4 = mat * point4;    
      }
    } else {
      MatrixP3f mat;
      Vector3f normal(1., 0., 0.);
      normalEq.normalize();
      double angle = acos(normal.dot(normalEq));
      Vector3f axis = normal.cross(normalEq);
      axis.normalize();
      mat.loadRotation3(angle, axis);

      point1 = mat * point1;    
      point2 = mat * point2;    
      point3 = mat * point3;    
      point4 = mat * point4;    
    }

    glBegin(GL_QUADS); // rendering the plane quad. Note, it should be big 
                       // enough to cover all clip edge area.
    GLfloat points[4][3] = { {0.0,  1000.0,  1000.0},
                             {0.0,  1000.0, -1000.0},
                             {0.0, -1000.0,  1000.0},
                             {0.0, -1000.0, -1000.0}};
    glVertex3fv(point1.array());
    glVertex3fv(point2.array());
    glVertex3fv(point4.array());
    glVertex3fv(point3.array());
    glEnd();
    glPopMatrix();
    // End rendering mesh's clip edge
    // Rendering mesh  
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_CLIP_PLANE0); // enabling clip plane again
    }

    doWork(mol);

    glPopAttrib();

    return true;
  }

  void SurfaceEngine::doWork(Molecule *mol) {
    glBegin(GL_TRIANGLES);
    if (m_colorMode == 1) { // ESP
      Color color;
      for(int i=0; i < m_isoGen->numTriangles(); ++i)
      {
        triangle t = m_isoGen->getTriangle(i);
        triangle n = m_isoGen->getNormal(i);

        color = espColor(mol, t.p0);
        color.applyAsMaterials();
        glNormal3fv(n.p0.array());
        glVertex3fv(t.p0.array());

        color = espColor(mol, t.p1);
        color.applyAsMaterials();
        glNormal3fv(n.p1.array());
        glVertex3fv(t.p1.array());

        color = espColor(mol, t.p2);
        color.applyAsMaterials();
        glNormal3fv(n.p2.array());
        glVertex3fv(t.p2.array());
      }
    } else { // RGB
      //m_color.applyAsMaterials();
      for(int i=0; i < m_isoGen->numTriangles(); ++i)
      {
        triangle t = m_isoGen->getTriangle(i);
        triangle n = m_isoGen->getNormal(i);

        glNormal3fv(n.p0.array());
        glVertex3fv(t.p0.array());

        glNormal3fv(n.p1.array());
        glVertex3fv(t.p1.array());

        glNormal3fv(n.p2.array());
        glVertex3fv(t.p2.array());
      }
    }
    glEnd();
  }

  inline double SurfaceEngine::radius(const Atom *a) const
  {
    return etab.GetVdwRad(a->GetAtomicNum());
  }

  double SurfaceEngine::radius(const PainterDevice *, const Primitive *p) const
  {
    // Atom radius
    if (p->type() == Primitive::AtomType)
    {
      if(primitives().contains(p))
      {
        //if (pd && pd->isSelected(p))
        //{
        //  return radius(static_cast<const Atom *>(p)) + SEL_ATOM_EXTRA_RADIUS;
        //}
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

  Engine::EngineFlags SurfaceEngine::flags() const
  {
    return Engine::Transparent | Engine::Atoms;
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
      if (value == 1) { // ESP
        m_settingsWidget->customColorButton->setEnabled(false);
      } else { // Custom color
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
     
      // clipping stuff
      connect(m_settingsWidget->clipCheckBox, SIGNAL(stateChanged(int)), this, SLOT(setClipEnabled(int)));
      connect(m_settingsWidget->ASpinBox, SIGNAL(valueChanged(double)), this, SLOT(setClipEqA(double)));
      connect(m_settingsWidget->BSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setClipEqB(double)));
      connect(m_settingsWidget->CSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setClipEqC(double)));
      connect(m_settingsWidget->DSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setClipEqD(double)));
      // clipping stuff

      m_settingsWidget->opacitySlider->setValue(20*m_alpha);
      m_settingsWidget->renderCombo->setCurrentIndex(m_renderMode);
      m_settingsWidget->colorCombo->setCurrentIndex(m_colorMode);
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
    //if (!m_mutex.tryLock())
    //  return;

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

    m_molecule = molecule;
    m_primitives = primitives;

    m_mutex.unlock();
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
    //if (!m_mutex.tryLock())
    //  return;

    //if (m_grid->grid() != NULL) // we already calculated this
    //  return;

    QList<Primitive*> surfaceAtoms = m_primitives.subList(Primitive::AtomType);
    OBFloatGrid grid;
    grid.Init(*m_molecule, m_stepSize, 2.5);
    vector3 min;
    int xDim, yDim, zDim;

    min = grid.GetMin();

    xDim = grid.GetXdim();
    yDim = grid.GetYdim();
    zDim = grid.GetZdim();

    vector3 coord;
    double distance, minDistance;

    // Now set up our VdW grid
    OBGridData *vdwGrid = new OBGridData;
    vector3 xAxis, yAxis, zAxis;
    xAxis = vector3(m_stepSize, 0.0, 0.0);
    yAxis = vector3(0.0, m_stepSize, 0.0);
    zAxis = vector3(0.0, 0.0, m_stepSize);

    vdwGrid->SetNumberOfPoints(xDim, yDim, zDim);
    vdwGrid->SetLimits(min, xAxis, yAxis, zAxis);

    for (int i = 0; i < xDim; ++i) {
      coord.SetX(min[0] + i * m_stepSize);
      for (int j = 0; j < yDim; ++j) {
        coord.SetY(min[1] + j * m_stepSize);
        for (int k = 0; k < zDim; ++k)
        {
          coord.SetZ(min[2] + k * m_stepSize);
          minDistance = 1.0E+10;
	        for (int ai=0; ai < surfaceAtoms.size(); ai++)
	        {
            distance = sqrt(coord.distSq(static_cast<Atom*>(surfaceAtoms[ai])->GetVector()));
            distance -= etab.GetVdwRad(static_cast<Atom*>(surfaceAtoms[ai])->GetAtomicNum());

            if (distance < minDistance)
              minDistance = distance;
          } // end checking atoms
          // negative = away from molecule, 0 = vdw surface, positive = inside
          vdwGrid->SetValue(i, j, k, -minDistance);
        } // z-axis
      } // y-axis
    } // x-axis

    m_grid->setGrid(vdwGrid);
    m_grid->setIsoValue(0.0);

    m_mutex.unlock();
  }

  void SurfaceEngine::writeSettings(QSettings &settings) const
  {
    Engine::writeSettings(settings);
    settings.setValue("opacity", 20*m_alpha);
    settings.setValue("renderMode", m_renderMode);
    settings.setValue("colorMode", m_colorMode);
    /*
    settings.setValue("colorRed", m_color.red());
    settings.setValue("colorGreen", m_color.green());
    settings.setValue("colorBlue", m_color.blue());
    */
  }

  void SurfaceEngine::readSettings(QSettings &settings)
  {
    Engine::readSettings(settings);
    setOpacity(settings.value("opacity", 20).toInt());
    setRenderMode(settings.value("renderMode", 0).toInt());
    setColorMode(settings.value("colorMode", 0).toInt());
    m_color.set(settings.value("colorRed", 1.0).toDouble(),
                     settings.value("colorGreen", 0.0).toDouble(),
		     settings.value("colorBlue", 0.0).toDouble());

    /*
    m_color.setRed(settings.value("colorRed", 1.0).toDouble());
    m_color.setGreen(settings.value("colorGreen", 0.0).toDouble());
    m_color.setBlue(settings.value("colorBlue", 0.0).toDouble());
    */
    if(m_settingsWidget)
    {
      m_settingsWidget->opacitySlider->setValue(20*m_alpha);
      m_settingsWidget->renderCombo->setCurrentIndex(m_renderMode);
      m_settingsWidget->colorCombo->setCurrentIndex(m_colorMode);
      QColor initial;
      initial.setRgbF(m_color.red(), m_color.green(), m_color.blue());
      m_settingsWidget->customColorButton->setColor(initial);
    }
  }

}

#include "surfaceengine.moc"

Q_EXPORT_PLUGIN2(surfaceengine, Avogadro::SurfaceEngineFactory)
