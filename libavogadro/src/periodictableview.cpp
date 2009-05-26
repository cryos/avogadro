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

#include "periodictableview.h"

#include <avogadro/elementtranslator.h>

#include <openbabel/data.h>

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QFont>
#include <QFontMetrics>
#include <QDebug>

namespace OpenBabel{
  OBElementTable etab;
}

namespace Avogadro {

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
    if (m_element == 1 || m_element == 2 || m_element == 9 || m_element == 47 || m_element == 78) {
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

    for (int i = 2; i < 16; ++i) {
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

  PeriodicTableView::PeriodicTableView(QWidget *parent) : QGraphicsView(parent)
  {
    // Use a small title bar (Qt::Tool) with no minimize or maximise buttons
    setWindowFlags(Qt::Dialog | Qt::Tool);

    PeriodicTableScene *table = new PeriodicTableScene;
    table->setSceneRect(-20, -20, 480, 260);
    table->setItemIndexMethod(QGraphicsScene::NoIndex);
    table->setBackgroundBrush(Qt::white);

    setScene(table);
    setRenderHint(QPainter::Antialiasing);
    setWindowTitle(tr("Periodic Table"));
    resize(490, 270);
    setFixedSize(490, 270);
    connect(table, SIGNAL(elementChanged(int)),
            this, SLOT(elementClicked(int)));
  }

  PeriodicTableView::~PeriodicTableView()
  {
    delete scene();
  }

  void PeriodicTableView::elementClicked(int id)
  {
    emit(elementChanged(id));
  }

  bool PeriodicTableView::event(QEvent *e)
  {
    return QGraphicsView::event(e);
  }

  void PeriodicTableView::mouseDoubleClickEvent(QMouseEvent *)
  {
    close();
  }

} // End namespace Avogadro

#include "periodictableview.moc"
