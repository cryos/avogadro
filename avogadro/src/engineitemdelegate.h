/**********************************************************************
  EngineItemDelegate - custom item delegate for engine items

  Copyright (C) 2009 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2 of the
  License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#ifndef ENGINEITEMDELEGATE_H
#define ENGINEITEMDELEGATE_H

#include <QtGui/QStyledItemDelegate>

namespace Avogadro
{
  /**
   * @class EngineItemDelegate
   * @brief Item delegate for Engine objects.
   *
   * This item delegate does some limited custom painting for engine items
   * used in the EngineViewWidget. Mainly to allow inline clicking of the
   * configuration dialog, although it could be extended for further functions.
   */
  class EngineItemDelegate : public QStyledItemDelegate
  {
    Q_OBJECT

  public:
    EngineItemDelegate(QObject *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
  };

} // End namespace Avogadro

#endif // ENGINEITEMDELEGATE_H
