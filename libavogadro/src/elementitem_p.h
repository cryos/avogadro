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

#ifndef ELEMENTITEM_P_H
#define ELEMENTITEM_P_H

#include <QGraphicsItem>

namespace Avogadro{

  /**
   * @class ElementItem
   * @author Marcus D. Hanwell
   * @brief An element item, intended to display a single element.
   *
   * This class implements a QGraphicsItem for displaying single elements in a
   * perdiodic table. It currently allows the setting of the proton number and
   * gets all other information from OpenBabel.
   */
  class ElementItem : public QGraphicsItem
  {
  public:
    /**
     * Constructor. Should be called with the element number for this item. The
     * constructor uses setData to set the element number using the key 0. This
     * is then used by PeriodicTable to figure out which element was clicked on.
     */
    ElementItem(int elementNumber = 0);

    /**
     * Destructor.
     */
    ~ElementItem();

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
     * along with its symbol.
     */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget);

  private:
    /**
     * Indicates if element is well-formed (e.g., has non-empty symbol)
     */
    bool m_valid;
    /**
     * The element numbers symbol.
     */
    QString m_symbol;

    /**
     * The color of the element which will also be used as the background color
     * for the item box.
     */
    QColor *m_color;

    /**
     * Width and height of the elements.
     */
    int m_width, m_height;

    /**
     * The proton number of the item - all other attributes are derived from this.
     */
    int m_element;
  };

} // End namespace Avogadro

#endif // ELEMENTITEM_P_H
