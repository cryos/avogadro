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

#include "engineitemdelegate.h"

#include <avogadro/engine.h>

#include <QtGui/QPainter>
#include <QtGui/QIcon>

namespace Avogadro
{

  EngineItemDelegate::EngineItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
  {
  }

  void EngineItemDelegate::paint(QPainter *painter,
                                 const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const
  {
    QStyledItemDelegate::paint(painter, option, index);

    if (index.isValid() && index.column() == 1) {
      bool hasSettings = index.data(Qt::ToolTipRole).toBool();
      if (!hasSettings)
        return;
      QIcon icon(":/icons/tool.png");
      QRect iconRect(option.rect.right() - option.rect.height(),
                     option.rect.top(),
                     option.rect.height(),
                     option.rect.height());
      icon.paint(painter, iconRect, Qt::AlignRight | Qt::AlignVCenter,
                 hasSettings ? QIcon::Normal : QIcon::Disabled);
    }

  }

} // End namespace Avogadro

#include "engineitemdelegate.moc"
