/**********************************************************************
  HTMLDelegate - Styled Text Delegate

  by Geoffrey R. Hutchison

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

#ifndef HTMLDELEGATE_H
#define HTMLDELEGATE_H

#include <QStyledItemDelegate>

namespace Avogadro {

// Allow HTML text (e.g., superscripts & subscripts) to be in a table
// Modified from http://stackoverflow.com/questions/1956542/how-to-make-item-view-render-rich-html-text-in-qt
// Original code by StackOverflow users serge_gubenko and Anton
class HTMLDelegate : public QStyledItemDelegate
{
  Q_OBJECT
    public:
  HTMLDelegate(QObject *parent = 0) : QStyledItemDelegate(parent) {};
  QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
  void paint(QPainter *p, const QStyleOptionViewItem &o, const QModelIndex &ind) const;
};

} // end namespace Avogadro

#endif
