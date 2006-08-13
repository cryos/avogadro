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

  GLuint testDL = glGenLists(1);
  static GLUquadric* quad = gluNewQuadric();
  glNewList(testDL, GL_COMPILE);
  gluSphere(quad, 0.08, 10, 6);
  glEndList();
  addDisplayList(testDL);
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

  std::vector<GLuint>::iterator i;
  for (i = _displayLists.begin(); i != _displayLists.end(); i++)
    glCallList(*i);

	glFlush();
}

void AGLWidget::addDisplayList(GLuint dl)
{
  _displayLists.push_back(dl);
}

void AGLWidget::deleteDisplayList(GLuint dl)
{
  std::vector<GLuint>::iterator i;
  for (i = _displayLists.begin(); i != _displayLists.end(); i++)
    if (*i == dl)
      _displayLists.erase(i);
}
