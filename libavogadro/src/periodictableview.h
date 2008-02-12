/**********************************************************************
 PeriodicTableView - Periodic Table Graphics View for Avogadro

 Copyright (C) 2007 by Marcus D. Hanwell

 This file is part of the Avogadro molecular editor project.
 For more information, see <http://avogadro.sourceforge.net/>

 Avogadro is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
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

#ifndef __PERIODICTABLEVIEW_H
#define __PERIODICTABLEVIEW_H

#include <QDialog>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QObject>

namespace Avogadro {
  /**
   * @class ElementItem periodictablegv.h <avogadro/periodictablegv.h>
   * @author Marcus D. Hanwell
   * @brief An element item intended to display a single element box.
   *
   * This class implements a QGraphicsItem for displaying single elements in a
   * perdiodic table. It currently allows the setting of the proton number and
   * gets all other information from OpenBabel.
   */
  class ElementItem : public QObject, public QGraphicsItem
  {
    Q_OBJECT

  public:
    /**
     * Constructor. Should be called with the element number for this item. The
     * constructor uses setData to set the element number using the key 0. This
     * is then used by PeriodicTable to figure out which element was clicked on.
     */
    ElementItem(int elementNumber = 0);

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
     * The element numbers symbol.
     */
    QString m_symbol;

    /**
     * The color of the element which will also be used as the background color
     * for the item box.
     */
    QColor *m_color;

    int m_width, m_height;    // width and height of the rectangle of the item

    /**
     * The proton number of the item - all other attributes are derived from this.
     */
    int m_element;            // element number
  };

  /**
   * @class ElementDetail periodictablegv.h <avogadro/periodictablegv.h>
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
    int m_width, m_height;    // width and height of the rectangle of the item

    /**
     * The proton number of the item - all other attributes are derived from this.
     */
    int m_element;            // element number
  };

  class PeriodicTableScene : public QGraphicsScene
  {
    Q_OBJECT

  public:
    PeriodicTableScene(QObject *parent = 0);

  Q_SIGNALS:
    /**
     * This signal is sent when the element is clicked on to indicate a change
     */
    void elementChanged(int element);

  protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
  };

  class PeriodicTableView : public QGraphicsView
  {
    Q_OBJECT

  public:
    PeriodicTableView(QWidget *parent = 0);
    PeriodicTableView(QGraphicsScene *scene, QWidget *parent = 0);

    void setSelectedElement(int element);

  private Q_SLOTS:
    void elementClicked(int element);

  Q_SIGNALS:
    void elementChanged(int element);

  private:
    int m_element;

  protected:
    void mouseDoubleClickEvent(QMouseEvent *event); 
  };

} // End of Avogadro namespace

#endif

