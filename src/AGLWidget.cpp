#include "AGLWidget.h"

AGLWidget::AGLWidget(QWidget *parent)
	: QGLWidget(parent)
{
	printf("Constructor\n");
}

void AGLWidget::initializeGL()
{
	printf("Initializing\n");

	glClearColor(0.0, 0.0, 0.0, 1.0);

  glShadeModel( GL_SMOOTH );
  glEnable( GL_DEPTH_TEST );
  glDepthFunc( GL_LEQUAL );
  //  glEnable( GL_CULL_FACE );
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

  glEnable( GL_NORMALIZE );
  glEnable( GL_LIGHTING );

  GLfloat ambientLight[] = { 0.4, 0.4, 0.4, 1.0 };
  GLfloat diffuseLight[] = { 0.8, 0.8, 0.8, 1.0 };
  GLfloat specularLight[] = { 1.0, 1.0, 1.0, 1.0 };
  GLfloat position[] = { 0.8, 0.7, 1.0, 0.0 };
  
  glLightfv( GL_LIGHT0, GL_AMBIENT, ambientLight );
  glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuseLight );
  glLightfv( GL_LIGHT0, GL_SPECULAR, specularLight );
  glLightfv( GL_LIGHT0, GL_POSITION, position );
  glEnable( GL_LIGHT0 );
  
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(10.0, -10.0, 10.0, -10.0, 0.0, 10.0);
	glMatrixMode(GL_MODELVIEW);
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
  glEnable(GL_COLOR_MATERIAL);

  glEnable(GL_LIGHTING);

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
