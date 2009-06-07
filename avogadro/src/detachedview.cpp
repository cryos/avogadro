/**********************************************************************
  DetachedView - a widget to display a GLWidget detached view

  Copyright (C) 2009 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2 of the
  License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#include "detachedview.h"

#include <avogadro/glwidget.h>

#include <QtGui/QHBoxLayout>

#include <QtCore/QDebug>

namespace Avogadro {

  DetachedView::DetachedView(GLWidget *glWidget, QWidget *parent,
                             Qt::WindowFlags f)
    : QWidget(parent, f), m_glWidget(glWidget)
  {
    setContentsMargins(0, 0, 0, 0);
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(m_glWidget);
    layout->setMargin(0);

    // Attach to the GLWidget destroyed signal so that we can destroy ourselves
    connect(m_glWidget, SIGNAL(destroyed()), this, SLOT(glWidgetDestroyed()));
  }

  DetachedView::~DetachedView()
  {
  }

  void DetachedView::mainWindowClosed()
  {
    hide();
    deleteLater();
  }

} // End namespace Avogadro

#include "detachedview.moc"
