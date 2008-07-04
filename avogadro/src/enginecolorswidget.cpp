/**********************************************************************
  EngineColorsWidget - Widget for setting the engine color map.

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

#include "enginecolorswidget.h"

#include <avogadro/engine.h>
#include <avogadro/color.h>
#include <avogadro/elementcolor.h>
#include <avogadro/residuecolor.h>

#include <QDialog>

namespace Avogadro {

  class EngineColorsWidgetPrivate
  {
    public:
      EngineColorsWidgetPrivate() : engine(0) {};

      Engine *engine;
  };

  EngineColorsWidget::EngineColorsWidget( QWidget *parent ) : QWidget(parent), d(new EngineColorsWidgetPrivate)
  {
    ui.setupUi(this);

    //ui.colorCombo->setEnabled(false);

    connect(ui.colorCombo, SIGNAL(currentIndexChanged(int)),
        this, SLOT(colorChanged(int)));
  }

  EngineColorsWidget::~EngineColorsWidget()
  {
    delete d;
  }

  void EngineColorsWidget::colorChanged(int index)
  {
    Color *map;

    switch (index) {
      default:
      case 0:
        map = new ElementColor();
        break;
      case 1:
        map = new ResidueColor();
        break;
      case 2:
        map = new Color(1.0, 0.0, 0.0, 1.0);
        break;
      case 3:
        map = new Color(0.0, 1.0, 0.0, 1.0);
        break;
      case 4:
        map = new Color(0.0, 0.0, 1.0, 1.0);
        break;
    }

    d->engine->setColorMap(map);
  }

  void EngineColorsWidget::setEngine( Engine *engine )
  {
    d->engine = engine;
    //ui.colorCombo->setEnabled(true);
  }

} // end namespace Avogadro

#include "enginecolorswidget.moc"
