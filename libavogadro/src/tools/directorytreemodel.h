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

#ifndef __DIRECTORYTREEMODEL_H
#define __DIRECTORYTREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QStringList>

namespace Avogadro {

  class FileTreeItem;

  class DirectoryTreeModel : public QAbstractItemModel
  {
    Q_OBJECT

      public:
    explicit DirectoryTreeModel(const QString &dirList, QObject *parent = 0);
    explicit DirectoryTreeModel(const QStringList &dirList, QObject *parent = 0);
    ~DirectoryTreeModel();

    QVariant data(const QModelIndex &index, int role) const;
    QString  filePath(const QModelIndex &index) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    const QStringList &directoryList() const;
    void setDirectoryList(const QStringList &dirList);
    void appendDirectory(const QString &path);
  
  private:
    void setupModelData(const QStringList &directories, FileTreeItem *parent);

    QStringList _directoryList;
    FileTreeItem *_rootItem;
  };

} // end namespace

#endif
