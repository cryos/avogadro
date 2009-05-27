/**********************************************************************
 ElementDetail - Element Detail, part of the Periodic Table Graphics View for
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

#include "elementdetail_p.h"

#include <avogadro/elementtranslator.h>

#include <openbabel/mol.h>

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QFont>
#include <QFontMetrics>
#include <QDebug>

namespace Avogadro{

  ElementDetail::ElementDetail(int elementNumber) : m_width(100), m_height(70),
    m_element(elementNumber)
  {
  }

  QRectF ElementDetail::boundingRect() const
  {
    return QRectF(-m_width/2, -m_height/2, m_width, m_height);
  }

  QPainterPath ElementDetail::shape() const
  {
    QPainterPath path;
    path.addRect(-m_width/2, -m_height/2, m_width, m_height);
    return path;
  }

  void ElementDetail::paint(QPainter *painter, const QStyleOptionGraphicsItem *,
                          QWidget *)
  {
    // Set up a font object and get its height
    QFont font("sans-serif");
    font.setPixelSize(12);
    painter->setFont(font);
    QFontMetrics fm(font);
    int pixelHeight = fm.height();

    QString symbol = OpenBabel::etab.GetSymbol(m_element);
    QString name(ElementTranslator::name(m_element));
    QString mass = QString("%L1").arg(OpenBabel::etab.GetMass(m_element), 0, 'f', 3);

    std::vector<double> color = OpenBabel::etab.GetRGB(m_element);
    QColor m_color;
    m_color.setRgbF(color[0], color[1], color[2]);

    // Draw the element detail border and fill with the element colour
    painter->setBrush(m_color);
    painter->setPen(Qt::black);
    QRectF rect(-m_width/2, -m_height/2, m_width, m_height);
    painter->drawRect(rect);

    // Draw the element symbol bigger than everything else
    font.setPixelSize(24);
    QFontMetrics fm2(font);
    pixelHeight = fm2.height();
    int pixelWidth = fm2.width(symbol);
    painter->setFont(font);
    QRectF symbolRect(-10, -m_height/2 + 8, pixelWidth, pixelHeight);
    painter->drawText(symbolRect, Qt::AlignCenter, symbol);

    // Reduce the font size to draw the other parts
    font.setPixelSize(12);
    int pixelHeight2 = fm.height();
    painter->setFont(font);

// I don't seem to be able to get a nice, cross platform layout working here
// I would really like to figure out how to make this more portable - ideas?
#ifdef Q_WS_MAC
    // Draw the proton number
    QRectF protonNumberRect(-m_width/2 - 10, -m_height/2 + 8, m_width/2, pixelHeight2);
    painter->drawText(protonNumberRect, Qt::AlignRight, QString::number(m_element));

    // Draw the mass
    QRectF massNumberRect(-m_width/2, -m_height/2 + 8 + pixelHeight*1.1, m_width, pixelHeight2);
    painter->drawText(massNumberRect, Qt::AlignCenter, mass);

    // Finally the full element name
    QRectF nameRect(-m_width/2, -m_height/2 + 4 + pixelHeight*1.1 + pixelHeight2, m_width, pixelHeight);
    painter->drawText(nameRect, Qt::AlignCenter, name);
#else
    // Draw the proton number
    QRectF protonNumberRect(-m_width/2 - 10, -m_height/2 + 16, m_width/2, pixelHeight2);
    painter->drawText(protonNumberRect, Qt::AlignRight, QString::number(m_element));

    // Draw the mass
    QRectF massNumberRect(-m_width/2, -m_height/2 + 4  + pixelHeight, m_width,
                          pixelHeight2);
    painter->drawText(massNumberRect, Qt::AlignCenter, mass);

    // Finally the full element name
    QRectF nameRect(-m_width/2, -m_height/2 + pixelHeight + 0.8 * pixelHeight2,
                    m_width, pixelHeight);
    painter->drawText(nameRect, Qt::AlignCenter, name);
#endif
  }

  void ElementDetail::elementChanged(int element)
  {
    m_element = element;
    emit update();
  }

} // End namespace Avogadro

#include "elementdetail_p.moc"
