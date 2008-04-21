/**********************************************************************
  FileTreeItem - general tree model item for files

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

#ifndef __FILETREEITEM_H
#define __FILETREEITEM_H

#include <QList>
#include <QVariant>
#include <QString>
#include <avogadro/global.h>

namespace Avogadro {

  /**
   * @class FileTreeItem filetreeitem.h
   * @brief An item for a Tree model/view of a file or directory
   * @author Geoffrey Hutchison
   *
   * This class implements a tree item for a model/view of a directory
   * structure. It is used in multiple places for list or tree views
   * for example in the Insert Fragment dialog.
   *
   * In particular, the class contains a non-user visible property
   * which stores the full path to the file it represents on disk.
   * Use filePath() or setFilePath() to access this.
   */
  class A_EXPORT FileTreeItem
  {
    public:
      FileTreeItem(const QList<QVariant> &data, FileTreeItem *parent = 0, QString path = "");
      ~FileTreeItem();

      void appendChild(FileTreeItem *child);
      void deleteChildren();

      FileTreeItem *child(int row);
      int childCount() const;
      int columnCount() const;
      QVariant data(int column) const;
      int row() const;
      FileTreeItem *parent();

      void setFilePath(QString path);
      QString filePath() const;

    private:
      QList<FileTreeItem*> childItems;
      QList<QVariant> itemData;
      FileTreeItem *parentItem;

      QString _filePath;                 //!< 
  };

}

#endif
