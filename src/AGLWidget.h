#ifndef __AGLWIDGET_H
#define __AGLWIDGET_H

#include <QGLWidget>
#include <vector>

class AGLWidget : public QGLWidget
{
	Q_OBJECT

	public:
		AGLWidget(QWidget *parent = 0);

    void addDisplayList(GLuint dl);
    void deleteDisplayList(GLuint dl);

	protected:
		void initializeGL();
		void paintGL();
		void resizeGL(int, int);

    std::vector<GLuint> _displayLists;
};

#endif
