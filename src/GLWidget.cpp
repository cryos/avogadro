/**********************************************************************
  GLWidget - general OpenGL display

  Copyright (C) 2006 by Geoffrey R. Hutchison
  Some portions Copyright (C) 2006 by Donald E. Curtis

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  Some code is based on Open Babel
  For more information, see <http://openbabel.sourceforge.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#include "GLWidget.h"

#include "MainWindow.h"

#include <stdio.h>
#define BUFSIZE 512

using namespace Avogadro;

GLWidget::GLWidget(QWidget *parent ) : QGLWidget(parent), defaultGLEngine(NULL), _clearColor(Qt::black)
{
  printf("Constructor\n");
  loadGLEngines();
}

GLWidget::GLWidget(const QGLFormat &format, QWidget *parent) : QGLWidget(format, parent), defaultGLEngine(NULL), _clearColor(Qt::black)
{
  printf("Constructor\n");
  loadGLEngines();
}

void GLWidget::initializeGL()
{
  printf("Initializing\n");

  qglClearColor ( _clearColor );

  //  glShadeModel( GL_SMOOTH );
  glEnable( GL_DEPTH_TEST );
  glDepthFunc( GL_LESS );
  glEnable( GL_CULL_FACE );

  GLfloat mat_ambient[] = { 0.5, 0.5, 0.5, 1.0 };
  GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
  GLfloat mat_shininess[] = { 30.0 };

  glEnable(GL_COLOR_MATERIAL);
  glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
  glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
  glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

  // Used to display semi-transparent relection rectangle
  glBlendFunc(GL_ONE, GL_ONE);

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

  // Initialize a clean rotation matrix
  glPushMatrix();
  glLoadIdentity();
  glGetDoublev( GL_MODELVIEW_MATRIX, _RotationMatrix);
  glPopMatrix();

  _TranslationVector[0] = 0.0;
  _TranslationVector[1] = 0.0;
  _TranslationVector[2] = 0.0;

  _Scale = 1.0;

  glEnableClientState( GL_VERTEX_ARRAY );
  glEnableClientState( GL_NORMAL_ARRAY );
}

void GLWidget::resizeGL(int width, int height)
{
  printf("Resizing.\n");
  glViewport(0,0,width,height);

}

void GLWidget::setCamera()
{
  // Reset the projection and set our perspective.
  gluPerspective(35,float(width())/height(),0.1,1000);

  // pull the camera back 20
  glTranslated ( 0.0, 0.0, -20.0 );
}

void GLWidget::render(GLenum mode)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // TODO: Be careful here.  For the time being we are actually changing the
  // orientation of our render in 3d space and not changing the camera.  And also
  // we're not changing the coordinates of the atoms.
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Translate our molecule as per user instructions
  glTranslated(_TranslationVector[0], _TranslationVector[1], _TranslationVector[2]);
  glScaled(_Scale, _Scale, _Scale);
  glMultMatrixd(_RotationMatrix);

  view->render();

  glFlush();
}

void GLWidget::paintGL()
{ 
  printf("Painting.\n");

  // Reset the projection
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  setCamera();

  // Reset the model view
  glMatrixMode(GL_MODELVIEW);
  render(GL_RENDER);
}

void GLWidget::addDisplayList(GLuint dl)
{
  _displayLists.push_back(dl);
  updateGL();
}

void GLWidget::deleteDisplayList(GLuint dl)
{
  std::vector<GLuint>::iterator i;
  for (i = _displayLists.begin(); i != _displayLists.end(); i++)
    if (*i == dl)
      _displayLists.erase(i);
}

void GLWidget::mousePressEvent( QMouseEvent * event )
{
  //   if ( event->buttons() & Qt::LeftButton ) {
  //     _leftButtonPressed = true;
  //   }
  //   else if ( event->buttons() & Qt::RightButton ) {
  //     _rightButtonPressed = true;
  //   }
  //   else if ( event->buttons() & Qt::MidButton ) {
  //     _midButtonPressed = true;
  //   }

  _movedSinceButtonPressed = false;
  _lastDraggingPosition = event->pos ();
  _initialDraggingPosition = event->pos ();
}

void GLWidget::startPicking(int x, int y)
{
  GLint viewport[4];

  glSelectBuffer(BUFSIZE,selectBuf);
  glRenderMode(GL_SELECT);

  // Setup our limited viewport for picking.
  glGetIntegerv(GL_VIEWPORT,viewport);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluPickMatrix(x,viewport[3]-y, 5,5,viewport);

  setCamera();

  // Get ready for rendering
  glMatrixMode(GL_MODELVIEW);
  glInitNames();
}

void GLWidget::stopPicking()
{
  int hits;

  // restoring the original projection matrix
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glFlush();

  // returning to normal rendering mode
  hits = glRenderMode(GL_RENDER);

  // if there are hits process them
  if (hits != 0)
    processHits(hits,selectBuf);
}

void GLWidget::processHits (GLint hits, GLuint buffer[])
{
  unsigned int i, j;
  GLuint ii, jj, names, *ptr;

  printf ("hits = %d\n", hits);
  ptr = (GLuint *) buffer;
  for (i = 0; i < hits; i++) {
    names = *ptr;
    printf (" number of names for this hit = %d\n", names); ptr++;
    printf("  z1 is %g;", (float) *ptr/0x7fffffff); ptr++;
    printf(" z2 is %g\n", (float) *ptr/0x7fffffff); ptr++;
    printf ("   names are "); 
    for (j = 0; j < names; j++) { /*  for each name */
      printf ("%d ", *ptr);
      if (j == 0)  /*  set row and column  */
        ii = *ptr;
      else if (j == 1)
        jj = *ptr;
      ptr++;
    }
    printf ("\n");
  }
}

void GLWidget::mouseReleaseEvent( QMouseEvent * event )
{
  if(!_movedSinceButtonPressed)
  {
    startPicking(event->pos().x(), event->pos().y());
    render(GL_SELECT);
    stopPicking();

    qDebug() << "OUT";
  }

  //   if( !( event->buttons() & Qt::LeftButton ) ) {
  //       _leftButtonPressed = false;
  //   }
  //   else if ( !( event->buttons() & Qt::RightButton ) ) {
  //     _rightButtonPressed = false;
  //   }
  //   else if ( !( event->buttons() & Qt::MidButton ) ) {
  //     _midButtonPressed = false;
  //   }
}

void GLWidget::mouseMoveEvent( QMouseEvent * event )
{
  QPoint deltaDragging = event->pos() - _lastDraggingPosition;
  _lastDraggingPosition = event->pos();
  if( ( event->pos()
        - _initialDraggingPosition ).manhattanLength() > 2 )
    _movedSinceButtonPressed = true;

  if( event->buttons() & Qt::LeftButton )
  {      
    glPushMatrix();
    glLoadIdentity();
    glRotated( deltaDragging.x(), 0.0, 1.0, 0.0 );
    glRotated( deltaDragging.y(), 1.0, 0.0, 0.0 );
    glMultMatrixd( _RotationMatrix );
    glGetDoublev( GL_MODELVIEW_MATRIX, _RotationMatrix );
    glPopMatrix();
  }
  else if ( event->buttons() & Qt::RightButton )
  {
    deltaDragging = _initialDraggingPosition - event->pos();

    _TranslationVector[0] = -deltaDragging.x() / 5.0;
    _TranslationVector[1] = deltaDragging.y() / 5.0;
  }
  else if ( event->buttons() & Qt::MidButton )
  {
    deltaDragging = _initialDraggingPosition - event->pos();
    int xySum = deltaDragging.x() + deltaDragging.y();

    if (xySum < 0)
      _Scale = deltaDragging.manhattanLength() / 5.0;
    else if (xySum > 0)
      _Scale = 1.0 / deltaDragging.manhattanLength();
  }

  updateGL();
}

void GLWidget::startScreenCoordinates() const
{
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, width(), height(), 0, 0.0, -1.0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
}

void GLWidget::stopScreenCoordinates() const
{
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

void GLWidget::setView(View *v)
{
  view = v;
}

void GLWidget::setDefaultGLEngine(int i) 
{
  setDefaultGLEngine(glEngines.at(i));
}

void GLWidget::setDefaultGLEngine(GLEngine *e) 
{
  if(e)
  {
    defaultGLEngine = e;
    updateGL();
  }
}

void GLWidget::loadGLEngines()
{
  QDir pluginsDir = QDir(qApp->applicationDirPath());

  if (!pluginsDir.cd("engines") && getenv("AVOGADRO_PLUGINS") != NULL)
  {
    pluginsDir.cd(getenv("AVOGADRO_PLUGINS"));
  }

  qDebug() << "PluginsDir:" << pluginsDir.absolutePath() << endl;
  // load static plugins first
//   foreach (QObject *plugin, QPluginLoader::staticInstances())
//   {
//     GLEngine *r = qobject_cast<GLEngine *>(plugin);
//     if (r)
//     {
//       qDebug() << "Loaded GLEngine: " << r->name() << endl;
//       if( defaultGLEngine == NULL )
//         defaultGLEngine = r;
//     }
//   }

  foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
    QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
    GLEngineFactory *factory = qobject_cast<GLEngineFactory *>(loader.instance());
    if (factory) {
      GLEngine *engine = factory->createInstance();
      qDebug() << "Found Plugin: " << engine->name() << " - " << engine->description(); 
      if (!defaultGLEngine)
      {
        qDebug() << "Setting Default GLEngine: " << engine->name() << " - " << engine->description(); 
        defaultGLEngine = engine;
      }
      glEngines.append(engine);
    }
  }
}

