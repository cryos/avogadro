/**********************************************************************
  DirectoryTreeModel - model for multiple directories / files

  Copyright (C) 2008 Geoffrey R. Hutchison

  Inspired by example code from Qt/Examples by Trolltech.

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

#include "directorytreemodel.h"

#include <avogadro/filetreeitem.h>

#include <QDirIterator>

namespace Avogadro {

  DirectoryTreeModel::DirectoryTreeModel(const QString &dirList, QObject *parent)
    : QAbstractItemModel(parent), _directoryList(dirList.split('\n'))
  {
    QList<QVariant> rootData;
    rootData << "Name"; // This is the header row -- add more columns, e.g. file size, etc.
    _rootItem = new FileTreeItem(rootData);

    setupModelData(_directoryList, _rootItem);
  }

  DirectoryTreeModel::DirectoryTreeModel(const QStringList &dirList, QObject *parent)
  : QAbstractItemModel(parent), _directoryList(dirList)
  {
    QList<QVariant> rootData;
    rootData << "Name"; // This is the header row -- add more columns here
    _rootItem = new FileTreeItem(rootData);

    setupModelData(_directoryList, _rootItem);
  }

  DirectoryTreeModel::~DirectoryTreeModel()
  {
    delete _rootItem;
  }

  int DirectoryTreeModel::columnCount(const QModelIndex &parent) const
  {
    if (parent.isValid()) {
      FileTreeItem* item = static_cast<FileTreeItem*>(parent.internalPointer());
      if (!item)
        return 0;
      else
        return item->columnCount();
    }
    else
      return _rootItem->columnCount();
  }

  QVariant DirectoryTreeModel::data(const QModelIndex &index, int role) const
  {
    if (!index.isValid())
      return QVariant();

    if (role != Qt::DisplayRole)
      return QVariant();

    FileTreeItem *item = static_cast<FileTreeItem*>(index.internalPointer());
    if (!item)
      return QVariant();

    return item->data(index.column());
  }

  QString DirectoryTreeModel::filePath(const QModelIndex &index) const
  {
    if (!index.isValid())
      return QString();

    FileTreeItem *item = static_cast<FileTreeItem*>(index.internalPointer());
    if (!item)
      return QString();

    return item->filePath(); // This is a special property of our tree items and isn't user-visible
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

    FileTreeItem *parentItem = NULL;

    if (!parent.isValid())
      parentItem = _rootItem;
    else
      parentItem = static_cast<FileTreeItem*>(parent.internalPointer());

    if (!parentItem)
      return QModelIndex();

    FileTreeItem *childItem = parentItem->child(row);
    if (childItem)
      return createIndex(row, column, childItem);
    else
      return QModelIndex();
  }

  QModelIndex DirectoryTreeModel::parent(const QModelIndex &index) const
  {
    if (!index.isValid())
      return QModelIndex();

    FileTreeItem *childItem = static_cast<FileTreeItem*>(index.internalPointer());
    if (!childItem)
      return QModelIndex();

    FileTreeItem *parentItem = childItem->parent();

    if (!parentItem || parentItem == _rootItem)
      return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
  }

  int DirectoryTreeModel::rowCount(const QModelIndex &parent) const
  {
    FileTreeItem *parentItem;
    if (parent.column() > 0)
      return 0;

    if (!parent.isValid())
      parentItem = _rootItem;
    else
      parentItem = static_cast<FileTreeItem*>(parent.internalPointer());

    if (!parentItem)
      return 0;
      
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
    // Split will count the first separator (e.g. /) and give an extra, empty string
    return path.split(QDir::separator()).count() - 1;
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

  void DirectoryTreeModel::setupModelData(const QStringList &dirList, FileTreeItem *parent)
  {
    emit layoutAboutToBeChanged(); // we need to tell the view that the data is going to change
    parent->deleteChildren(); // remove any previous data

    int position = 0; // current number of subdirectories (i.e., the relative path depth)
    int absoluteDepth = 0; // depth of the parent directory
  
    foreach (const QString& dir, dirList) {
      QDir currentDir(dir);

      if (currentDir.exists()) {
        absoluteDepth = directoryDepth(currentDir.absolutePath());

        QList<FileTreeItem*> parents;
        QList<int> indentations; // number of subdirectories for each item in the model
        parents << parent;
        indentations << 0;          
          
        // set the first item to be the top-level directory itself
        QList<QVariant> topLevel;
        topLevel << currentDir.dirName();
        parent->appendChild(new FileTreeItem(topLevel, parent));
        parents << parents.last()->child(parents.last()->childCount()-1);
        indentations << 0;
        
        QDirIterator dirIterator(currentDir.absolutePath(),
                             QDir::Dirs | QDir::Files | QDir::Readable 
                                 | QDir::NoSymLinks | QDir::NoDotAndDotDot,
                             QDirIterator::Subdirectories);
          
        do {
          dirIterator.next();
          position = directoryDepth(dirIterator.filePath()) - absoluteDepth;

          // First handle the case where we just moved up some directory levels
          if (position < indentations.last()) {
            while (position < indentations.last() && parents.count() > 0) {
              parents.pop_back();
              indentations.pop_back();
            }
          }           

          // If this is a real directory, add it as a new subdirectory
          if (dirIterator.fileInfo().isDir() && !dirIterator.fileInfo().isBundle()) {
            
            // insert a new nested directory
            QList<QVariant> dirData;
            dirData << dirIterator.fileName();
            parents.last()->appendChild(new FileTreeItem(dirData, parents.last()));
            // Now the new child becomes the parent for any files in that directory
            parents << parents.last()->child(parents.last()->childCount()-1);
            indentations << position + 1;

            continue;
          }

          // check to see if its an excluded file
          // (hidden or not readable or a Mac OS X bundle
          if (dirIterator.fileInfo().isHidden() 
              || !dirIterator.fileInfo().isReadable()
              || dirIterator.fileInfo().isBundle())
            continue;
          
          // OK, this is a file, and we've set the correct path structure
          // Add the filename as the first column
          QList<QVariant> columnData;
          columnData << dirIterator.fileName();
                        
          // Append a new item to the current parent's list of children.
          parents.last()->appendChild(new FileTreeItem(columnData, parents.last(), dirIterator.filePath()));

        } while (dirIterator.hasNext());

      }
    }

    invalidateIndexes();
    emit layoutChanged(); // again, tell the view that we're finished
  }
  
  /* From Qt API doc:
   * void QAbstractItemModel::layoutAboutToBeChanged ()   [signal]
   *
   * ... Subclasses should update any persistent model indexes after emitting 
   * layoutAboutToBeChanged(). ...
   * 
   * and: http://der-dakon.net/blog/KDE/persistent-crash.html
   */
  void DirectoryTreeModel::invalidateIndexes()
  {
    for (int i = 0; i < persistentIndexList().count(); i++) {
      QModelIndex idx = persistentIndexList().at(i);

      FileTreeItem *parentItem;
      if (!idx.isValid())
        parentItem = _rootItem;
      else
        parentItem = static_cast<FileTreeItem*>(idx.internalPointer());

      if (parentItem == _rootItem)
        continue;

      changePersistentIndex(idx, QModelIndex());
    }
  }

} // end namespace Avogadro

#include "directorytreemodel.moc"
