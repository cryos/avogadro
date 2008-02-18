/**********************************************************************
  OrbitalEngine - Engine for display of isosurfaces

  Copyright (C) 2008 Geoffrey R. Hutchison
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

#include <config.h>
#include "orbitalengine.h"

#include <avogadro/primitive.h>
#include <avogadro/color.h>

#include <openbabel/math/vector3.h>
#include <openbabel/griddata.h>
#include <openbabel/grid.h>

#include <QGLWidget>
#include <QDebug>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro {

  OrbitalEngine::OrbitalEngine(QObject *parent) : Engine(parent), m_settingsWidget(0),
  m_grid(0), m_isoGen(0), m_min(0., 0., 0.),
  m_alpha(0.75), m_stepSize(0.33333), m_iso(0.01), m_renderMode(0), m_update(true)
  {
    setDescription(tr("Orbital rendering"));
    m_grid = new Grid;
    m_grid2 = new Grid;
    m_isoGen = new IsoGen;
    m_isoGen2 = new IsoGen;
    connect(m_isoGen, SIGNAL(finished()), this, SLOT(isoGenFinished()));
    m_posColor = Color(1.0, 0.0, 0.0, m_alpha);
    m_negColor = Color(0.0, 0.0, 1.0, m_alpha);
  }

  OrbitalEngine::~OrbitalEngine()
  {
    delete m_grid;
    delete m_grid2;
    delete m_isoGen;
    delete m_isoGen2;

    // Delete the settings widget if it exists
    if(m_settingsWidget)
      m_settingsWidget->deleteLater();
  }

  Engine *OrbitalEngine::clone() const
  {
    OrbitalEngine *engine = new OrbitalEngine(parent());
    engine->setName(name());
    engine->setEnabled(isEnabled());

    return engine;
  }

  bool OrbitalEngine::renderOpaque(PainterDevice *pd)
  {
    Molecule *mol = const_cast<Molecule *>(pd->molecule());

    if (!mol->HasData(OBGenericDataType::GridData))
    {
      // ultimately allow the user to attach a new data file
      qDebug() << "No grid data found -> no orbitals.";
      return false;
    }
    else
    {
      qDebug() << "Molecular orbital grid found!";
      m_grid->setGrid(static_cast<OBGridData *>(mol->GetData(OBGenericDataType::GridData)));
      m_grid2->setGrid(static_cast<OBGridData *>(mol->GetData(OBGenericDataType::GridData)));
    }

    qDebug() << " set surface ";

    qDebug() << "Min value = " << m_grid->grid()->GetMinValue()
             << "Max value = " << m_grid->grid()->GetMaxValue();

    // Find the minima for the grid
    m_min = Vector3f(m_grid->grid()->GetOriginVector().x(),
        m_grid->grid()->GetOriginVector().y(),
        m_grid->grid()->GetOriginVector().z());

    qDebug() << "Origin: " << m_min.x() << m_min.y() << m_min.z();

    // For orbitals, we'll need to set this iso value and make sure it's
    // for +/- 0.001 for example
    // We may need some logic to check if a cube is an orbital or not...
    // (e.g., someone might bring in spin density = always positive)
    if (m_update)
    {
      m_grid->setIsoValue(m_iso);
      m_isoGen->init(m_grid, m_stepSize, m_min);
      m_isoGen->start();
      m_grid2->setIsoValue(-m_iso);
      m_isoGen2->init(m_grid2, m_stepSize, m_min);
      m_isoGen2->start();
      m_update = false;
    }

    qDebug() << " rendering surface ";

    glPushAttrib(GL_ALL_ATTRIB_BITS);
//    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glShadeModel(GL_SMOOTH);

//    glPushName(Primitive::SurfaceType);
//    glPushName(1);

    qDebug() << "Number of triangles = " << m_isoGen->numTriangles();

    switch (m_renderMode) {
    case 0:
      glPolygonMode(GL_FRONT, GL_FILL);
      break;
    case 1:
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      break;
    case 2:
      glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
      break;
    }

    glBegin(GL_TRIANGLES);
      m_posColor.applyAsMaterials();
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
    glEnd();

    glBegin(GL_TRIANGLES);
      m_negColor.applyAsMaterials();
      for(int i=0; i < m_isoGen2->numTriangles(); ++i)
      {
        triangle t = m_isoGen2->getTriangle(i);
        triangle n = m_isoGen2->getNormal(i);

        // In order to fix the lighting reverse the normals too
        n.p0 *= -1;
        n.p1 *= -1;
        n.p2 *= -1;

        glNormal3fv(n.p2.array());
        glVertex3fv(t.p2.array());

        glNormal3fv(n.p1.array());
        glVertex3fv(t.p1.array());

        glNormal3fv(n.p0.array());
        glVertex3fv(t.p0.array());
      }
    glEnd();

    glPopAttrib();

    return true;
  }

  double OrbitalEngine::transparencyDepth() const
  {
    return 1.0;
  }

  Engine::EngineFlags OrbitalEngine::flags() const
  {
    return Engine::Transparent | Engine::Atoms;
  }

  void OrbitalEngine::setOpacity(int value)
  {
    m_alpha = 0.05 * value;
    emit changed();
  }

  void OrbitalEngine::setRenderMode(int value)
  {
    m_renderMode = value;
    emit changed();
  }

  void OrbitalEngine::setStepSize(double d)
  {
    m_stepSize = d;
    m_update = true;
    emit changed();
  }

  void OrbitalEngine::setIso(double d)
  {
    m_iso = d;
    m_update = true;
    emit changed();
  }

  void OrbitalEngine::setPosColor(QColor color)
  {
    m_posColor.set(color.redF(), color.greenF(), color.blueF(), m_alpha);
    emit changed();
  }

  void OrbitalEngine::setNegColor(QColor color)
  {
    m_negColor.set(color.redF(), color.greenF(), color.blueF(), m_alpha);
    emit changed();
  }

  QWidget* OrbitalEngine::settingsWidget()
  {
    if(!m_settingsWidget)
    {
      m_settingsWidget = new OrbitalSettingsWidget();
      connect(m_settingsWidget->opacitySlider, SIGNAL(valueChanged(int)), this, SLOT(setOpacity(int)));
      connect(m_settingsWidget->renderCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setRenderMode(int)));
      connect(m_settingsWidget->stepSizeSpin, SIGNAL(valueChanged(double)), this, SLOT(setStepSize(double)));
      connect(m_settingsWidget->isoSpin, SIGNAL(valueChanged(double)), this, SLOT(setIso(double)));
      connect(m_settingsWidget->posColor, SIGNAL(colorChanged(QColor)), this, SLOT(setPosColor(QColor)));
      connect(m_settingsWidget->negColor, SIGNAL(colorChanged(QColor)), this, SLOT(setNegColor(QColor)));
      connect(m_settingsWidget, SIGNAL(destroyed()), this, SLOT(settingsWidgetDestroyed()));

      // Initialise the colour buttons
      QColor initial;
      initial.setRgbF(m_posColor.red(), m_posColor.green(), m_posColor.blue());
      m_settingsWidget->posColor->setColor(initial);
      initial.setRgbF(m_negColor.red(), m_negColor.green(), m_negColor.blue());
      m_settingsWidget->negColor->setColor(initial);
    }
    return m_settingsWidget;
  }

  void OrbitalEngine::isoGenFinished()
  {
    emit changed();
  }

  void OrbitalEngine::settingsWidgetDestroyed()
  {
    qDebug() << "Destroyed Settings Widget";
    m_settingsWidget = 0;
  }

  void OrbitalEngine::setPrimitives(const PrimitiveList &primitives)
  {
    Engine::setPrimitives(primitives);
    m_update = true;
  }

  void OrbitalEngine::addPrimitive(Primitive *primitive)
  {
    Engine::addPrimitive(primitive);
    m_update = true;
  }

  void OrbitalEngine::updatePrimitive(Primitive *)
  {
    m_update = true;
  }

  void OrbitalEngine::removePrimitive(Primitive *primitive)
  {
    Engine::removePrimitive(primitive);
    m_update = true;
  }

  /*
  void OrbitalEngine::writeSettings(QSettings &settings) const
  {
    Engine::writeSettings(settings);
    settings.setValue("alpha", m_alpha);
    settings.setValue("stepSize", m_stepSize);
    settings.setValue("padding", m_padding);
    //settings.setValue("renderMode", m_renderMode);
    //settings.setValue("colorMode", m_colorMode);
  }

  void OrbitalEngine::readSettings(QSettings &settings)
  {
    Engine::readSettings(settings);
    //m_alpha = settings.value("alpha", 0.5).toDouble();
    m_stepSize = settings.value("stepSize", 0.33333).toDouble();
    m_padding = settings.value("padding", 2.5).toDouble();
    m_renderMode = 0;
    m_colorMode = 0;
  }
  */
}

#include "orbitalengine.moc"

Q_EXPORT_PLUGIN2(orbitalengine, Avogadro::OrbitalEngineFactory)
