/**********************************************************************
  PluginListView - View for listing plugins

  Copyright (C) 2007 by Geoffrey R. Hutchison
  Some portions Copyright (C) 2008 Tim Vandermeersch
  Some portions Copyright (C) 2009 Konstantin L. Tokarev

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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

#ifndef PLUGINLISTVIEW_H
#define PLUGINLISTVIEW_H

#include <avogadro/pluginmanager.h>

#include <QListView>

namespace Avogadro {

  class PluginListView : public QListView
  {
    Q_OBJECT

    public:
      explicit PluginListView( QWidget *parent = 0 );
      ~PluginListView();

      PluginItem* selectedPlugin() const;

    protected Q_SLOTS:
	  void currentChanged ( const QModelIndex & current, const QModelIndex & previous );
	  
    private Q_SLOTS:
      void selectPlugin( const QModelIndex &index );

    Q_SIGNALS:
      void selectionChanged( PluginItem * );
	  
  };

}

#endif
