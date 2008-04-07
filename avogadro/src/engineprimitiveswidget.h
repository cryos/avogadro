#ifndef ENGINEPRIMITIVESWIDGET_H
#define ENGINEPRIMITIVESWIDGET_H
/**********************************************************************
  EnginePrimitivesWidget - View for listing engines

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

#include "ui_engineprimitiveswidget.h"

class QAbstractButton;
class QStandardItem;
namespace Avogadro
{

  class GLWidget;
  class Engine;
  /**
    * @class EnginePrimitivesWidget
    * @brief Widget for change engine settings and members of engines.
    *
    * This widget allows us to modify engine settings and select which objects
    * are to be rendered by this engine when partially rendering.
    */
  class EnginePrimitivesWidgetPrivate;
  class EnginePrimitivesWidget : public QWidget
  {
      Q_OBJECT

    public:
      explicit EnginePrimitivesWidget ( GLWidget *glWidget, QWidget *parent = 0 );
      ~EnginePrimitivesWidget();

      GLWidget *glWidget() const;

    public Q_SLOTS:
      void setEngine ( Engine *engine );
      void addSelection();
      void removeSelection();
      void fromSelection();
      void toSelection();
      void addAll();

    private:
      EnginePrimitivesWidgetPrivate *const d;

      Ui::EnginePrimitivesWidget ui;

  };

}

#endif // ENGINEPRIMITIVESWIDGET_H
