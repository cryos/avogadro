/**********************************************************************
 ElementItem - Element Item, part of the Periodic Table Graphics View for
 Avogadro

 Copyright (C) 2007-2009 by Marcus D. Hanwell

 This file is part of the Avogadro molecular editor project.
 For more information, see <http://avogadro.openmolecules.net/>

 Avogadro is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or
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

#include "elementitem_p.h"

#include <avogadro/elementtranslator.h>

#include <openbabel/mol.h>

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QFont>
#include <QFontMetrics>
#include <QDebug>

namespace Avogadro{

  ElementItem::ElementItem(int elementNumber) : m_width(26), m_height(26),
    m_element(elementNumber)
  {
    // Want these items to be selectable
    setFlags(QGraphicsItem::ItemIsSelectable);

    m_symbol = OpenBabel::etab.GetSymbol(m_element);
    std::vector<double> color = OpenBabel::etab.GetRGB(m_element);
    m_color = new QColor();
    m_color->setRgbF(color[0], color[1], color[2]);
    // Set some custom data to make it easy to figure out which element we are
    setData(0, m_element);
  }

  ElementItem::~ElementItem()
  {
    delete m_color;
  }

  QRectF ElementItem::boundingRect() const
  {
    return QRectF(-m_width/2, -m_height/2, m_width, m_height);
  }

  QPainterPath ElementItem::shape() const
  {
    QPainterPath path;
    path.addRect(-m_width/2, -m_height/2, m_width, m_height);
    return path;
  }

  void ElementItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *,
                          QWidget *)
  {
    // Fill the rectangle with the element colour
    painter->setBrush(*m_color);

    // Handle the case where the item is selected
//    if (m_color->value() > 240) {
    if (m_element == 1 || m_element == 2 || m_element == 9 || m_element == 47
        || m_element == 78) {
      if (isSelected())
        painter->setPen(Qt::darkGray);
      else
        painter->setPen(Qt::black);
    }
    else {
      if (isSelected())
        painter->setPen(Qt::white);
      else
        painter->setPen(Qt::black);
    }

    QRectF rect(-m_width/2, -m_height/2, m_width, m_height);
    painter->drawRect(rect);
    painter->drawText(rect, Qt::AlignCenter, m_symbol);
  }

} // End namespace Avogadro
