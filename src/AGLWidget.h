#ifndef __AGLWIDGET_H
#define __AGLWIDGET_H

#include <QGLWidget>

class AGLWidget : public QGLWidget
{
	Q_OBJECT

	public:
		AGLWidget(QWidget *parent = 0);

	protected:
		void initializeGL();
		void paintGL();
		void resizeGL(int, int);
};

#endif
