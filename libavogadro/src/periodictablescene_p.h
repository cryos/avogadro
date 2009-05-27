/**********************************************************************
 PeriodicTableScene - Periodic Table Graphics Scene for Avogadro

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

#ifndef PERIODICTABLESCENE_P_H
#define PERIODICTABLESCENE_P_H

#include <QGraphicsScene>

namespace Avogadro {

  /**
   * @class PeriodicTableScene
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

} // End namespace Avogadro

#endif // PERIODICTABLESCENE_P_H
