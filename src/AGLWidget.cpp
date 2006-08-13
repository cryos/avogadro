#include "AGLWidget.h"

AGLWidget::AGLWidget(QWidget *parent)
	: QGLWidget(parent)
{
	printf("Constructor\n");
}

void AGLWidget::initializeGL()
{
	printf("Initializing\n");

  glEnable(GL_LIGHT1);
  glEnable(GL_LIGHTING);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_COLOR_MATERIAL);

	glClearColor(0.0, 0.0, 0.0, 0.0);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(10.0, -10.0, 10.0, -10.0, 0.0, 10.0);
	glMatrixMode(GL_MODELVIEW);

  // example -- this should be done by a render class
  GLuint dlist = glGenLists(1);
  glNewList(dlist, GL_COMPILE);
  glPushMatrix();
  glTranslated(0.0, 0.0, 0.0);
  GLUquadricObj* q = gluNewQuadric();
  gluQuadricDrawStyle(q, GLU_FILL );

  glColor3f(1.0, 1.0, 1.0);
  glTranslated(0.0, 1.7, 0.0);
  gluSphere(q, 0.7, 10, 10);
  glTranslated(0.8, 1.2, 0.8);
  glColor3f(0.0, 0.7, 0.3);
  gluSphere(q, 0.7, 10, 10);

  gluDeleteQuadric(q);
  glPopMatrix();
  glEndList();

  addDisplayList(dlist);
}

void AGLWidget::resizeGL(int width, int height)
{
	printf("Resizing.\n");
	int side = qMin(width, height);
	glViewport((width - side) / 2, (height - side) / 2, side, side);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(10.0, -10.0, 10.0, -10.0, 0.0, 10.0);
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
