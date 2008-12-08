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

#include <avogadro/molecule.h>
#include <avogadro/cube.h>
#include <avogadro/meshgenerator.h>

#include <QReadWriteLock>
#include <QDebug>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro {

  OrbitalEngine::OrbitalEngine(QObject *parent) : Engine(parent),
    m_settingsWidget(0), m_mesh1(0), m_mesh2(0), m_min(0., 0., 0.), m_max(0.,0.,0.),
    m_alpha(0.75), m_iso(0.01), m_renderMode(0), m_drawBox(false),
    m_update(true), m_molecule(0)
  {
    setDescription(tr("Orbital Rendering"));
    m_meshGen1 = new MeshGenerator;
    m_meshGen2 = new MeshGenerator;
    connect(m_meshGen1, SIGNAL(finished()), this, SLOT(isoGenFinished()));
    connect(m_meshGen2, SIGNAL(finished()), this, SLOT(isoGenFinished()));

    m_negColor = Color(1.0, 0.0, 0.0, m_alpha);
    m_posColor = Color(0.0, 0.0, 1.0, m_alpha);
  }

  OrbitalEngine::~OrbitalEngine()
  {
    delete m_meshGen1;
    delete m_meshGen2;
    /// FIXME Meshes should be managed by displays, not engines!
    delete m_mesh1;
    delete m_mesh2;

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
      if (!m_mesh1 || !m_mesh2)
        return false;

      if (m_mesh1->stable()) {
        pd->painter()->setColor(&m_posColor);
        pd->painter()->drawMesh(*m_mesh1, m_renderMode);
      }

      if (m_mesh2->stable()) {
        pd->painter()->setColor(&m_negColor);
        pd->painter()->drawMesh(*m_mesh2, m_renderMode);
      }

      renderSurfaces(pd);
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
      if (!m_mesh1 || !m_mesh2)
        return false;

      if (m_renderMode == 0) {
        glEnable(GL_BLEND);
        glDepthMask(GL_TRUE);
      }

      if (m_mesh1->stable()) {
        pd->painter()->setColor(&m_posColor);
        pd->painter()->drawMesh(*m_mesh1, m_renderMode);
      }

      if (m_mesh2->stable()) {
        pd->painter()->setColor(&m_negColor);
        pd->painter()->drawMesh(*m_mesh2, m_renderMode);
      }

      renderSurfaces(pd);

      if (m_renderMode == 0) {
        glDisable(GL_BLEND);
        glDepthMask(GL_FALSE);
      }
    }
    return true;
  }

  bool OrbitalEngine::renderQuick(PainterDevice *pd)
  {
    if (m_update)
      updateSurfaces(pd);
    if (!m_mesh1 || !m_mesh2)
        return false;

    if (m_mesh1->stable() && m_mesh2->stable()) {
      if (m_renderMode < 2) {
        pd->painter()->setColor(&m_posColor);
        pd->painter()->drawMesh(*m_mesh1, 1);
        pd->painter()->setColor(&m_negColor);
        pd->painter()->drawMesh(*m_mesh2, 1);
      }
      else {
        pd->painter()->setColor(&m_posColor);
        pd->painter()->drawMesh(*m_mesh1, 2);
        pd->painter()->setColor(&m_negColor);
        pd->painter()->drawMesh(*m_mesh2, 2);
      }
      renderSurfaces(pd);
    }

    return true;
  }

  bool OrbitalEngine::renderSurfaces(PainterDevice *pd)
  {
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
    // Attempt to find a cube
    m_molecule = pd->molecule();
    int iCube = 0;
    QList<Cube *> cubes = pd->molecule()->cubes();
    if (cubes.size() == 0) {
      qDebug() << "No cubes.";
      return;
    }
    else {
      if (m_settingsWidget) {
        // Add the orbitals
        connect(m_molecule, SIGNAL(updated()),
                this, SLOT(updateOrbitalCombo()));
        if (!m_settingsWidget->orbitalCombo->count()) {
          // Use first cube/orbital
          // Two meshes -- one for positive isovalue, one for negative
          iCube = 0;
          updateOrbitalCombo();
        }
        else {
          iCube = m_settingsWidget->orbitalCombo->currentIndex();
          if (iCube >= cubes.size()) {
            qDebug() << "Invalid orbital selected.";
            return;
          }
        }
      }
      else {
        iCube = 0;
      }
    }

    // attribute is the text key for the grid (as an std::string)
    qDebug() << " Orbital title: " << cubes[iCube]->name();

    if (!m_mesh1 || !m_mesh2) {
      m_mesh1 = new Mesh;
      m_mesh2 = new Mesh;
    }

    m_min = cubes[iCube]->min();
    m_max = cubes[iCube]->max();
    m_meshGen1->initialize(cubes[iCube], m_mesh1, m_iso);
    m_meshGen1->start();
    m_meshGen2->initialize(cubes[iCube], m_mesh2, -m_iso);
    m_meshGen2->start();

    m_update = false;
  }

  void OrbitalEngine::updateOrbitalCombo()
  {
    if (!m_settingsWidget)
      return;
    // Reset the orbital combo
    qDebug() << "Update orbital combo called...";
    int tmp = m_settingsWidget->orbitalCombo->currentIndex();
    if (tmp < 0) tmp = 0;
    m_settingsWidget->orbitalCombo->clear();
    foreach(Cube *cube, m_molecule->cubes()) {
      if (!cube->lock()->tryLockForRead()) {
        qDebug() << "Cannot get a read lock...";
        continue;
      }
      m_settingsWidget->orbitalCombo->addItem(cube->name());
      qDebug() << "Adding Cube:" << cube->name();
      cube->lock()->unlock();
    }
    // If all of the orbitals disappear the molecule has been cleared
    if (m_molecule->cubes().size() == 0 && m_mesh1 && m_mesh2) {
      /// FIXME Again - engines should not really manage their primitives!
      delete m_mesh1;
      m_mesh1 = 0;
      delete m_mesh2;
      m_mesh2 = 0;
      m_meshGen1->initialize(0, 0, 0);
      m_meshGen2->initialize(0, 0, 0);
    }
    m_settingsWidget->orbitalCombo->setCurrentIndex(tmp);
  }

  double OrbitalEngine::transparencyDepth() const
  {
    return 1.0;
  }

  Engine::EngineFlags OrbitalEngine::flags() const
  {
    return Engine::Transparent | Engine::Surfaces;
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
      updateOrbitalCombo();
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
    m_posColor.setAlpha(m_alpha);
    m_negColor.setAlpha(m_alpha);
    m_iso = settings.value("iso", 0.02).toDouble();
    m_renderMode = settings.value("renderMode", 0).toInt();
    m_drawBox = settings.value("drawBox", false).toBool();
  }

}

#include "orbitalengine.moc"

Q_EXPORT_PLUGIN2(orbitalengine, Avogadro::OrbitalEngineFactory)
