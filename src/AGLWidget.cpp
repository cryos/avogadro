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
  glDepthFunc( GL_LESS );
  glEnable( GL_CULL_FACE );

  GLfloat mat_ambient[] = { 0.5, 0.5, 0.5, 1.0 };
  GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
  GLfloat mat_shininess[] = { 30.0 };

  glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
  glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glLoadIdentity();
  glGetDoublev( GL_MODELVIEW_MATRIX, _RotationMatrix);
  glPopMatrix();
  
	glMatrixMode(GL_PROJECTION);
  glOrtho(10.0, -10.0, 10.0, -10.0, -10.0, 10.0);
}

void AGLWidget::resizeGL(int width, int height)
{
	printf("Resizing.\n");
	int side = qMin(width, height);
	glViewport((width - side) / 2, (height - side) / 2, side, side);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(10.0, -10.0, 10.0, -10.0, -10.0, 10.0);
	glMatrixMode(GL_MODELVIEW);
}

void AGLWidget::paintGL()
{
	printf("Painting.\n");
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  glPushMatrix();
  glMultMatrixd(_RotationMatrix);
  std::vector<GLuint>::iterator i;
  for (i = _displayLists.begin(); i != _displayLists.end(); i++)
    glCallList(*i);
  glPopMatrix();

	glFlush();
}

void AGLWidget::addDisplayList(GLuint dl)
{
  _displayLists.push_back(dl);
  updateGL();
}

void AGLWidget::deleteDisplayList(GLuint dl)
{
  std::vector<GLuint>::iterator i;
  for (i = _displayLists.begin(); i != _displayLists.end(); i++)
    if (*i == dl)
      _displayLists.erase(i);
}

void AGLWidget::mousePressEvent( QMouseEvent * event )
{
  if( event->buttons() & Qt::LeftButton )
    {       
      _leftButtonPressed = true;
      _movedSinceButtonPressed = false;
      _lastDraggingPosition = event->pos ();
      _initialDraggingPosition = event->pos ();
      updateGL();
    }
}

void AGLWidget::mouseReleaseEvent( QMouseEvent * event )
{
  if( !( event->buttons() & Qt::LeftButton ) )
    {
      _leftButtonPressed = false;
    }
}

void AGLWidget::mouseMoveEvent( QMouseEvent * event )
{
  if( _leftButtonPressed )
    {
      QPoint deltaDragging = event->pos() - _lastDraggingPosition;
      _lastDraggingPosition = event->pos();
      if( ( event->pos()
            - _initialDraggingPosition ).manhattanLength() > 2 )
        _movedSinceButtonPressed = true;
      
      glPushMatrix();
      glLoadIdentity();
      glRotated( deltaDragging.x(), 0.0, 1.0, 0.0 );
      glRotated( deltaDragging.y(), 1.0, 0.0, 0.0 );
      glMultMatrixd( _RotationMatrix );
      glGetDoublev( GL_MODELVIEW_MATRIX, _RotationMatrix );
      glPopMatrix();
      updateGL();
    }
}
