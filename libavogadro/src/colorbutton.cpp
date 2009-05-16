/**********************************************************************
  ColorButton - Button widget to display the current selected color
                and bring up the color picker when clicked

  Copyright (C) 2008 Geoffrey Hutchison

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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

#include "colorbutton.h"
#include <QColorDialog>
#include <QPainter>

namespace Avogadro {

  ColorButton::ColorButton(QWidget *parent)
    : QAbstractButton(parent), m_color(Qt::white)
  {
    setMinimumSize(35, 20);

    connect(this, SIGNAL(clicked()), this, SLOT(changeColor()));
  }

  ColorButton::ColorButton(const QColor& initial, QWidget *parent)
    : QAbstractButton(parent), m_color(initial)
  {
    setMinimumSize(35, 20);

    connect(this, SIGNAL(clicked()), this, SLOT(changeColor()));
  }

  void ColorButton::changeColor()
  {
    // This could be an ifdef for KColorDialog if KDE is present
    m_color = QColorDialog::getColor(m_color, this);
    update();

    emit colorChanged(m_color);
  }

  void ColorButton::setColor(const QColor& color)
  {
    m_color = color;
    update();

    emit colorChanged(m_color);
  }

  QColor ColorButton::color() const
  {
    return m_color;
  }

  void ColorButton::paintEvent(QPaintEvent *)
  {
    // TODO: If we go to RGBA colors, we should really show two pieces
    // e.g.  -----------
    //       |        /|
    //       | non   / |
    //       | alpha/  |
    //       |     /   |
    //       |    /alpha
    //       |   /     |
    //       -----------

    QPainter painter(this);

    // outer border
    painter.drawRect(0,0, width(), height());
    // inner color
    painter.setBrush(m_color);
    painter.drawRect(4,4, width()-8, height()-8);
  }

  bool ColorButton::event(QEvent *e)
  {
    return QAbstractButton::event(e);
  }

} // end namespace

#include "colorbutton.moc"
