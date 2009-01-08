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
  class ProjectTreeModelDelegate;

  class ProjectTreeModelPrivate;
  class ProjectTreeModel : public QAbstractItemModel
  {
    Q_OBJECT

    public:
      explicit ProjectTreeModel( GLWidget *widget, QObject *parent = 0 );
      ~ProjectTreeModel();

      /**********************************************************************
       * The following functions are used by the QTreeView to get the data.
       **********************************************************************/

      /**
       * @return The data at position @p index.
       */
      QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
      /**
       * @return The header data at position @p index.
       */
      QVariant headerData( int section, Qt::Orientation orientation, int role ) const;
      /**
       * @return A global model QModelIndex based on a parent, row and column
       */
      QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
      /**
       * @return A QModelIndex for the parent of @p index.
       */
      QModelIndex parent( const QModelIndex & index ) const;
      /**
       * @return The number of rows for @p parent.
       */
      int rowCount( const QModelIndex & parent = QModelIndex() ) const;
      /**
       * @return The number of columns in the model.
       *
       * This is always the same values as d->rootItem->columnCount(), you
       * can add more columns by adding more data items to d->rootItem in
       * the constructor.
       */
      int columnCount( const QModelIndex & parent = QModelIndex() ) const;
      /**
       * @return The flags for item with index @p index.
       */
      Qt::ItemFlags flags ( const QModelIndex & index ) const;

      /**********************************************************************
       * The following functions are used by the various ProjectItems to 
       * add, remove and update the ProjectTreeItems.
       **********************************************************************/
      
      /*
       * Add @p rows rows at position @p position to parent @p parentItem.
       */
      bool insertRows(ProjectTreeItem *parentItem, int position, int rows);
      /*
       * Remove @p rows rows at position @p position in parent @p parentItem.
       */
      bool removeRows(ProjectTreeItem *parentItem, int position, int rows);

      /**
       * Use this function in the ProjectItems classes to make the associated
       * QTreeView aware that some data has changed. For example, you can call 
       * this after responding to a primitiveUpdated(...) signal.
       */
      void emitDataChanged(ProjectTreeItem *parentItem, int row);
      /**
       * @return The ProjectTreeItem for @p index
       */
      ProjectTreeItem* item(const QModelIndex& index) const;


      /**********************************************************************
       * These functions are used to handle dynamic model creation as needed.
       * In other words, create items when the parent is expaneded.
       **********************************************************************/

      /**
       * @return true if the item with index @p parent has children.
       *
       * QTreeView uses this (and not rowCount()) to determine whether it
       * should draw a expandable item (with the plus/triangle)
       */
      bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
      /**
       * @return true if there is more data available for parent, 
       * otherwise false.
       *
       * If a user expands an item, Qt will call:
       * if (canFetchMore(parent)) fetchMore()
       *
       * So we can delay the initialization of the model (and connecting the
       * signal) until the user actually expands an item.)   
       *
       * The default Qt implementation always returns false, we return true
       * for non-terminal items.
       */
      bool canFetchMore(const QModelIndex& parent) const;
      /**
       * Initialize more model data, see canFetchMore(). This function does 
       * the actual updating of the model.
       */
      void fetchMore(const QModelIndex& parent);
      
      
      /**
       * Some delegates may delegate their work to other delegates. However, to keep the
       * model informed about all the delegates, these delegates will export thier delegates
       * using ProjectTreeModelDelegate::exportDelegate(...). This function will then call 
       * importDelegate(...). This will also steal the pointer, you don't need to delete them.
       *
       * See MoleculeDelegate for example.
       */
      void importDelegate(ProjectTreeModelDelegate *delegate);
 

    private Q_SLOTS:
      void init();

    private:
      ProjectTreeModelPrivate * const d;


  };

}  // end namespace Avogadro

#endif // __PRIMITIVEITEMMODEL_H
