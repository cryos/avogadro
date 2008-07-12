/**********************************************************************
  OverlayEngine - Engine for displaying overlays.

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

#include "overlayengine.h"

#include <config.h>
#include <avogadro/elementtranslate.h>
#include <avogadro/camera.h>
#include <avogadro/painter.h>
#include <avogadro/glwidget.h>

#include <QGLWidget> // for OpenGL bits
#include <QDebug>
#include <QPainter>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro {

  OverlayEngine::OverlayEngine(QObject *parent) : Engine(parent),
  m_settingsWidget(0)
  {
    setDescription(tr("Renders overlays"));
  }

  Engine *OverlayEngine::clone() const
  {
    OverlayEngine *engine = new OverlayEngine(parent());
    engine->setName(name());
    engine->setEnabled(isEnabled());

    return engine;
  }

  bool OverlayEngine::renderOpaque(PainterDevice *pd)
  {
    QPainter p;
    p.begin((QGLWidget*)GLWidget::current());
    p.save();
    // draw the transparent background
    p.setBrush(QColor(255,255,255,100));
    p.drawRect(pd->width() / 2 - 200, pd->height() - 70, 400, 60);
    // draw the gradient
    QLinearGradient gradient(QPointF(pd->width() / 2 - 150, 0), QPointF(pd->width() / 2 + 150, 0));
    gradient.setColorAt(0, QColor(255, 0, 0, 255));
    gradient.setColorAt(0.5, QColor(0, 255, 0, 255));
    gradient.setColorAt(1, QColor(0, 0, 255, 255));
    p.setBrush(gradient);
    p.drawRect(pd->width() / 2 - 150, pd->height() - 60, 300, 40);

    p.setBrush(QColor(0, 0, 0, 255));
    p.drawRect(pd->width() / 2 - 190, pd->height() - 44, 30, 8);
    p.drawRect(pd->width() / 2 + 160, pd->height() - 44, 30, 8);
    p.drawRect(pd->width() / 2 + 171, pd->height() - 55, 8, 30);
 
    p.restore();
    p.end();
    
    return true;
  }

  bool OverlayEngine::renderQuick(PainterDevice *)
  {
    // Don't render text when moving...
    return true;
  }

  QWidget *OverlayEngine::settingsWidget()
  {
    if(!m_settingsWidget)
    {
      m_settingsWidget = new OverlaySettingsWidget();
      connect(m_settingsWidget, SIGNAL(destroyed()), this, SLOT(settingsWidgetDestroyed()));
    }
    return m_settingsWidget;
  }

  void OverlayEngine::settingsWidgetDestroyed()
  {
    qDebug() << "Destroyed Settings Widget";
    m_settingsWidget = 0;
  }

  Engine::EngineFlags OverlayEngine::flags() const
  {
    return Engine::Overlay;
  }

  void OverlayEngine::writeSettings(QSettings &settings) const
  {
    Engine::writeSettings(settings);
  }

  void OverlayEngine::readSettings(QSettings &settings)
  {
    Engine::readSettings(settings);
  }
}

#include "overlayengine.moc"

Q_EXPORT_PLUGIN2(overlayengine, Avogadro::OverlayEngineFactory)
