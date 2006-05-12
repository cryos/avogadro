#include "AGLWidget.h"

AGLWidget::AGLWidget(QWidget *parent)
	: QGLWidget(parent)
{
	printf("Constructor\n");
}

void AGLWidget::initializeGL()
{
	printf("Initializing\n");
	glClearColor(0.0, 0.0, 0.0, 0.0);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
}

void AGLWidget::resizeGL(int width, int height)
{
	printf("Resizing.\n");
	int side = qMin(width, height);
	glViewport((width - side) / 2, (height - side) / 2, side, side);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
}

void AGLWidget::paintGL()
{
	printf("Painting.\n");
	glClear(GL_COLOR_BUFFER_BIT);

	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_POLYGON);
	glVertex3f(0.25, 0.25, 0.0);
	glVertex3f(0.75, 0.25, 0.0);
	glVertex3f(0.75, 0.75, 0.0);
	glVertex3f(0.25, 0.75, 0.0);
	glEnd();

	glFlush();
}

