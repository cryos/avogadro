/**********************************************************************
  PluginListView - View for listing plugins

  Copyright (C) 2007 by Geoffrey R. Hutchison
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

#include "pluginlistview.h"
#include "pluginitemmodel.h"

#include <QDialog>

namespace Avogadro {

  PluginListView::PluginListView( QWidget *parent ) : QListView(parent)
  {
  }

  PluginListView::~PluginListView()
  {
  }

  void PluginListView::selectPlugin( const QModelIndex &index )
  {
    PluginItemModel *m = (PluginItemModel*) model();
    PluginItem *plugin = m->plugin(index);
    
    if(plugin) 
    {
      emit selectionChanged(plugin);
    }
  }

  void PluginListView::currentChanged ( const QModelIndex & current, const QModelIndex & previous )
  {
     QAbstractItemView::currentChanged(current,previous);
     selectPlugin(current);
  }
  
  PluginItem* PluginListView::selectedPlugin() const
  {
    const QModelIndex idx = currentIndex();
    
    if(idx.isValid())
    {
      PluginItemModel *m = (PluginItemModel*) model();
      PluginItem *plugin = m->plugin(idx);
      return plugin;
    }

    return 0;
  }

} // end namespace Avogadro

#include "pluginlistview.moc"
