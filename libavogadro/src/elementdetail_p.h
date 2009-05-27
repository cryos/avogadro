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

#ifndef ELEMENTDETAIL_P_H
#define ELEMENTDETAIL_P_H

#include <QGraphicsItem>
#include <QObject>

namespace Avogadro{

  /**
   * @class ElementDetail
   * @author Marcus D. Hanwell
   * @brief An item box displaying more detailed information on the element.
   *
   * This class implements a QGraphicsItem for displaying a larger box that
   * gives greater detail about the selected element such as its full name,
   * proton number and average atomic mass.
   */
  class ElementDetail : public QObject, public QGraphicsItem
  {
    Q_OBJECT

  public:
    /**
     * Constructor. Should be called with the element number for this item.
     */
    ElementDetail(int elementNumber = 0);

    /**
     * @return the bounding rectangle of the element item.
     */
    QRectF boundingRect() const;

    /**
     * @return the painter path which is also a rectangle in this case.
     */
    QPainterPath shape() const;

    /**
     * This is where most of the action takes place. The element box is drawn
     * along with its symbol, proton number, mass and full name.
     */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget);

  public Q_SLOTS:
    /**
     * This slot can be used to indicate the active element has changed which
     * will cause this item to redraw with the details of the new element.
     */
    void elementChanged(int element);

  private:
    /**
     * Width and height of the item.
     */
    int m_width, m_height;

    /**
     * The proton number of the item - all other attributes are derived from this.
     */
    int m_element;
  };

} // End namespace Avogadro

#endif // ELEMENTDETAIL_P_H
