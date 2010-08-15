/**********************************************************************
 ElementItem - Element Item, part of the Periodic Table Graphics View for
 Avogadro

 Copyright (C) 2007-2009 by Marcus D. Hanwell
 Some portions (C) 2010 by Konstantin Tokarev

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
#include "elementtranslator.h"

#include <openbabel/mol.h>

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QFont>
#include <QFontMetrics>
#include <QDebug>

namespace Avogadro{

  ElementItem::ElementItem(int elementNumber) : m_valid(false), m_width(26), m_height(26),
                                                m_element(elementNumber)
  {
    // Want these items to be selectable
    setFlags(QGraphicsItem::ItemIsSelectable);

    m_symbol = OpenBabel::etab.GetSymbol(m_element);
    if(!m_symbol.isEmpty())
      m_valid = true;
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
    if(!m_valid)
      return;

    // Fill the rectangle with the element colour
    QColor bgColor;
    QPen pen;
    if (isSelected()) {
      bgColor = QColor(*m_color).lighter(150);
      pen.setColor(QColor(*m_color).darker(150));
      pen.setWidth(4);
    } else {
      bgColor = QColor(*m_color);
    }
    painter->setPen(pen);
    painter->setBrush(bgColor);
    QRectF rect(-m_width/2, -m_height/2, m_width, m_height);
    painter->drawRect(rect);
    // Handle the case where the item is selected
    if (bgColor.value() < 150)
      pen.setColor(Qt::white);
    else
      pen.setColor(Qt::black);
    painter->setPen(pen);
    painter->drawText(rect, Qt::AlignCenter, m_symbol);
  }

} // End namespace Avogadro
