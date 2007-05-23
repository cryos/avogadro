/**********************************************************************
  EngineTabWidget - View for listing engines

  Copyright (C) 2007 Donald Ephraim Curtis

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

#ifndef __ENGINETABWIDGET_H
#define __ENGINETABWIDGET_H

#include <QObject>
#include <QTabWidget>

#include "ui_engineconfigurationwidget.h"

class QAbstractButton;
class QStandardItem;
namespace Avogadro {

  class GLWidget;
  class Engine;
  /**
   * @class EngineTabWidget
   * @brief Widget for change engine settings and members of engines.
   *
   * This widget allows us to modify engine settings and select which objects
   * are to be rendered by this engine when partially rendering.
   */
  class EngineTabWidgetPrivate;
  class EngineTabWidget : public QWidget
  {
    Q_OBJECT;

    public:
      EngineTabWidget( GLWidget *glWidget, QWidget *parent = 0 );
      ~EngineTabWidget();

      GLWidget *glWidget() const;

    public Q_SLOTS:
      void setCurrentEngine(Engine *engine);
      void addSelection();
      void removeSelection();
      void addAll();

    private:
      EngineTabWidgetPrivate *const d;

      Ui::EngineConfigurationWidget ui;

  };

}

#endif
