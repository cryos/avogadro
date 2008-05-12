/**********************************************************************
  EngineSetupWidget - View for listing engines

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

#ifndef ENGINESETUPWIDGET_H
#define ENGINESETUPWIDGET_H

#include <QObject>
#include <QTabWidget>

#include "ui_enginesetupwidget.h"

class QAbstractButton;
class QStandardItem;
namespace Avogadro
{

  class GLWidget;
  class Engine;
  /**
    * @class EngineSetupWidget
    * @brief Widget for change engine settings and members of engines.
    *
    * This widget allows us to modify engine settings and select which objects
    * are to be rendered by this engine when partially rendering.
    */
  class EngineSetupWidgetPrivate;
  class EngineSetupWidget : public QWidget
  {
      Q_OBJECT

    public:
      explicit EngineSetupWidget ( GLWidget *glWidget, QWidget *parent = 0 );
      ~EngineSetupWidget();

      GLWidget *glWidget() const;

    public Q_SLOTS:
      /**
       * Set an engine to be the currently selected engine
       *
       * @param engine pointer to the engine to set current
       */
      void setCurrentEngine ( Engine *engine );

      /**
       * Add the selected primitives in the GLWidget to the current engine
       */
      void addSelection();

      /**
       * Remove the currently selected primitives to the engine
       */
      void removeSelection();

      /**
       * Add all primitives to the current engine
       */
      void addAll();

    protected Q_SLOTS:
      /**
       * Add an engine to the list
       */
      void addEngine(Engine *engine);

      /**
       * Remove and engine from the list
       */
      void removeEngine(Engine *engine);

    private:
      EngineSetupWidgetPrivate *const d;

      Ui::EngineSetupWidget ui;

  };

}

#endif
