/**********************************************************************
  icontabwidget.cpp - IconTabWidget that only displays icons

  Copyright (C) 2007 Donald Ephraim Curtis <dcurtis3@sourceforge.net>

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

#include "icontabwidget.h"

#include <QDebug>

namespace Avogadro {

  IconTabWidget::IconTabWidget(QWidget *parent) : QTabWidget(parent)
  {
    IconTabBar *iconTabBar = new IconTabBar(this);
    setTabBar(iconTabBar);
  }

  IconTabBar::IconTabBar(QWidget *parent) : QTabBar(parent) {}

  QSize IconTabBar::tabSizeHint(int index) const
  {
    QSize size = QTabBar::tabSizeHint(index);
    size.rwidth()=32;
    return size;
  }
}

#include "icontabwidget.moc"
