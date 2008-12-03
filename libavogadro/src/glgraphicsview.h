

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
    GLGraphicsView(GLWidget *widget, QWidget *parent = 0);
    GLGraphicsView(QGraphicsScene *scene, QWidget *parent = 0);

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

