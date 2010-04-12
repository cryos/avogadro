/**********************************************************************
  SurfaceEngine - Engine for display of isosurface meshes

  Copyright (C) 2008-2009 Marcus D. Hanwell
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

#include "surfaceengine.h"

#include "ui_surfacesettingswidget.h"

#include <avogadro/molecule.h>
#include <avogadro/cube.h>
#include <avogadro/mesh.h>
#include <avogadro/color3f.h>
#include <avogadro/painterdevice.h>

#include <QReadWriteLock>
#include <QDebug>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro {

  // Our settings widget class
  class SurfaceSettingsWidget : public QWidget, public Ui::SurfaceSettingsWidget
  {
    public:
      SurfaceSettingsWidget(QWidget *parent=0) : QWidget(parent) {
        setupUi(this);
      }
  };

  SurfaceEngine::SurfaceEngine(QObject *parent) : Engine(parent),
    m_settingsWidget(0), m_mesh1(0), m_mesh2(0), m_min(0., 0., 0.), m_max(0.,0.,0.),
    m_alpha(0.75), m_renderMode(0), m_drawBox(false), m_colored(false)
  {
    // default is red for negative, blue for positive
    m_negColor.setFromRgba(1.0, 0.0, 0.0, m_alpha);
    m_posColor.setFromRgba(0.0, 0.0, 1.0, m_alpha);
  }

  SurfaceEngine::~SurfaceEngine()
  {
  }

  Engine *SurfaceEngine::clone() const
  {
    SurfaceEngine *engine = new SurfaceEngine(parent());
    engine->setAlias(alias());
    engine->setEnabled(isEnabled());

    return engine;
  }

  bool SurfaceEngine::renderOpaque(PainterDevice *pd)
  {
    // Render the opaque surface if m_alpha is 1
    if (m_alpha >= 0.999) {
      if (m_mesh1) {
        if (m_mesh1->lock()->tryLockForRead()) {
          if (m_colored)
            pd->painter()->drawColorMesh(*m_mesh1, m_renderMode);
          else {
            pd->painter()->setColor(&m_posColor);
            pd->painter()->drawMesh(*m_mesh1, m_renderMode);
          }
          m_mesh1->lock()->unlock();
        }
      }
      if (m_mesh2) {
        if (m_mesh2->lock()->tryLockForRead()) {
          if (m_colored)
            pd->painter()->drawColorMesh(*m_mesh2, m_renderMode);
          else {
            pd->painter()->setColor(&m_negColor);
            pd->painter()->drawMesh(*m_mesh2, m_renderMode);
          }
          m_mesh2->lock()->unlock();
        }
      }
    }

    if (m_drawBox)
      renderBox(pd);

    return true;
  }

  bool SurfaceEngine::renderTransparent(PainterDevice *pd)
  {
    // Render the transparent surface if m_alpha is between 0 and 1.
    if (m_alpha > 0.001 && m_alpha < 0.999) {
      if (m_mesh1) {
        if (m_mesh1->lock()->tryLockForRead()) {
          if (m_colored) {
            pd->painter()->setColor(&m_posColor); // For transparency
            pd->painter()->drawColorMesh(*m_mesh1, m_renderMode);
          }
          else {
            pd->painter()->setColor(&m_posColor);
            pd->painter()->drawMesh(*m_mesh1, m_renderMode);
          }
          m_mesh1->lock()->unlock();
        }
      }
      if (m_mesh2) {
        if (m_mesh2->lock()->tryLockForRead()) {
          if (m_colored) {
            pd->painter()->setColor(&m_negColor); // For transparency
            pd->painter()->drawColorMesh(*m_mesh2, m_renderMode);
          }
          else {
            pd->painter()->setColor(&m_negColor);
            pd->painter()->drawMesh(*m_mesh2, m_renderMode);
          }
          m_mesh2->lock()->unlock();
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

    if (m_mesh1) {
      if (m_mesh1->lock()->tryLockForRead()) {
        pd->painter()->setColor(&m_posColor);
        pd->painter()->drawMesh(*m_mesh1, renderMode);
        m_mesh1->lock()->unlock();
      }
    }
    if (m_mesh2) {
      if (m_mesh2->lock()->tryLockForRead()) {
        pd->painter()->setColor(&m_negColor);
        pd->painter()->drawMesh(*m_mesh2, renderMode);
        m_mesh2->lock()->unlock();
      }
    }
    if (m_drawBox)
      renderBox(pd);

    return true;
  }

  inline bool SurfaceEngine::renderBox(PainterDevice *pd)
  {
    // Draw the extents of the cube if requested to
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

    return true;
  }

  void SurfaceEngine::updateOrbitalCombo()
  {
    if (!m_settingsWidget || !m_molecule)
      return;

    if (!m_molecule->numMeshes())
      return;

    // Reset the orbital combo
    int index = m_settingsWidget->orbital1Combo->currentIndex();
    if (index < 0) index = 0;
    m_settingsWidget->orbital1Combo->clear();

    // Build up a new list mapping combo box indices to meshes
    m_meshes.clear();

    foreach(Mesh *mesh, m_molecule->meshes()) {
      if (!mesh->lock()->tryLockForRead()) {
        qDebug() << "Cannot get a read lock on the mesh...";
        continue;
      }
      // Update the index if we have hit the currently selected Mesh
      if (m_mesh1 && m_mesh1->id() == mesh->id())
        index = m_settingsWidget->orbital1Combo->count();

      // Now figure out the mesh type and add it to the map
      Cube::Type cubeType = m_molecule->cubeById(mesh->cube())->cubeType();
      QString comboText;
      if (cubeType == Cube::VdW) {
        comboText = tr("Van der Waals, isosurface = %L1",
                       "Van der Waals isosurface with a cutoff of %1");
        m_settingsWidget->orbital1Combo->addItem(comboText.arg(mesh->isoValue()));
        m_meshes.push_back(mesh->id());
      }
      else if (cubeType == Cube::ElectronDensity) {
        comboText = tr("Electron density, isosurface = %L1",
                       "Electron density isosurface with a cutoff of %1");
        m_settingsWidget->orbital1Combo->addItem(comboText.arg(mesh->isoValue()));
        m_meshes.push_back(mesh->id());
      }
      else if (cubeType == Cube::MO) {
        if (mesh->isoValue() > 0.0) {
          comboText = tr("%1, isosurface = %L2",
                         "%1 is mesh name, %2 is the isosurface cutoff");
          m_settingsWidget->orbital1Combo->addItem(comboText
                                                   .arg(mesh->name())
                                                   .arg(mesh->isoValue()));
          m_meshes.push_back(mesh->id());
        }
      }
      mesh->lock()->unlock();
    }
    m_settingsWidget->orbital1Combo->setCurrentIndex(index);
  }

  double SurfaceEngine::transparencyDepth() const
  {
    return 1.0;
  }

  Engine::Layers SurfaceEngine::layers() const
  {
    return Engine::Transparent;
  }

  Engine::PrimitiveTypes SurfaceEngine::primitiveTypes() const
  {
    return Engine::Surfaces; // i.e., don't display the "primitives tab"
  }

  Engine::ColorTypes SurfaceEngine::colorTypes() const
  {
    return Engine::IndexedColors;
  }

  void SurfaceEngine::setOrbital(int n)
  {
    if (m_meshes.size() && n >= 0 && n < m_meshes.size()) {
      m_mesh1 = m_molecule->meshById(m_meshes.at(n));
      m_mesh2 = m_molecule->meshById(m_mesh1->otherMesh());
      Cube *cube = m_molecule->cubeById(m_mesh1->cube());
      m_min = cube->min();
      m_max = cube->max();
      if (m_mesh1->colors().size() == 0)
          m_colored = false;

      // Enable the combo if appropriate for mapped color
      if (m_settingsWidget) {
        m_settingsWidget->colorCombo->setEnabled(m_mesh1->vertices().size()
                                                 == m_mesh1->colors().size());
        m_settingsWidget->colorCombo->setCurrentIndex(m_colored ? 1 : 0);
      }

      emit changed();
    }
  }

  void SurfaceEngine::setOpacity(int value)
  {
    m_alpha = 0.05 * value;
    m_posColor.setAlpha(m_alpha);
    m_negColor.setAlpha(m_alpha);
    emit changed();
  }

  void SurfaceEngine::setRenderMode(int value)
  {
    m_renderMode = value;
    emit changed();
  }

  void SurfaceEngine::setDrawBox(int value)
  {
    if (value == 0) m_drawBox = false;
    else m_drawBox = true;
    emit changed();
  }

  void SurfaceEngine::setColorMode(int value)
  {
    m_colored = static_cast<bool>(value);
    emit changed();
  }


  void SurfaceEngine::setPosColor(const QColor& color)
  {
    m_posColor.setFromRgba(color.redF(), color.greenF(), color.blueF(), m_alpha);
    emit changed();
  }

  void SurfaceEngine::setNegColor(const QColor& color)
  {
    m_negColor.setFromRgba(color.redF(), color.greenF(), color.blueF(), m_alpha);
    emit changed();
  }

  QWidget* SurfaceEngine::settingsWidget()
  {
    if(!m_settingsWidget) {
      m_settingsWidget = new SurfaceSettingsWidget(qobject_cast<QWidget *>(parent()));
      connect(m_settingsWidget->orbital1Combo, SIGNAL(currentIndexChanged(int)),
              this, SLOT(setOrbital(int)));
      connect(m_settingsWidget->opacitySlider, SIGNAL(valueChanged(int)),
              this, SLOT(setOpacity(int)));
      connect(m_settingsWidget->renderCombo, SIGNAL(currentIndexChanged(int)),
              this, SLOT(setRenderMode(int)));
      connect(m_settingsWidget->drawBoxCheck, SIGNAL(stateChanged(int)),
              this, SLOT(setDrawBox(int)));
      connect(m_settingsWidget->colorCombo, SIGNAL(currentIndexChanged(int)),
              this, SLOT(setColorMode(int)));
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
      m_settingsWidget->colorCombo->setCurrentIndex(m_colored ? 1 : 0);

      // Initialise the colour buttons
      QColor initial;
      initial.setRgbF(m_posColor.red(), m_posColor.green(), m_posColor.blue());
      m_settingsWidget->posColor->setColor(initial);
      initial.setRgbF(m_negColor.red(), m_negColor.green(), m_negColor.blue());
      m_settingsWidget->negColor->setColor(initial);
      updateOrbitalCombo();

      // Connect the molecule updated signal
      if (m_molecule)
        connect(m_molecule, SIGNAL(updated()), this, SLOT(updateOrbitalCombo()));
    }
    return m_settingsWidget;
  }

  void SurfaceEngine::settingsWidgetDestroyed()
  {
    qDebug() << "Destroyed Settings Widget";
    m_settingsWidget = 0;
  }

  void SurfaceEngine::setPrimitives(const PrimitiveList &primitives)
  {
    Engine::setPrimitives(primitives);
    // This is used to load new molecules and so there could be a new cube file
    updateOrbitalCombo();
  }

  void SurfaceEngine::addPrimitive(Primitive *primitive)
  {
    // Rebuild the combo if a new mesh was added
    if (primitive->type() == Primitive::MeshType)
      updateOrbitalCombo();
  }

  void SurfaceEngine::updatePrimitive(Primitive *primitive)
  {
    // Updating primitives does not invalidate these surfaces...
    if (primitive->type() == Primitive::MeshType)
      updateOrbitalCombo();
  }

  void SurfaceEngine::removePrimitive(Primitive *primitive)
  {
    if (primitive->type() == Primitive::MeshType)
      updateOrbitalCombo();
  }

  void SurfaceEngine::setMolecule(const Molecule *molecule)
  {
    Engine::setMolecule(molecule);
    if (m_molecule) {
      connect(m_molecule, SIGNAL(primitiveAdded(Primitive*)),
            this, SLOT(addPrimitive(Primitive*)));
      connect(m_molecule, SIGNAL(primitiveUpdated(Primitive*)),
            this, SLOT(updatePrimitive(Primitive*)));
      connect(m_molecule, SIGNAL(primitiveRemoved(Primitive*)),
            this, SLOT(removePrimitive(Primitive*)));
    }

    updateOrbitalCombo();
  }

  void SurfaceEngine::setMolecule(Molecule *molecule)
  {
    Engine::setMolecule(molecule);
    if (m_molecule) {
      connect(m_molecule, SIGNAL(primitiveAdded(Primitive*)),
            this, SLOT(addPrimitive(Primitive*)));
      connect(m_molecule, SIGNAL(primitiveUpdated(Primitive*)),
            this, SLOT(updatePrimitive(Primitive*)));
      connect(m_molecule, SIGNAL(primitiveRemoved(Primitive*)),
            this, SLOT(removePrimitive(Primitive*)));
    }

    updateOrbitalCombo();
  }

  void SurfaceEngine::writeSettings(QSettings &settings) const
  {
    Engine::writeSettings(settings);
    settings.setValue("alpha", m_alpha);
    settings.setValue("renderMode", m_renderMode);
    settings.setValue("drawBox", m_drawBox);
    settings.setValue("colorMode", m_colored);
    if (m_mesh1)
      settings.setValue("mesh1Id", static_cast<int>(m_mesh1->id()));
    if (m_mesh2)
      settings.setValue("mesh2Id", static_cast<int>(m_mesh2->id()));
//    settings.setValue("posColor", m_posColor);
//    settings.setValue("posColor", m_negColor);
  }

  void SurfaceEngine::readSettings(QSettings &settings)
  {
    Engine::readSettings(settings);
    m_alpha = settings.value("alpha", 0.5).toDouble();
    m_posColor.setAlpha(m_alpha);
    m_negColor.setAlpha(m_alpha);
    m_renderMode = settings.value("renderMode", 0).toInt();
    m_colored = settings.value("colorMode", false).toBool();
    m_drawBox = settings.value("drawBox", false).toBool();
    if (m_molecule) {
      m_mesh1 = m_molecule->meshById(settings.value("mesh1Id",
                                                    qulonglong(FALSE_ID)).toInt());
      m_mesh2 = m_molecule->meshById(settings.value("mesh2Id",
                                                    qulonglong(FALSE_ID)).toInt());
      Cube *cube = m_molecule->cubeById(m_mesh1->cube());
      m_min = cube->min();
      m_max = cube->max();
      if (m_mesh1->colors().size() == 0)
        m_colored = false;
    }
  }

}

Q_EXPORT_PLUGIN2(surfaceengine, Avogadro::SurfaceEngineFactory)
