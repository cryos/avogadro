/**********************************************************************
  EngineColorsWidget - Widget for setting the engine color map.

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

#include "enginecolorswidget.h"

#include <avogadro/engine.h>
#include <avogadro/color.h>
#include <avogadro/pluginmanager.h>

#include <QDialog>
#include <QDebug>

namespace Avogadro {

  class EngineColorsWidgetPrivate
  {
  public:
    EngineColorsWidgetPrivate() : engine(0), currentSettingsWidget(0) {};
    
    Engine *engine;
    PluginManager *pluginManager;
    QWidget *currentSettingsWidget;
  };

  EngineColorsWidget::EngineColorsWidget( PluginManager *pluginManager,
                                          QWidget *parent )
    : QWidget(parent), d(new EngineColorsWidgetPrivate)
  {
    ui.setupUi(this);
    d->pluginManager = pluginManager;

    foreach(Color *color, pluginManager->colors())
    {
      ui.colorCombo->addItem(color->name());
    }

    connect(ui.colorCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(colorChanged(int)));
    // we should actually check the engine settings for the current color choice
  }

  EngineColorsWidget::~EngineColorsWidget()
  {
    d->currentSettingsWidget = NULL;
    delete d;
  }

  void EngineColorsWidget::colorChanged(int index)
  {
    Color *color = d->pluginManager->colors().at(index);

    if (!color)
      return;

    d->engine->setColorMap(color);
    if (d->currentSettingsWidget) {
      d->currentSettingsWidget->hide();
      ui.verticalLayout->removeWidget(d->currentSettingsWidget);

      // Remove the bottom spacer
      QLayoutItem *space;
      space = ui.verticalLayout->takeAt(-1);
      ui.verticalLayout->removeItem(space);
      
      d->currentSettingsWidget = NULL;
    }
    
    if (color->settingsWidget()) {
      d->currentSettingsWidget = color->settingsWidget();
      ui.verticalLayout->addWidget(d->currentSettingsWidget);
      ui.verticalLayout->addStretch();
      d->currentSettingsWidget->show();
    }
  }

  void EngineColorsWidget::setEngine( Engine *engine )
  {
    d->engine = engine;
    //ui.colorCombo->setEnabled(true);
  }

} // end namespace Avogadro

#include "enginecolorswidget.moc"
