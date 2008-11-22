/**********************************************************************
  PrimitiveItemModel - Model for representing primitives.

  Copyright (C) 2007 Donald Ephraim Curtis <dcurtis3@sourceforge.net>

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

#ifndef PROJECTTREEMODEL_H
#define PROJECTTREEMODEL_H

#include <avogadro/global.h>

#include "projecttreeitem.h"

#include <QAbstractItemModel>

class QTreeView;
namespace Avogadro {

  class GLWidget;
  class Primitive;

  class ProjectTreeModelPrivate;
  class A_EXPORT ProjectTreeModel : public QAbstractItemModel
  {
    Q_OBJECT

    public:
      explicit ProjectTreeModel( GLWidget *widget, QObject *parent = 0 );
      ~ProjectTreeModel();

      // read-only functions
      QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
      QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
      QModelIndex parent( const QModelIndex & index ) const;
      int rowCount( const QModelIndex & parent = QModelIndex() ) const;
      int columnCount( const QModelIndex & parent = QModelIndex() ) const;
      
      // resizing functions
      Qt::ItemFlags flags ( const QModelIndex & index ) const;
      bool insertRows(ProjectTreeItem *parentItem, int position, int rows);
      bool removeRows(ProjectTreeItem *parentItem, int position, int rows);

      void emitDataChanged(ProjectTreeItem *parentItem, int row);

      ProjectTreeItem* item(const QModelIndex& index) const;

    private Q_SLOTS:
      void init();

    private:
      ProjectTreeModelPrivate * const d;


  };

}  // end namespace Avogadro

#endif // __PRIMITIVEITEMMODEL_H
