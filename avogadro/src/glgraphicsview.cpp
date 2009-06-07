/**********************************************************************
  GLGraphicsView - a custom GL Graphics View - not finished yet...

  Copyright (C) 2008-2009 Marcus D. Hanwell

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

#include "glgraphicsview.h"

#include <avogadro/glwidget.h>

#include <QDebug>

namespace Avogadro {

  GLGraphicsView::GLGraphicsView(QWidget *parent) : QGraphicsView(parent)
  {
    m_glwidget = new GLWidget();
    constructor();
  }

  GLGraphicsView::GLGraphicsView(GLWidget *widget, QWidget *parent)
    : QGraphicsView(parent)
  {
    m_glwidget = widget;
    constructor();
  }

  GLGraphicsView::GLGraphicsView(QGraphicsScene *scene, QWidget *parent)
    : QGraphicsView(scene, parent)
  {
    m_glwidget = new GLWidget();
    constructor();
  }

  void GLGraphicsView::constructor()
  {
    setViewport(m_glwidget);
    m_glwidget->setAutoBufferSwap(true);
    setFrameStyle(QFrame::NoFrame);
  }

  void GLGraphicsView::drawBackground(QPainter *painter, const QRectF& rect)
  {
//    m_glwidget->initializeGL();
    m_glwidget->paintGL2();
    QGraphicsView::drawBackground(painter, rect);
  }

  void GLGraphicsView::resizeEvent(QResizeEvent *event)
  {
    m_glwidget->resizeEvent(event);
    QGraphicsView::resizeEvent(event);
  }

  bool GLGraphicsView::event(QEvent *event)
  {
    //m_glwidget->event(event);
    return QGraphicsView::event(event);
  }

  void GLGraphicsView::mouseMoveEvent(QMouseEvent *event)
  {
    m_glwidget->mouseMoveEvent(event);
    QGraphicsView::mouseMoveEvent(event);
  }

  void GLGraphicsView::mousePressEvent(QMouseEvent *event)
  {
    m_glwidget->mousePressEvent(event);
    QGraphicsView::mousePressEvent(event);
  }

  void GLGraphicsView::mouseReleaseEvent(QMouseEvent *event)
  {
    m_glwidget->mouseReleaseEvent(event);
    QGraphicsView::mouseReleaseEvent(event);
  }

  void GLGraphicsView::wheelEvent(QWheelEvent *event)
  {
    m_glwidget->wheelEvent(event);
    QGraphicsView::wheelEvent(event);
  }

  GLWidget * GLGraphicsView::glWidget()
  {
    return m_glwidget;
  }

} // End namespace Avogadro

#include "glgraphicsview.moc"

