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

#include "sortfiltertreeproxymodel.h"

namespace Avogadro {

  bool SortFilterTreeProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
  {
    // First check to see if our parent matches (i.e., all files under a matching directory work)
    if ( sourceParent.isValid() && sourceModel()->data(sourceParent).toString().contains(filterRegExp()) )
      return true;

    // Make sure the root is always accepted, or we become rootless
    // See http://stackoverflow.com/questions/3212392/qtreeview-qfilesystemmodel-setrootpath-and-qsortfilterproxymodel-with-regexp-fo
    QModelIndex subIndex = sourceModel()->index(sourceRow, 0, sourceParent);
    if (!subIndex.isValid())
      return false;
    if (m_sourceRoot.isValid() && subIndex == m_sourceRoot)
      return true;

    // Check if the data for this row matches. If so, the default is to accept
    QString data = sourceModel()->data(subIndex).toString();
    bool ret = data.contains(filterRegExp());

    // Now we have to check the child nodes
    // We'll show the row if any child is accepted
    // (i.e., if a file matches, we'll show the directory path to it)
    // Try to fetchMore() first
    sourceModel()->fetchMore(subIndex);
    for(int i = 0; i < sourceModel()->rowCount(subIndex); ++i) {
      ret = ret || filterAcceptsRow(i, subIndex);
    }
    return ret;
  }

}
