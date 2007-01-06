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

#include "glwidget.moc"

#include <stdio.h>
#include <vector>

using namespace Avogadro;

GLWidget::GLWidget(QWidget *parent ) 
: QGLWidget(parent), defaultEngine(NULL), _clearColor(Qt::black), molecule(NULL)
{
  init();
}

GLWidget::GLWidget(const QGLFormat &format, QWidget *parent) 
: QGLWidget(format, parent), defaultEngine(NULL), _clearColor(Qt::black), molecule(NULL)
{
  init();
}

void GLWidget::init()
{
  loadEngines();

  molecule = NULL;
}

void GLWidget::initializeGL()
{
  qDebug() << "GLWidget::initializeGL";

  qglClearColor ( _clearColor );

  glShadeModel( GL_SMOOTH );
  glEnable( GL_DEPTH_TEST );
  glDepthFunc( GL_LESS );
  glEnable( GL_CULL_FACE );
  glEnable( GL_COLOR_SUM_EXT );

  GLfloat mat_ambient[] = { 0.5, 0.5, 0.5, 1.0 };
  GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
  GLfloat mat_shininess[] = { 30.0 };

  glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
  glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
  glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

  // Used to display semi-transparent relection rectangle
  //  glBlendFunc(GL_ONE, GL_ONE);
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

  glEnable( GL_NORMALIZE );
  glEnable( GL_LIGHTING );

  GLfloat ambientLight[] = { 0.4, 0.4, 0.4, 1.0 };
  GLfloat diffuseLight[] = { 0.8, 0.8, 0.8, 1.0 };
  GLfloat specularLight[] = { 1.0, 1.0, 1.0, 1.0 };
  GLfloat position[] = { 0.8, 0.7, 1.0, 0.0 };

  glLightModeli( GL_LIGHT_MODEL_COLOR_CONTROL_EXT,
      GL_SEPARATE_SPECULAR_COLOR_EXT );
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
  qDebug() << "GLWidget::resizeGL";
  glViewport(0,0,width,height);

}

void GLWidget::setCamera()
{
  //qDebug() << "GLWidget::setCamera";
  // Reset the projection and set our perspective.
  gluPerspective(35,float(width())/height(),1.0,100);

  // pull the camera back 20
  glTranslated ( 0.0, 0.0, -20.0 );
}

void GLWidget::rotate(float x, float y, float z)
{
  //qDebug() << "GLWidget::rotate";
  glPushMatrix();
  glLoadIdentity();
  glRotated( x, 1.0, 0.0, 0.0 );
  glRotated( y, 0.0, 1.0, 0.0 );
  glRotated( z, 0.0, 0.0, 1.0 );
  glMultMatrixd( _RotationMatrix );
  glGetDoublev( GL_MODELVIEW_MATRIX, _RotationMatrix );
  glPopMatrix();
}

void GLWidget::translate(float x, float y, float z)
{
  qDebug() << "GLWidget::translate";
  _TranslationVector[0] = _TranslationVector[0] + x;
  _TranslationVector[1] = _TranslationVector[1] + y;
  _TranslationVector[2] = _TranslationVector[2] + z;
}

void GLWidget::setScale(float s)
{
  _Scale = s;
}

float GLWidget::getScale()
{
  return _Scale;
}

void GLWidget::render(GLenum mode)
{
  //qDebug() << "GLWidget::render";
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // TODO: Be careful here.  For the time being we are actually changing the
  // orientation of our render in 3d space and changing the camera.  And also
  // we're not changing the coordinates of the atoms.

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Translate our molecule as per user instructions
  glScaled(_Scale, _Scale, _Scale);
  glMultMatrixd(_RotationMatrix);
  glTranslated(_TranslationVector[0], _TranslationVector[1], _TranslationVector[2]);

  if(defaultEngine)
    defaultEngine->render(&defaultQueue);

  for(int i=0; i<_displayLists.size(); i++) {
    qDebug() << "Calling DL: " << _displayLists[i] << endl;
    glCallList(_displayLists[i]);
  }

  glFlush();
}

void GLWidget::paintGL()
{ 
  //qDebug() << "GLWidget::paintGL";
  // Reset the projection
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  setCamera();

  // Reset the model view
  glMatrixMode(GL_MODELVIEW);
  render(GL_RENDER);
}

void GLWidget::mousePressEvent( QMouseEvent * event )
{
  //qDebug() << "GLWidget::mousePressEvent";
  emit mousePress(event);
}

void GLWidget::mouseReleaseEvent( QMouseEvent * event )
{
  //qDebug() << "GLWidget::mouseReleaseEvent";
  emit mouseRelease(event);
}

void GLWidget::mouseMoveEvent( QMouseEvent * event )
{
  //qDebug() << "GLWidget::mouseMoveEvent";
  emit mouseMove(event);
}

void GLWidget::addDL(GLuint dl)
{
  //qDebug() << "GLWidget::addDL";
  _displayLists.append(dl);
}

void GLWidget::removeDL(GLuint dl)
{
  _displayLists.removeAll(dl);
}

void GLWidget::setMolecule(Molecule *m)
{
  //qDebug() << "GLWidget::setMolecule";
  for( int i=0; i < queues.size(); i++ ) {
    queues[i].clear();
  }

  defaultQueue.clear();

  molecule = m;

  // add the atoms to the default queue
  std::vector<OpenBabel::OBNodeBase*>::iterator i;
  for(Atom *atom = (Atom*)m->BeginAtom(i); atom; atom = (Atom*)m->NextAtom(i))
  {
    defaultQueue.add(atom);
  }

  // add the bonds to the default queue
  std::vector<OpenBabel::OBEdgeBase*>::iterator j;
  for(Bond *bond = (Bond*)m->BeginBond(j); bond; bond = (Bond*)m->NextBond(j))
  {
    defaultQueue.add(bond);
  }

  // add the residues to the default queue
  std::vector<OpenBabel::OBResidue*>::iterator k;
  for(Residue *residue = (Residue*)m->BeginResidue(k); residue;
      residue = (Residue *)m->NextResidue(k)) {
    defaultQueue.add(residue);
  }

  // add the molecule to the default queue
  defaultQueue.add(m);

  // connect our signals so if the molecule gets updated
  QObject::connect(m, SIGNAL(atomAdded(Atom *)), this, SLOT(addAtom(Atom *)));
//dc:   QObject::connect(m, SIGNAL(bondAdded(Bond *)), this, SLOT(addBond(Bond *)));
//dc:   QObject::connect(m, SIGNAL(residueAdded(Residue *)), this, SLOT(addResidue(Residue *)));

}

void GLWidget::addAtom(Atom *atom)
{
  qDebug() << "GLWidget::addAtom";
  defaultQueue.add(atom);
}

void GLWidget::setDefaultEngine(int i) 
{
  //qDebug() << "GLWidget::setDefaultEngine";
  setDefaultEngine(engines.at(i));
}

void GLWidget::setDefaultEngine(Engine *e) 
{
  //qDebug() << "GLWidget::setDefaultEngine";
  if(e)
  {
    defaultEngine = e;
    updateGL();
  }
}

//X:void GLWidget::setTool(Tool *tool)
//X:{
//X:  qDebug() << "Setting Current Tool: " << tool->name() << " - " << tool->description(); 
//X:  currentTool = tool;
//X:}
//X:
//X:Tool* GLWidget::getTool()
//X:{
//X:  return currentTool;
//X:}
//X:
//X:void GLWidget::loadTools()
//X:{
//X:  QDir pluginsDir = QDir(qApp->applicationDirPath());
//X:
//X:  if (!pluginsDir.cd("tools") && getenv("AVOGADRO_TOOLS") != NULL)
//X:  {
//X:    pluginsDir.cd(getenv("AVOGADRO_TOOLS"));
//X:  }
//X:
//X:  qDebug() << "PluginsDir:" << pluginsDir.absolutePath() << endl;
//X:  foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
//X:    QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
//X:    Tool *tool = qobject_cast<Tool *>(loader.instance());
//X:    if (tool) {
//X:      qDebug() << "Found Tool: " << tool->name() << " - " << tool->description(); 
//X:      if (!currentTool)
//X:      {
//X:        setTool(tool);
//X:      }
//X:      tools.append(tool);
//X:    }
//X:  }
//X:}

void GLWidget::loadEngines()
{
  QDir pluginsDir("/usr/local/lib/avogadro");

  if(getenv("AVOGADRO_ENGINES") != NULL)
  {
    pluginsDir.cd(getenv("AVOGADRO_ENGINES"));
  }

//X:  if (!pluginsDir.cd("") && getenv("AVOGADRO_ENGINES") != NULL)
//X:  {
//X:    pluginsDir.cd(getenv("AVOGADRO_ENGINES"));
//X:  }
//X:
  qDebug() << "PluginsDir:" << pluginsDir.absolutePath() << endl;
  // load static plugins first
  //   foreach (QObject *plugin, QPluginLoader::staticInstances())
  //   {
  //     Engine *r = qobject_cast<Engine *>(plugin);
  //     if (r)
  //     {
  //       qDebug() << "Loaded Engine: " << r->name() << endl;
  //       if( defaultEngine == NULL )
  //         defaultEngine = r;
  //     }
  //   }

  foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
    QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
//dc:     qDebug() << "File: " << fileName;
    EngineFactory *factory = qobject_cast<EngineFactory *>(loader.instance());
    if (factory) {
      Engine *engine = factory->createInstance();
      qDebug() << "Found Engine: " << engine->name() << " - " << engine->description(); 
      if (!defaultEngine)
      {
        qDebug() << "Setting Default Engine: " << engine->name() << " - " << engine->description(); 
        defaultEngine = engine;
      }
      engines.append(engine);
    }
  }
}

#define GL_SEL_BUF_SIZE 512

QList<GLHit> GLWidget::getHits(int x, int y, int w, int h)
{
  QList<GLHit> hits;
  GLuint selectBuf[GL_SEL_BUF_SIZE];
  GLint viewport[4];
  unsigned int hit_count;

  int cx = w/2 + x;
  int cy = h/2 + y;

  //X   hits.clear();

  glSelectBuffer(GL_SEL_BUF_SIZE,selectBuf);
  glRenderMode(GL_SELECT);

  // Setup our limited viewport for picking.
  glGetIntegerv(GL_VIEWPORT,viewport);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluPickMatrix(cx,viewport[3]-cy, w, h,viewport);

  setCamera();

  // Get ready for rendering
  glMatrixMode(GL_MODELVIEW);
  glInitNames();
  render(GL_SELECT);

  // restoring the original projection matrix
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glFlush();

  // returning to normal rendering mode
  hit_count = glRenderMode(GL_RENDER);

  // if there are hits process them
  if (hit_count != 0)
  {
    unsigned int i, j;
    GLuint names, type, *ptr;
    GLuint minZ, maxZ, name;

    //X   printf ("hits = %d\n", hits);
    ptr = (GLuint *) selectBuf;
    for (i = 0; i < hit_count; i++) {
      names = *ptr++;
      minZ = *ptr++;
      maxZ = *ptr++;
      //X     printf (" number of names for this hit = %d\n", names); names;
      //X     printf("  z1 is %g;", (float) *ptr/0x7fffffff); minZ;
      //X     printf(" z2 is %g\n", (float) *ptr/0x7fffffff); maxZ;
      //X     printf ("   names are "); 
      name = 0;
      for (j = 0; j < names/2; j++) { /*  for each name */
        type = *ptr++;
        name = *ptr++;
        printf ("%d(%d) ", name,type);
        //X       if (j == 0)  /*  set row and column  */
        //X         ii = *ptr;
        //X       else if (j == 1)
        //X         jj = *ptr;
        //X       ptr++;
      }
      if (name)
      {
        hits.append(GLHit(name, type, minZ, maxZ));
      }
    }
    printf ("\n");
    qSort(hits);
  }

  return(hits);
}
