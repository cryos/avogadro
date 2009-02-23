/**********************************************************************
  OrbitalEngine - Engine for display of molecular orbitals

  Copyright (C) 2008 Marcus D. Hanwell
  Copyright (C) 2008 Geoffrey R. Hutchison
  Copyright (C) 2008 Tim Vandermeersch

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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
#include <avogadro/painterdevice.h>

#include <QReadWriteLock>
#include <QDebug>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro {

  OrbitalEngine::OrbitalEngine(QObject *parent) : Engine(parent),
    m_settingsWidget(0), m_mesh1(0), m_mesh2(0), m_min(0., 0., 0.), m_max(0.,0.,0.),
    m_alpha(0.75), m_renderMode(0), m_drawBox(false), m_update(true)
  {
    m_negColor.set(1.0, 0.0, 0.0, m_alpha);
    m_posColor.set(0.0, 0.0, 1.0, m_alpha);
  }

  OrbitalEngine::~OrbitalEngine()
  {
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

      if (m_mesh1) {
        if (m_mesh1->stable()) {
          pd->painter()->setColor(&m_posColor);
          pd->painter()->drawMesh(*m_mesh1, m_renderMode);
        }
      }
      if (m_mesh2) {
        if (m_mesh2->stable()) {
          pd->painter()->setColor(&m_negColor);
          pd->painter()->drawMesh(*m_mesh2, m_renderMode);
        }
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

      if (m_mesh1) {
        if (m_mesh1->stable()) {
          pd->painter()->setColor(&m_posColor);
          pd->painter()->drawMesh(*m_mesh1, m_renderMode);
        }
      }

      if (m_mesh2) {
        if (m_mesh2->stable()) {
          pd->painter()->setColor(&m_negColor);
          pd->painter()->drawMesh(*m_mesh2, m_renderMode);
        }
      }

      renderSurfaces(pd);
    }
    return true;
  }

  bool OrbitalEngine::renderQuick(PainterDevice *pd)
  {
    if (m_update)
      updateSurfaces(pd);

    int renderMode = 1;
    if (m_renderMode == 2)
      renderMode = 2;

    if (m_mesh1) {
      if (m_mesh1->stable()) {
        pd->painter()->setColor(&m_posColor);
        pd->painter()->drawMesh(*m_mesh1, renderMode);
      }
    }
    if (m_mesh2) {
      if (m_mesh2->stable()) {
        pd->painter()->setColor(&m_negColor);
        pd->painter()->drawMesh(*m_mesh2, renderMode);
      }
    }
    renderSurfaces(pd);

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
    int iMesh1 = 0;
    int iMesh2 = 1;
    if (m_mesh1)
      iMesh1 = m_mesh1->index();
    if (m_mesh2)
      iMesh2 = m_mesh2->index();
    QList<Mesh *> meshes = pd->molecule()->meshes();
    if (meshes.size() == 0)
      return;
    if (meshes.size() == 1)
      iMesh2 = 0;
    else {
      if (m_settingsWidget) {
        // Add the orbitals
        connect(m_molecule, SIGNAL(updated()),
                this, SLOT(updateOrbitalCombo()));
        if (!m_settingsWidget->orbital1Combo->count()) {
          // Use first mesh/orbital
          // Two meshes -- one for positive isovalue, one for negative
          updateOrbitalCombo();
        }
        else {
          // Valid settings widget with populated orbital combos
          iMesh1 = m_settingsWidget->orbital1Combo->currentIndex();
          if (iMesh1 >= meshes.size()) {
            qDebug() << "Invalid orbital selected.";
            return;
          }
        }
      }
    }

    // attribute is the text key for the Mesh
    m_mesh1 = meshes[iMesh1];
    m_mesh2 = m_molecule->meshById(m_mesh1->otherMesh());
    qDebug() << " Orbital 1 title: " << m_mesh1->name();
    qDebug() << " Orbital 2 title: " << m_mesh2->name();

    // Get the cube extents
    Cube *cube = m_molecule->cubeById(m_mesh1->cube());
    m_min = cube->min();
    m_max = cube->max();

    m_update = false;
  }

  void OrbitalEngine::updateOrbitalCombo()
  {
    if (!m_settingsWidget || !m_molecule)
      return;
    // Reset the orbital combo
    qDebug() << "Update orbital combo called...";
    int tmp1 = m_settingsWidget->orbital1Combo->currentIndex();
    if (tmp1 < 0) tmp1 = 0;
    m_settingsWidget->orbital1Combo->clear();

    QList<Mesh *> meshList = m_molecule->meshes();
    if (meshList.empty())
      return;

    foreach(Mesh *mesh, meshList) {
      if (!mesh->lock()->tryLockForRead()) {
        qDebug() << "Cannot get a read lock on the mesh...";
        continue;
      }
      if (mesh->isoValue() > 0.0) {
        if (m_mesh1)
          if (m_mesh1->id() == mesh->id())
            tmp1 = m_settingsWidget->orbital1Combo->count();
        m_settingsWidget->orbital1Combo->addItem(mesh->name() + ", isosurface = "
                     + QString::number(mesh->isoValue()));
      }
      mesh->lock()->unlock();
    }
    // If all of the orbitals disappear the molecule has been cleared
    if (m_molecule->numMeshes() == 0) {
      m_mesh1 = 0;
      m_mesh2 = 0;
    }
    else
      m_settingsWidget->orbital1Combo->setCurrentIndex(tmp1);
  }

  double OrbitalEngine::transparencyDepth() const
  {
    return 1.0;
  }

  Engine::Layers OrbitalEngine::layers() const
  {
    return Engine::Transparent;
  }

  Engine::PrimitiveTypes OrbitalEngine::primitiveTypes() const
  {
    return Engine::Surfaces; // i.e., don't display the "primitives tab"
  }

  Engine::ColorTypes OrbitalEngine::colorTypes() const
  {
    return Engine::IndexedColors;
  }

  void OrbitalEngine::setOrbital1(int)
  {
    m_update = true;
    emit changed();
  }

  void OrbitalEngine::setOrbital2(int)
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
      connect(m_settingsWidget->orbital1Combo, SIGNAL(currentIndexChanged(int)),
              this, SLOT(setOrbital1(int)));
      connect(m_settingsWidget->opacitySlider, SIGNAL(valueChanged(int)),
              this, SLOT(setOpacity(int)));
      connect(m_settingsWidget->renderCombo, SIGNAL(currentIndexChanged(int)),
              this, SLOT(setRenderMode(int)));
      connect(m_settingsWidget->drawBoxCheck, SIGNAL(stateChanged(int)),
              this, SLOT(setDrawBox(int)));
      connect(m_settingsWidget->posColor, SIGNAL(colorChanged(QColor)),
              this, SLOT(setPosColor(QColor)));
      connect(m_settingsWidget->negColor, SIGNAL(colorChanged(QColor)),
              this, SLOT(setNegColor(QColor)));
      connect(m_settingsWidget, SIGNAL(destroyed()),
              this, SLOT(settingsWidgetDestroyed()));

      // Initialise the widget from saved settings
      m_settingsWidget->opacitySlider->setValue(static_cast<int>(m_alpha * 20));
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
    if (primitive->type() == Primitive::MeshType)
      m_update = true;
  }

  void OrbitalEngine::updatePrimitive(Primitive *primitive)
  {
    // Updating primitives does not invalidate these surfaces...
    if (primitive->type() == Primitive::MeshType)
      m_update = true;
  }

  void OrbitalEngine::removePrimitive(Primitive *primitive)
  {
    Engine::removePrimitive(primitive);
    if (primitive->type() == Primitive::MeshType)
      m_update = true;
  }

  void OrbitalEngine::writeSettings(QSettings &settings) const
  {
    Engine::writeSettings(settings);
    settings.setValue("alpha", m_alpha);
    settings.setValue("renderMode", m_renderMode);
    settings.setValue("drawBox", m_drawBox);
    if (m_mesh1)
      settings.setValue("mesh1Id", static_cast<int>(m_mesh1->id()));
    if (m_mesh2)
      settings.setValue("mesh2Id", static_cast<int>(m_mesh2->id()));
//    settings.setValue("posColor", m_posColor);
//    settings.setValue("posColor", m_negColor);
  }

  void OrbitalEngine::readSettings(QSettings &settings)
  {
    Engine::readSettings(settings);
    m_alpha = settings.value("alpha", 0.5).toDouble();
    m_posColor.setAlpha(m_alpha);
    m_negColor.setAlpha(m_alpha);
    m_renderMode = settings.value("renderMode", 0).toInt();
    m_drawBox = settings.value("drawBox", false).toBool();
    if (m_molecule) {
      m_mesh1 = m_molecule->meshById(settings.value("mesh1Id", 0).toInt());
      m_mesh2 = m_molecule->meshById(settings.value("mesh2Id", 0).toInt());
    }
  }

}

#include "orbitalengine.moc"

Q_EXPORT_PLUGIN2(orbitalengine, Avogadro::OrbitalEngineFactory)
