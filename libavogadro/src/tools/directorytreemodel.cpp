/**********************************************************************
  DirectoryTreeModel - model for multiple directories / files

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

#include <QtGui>

#include <avogadro/treeitem.h>
#include "directorytreemodel.h"

namespace Avogadro {

  DirectoryTreeModel::DirectoryTreeModel(const QString &dirList, QObject *parent)
    : QAbstractItemModel(parent), _directoryList(dirList.split('\n'))
  {
    QList<QVariant> rootData;
    rootData << "Name";
    _rootItem = new TreeItem(rootData);

    setupModelData(_directoryList, _rootItem);
  }

  DirectoryTreeModel::DirectoryTreeModel(const QStringList &dirList, QObject *parent)
  : QAbstractItemModel(parent), _directoryList(dirList)
  {
    QList<QVariant> rootData;
    rootData << "Name";
    _rootItem = new TreeItem(rootData);

    setupModelData(_directoryList, _rootItem);
  }

  DirectoryTreeModel::~DirectoryTreeModel()
  {
    delete _rootItem;
  }

  int DirectoryTreeModel::columnCount(const QModelIndex &parent) const
  {
    if (parent.isValid())
      return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
    else
      return _rootItem->columnCount();
  }

  QVariant DirectoryTreeModel::data(const QModelIndex &index, int role) const
  {
    if (!index.isValid())
      return QVariant();

    if (role != Qt::DisplayRole)
      return QVariant();

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    return item->data(index.column());
  }

  QString DirectoryTreeModel::filePath(const QModelIndex &index) const
  {
    if (!index.isValid())
      return QString();

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    return item->filePath();
  }

  Qt::ItemFlags DirectoryTreeModel::flags(const QModelIndex &index) const
  {
    if (!index.isValid())
      return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  }

  QVariant DirectoryTreeModel::headerData(int section, Qt::Orientation orientation,
                                          int role) const
  {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
      return _rootItem->data(section);

    return QVariant();
  }

  QModelIndex DirectoryTreeModel::index(int row, int column, const QModelIndex &parent)
    const
  {
    if (!hasIndex(row, column, parent))
      return QModelIndex();

    TreeItem *parentItem;

    if (!parent.isValid())
      parentItem = _rootItem;
    else
      parentItem = static_cast<TreeItem*>(parent.internalPointer());

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
      return createIndex(row, column, childItem);
    else
      return QModelIndex();
  }

  QModelIndex DirectoryTreeModel::parent(const QModelIndex &index) const
  {
    if (!index.isValid())
      return QModelIndex();

    TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem *parentItem = childItem->parent();

    if (parentItem == _rootItem)
      return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
  }

  int DirectoryTreeModel::rowCount(const QModelIndex &parent) const
  {
    TreeItem *parentItem;
    if (parent.column() > 0)
      return 0;

    if (!parent.isValid())
      parentItem = _rootItem;
    else
      parentItem = static_cast<TreeItem*>(parent.internalPointer());

    return parentItem->childCount();
  }

  const QStringList &DirectoryTreeModel::directoryList() const
  {
    return _directoryList;
  }

  void DirectoryTreeModel::setDirectoryList(const QStringList &dirList)
  {
    _directoryList = dirList;
    setupModelData(_directoryList, _rootItem);
  }

  void DirectoryTreeModel::appendDirectory(const QString &path)
  {
    _directoryList.append(path);
    setupModelData(_directoryList, _rootItem);
  }

  // HELPER function: return the number of directories in the path
  int directoryDepth(QString path)
  {
    return path.split(QDir::separator()).count();
  }

  // HELPER function: return the name of the last directory
  QString lastDirectory(QFileInfo fileInfo)
  {
    QStringList directoryPath = fileInfo.filePath().split(QDir::separator());
    if (fileInfo.isFile())
      return directoryPath[directoryPath.size() - 2]; // next to last item
    else // must be a directory, since we only iterator over files or dirs
      return directoryPath[directoryPath.size() - 1]; // last item
  }

  void DirectoryTreeModel::setupModelData(const QStringList &dirList, TreeItem *parent)
  {
    parent->deleteChildren(); // remove any previous data

    int position = 0; // current number of subdirectories (i.e., the relative path depth)
    int absoluteDepth = 0; // depth of the parent directory
  
    foreach (QString dir, dirList) {
      QDir currentDir(dir);

      if (currentDir.exists()) {
        absoluteDepth = directoryDepth(currentDir.absolutePath());
        qDebug() << dir << absoluteDepth;

        QList<TreeItem*> parents;
        QList<int> indentations; // number of subdirectories for each item in the model
        parents << parent;
        indentations << 0;          
          
        // set the first item to be the top-level directory itself
        QList<QVariant> topLevel;
        topLevel << currentDir.dirName();
        parent->appendChild(new TreeItem(topLevel, parent));
        parents << parents.last()->child(parents.last()->childCount()-1);
        indentations << 0;
          
        QDirIterator dirIterator(currentDir.absolutePath(),
                             QDir::Dirs | QDir::Files | QDir::Readable | QDir::NoSymLinks | QDir::NoDotAndDotDot,
                             QDirIterator::Subdirectories);
          
        do {
          dirIterator.next();
           
          // Add the filename as the first column
          QList<QVariant> columnData;
          position = directoryDepth(dirIterator.filePath()) - absoluteDepth;
          columnData << dirIterator.fileName();
           
          if (position < indentations.last()) {
            while (position < indentations.last() && parents.count() > 0) {
              parents.pop_back();
              indentations.pop_back();
            }
          }           
           
          // If this is a real directory (and not a Mac OS X "bundle")
          if (dirIterator.fileInfo().isDir() && !dirIterator.fileInfo().isBundle()) {
                            
            // insert a new nested directory
            QList<QVariant> dirData;
            dirData << dirIterator.fileName();
            parents.last()->appendChild(new TreeItem(dirData, parents.last()));
            // Now the new child becomes the parent for any files in that directory
            parents << parents.last()->child(parents.last()->childCount()-1);
            indentations << position + 1;

            continue;
          }
                          
          // Append a new item to the current parent's list of children.
          parents.last()->appendChild(new TreeItem(columnData, parents.last(), dirIterator.filePath()));

        } while (dirIterator.hasNext());

      }
    }
  }

} // end namespace Avogadro

#include "directorytreemodel.moc"
