/**********************************************************************
  ProjectTreeItem - 

  Copyright (C) 2008 Tim Vandermeersch

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

#ifndef PROJECTTREEITEM_H
#define PROJECTTREEITEM_H

#include <avogadro/global.h>
#include <avogadro/primitivelist.h>

#include <QVector>

class QTreeView;
namespace Avogadro {

  class ProjectTreeItem
  {
    public:
      ProjectTreeItem(const QVector<QVariant> &data, ProjectTreeItem *parent = 0);
      ~ProjectTreeItem();

      ProjectTreeItem *child(int number);
      int childCount() const;
      int columnCount() const;
      QVariant data(int column) const;
      bool insertChildren(int position, int count, int columns);
      bool insertColumns(int position, int columns);
      ProjectTreeItem *parent();
      bool removeChildren(int position, int count);
      bool removeColumns(int position, int columns);
      int childNumber() const;
      bool setData(int column, const QVariant &value);

      PrimitiveList primitives() const;
      void setPrimitives(const PrimitiveList& primitives);

    private:
      QList<ProjectTreeItem*> m_childItems;
      QVector<QVariant> m_itemData;
      PrimitiveList m_primitives;
      ProjectTreeItem *m_parentItem;
  };
  
}  // end namespace Avogadro

#endif 
