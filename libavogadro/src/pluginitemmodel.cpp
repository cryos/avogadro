/**********************************************************************
  PluginItemModel - List Model for Plugins

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

#include "pluginitemmodel.h"

#include <QString>
#include <QDebug>

namespace Avogadro {

  class PluginItemModelPrivate 
  {
    public:
      int type;
      QList<PluginItem *> plugins;
  };

  PluginItemModel::PluginItemModel( int type, QObject *parent ) : 
      QAbstractItemModel(parent), 
      d(new PluginItemModelPrivate)
  {
    d->type = type;
    d->plugins = pluginManager.plugins(type);
  }

  int PluginItemModel::columnCount( const QModelIndex & ) const
  {
    return 1;
  }

  int PluginItemModel::rowCount( const QModelIndex & parent ) const
  {
    Q_UNUSED(parent);

    return d->plugins.size();
  }

  QVariant PluginItemModel::data ( const QModelIndex & index, int role ) const
  {
    if(!index.isValid() || !index.internalPointer()) {
      return false;
    }

    PluginItem *plugin = static_cast<PluginItem *>(index.internalPointer());
    if(role == Qt::DisplayRole) {
      return plugin->name();
    } else if ( role == Qt::CheckStateRole) {
      if (plugin->isEnabled()) {
        return Qt::Checked;
      } else {
        return Qt::Unchecked;
      }
    } 

    return QVariant();
  }

  bool PluginItemModel::setData ( const QModelIndex & index, const QVariant & value, int role )
  {
    if(!index.isValid() || !index.internalPointer()) {
      return false;
    }

    PluginItem *plugin = static_cast<PluginItem *>(index.internalPointer());
    if(role == Qt::CheckStateRole) {
      if(value == Qt::Checked) {
        plugin->setEnabled(true);
      } else {
        plugin->setEnabled(false);
      }
      emit dataChanged(index, index);
      return true;
    } 
    
    return false;
  }

  Qt::ItemFlags PluginItemModel::flags ( const QModelIndex & ) const
  {
    return (Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
  }

  QModelIndex PluginItemModel::index ( int row, int column, const QModelIndex & parent ) const
  {
    if(!parent.isValid() && row >=0 && row < d->plugins.size())
    {
      PluginItem *plugin = d->plugins.at(row);
      return createIndex(row, column, plugin);
    }
    
    return QModelIndex();
  }
      
  PluginItem *PluginItemModel::plugin ( const QModelIndex & index ) const
  {
    if(!index.isValid() || !index.internalPointer()) {
      return false;
    }

    PluginItem *plugin = static_cast<PluginItem *>(index.internalPointer());
    return plugin;
  }

} // end namespace Avogadro

#include "pluginitemmodel.moc"
