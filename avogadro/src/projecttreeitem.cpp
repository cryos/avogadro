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

#include "projecttreeitem.h"

#include <QVariant>
#include <QVector>

namespace Avogadro {

  ProjectTreeItem::ProjectTreeItem(const QVector<QVariant> &data, ProjectTreeItem *parent)
  {
    m_parentItem = parent;
    m_itemData = data;
    m_terminal = true;
  }

  ProjectTreeItem::~ProjectTreeItem()
  {
    qDeleteAll(m_childItems);
  }

  ProjectTreeItem *ProjectTreeItem::child(int number)
  {
    return m_childItems.value(number);
  }

  int ProjectTreeItem::childCount() const
  {
    return m_childItems.count();
  }

  int ProjectTreeItem::childNumber() const
  {
    if (m_parentItem)
      return m_parentItem->m_childItems.indexOf(const_cast<ProjectTreeItem*>(this));

    return 0;
  }

  int ProjectTreeItem::columnCount() const
  {
    return m_itemData.count();
  }

  QVariant ProjectTreeItem::data(int column) const
  {
    return m_itemData.value(column);
  }

  bool ProjectTreeItem::insertChildren(int position, int count, int columns)
  {
    if (position < 0 || position > m_childItems.size())
      return false;

    for (int row = 0; row < count; ++row) {
      QVector<QVariant> data(columns);
      ProjectTreeItem *item = new ProjectTreeItem(data, this);
      m_childItems.insert(position, item);
    }

    return true;
  }

  bool ProjectTreeItem::insertColumns(int position, int columns)
  {
    if (position < 0 || position > m_itemData.size())
      return false;

    for (int column = 0; column < columns; ++column)
      m_itemData.insert(position, QVariant());

    foreach (ProjectTreeItem *child, m_childItems)
      child->insertColumns(position, columns);

    return true;
  }

  ProjectTreeItem *ProjectTreeItem::parent()
  {
    return m_parentItem;
  }

  bool ProjectTreeItem::removeChildren(int position, int count)
  {
    if (position < 0 || position + count > m_childItems.size())
      return false;

    for (int row = 0; row < count; ++row) {
      ProjectTreeItem *childItem = m_childItems.takeAt(position);
      delete childItem;
      childItem = 0;
    }

    return true;
  }

  bool ProjectTreeItem::removeColumns(int position, int columns)
  {
    if (position < 0 || position + columns > m_itemData.size())
      return false;

    for (int column = 0; column < columns; ++column)
      m_itemData.remove(position);

    foreach (ProjectTreeItem *child, m_childItems)
      child->removeColumns(position, columns);

    return true;
  }

  bool ProjectTreeItem::setData(int column, const QVariant &value)
  {
    if (column < 0 || column >= m_itemData.size())
      return false;

    m_itemData[column] = value;
    return true;
  }

  PrimitiveList ProjectTreeItem::primitives() const
  {
    return m_primitives;  
  }
  
  void ProjectTreeItem::setPrimitives(const PrimitiveList& primitives)
  {
    m_primitives = primitives;  
  }
  
  bool ProjectTreeItem::isTerminal() const
  {
    return m_terminal;
  }

  void ProjectTreeItem::setTerminal(bool terminal)
  {
    m_terminal = terminal;
  }



} // end namespace Avogadro

