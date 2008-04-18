/**********************************************************************
  FileTreeItem - general tree model item

  Copyright (C) 2008 Geoffrey R. Hutchison

  Inspired by example code from Qt/Examples by Trolltech.

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

#include <avogadro/FileTreeItem.h>

namespace Avogadro {

  FileTreeItem::FileTreeItem(const QList<QVariant> &data, FileTreeItem *parent, QString path): itemData(data), parentItem(parent), _filePath(path)
{
}

FileTreeItem::~FileTreeItem()
{
  deleteChildren();
}

void FileTreeItem::appendChild(FileTreeItem *item)
{
    childItems.append(item);
}

void FileTreeItem::deleteChildren()
{
  qDeleteAll(childItems);
  childItems.clear();
}

FileTreeItem *FileTreeItem::child(int row)
{
    return childItems.value(row);
}

int FileTreeItem::childCount() const
{
    return childItems.count();
}

int FileTreeItem::columnCount() const
{
    return itemData.count();
}

QVariant FileTreeItem::data(int column) const
{
    return itemData.value(column);
}

FileTreeItem *FileTreeItem::parent()
{
    return parentItem;
}

int FileTreeItem::row() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<FileTreeItem*>(this));

    return 0;
}

void FileTreeItem::setFilePath(QString path)
{
  _filePath = path;
}

QString FileTreeItem::filePath() const
{
  return _filePath;
}

} // end namespace Avogadro

