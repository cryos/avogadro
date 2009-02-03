/**********************************************************************
  PluginItemModel - List Model for Plugins

  Copyright (C) 2007 Donald Ephraim Curtis
  Some portions Copyright (C) 2008 Tim Vandermeersch

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

#ifndef PLUGINITEMMODEL_H
#define PLUGINITEMMODEL_H

#include <avogadro/pluginmanager.h>

#include <QModelIndex>

namespace Avogadro {

  class PluginItemModelPrivate;
  class PluginItemModel : public QAbstractItemModel
  {
    Q_OBJECT

    public:
      explicit PluginItemModel( Plugin::Type type, QObject *parent = 0 );
      QModelIndex parent( const QModelIndex & ) const { return QModelIndex(); }
      int rowCount( const QModelIndex & parent = QModelIndex() ) const;
      int columnCount( const QModelIndex & parent = QModelIndex() ) const;
      QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
      Qt::ItemFlags flags ( const QModelIndex & index ) const;
      bool setData ( const QModelIndex & index, const QVariant & value, int role );
      QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
      
      PluginItem *plugin ( const QModelIndex & index ) const;

      bool changed() const;

    private:
      PluginItemModelPrivate * const d;

  };

} // end namespace Avogadro

#endif
