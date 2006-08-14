#ifndef __AGLWIDGET_H
#define __AGLWIDGET_H

#include <QGLWidget>
#include <QMouseEvent>
#include <vector>

class AGLWidget : public QGLWidget
{
	Q_OBJECT

	public:
		AGLWidget(QWidget *parent = 0);

    void addDisplayList(GLuint dl);
    void deleteDisplayList(GLuint dl);

	protected:
		virtual void initializeGL();
		virtual void paintGL();
		virtual void resizeGL(int, int);

    virtual void mousePressEvent( QMouseEvent * event );
    virtual void mouseReleaseEvent( QMouseEvent * event );
    virtual void mouseMoveEvent( QMouseEvent * event );

    std::vector<GLuint> _displayLists;
    bool                _leftButtonPressed;
    bool                _movedSinceButtonPressed;

    QPoint              _initialDraggingPosition;
    QPoint              _lastDraggingPosition;
    
    GLdouble            _RotationMatrix[16];
};

#endif
