/**********************************************************************
  EngineListView - View for listing engines

  Copyright (C) 2007 by Geoffrey R. Hutchison

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  Some code is based on Open Babel
  For more information, see <http://openbabel.sourceforge.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#ifndef __ENGINELISTVIEW_H
#define __ENGINELISTVIEW_H

#include <QListView>

class QAbstractButton;
class QStandardItem;
namespace Avogadro {

  class GLWidget;
  class Engine;
  /**
   * @class EngineListView
   * @brief Widget for listing and editing widgets
   *
   * This widget is provided to list the available engines of a
   * GLWidget and allow them to be modified.  Thus allowing
   * us to select what features are rendered.
   */
  class EngineListViewPrivate;
  class EngineListView : public QListView
  {
    Q_OBJECT;

    public:
      explicit EngineListView( GLWidget *glWidget, QWidget *parent = 0 );
      ~EngineListView();

      GLWidget *glWidget() const;

      Engine *selectedEngine() const;
//       void setSettingsButton( QAbstractButton *button );
//       QAbstractButton *settingsButton() const;

    private:
      EngineListViewPrivate *const d;

    private Q_SLOTS:
      void selectEngine( const QModelIndex &index );
      // void showEngineSettings();

    Q_SIGNALS:
      void itemChanged( QStandardItem * );
      void clicked( Engine * );
  };

}

#endif
