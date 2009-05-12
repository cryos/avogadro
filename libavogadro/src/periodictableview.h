/**********************************************************************
 PeriodicTableView - Periodic Table Graphics View for Avogadro

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

#ifndef PERIODICTABLEVIEW_H
#define PERIODICTABLEVIEW_H

#include <avogadro/global.h>

#include <QDialog>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QObject>

namespace Avogadro {
  /**
   * @class ElementItem periodictableview.h <avogadro/periodictableview.h>
   * @author Marcus D. Hanwell
   * @brief An element item, intended to display a single element.
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

  /**
   * @class ElementDetail periodictableview.h <avogadro/periodictableview.h>
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

  /**
   * @class PeriodicTableScene periodictableview.h <avogadro/periodictableview.h>
   * @author Marcus D. Hanwell
   * @brief This class encapsulates the scene, all items are contained in it.
   *
   * This class implements a QGraphicsScene that holds all of the element items.
   * Any items owned by this class are automatically deleted by it.
   */
  class PeriodicTableScene : public QGraphicsScene
  {
  Q_OBJECT

  public:
    /**
     * Constructor.
     */
    PeriodicTableScene(QObject *parent = 0);

  Q_SIGNALS:
    /**
     * This signal is emitted when an element item is clicked.
     */
    void elementChanged(int element);

  protected:
    /**
     * Generic event handler, currently defaults to calling parent class
     * (included for future compatibility)
     */
    bool event(QEvent *e);

    /**
     * Handles the mouse press events to change the active element.
     */
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

    /**
     * Not used at present.
     */
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

    /**
     * Not used at present.
     */
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
  };

  /**
   * @class PeriodicTableView periodictableview.h <avogadro/periodictableview.h>
   * @author Marcus D. Hanwell
   * @brief This class implements the view of the PeriodicTableScene.
   *
   * This is the class that actually draws the widget onto screen. This is
   * the class that should normally be instantiated in order to display a
   * Periodic Table.
   */
  class A_EXPORT PeriodicTableView : public QGraphicsView
  {
    Q_OBJECT

  public:
    /**
     * Constructor - contructs a new PeriodicTableView with an internal instance
     * of PeriodicTableScene.
     */
    explicit PeriodicTableView(QWidget *parent = 0);

    /**
     * Destructor.
     */
    ~PeriodicTableView();

  private Q_SLOTS:
    /**
     * Use this slot to change the active element.
     */
    void elementClicked(int element);

  Q_SIGNALS:
    /**
     * Signal emitted when the active element in the PeriodicTableView changes.
     */
    void elementChanged(int element);

  private:
    /**
     * Proton number of the active element.
     */
    int m_element;

  protected:
    /**
     * Generic event handler, currently defaults to calling parent class
     * (included for future compatibility)
     */
    bool event(QEvent *e);

    /**
     * Double click event - select an element and hide the PeriodicTableView.
     */
    void mouseDoubleClickEvent(QMouseEvent *event);
  };

} // End of Avogadro namespace

#endif

