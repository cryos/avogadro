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

#include "periodictablescene_p.h"
#include "elementitem_p.h"
#include "elementdetail_p.h"
#include "elementtranslator.h"

#include <openbabel/data.h>

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QFont>
#include <QFontMetrics>
#include <QDebug>

// The use of postfix operators here is just fine!
// We're using int for postfix, so it's almost zero overhead
// krazy:excludeall=postfixop
namespace Avogadro {

  PeriodicTableScene::PeriodicTableScene(QObject *parent)
    : QGraphicsScene(parent)
  {
    int width = 26;
    int height = 26;

    ElementDetail *detail = new ElementDetail(1);
    detail->setPos(6.5 * width, 0.75 * height);
    addItem(detail);

    // Connect the slot and the signal...
    connect(this, SIGNAL(elementChanged(int)),
            detail, SLOT(elementChanged(int)));

    ElementItem *item = new ElementItem(1);
    item->setPos( 0 * width, 0 * height);
    addItem(item);

    item = new ElementItem(2);
    item->setPos(17 * width, 0 * height);
    addItem(item);

    item = new ElementItem(3);
    item->setPos( 0 * width, 1 * height);
    addItem(item);

    item = new ElementItem(4);
    item->setPos( 1 * width, 1 * height);
    addItem(item);

    item = new ElementItem(5);
    item->setPos(12 * width, 1 * height);
    addItem(item);

    item = new ElementItem(6);
    item->setPos(13 * width, 1 * height);
    addItem(item);

    item = new ElementItem(7);
    item->setPos(14 * width, 1 * height);
    addItem(item);

    item = new ElementItem(8);
    item->setPos(15 * width, 1 * height);
    addItem(item);

    item = new ElementItem(9);
    item->setPos(16 * width, 1 * height);
    addItem(item);

    item = new ElementItem(10);
    item->setPos(17 * width, 1 * height);
    addItem(item);

    item = new ElementItem(11);
    item->setPos( 0 * width, 2 * height);
    addItem(item);

    item = new ElementItem(12);
    item->setPos( 1 * width, 2 * height);
    addItem(item);

    item = new ElementItem(13);
    item->setPos(12 * width, 2 * height);
    addItem(item);

    item = new ElementItem(14);
    item->setPos(13 * width, 2 * height);
    addItem(item);

    item = new ElementItem(15);
    item->setPos(14 * width, 2 * height);
    addItem(item);

    item = new ElementItem(16);
    item->setPos(15 * width, 2 * height);
    addItem(item);

    item = new ElementItem(17);
    item->setPos(16 * width, 2 * height);
    addItem(item);

    item = new ElementItem(18);
    item->setPos(17 * width, 2 * height);
    addItem(item);

    int element = 19;
    for (int i = 3; i < 5; i++) {
      for (int j = 0; j < 18; j++) {
        item = new ElementItem(element++);
        item->setPos(j * width, i * height);
        addItem(item);
      }
    }

    item = new ElementItem(element++);
    item->setPos( 0 * width, 5 * height);
    addItem(item);

    item = new ElementItem(element++);
    item->setPos( 1 * width, 5 * height);
    addItem(item);

    element = 71;

    for (int i = 2; i < 18; ++i) {
      item = new ElementItem(element++);
      item->setPos(i * width, 5 * height);
      addItem(item);
    }

    item = new ElementItem(element++);
    item->setPos( 0 * width, 6 * height);
    addItem(item);

    item = new ElementItem(element++);
    item->setPos( 1 * width, 6 * height);
    addItem(item);

    element = 103;
    // Goes up to element 118
    for (int i = 2; i < 18; ++i) {
      item = new ElementItem(element++);
      item->setPos(i * width, 6 * height);
      addItem(item);
    }

    // Now for the weird ones at the bottom...
    element = 57;
    for (int i = 2; i < 16; ++i) {
      item = new ElementItem(element++);
      item->setPos(i * width, 7.5 * height);
      addItem(item);
    }
    element = 89;
    for (int i = 2; i < 16; ++i) {
      item = new ElementItem(element++);
      item->setPos(i * width, 8.5 * height);
      addItem(item);
    }
  }

  bool PeriodicTableScene::event(QEvent *e)
  {
    return QGraphicsScene::event(e);
  }

  void PeriodicTableScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
  {
    if (event->button() != Qt::LeftButton)
      return;

    QGraphicsItem *item = QGraphicsScene::itemAt(event->scenePos());
    if (item->data(0).toInt() > 0 && item->data(0).toInt() < 119)
      emit(elementChanged(item->data(0).toInt()));

    QGraphicsScene::mousePressEvent(event);
  }

  void PeriodicTableScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
  {
    QGraphicsScene::mouseMoveEvent(event);
  }

  void PeriodicTableScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
  {
    QGraphicsScene::mouseReleaseEvent(event);
  }

} // End namespace Avogadro

#include "periodictablescene_p.moc"
