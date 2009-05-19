/**********************************************************************
  ProjectTreeItem - 

  Copyright (C) 2008 Tim Vandermeersch

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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

#ifndef PROJECTTREEITEM_H
#define PROJECTTREEITEM_H

#include <avogadro/global.h>
#include <avogadro/primitivelist.h>

#include <QVector>

class QTreeView;
namespace Avogadro {

  class ProjectTreeItem
  {
    friend class ProjectTreeModel;

    public:
      explicit ProjectTreeItem(const QVector<QVariant> &data, ProjectTreeItem *parent = 0);
      ~ProjectTreeItem();

      /**
       * @return Child @p number.
       */
      ProjectTreeItem *child(int number);
      /**
       * @return The number of children.
       */
      int childCount() const;
      /**
       * @return The number of columns. (Set to data.size in constructor)
       */
      int columnCount() const;
      /**
       * @return The data for @p column.
       */
      QVariant data(int column) const;
      /**
       * @return The parent for this item.
       */
      ProjectTreeItem *parent();
      /**
       * @return The row number for this item with it's parent.
       */
      int childNumber() const;
      /**
       * Set the data for @p column.
       */
      bool setData(int column, const QVariant &value);

      /**
       * @return The primitives for this item.
       */ 
      PrimitiveList primitives() const;
      /**
       * Set the primitives for this item.
       */
      void setPrimitives(const PrimitiveList& primitives);

      /**
       * @return true if this item is terminal and will never have 
       * children. This is used in ProjectTreeModel::hasChildren(...)
       * to handle the dynamic model initializatyion.
       */
      bool isTerminal() const;
      /**
       * Set the terminal flag for this item. (Default, set by constructor, is true)
       */
      void setTerminal(bool terminal);

    protected:
      /**
       * Insert @p count children starting at @p position. All items
       * have @p columns columns.
       */
      bool insertChildren(int position, int count, int columns);
      /**
       * Insert columns... (not used at the moment)
       */
      bool insertColumns(int position, int columns);
      /**
       * Remove @p count columns starting at @p position.
       */
      bool removeChildren(int position, int count);
      /**
       * Remove columns... (not used at the moment)
       */
      bool removeColumns(int position, int columns);
 
    private:
      QList<ProjectTreeItem*>   m_childItems;
      QVector<QVariant>         m_itemData;
      PrimitiveList             m_primitives;
      ProjectTreeItem          *m_parentItem;
      bool                      m_terminal;
  };
  
}  // end namespace Avogadro

#endif 
