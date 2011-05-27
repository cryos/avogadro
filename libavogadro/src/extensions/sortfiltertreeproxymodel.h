/**********************************************************************
  SortFilterTreeProxyModel - Sorting / Filter proxy which works on trees

  Based on code from http://kodeclutz.blogspot.com/2008/12/filtering-qtreeview.html

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#ifndef SORTFILTERTREEPROXYMODEL_H
#define SORTFILTERTREEPROXYMODEL_H

#include <QSortFilterProxyModel>

namespace Avogadro {

  class SortFilterTreeProxyModel: public QSortFilterProxyModel
  {
    Q_OBJECT
  public:
    SortFilterTreeProxyModel(QObject *parent = 0): QSortFilterProxyModel(parent) {};
    // From http://kodeclutz.blogspot.com/2008/12/filtering-qtreeview.html
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const;

    // This is a hack to prevent us from becoming root-less
    // See http://stackoverflow.com/questions/3212392/qtreeview-qfilesystemmodel-setrootpath-and-qsortfilterproxymodel-with-regexp-fo
    void setSourceRoot(const QModelIndex &sourceRoot)
    { m_sourceRoot = sourceRoot; }

    QModelIndex m_sourceRoot;
  };

}

#endif
