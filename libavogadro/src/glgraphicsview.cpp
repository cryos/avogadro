

#include "glgraphicsview.h"

#include "glwidget.h"

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

