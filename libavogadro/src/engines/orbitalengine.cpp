/**********************************************************************
  OrbitalEngine - Engine for display of molecular orbitals

  Copyright (C) 2008 Marcus D. Hanwell
  Copyright (C) 2008 Geoffrey R. Hutchison
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

#include "orbitalengine.h"

#include <config.h>
#include <avogadro/primitive.h>

#include <openbabel/math/vector3.h>
#include <openbabel/griddata.h>
#include <openbabel/grid.h>

#include <QGLWidget>
#include <QReadWriteLock>
#include <QDebug>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro {

  OrbitalEngine::OrbitalEngine(QObject *parent) : Engine(parent),
    m_settingsWidget(0), m_min(0., 0., 0.), m_alpha(0.75), m_iso(0.01),
    m_renderMode(0), m_drawBox(false), m_update(true), m_molecule(0)
  {
    setDescription(tr("Orbital Rendering"));
    m_grid = new Grid;
    m_grid2 = new Grid;
    m_isoGen = new IsoGen;
    m_isoGen2 = new IsoGen;
    connect(m_isoGen, SIGNAL(finished()), this, SLOT(isoGenFinished()));
    connect(m_isoGen2, SIGNAL(finished()), this, SLOT(isoGenFinished()));
    m_negColor = Color(1.0, 0.0, 0.0, m_alpha);
    m_posColor = Color(0.0, 0.0, 1.0, m_alpha);
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
    engine->setAlias(alias());
    engine->setEnabled(isEnabled());

    return engine;
  }

  bool OrbitalEngine::renderOpaque(PainterDevice *pd)
  {
    // Render the opaque surface if m_alpha is 1
    if (m_alpha >= 0.999)
    {
      if (m_update)
        updateSurfaces(pd);

      switch (m_renderMode)
      {
      case 0:
        glPolygonMode(GL_FRONT, GL_FILL);
        break;
      case 1:
        glPolygonMode(GL_FRONT, GL_LINE);
        glDisable(GL_LIGHTING);
        break;
      case 2:
        glPolygonMode(GL_FRONT, GL_POINT);
        glDisable(GL_LIGHTING);
        break;
      }

      renderSurfaces(pd);

      if (m_renderMode)
      {
        glPolygonMode(GL_FRONT, GL_FILL);
        glEnable(GL_LIGHTING);
      }
    }

    return true;
  }

  bool OrbitalEngine::renderTransparent(PainterDevice *pd)
  {
    // Render the transparent surface if m_alpha is between 0 and 1.
    if (m_alpha > 0.001 && m_alpha < 0.999)
    {
      if (m_update)
        updateSurfaces(pd);

      switch (m_renderMode)
      {
      case 0:
        glPolygonMode(GL_FRONT, GL_FILL);
        glEnable(GL_BLEND);
        glDepthMask(GL_TRUE);
        break;
      case 1:
        glPolygonMode(GL_FRONT, GL_LINE);
        glDisable(GL_LIGHTING);
        break;
      case 2:
        glPolygonMode(GL_FRONT, GL_POINT);
        glDisable(GL_LIGHTING);
        break;
      }

      renderSurfaces(pd);

      if (m_renderMode == 0)
      {
        glDisable(GL_BLEND);
        glDepthMask(GL_FALSE);
      }
      else
      {
        glPolygonMode(GL_FRONT, GL_FILL);
        glEnable(GL_LIGHTING);
      }
    }
    return true;
  }

  bool OrbitalEngine::renderQuick(PainterDevice *pd)
  {
    if (m_update)
      updateSurfaces(pd);

    switch (m_renderMode)
    {
      case 0:
        ;
      case 1:
        glPolygonMode(GL_FRONT, GL_LINE);
        glDisable(GL_LIGHTING);
        break;
      case 2:
        glPolygonMode(GL_FRONT, GL_POINT);
        glDisable(GL_LIGHTING);
        break;
    }

    renderSurfaces(pd);

    glPolygonMode(GL_FRONT, GL_FILL);
    glEnable(GL_LIGHTING);

    return true;
  }

  bool OrbitalEngine::renderSurfaces(PainterDevice *pd)
  {
    glBegin(GL_TRIANGLES);

    // Render the positive surface
    m_posColor.apply();
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

    // Render the negative surface
    m_negColor.apply();
    m_negColor.applyAsMaterials();
    for(int i=0; i < m_isoGen2->numTriangles(); ++i)
    {
      triangle t = m_isoGen2->getTriangle(i);
      triangle n = m_isoGen2->getNormal(i);
      // Fix the lighting by reversing the normals and the triangle winding
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

    // Draw the extents of the cube if requested to
    if (m_drawBox) {
      pd->painter()->setColor(1.0, 1.0, 1.0);

      pd->painter()->drawLine(Vector3d(m_min.x(), m_min.y(), m_min.z()),
                              Vector3d(m_max.x(), m_min.y(), m_min.z()), 1.0);
      pd->painter()->drawLine(Vector3d(m_min.x(), m_min.y(), m_min.z()),
                              Vector3d(m_max.x(), m_min.y(), m_min.z()), 1.0);
      pd->painter()->drawLine(Vector3d(m_min.x(), m_min.y(), m_min.z()),
                              Vector3d(m_min.x(), m_max.y(), m_min.z()), 1.0);
      pd->painter()->drawLine(Vector3d(m_min.x(), m_min.y(), m_min.z()),
                              Vector3d(m_min.x(), m_min.y(), m_max.z()), 1.0);

      pd->painter()->drawLine(Vector3d(m_max.x(), m_min.y(), m_min.z()),
                              Vector3d(m_max.x(), m_max.y(), m_min.z()), 1.0);
      pd->painter()->drawLine(Vector3d(m_max.x(), m_min.y(), m_min.z()),
                              Vector3d(m_max.x(), m_min.y(), m_max.z()), 1.0);

      pd->painter()->drawLine(Vector3d(m_min.x(), m_max.y(), m_min.z()),
                              Vector3d(m_max.x(), m_max.y(), m_min.z()), 1.0);
      pd->painter()->drawLine(Vector3d(m_min.x(), m_max.y(), m_min.z()),
                              Vector3d(m_min.x(), m_max.y(), m_max.z()), 1.0);

      pd->painter()->drawLine(Vector3d(m_min.x(), m_min.y(), m_max.z()),
                              Vector3d(m_min.x(), m_max.y(), m_max.z()), 1.0);
      pd->painter()->drawLine(Vector3d(m_min.x(), m_min.y(), m_max.z()),
                              Vector3d(m_max.x(), m_min.y(), m_max.z()), 1.0);

      pd->painter()->drawLine(Vector3d(m_max.x(), m_max.y(), m_max.z()),
                              Vector3d(m_max.x(), m_max.y(), m_min.z()), 1.0);
      pd->painter()->drawLine(Vector3d(m_max.x(), m_max.y(), m_max.z()),
                              Vector3d(m_max.x(), m_min.y(), m_max.z()), 1.0);
      pd->painter()->drawLine(Vector3d(m_max.x(), m_max.y(), m_max.z()),
                              Vector3d(m_min.x(), m_max.y(), m_max.z()), 1.0);
    }

    return true;
  }

  void OrbitalEngine::updateSurfaces(PainterDevice *pd)
  {
    // Attempt to find a grid
    Molecule *mol = const_cast<Molecule *>(pd->molecule());
    if (!mol->HasData(OBGenericDataType::GridData))
      return;
    else
    {
      if (!m_settingsWidget)
      {
        // Use first grid/orbital
        m_grid->setGrid(static_cast<OBGridData *>(mol->GetData(OBGenericDataType::GridData)));
        m_grid2->setGrid(static_cast<OBGridData *>(mol->GetData(OBGenericDataType::GridData)));
      }
      else
      {
        if (!m_settingsWidget->orbitalCombo->count())
        {
          // Use first grid/orbital
          // Two grids -- one for positive isovalue, one for negative
          m_grid->setGrid(static_cast<OBGridData *>(mol->GetData(OBGenericDataType::GridData)));
          m_grid2->setGrid(static_cast<OBGridData *>(mol->GetData(OBGenericDataType::GridData)));

          // Add the orbitals
          m_molecule = mol;
          connect(m_molecule, SIGNAL(updated()),
                  this, SLOT(updateOrbitalCombo()));
          updateOrbitalCombo();
        }
        else
        {
          vector<OBGenericData*> data = mol->GetAllData(OBGenericDataType::GridData);
          unsigned int index = m_settingsWidget->orbitalCombo->currentIndex();
          if (index >= data.size())
          {
            qDebug() << "Invalid orbital selected.";
            return;
          }
          m_grid->setGrid(static_cast<OBGridData *>(data[index]));
          m_grid2->setGrid(static_cast<OBGridData *>(data[index]));
        }
      }
    }

    // attribute is the text key for the grid (as an std::string)
    qDebug() << " Orbital title: " << m_grid->grid()->GetAttribute().c_str();

    qDebug() << "Min value = " << m_grid->grid()->GetMinValue()
             << "Max value = " << m_grid->grid()->GetMaxValue();

    // Find the minima for the grid
    m_min = Vector3f(m_grid->grid()->GetOriginVector().x(),
                     m_grid->grid()->GetOriginVector().y(),
                     m_grid->grid()->GetOriginVector().z());
    m_max = Vector3f(m_grid->grid()->GetMaxVector().x(),
                     m_grid->grid()->GetMaxVector().y(),
                     m_grid->grid()->GetMaxVector().z());

    // We may need some logic to check if a cube is an orbital or not...
    // (e.g., someone might bring in spin density = always positive)
    m_grid->setIsoValue(m_iso);
    m_isoGen->init(m_grid, pd, false);
    m_isoGen->start();
    m_grid2->setIsoValue(-m_iso);
    m_isoGen2->init(m_grid2, pd, false);
    m_isoGen2->start();
    m_update = false;
  }

  void OrbitalEngine::updateOrbitalCombo()
  {
    // Reset the orbital combo
    int tmp = m_settingsWidget->orbitalCombo->currentIndex();
    if (tmp < 0) tmp = 0;
    m_settingsWidget->orbitalCombo->clear();
    m_molecule->lock()->lockForRead();
    vector<OBGenericData*> data = m_molecule->GetAllData(OBGenericDataType::GridData);
    for (unsigned int i = 0; i < data.size(); ++i) {
      QString str = QString(data[i]->GetAttribute().c_str());
      m_settingsWidget->orbitalCombo->addItem(str);
    }
    // If all of the orbitals disappear the molecule has been cleared
    if (data.size() == 0) {
      m_grid->setGrid(0);
      m_grid2->setGrid(0);
      disconnect(m_isoGen, 0, this, 0);
      disconnect(m_isoGen2, 0, this, 0);
      delete m_isoGen;
      m_isoGen = new IsoGen;
      delete m_isoGen2;
      m_isoGen2 = new IsoGen;
      connect(m_isoGen, SIGNAL(finished()), this, SLOT(isoGenFinished()));
      connect(m_isoGen2, SIGNAL(finished()), this, SLOT(isoGenFinished()));
    }
    m_molecule->lock()->unlock();
    m_settingsWidget->orbitalCombo->setCurrentIndex(tmp);
  }

  double OrbitalEngine::transparencyDepth() const
  {
    return 1.0;
  }

  Engine::EngineFlags OrbitalEngine::flags() const
  {
    return Engine::Transparent | Engine::Atoms;
  }

  void OrbitalEngine::setOrbital(int)
  {
    m_update = true;
    emit changed();
  }

  void OrbitalEngine::setOpacity(int value)
  {
    m_alpha = 0.05 * value;
    m_posColor.setAlpha(m_alpha);
    m_negColor.setAlpha(m_alpha);
    emit changed();
  }

  void OrbitalEngine::setRenderMode(int value)
  {
    m_renderMode = value;
    emit changed();
  }

  void OrbitalEngine::setDrawBox(int value)
  {
    if (value == 0) m_drawBox = false;
    else m_drawBox = true;
    emit changed();
  }


  void OrbitalEngine::setIso(double d)
  {
    m_iso = d;
  }

  void OrbitalEngine::isoDone()
  {
    m_update = true;
    emit changed();
  }

  void OrbitalEngine::setPosColor(const QColor& color)
  {
    m_posColor.set(color.redF(), color.greenF(), color.blueF(), m_alpha);
    emit changed();
  }

  void OrbitalEngine::setNegColor(const QColor& color)
  {
    m_negColor.set(color.redF(), color.greenF(), color.blueF(), m_alpha);
    emit changed();
  }

  QWidget* OrbitalEngine::settingsWidget()
  {
    if(!m_settingsWidget)
    {
      m_settingsWidget = new OrbitalSettingsWidget();
      connect(m_settingsWidget->orbitalCombo, SIGNAL(currentIndexChanged(int)),
              this, SLOT(setOrbital(int)));
      connect(m_settingsWidget->opacitySlider, SIGNAL(valueChanged(int)),
              this, SLOT(setOpacity(int)));
      connect(m_settingsWidget->renderCombo, SIGNAL(currentIndexChanged(int)),
              this, SLOT(setRenderMode(int)));
      connect(m_settingsWidget->drawBoxCheck, SIGNAL(stateChanged(int)),
              this, SLOT(setDrawBox(int)));
      connect(m_settingsWidget->isoSpin, SIGNAL(valueChanged(double)),
              this, SLOT(setIso(double)));
      connect(m_settingsWidget->isoSpin, SIGNAL(editingFinished()),
              this, SLOT(isoDone()));
      connect(m_settingsWidget->posColor, SIGNAL(colorChanged(QColor)),
              this, SLOT(setPosColor(QColor)));
      connect(m_settingsWidget->negColor, SIGNAL(colorChanged(QColor)),
              this, SLOT(setNegColor(QColor)));
      connect(m_settingsWidget, SIGNAL(destroyed()),
              this, SLOT(settingsWidgetDestroyed()));

      // Initialise the widget from saved settings
      m_settingsWidget->opacitySlider->setValue(static_cast<int>(m_alpha * 20));
      m_settingsWidget->isoSpin->setValue(m_iso);
      m_settingsWidget->renderCombo->setCurrentIndex(m_renderMode);
      m_settingsWidget->drawBoxCheck->setChecked(m_drawBox);

      // Initialise the colour buttons
      QColor initial;
      initial.setRgbF(m_posColor.red(), m_posColor.green(), m_posColor.blue());
      m_settingsWidget->posColor->setColor(initial);
      initial.setRgbF(m_negColor.red(), m_negColor.green(), m_negColor.blue());
      m_settingsWidget->negColor->setColor(initial);
      m_update = true;
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
    // This is used to load new molecules and so there could be a new cube file
    m_update = true;
  }

  void OrbitalEngine::addPrimitive(Primitive *primitive)
  {
    Engine::addPrimitive(primitive);
    // Updating primitives does not invalidate these surfaces...
  }

  void OrbitalEngine::updatePrimitive(Primitive *)
  {
    // Updating primitives does not invalidate these surfaces...
  }

  void OrbitalEngine::removePrimitive(Primitive *primitive)
  {
    Engine::removePrimitive(primitive);
    // Updating primitives does not invalidate these surfaces...
  }

  void OrbitalEngine::writeSettings(QSettings &settings) const
  {
    Engine::writeSettings(settings);
    settings.setValue("alpha", m_alpha);
    settings.setValue("iso", m_iso);
    settings.setValue("renderMode", m_renderMode);
    settings.setValue("drawBox", m_drawBox);
//    settings.setValue("posColor", m_posColor);
//    settings.setValue("posColor", m_negColor);
  }

  void OrbitalEngine::readSettings(QSettings &settings)
  {
    Engine::readSettings(settings);
    m_alpha = settings.value("alpha", 0.5).toDouble();
    m_iso = settings.value("iso", 0.02).toDouble();
    m_renderMode = settings.value("renderMode", 0).toInt();
    m_drawBox = settings.value("drawBox", false).toBool();
  }

}

#include "orbitalengine.moc"

Q_EXPORT_PLUGIN2(orbitalengine, Avogadro::OrbitalEngineFactory)
