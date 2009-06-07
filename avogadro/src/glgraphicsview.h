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

#ifndef GLGRAPHICSVIEW_H
#define GLGRAPHICSVIEW_H

#include <avogadro/global.h>
#include <QGraphicsView>

namespace Avogadro {

  class GLWidget;

  class A_EXPORT GLGraphicsView : public QGraphicsView
  {
  Q_OBJECT

  public:
    GLGraphicsView(QWidget *parent = 0);
    explicit GLGraphicsView(GLWidget *widget, QWidget *parent = 0);
    explicit GLGraphicsView(QGraphicsScene *scene, QWidget *parent = 0);

    void constructor();

    void resizeEvent(QResizeEvent *event);

    GLWidget * glWidget();

  protected:
    virtual void drawBackground(QPainter *painter, const QRectF &rect);
    virtual bool event(QEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent * event);

  private:
    GLWidget *m_glwidget;
  };

}

#endif

