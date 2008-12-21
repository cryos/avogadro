/**********************************************************************
  PluginDialog - Dialog for Plugin Manager

  Copyright (C) 2008 by Tim Vandermeersch

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

#ifndef PLUGINDIALOG_H
#define PLUGINDIALOG_H

#include "pluginlistview.h"
#include "ui_plugindialog.h"

#include <QDialog>
#include <QModelIndex>

namespace Avogadro
{
  class PluginDialog : public QDialog
  {
      Q_OBJECT

    public:
      //! Constructor
      explicit PluginDialog( QWidget *parent = 0, Qt::WindowFlags f = 0 );
      //! Desconstructor
      ~PluginDialog();

    public Q_SLOTS:
      void accept();
      void selectPluginType( int index );
      void selectPlugin( PluginItem * );

    Q_SIGNALS:
      void reloadPlugins();

    private:
      Ui::PluginDialog ui;
  };
}

#endif
