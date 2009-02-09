/**********************************************************************
  SurfaceEngine - Engine for display of isosurfaces

  Copyright (C) 2007 Geoffrey R. Hutchison
  Copyright (C) 2008-2009 Marcus D. Hanwell
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

#include <config.h>
#include <avogadro/mesh.h>
#include <avogadro/molecule.h>
#include <avogadro/painterdevice.h>

#include <QDebug>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro {

  SurfaceEngine::SurfaceEngine(QObject *parent) : Engine(parent),
    m_settingsWidget(0), m_mesh(0), m_alpha(0.5), m_renderMode(0),
    m_colorMode(0), m_drawBox(false), m_coloredMesh(false)
  {
    setDescription(tr("Surface rendering"));
    m_color = Color(1.0, 0.0, 0.0, m_alpha);
  }

  SurfaceEngine::~SurfaceEngine()
  {
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

  bool SurfaceEngine::renderOpaque(PainterDevice *pd)
  {
    // Render the opaque surface if m_alpha is 1
    if (m_alpha >= 0.999)
    {
      if (m_mesh) {
        if (m_mesh->stable()) {
          if (m_coloredMesh)
            pd->painter()->drawColorMesh(*m_mesh, m_renderMode);
          else {
            pd->painter()->setColor(&m_color);
            pd->painter()->drawMesh(*m_mesh, m_renderMode);
          }
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
          if (m_coloredMesh)
            pd->painter()->drawColorMesh(*m_mesh, m_renderMode);
          else {
            pd->painter()->setColor(&m_color);
            pd->painter()->drawMesh(*m_mesh, m_renderMode);
          }
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
          if (m_coloredMesh)
            pd->painter()->drawColorMesh(*m_mesh, m_renderMode);
          else {
            pd->painter()->setColor(&m_color);
            pd->painter()->drawMesh(*m_mesh, m_renderMode);
          }
      }
    }
    return true;
  }

  double SurfaceEngine::radius(const PainterDevice *, const Primitive *) const
  {
    return 0.0;
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

  void SurfaceEngine::settingsWidgetDestroyed()
  {
    qDebug() << "Destroyed Settings Widget";
    m_settingsWidget = 0;
  }

  void SurfaceEngine::setPrimitives(const PrimitiveList &primitives)
  {
    Engine::setPrimitives(primitives);
  }

  void SurfaceEngine::addPrimitive(Primitive *primitive)
  {
    Engine::addPrimitive(primitive);
  }

  void SurfaceEngine::updatePrimitive(Primitive *primitive)
  {
    Engine::updatePrimitive(primitive);
  }

  void SurfaceEngine::removePrimitive(Primitive *primitive)
  {
    if (primitive->type() == Primitive::MeshType ||
        primitive->type() == Primitive::MoleculeType) {
      m_mesh = 0;
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
    settings.setValue("coloredMesh", m_coloredMesh);
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
    m_coloredMesh = settings.value("coloredMesh").toBool();

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

}

#include "surfaceengine.moc"

Q_EXPORT_PLUGIN2(surfaceengine, Avogadro::SurfaceEngineFactory)
