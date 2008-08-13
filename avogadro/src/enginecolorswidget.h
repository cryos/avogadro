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

#ifndef ENGINECOLORSWIDGET_H
#define ENGINECOLORSWIDGET_H

#include "ui_enginecolorswidget.h"

namespace Avogadro
{

  class PluginManager;
  class Engine;
  /**
    * @class EngineColorsWidget
    * @brief Widget for changing engine color map.
    *
    * This widget allows us to modify the engine color map.
    */
  class EngineColorsWidgetPrivate;
  class EngineColorsWidget : public QWidget
  {
    Q_OBJECT

    public:
      explicit EngineColorsWidget ( const PluginManager *pluginManager, QWidget *parent = 0 );
      ~EngineColorsWidget();

    public Q_SLOTS:
      void setEngine ( Engine *engine );
      void colorChanged( int );

    private:
      EngineColorsWidgetPrivate *const d;
      Ui::EngineColorsWidget ui;
  };

}

#endif // ENGINECOLORSWIDGET_H
