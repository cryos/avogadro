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

#include "config.h"

#include <avogadro/glwidget.h>

#include <stdio.h>
#include <vector>

namespace Avogadro {
  class GLHitPrivate
  {
    public:
      GLHitPrivate() {};

      GLuint type;
      GLuint name;
      GLuint minZ;
      GLuint maxZ;
  };

  GLHit::GLHit() : d(new GLHitPrivate) {}

  GLHit::GLHit(const GLHit &other) : d(new GLHitPrivate) { 
    GLHitPrivate *e = other.d;
    d->type = e->type;
    d->name = e->name;
    d->minZ = e->minZ;
    d->maxZ = e->maxZ;
  }

  GLHit::GLHit(GLuint name, GLuint type, GLuint minZ, GLuint maxZ) : d(new GLHitPrivate) { 
    d->name = name;
    d->type = type;
    d->minZ = minZ;
    d->maxZ = maxZ;
  }

  GLHit &GLHit::operator=(const GLHit &other)
  {
    GLHitPrivate *e = other.d;
    d->type = e->type;
    d->name = e->name;
    d->minZ = e->minZ;
    d->maxZ = e->maxZ;
  }

  GLHit::~GLHit() {
    delete d;
  }

  bool GLHit::operator<(const GLHit &other) const { 
    GLHitPrivate *e = other.d;
    return e->minZ < e->minZ; 
  }

  bool GLHit::operator==(const GLHit &other) const { 
    GLHitPrivate *e = other.d;
    return ((d->type == e->type) && (d->name == e->name)); 
  }

  GLuint GLHit::name() const { return d->name; }
  GLuint GLHit::type() const { return d->type; }
  GLuint GLHit::minZ() const { return d->minZ; }
  GLuint GLHit::maxZ() const { return d->maxZ; }

  void GLHit::setName(GLuint name) { d->name = name; }
  void GLHit::setType(GLuint type) { d->type = type; }
  void GLHit::setMinZ(GLuint minZ) { d->minZ = minZ; }
  void GLHit::setMaxZ(GLuint maxZ) { d->maxZ = maxZ; }
  
  class GLWidgetPrivate {
    public:
      GLWidgetPrivate() : defaultEngine(0), background(Qt::black), molecule(0) {}
      
      Engine *defaultEngine;
      QList<Engine *> engines;
    
      PrimitiveQueue defaultQueue;
      QList<PrimitiveQueue> queues;
    
      Molecule *molecule;
      QList<GLuint> displayLists;
    
      GLdouble            rotationMatrix[16];
      GLdouble            translationVector[3];
      GLdouble            scale;
      QColor              background;
  };


  GLWidget::GLWidget(QWidget *parent ) 
  : QGLWidget(parent), d(new GLWidgetPrivate)
  {
    constructor();
  }
  
  GLWidget::GLWidget(const QGLFormat &format, QWidget *parent) 
  : QGLWidget(format, parent), d(new GLWidgetPrivate)
  {
    constructor();
  }
  
  GLWidget::GLWidget(Molecule *molecule, const QGLFormat &format, QWidget *parent) 
  : QGLWidget(format, parent), d(new GLWidgetPrivate)
  {
    constructor();
    setMolecule(molecule);
  }
  
  void GLWidget::constructor()
  {
    setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
    loadEngines();
  }
  
  void GLWidget::initializeGL()
  {
    qDebug() << "GLWidget::initializeGL";
  
    qglClearColor ( d->background );
  
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
    glGetDoublev( GL_MODELVIEW_MATRIX, d->rotationMatrix);
    glPopMatrix();
  
    d->translationVector[0] = 0.0;
    d->translationVector[1] = 0.0;
    d->translationVector[2] = 0.0;
  
    d->scale = 1.0;
  
    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_NORMAL_ARRAY );
  }
  
  void GLWidget::resizeGL(int width, int height)
  {
    qDebug() << "GLWidget::resizeGL";
    glViewport(0,0,width,height);
  
  }
  
  void GLWidget::setCamera() const
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
    glMultMatrixd( d->rotationMatrix );
    glGetDoublev( GL_MODELVIEW_MATRIX, d->rotationMatrix );
    glPopMatrix();
  }
  
  void GLWidget::translate(float x, float y, float z)
  {
    qDebug() << "GLWidget::translate";
    d->translationVector[0] = d->translationVector[0] + x;
    d->translationVector[1] = d->translationVector[1] + y;
    d->translationVector[2] = d->translationVector[2] + z;
  }
  
  void GLWidget::setBackground(const QColor &background) { 
    d->background = background; qglClearColor(background);
  }
  
  QColor GLWidget::background() const {
    return d->background;
  }

  void GLWidget::setScale(float s)
  {
    d->scale = s;
  }
  
  float GLWidget::scale() const
  {
    return d->scale;
  }
  
  void GLWidget::render(GLenum mode) const
  {
    //qDebug() << "GLWidget::render";
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
    // TODO: Be careful here.  For the time being we are actually changing the
    // orientation of our render in 3d space and changing the camera.  And also
    // we're not changing the coordinates of the atoms.
  
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
  
    // Translate our molecule as per user instructions
    glScaled(d->scale, d->scale, d->scale);
    glMultMatrixd(d->rotationMatrix);
    glTranslated(d->translationVector[0], d->translationVector[1], d->translationVector[2]);
  
    if(d->defaultEngine)
      d->defaultEngine->render(&(d->defaultQueue));
  
    for(int i=0; i<d->displayLists.size(); i++) {
      qDebug() << "Calling DL: " << d->displayLists[i] << endl;
      glCallList(d->displayLists[i]);
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
    d->displayLists.append(dl);
  }
  
  void GLWidget::removeDL(GLuint dl)
  {
    d->displayLists.removeAll(dl);
  }
  
  void GLWidget::setMolecule(Molecule *molecule)
  {
    if(!molecule)
      return;
  
    // disconnect from our old molecule
    if(d->molecule)
      QObject::disconnect(d->molecule, 0, this, 0);
  
    d->molecule = molecule;
  
    // clear our engine queues
    for( int i=0; i < d->queues.size(); i++ ) {
      d->queues[i].clear();
    }
  
    qDebug() << d->defaultQueue.size();
    d->defaultQueue.clear();
    qDebug() << d->defaultQueue.size();
  
    // add the atoms to the default queue
    std::vector<OpenBabel::OBNodeBase*>::iterator i;
    for(Atom *atom = (Atom*)d->molecule->BeginAtom(i); 
        atom; atom = (Atom*)d->molecule->NextAtom(i))
    {
      d->defaultQueue.addPrimitive(atom);
    }
  
    // add the bonds to the default queue
    std::vector<OpenBabel::OBEdgeBase*>::iterator j;
    for(Bond *bond = (Bond*)d->molecule->BeginBond(j); 
        bond; bond = (Bond*)d->molecule->NextBond(j))
    {
      d->defaultQueue.addPrimitive(bond);
    }
  
    // add the residues to the default queue
    std::vector<OpenBabel::OBResidue*>::iterator k;
    for(Residue *residue = (Residue*)d->molecule->BeginResidue(k); 
        residue; residue = (Residue *)d->molecule->NextResidue(k))
    {
      d->defaultQueue.addPrimitive(residue);
    }
  
    // add the molecule to the default queue
    d->defaultQueue.addPrimitive(d->molecule);
  
    // connect our signals so if the molecule gets updated
    connect(d->molecule, SIGNAL(primitiveAdded(Primitive*)), 
        this, SLOT(addPrimitive(Primitive*)));
    connect(d->molecule, SIGNAL(primitiveUpdated(Primitive*)), 
        this, SLOT(updatePrimitive(Primitive*)));
    connect(d->molecule, SIGNAL(primitiveRemoved(Primitive*)), 
        this, SLOT(removePrimitive(Primitive*)));
    connect(d->molecule, SIGNAL(updated(Primitive*)), this, SLOT(updateModel()));
  
    updateGL();
  }
  
  inline
  const Molecule* GLWidget::molecule() const
  {
    return d->molecule;
  }
  
  Engine * GLWidget::defaultEngine() const
  {
    return d->defaultEngine;
  }
  
  QList<Engine *> GLWidget::engines() const
  {
    return d->engines;
  }

  void GLWidget::updateModel()
  {
    updateGL();
  }
  
  void GLWidget::addPrimitive(Primitive *primitive)
  {
    qDebug() << "GLWidget::addPrimitive";
    if(primitive)
      d->defaultQueue.addPrimitive(primitive);
  }
  
  void GLWidget::updatePrimitive(Primitive *primitive)
  {
    updateGL();
  }
  
  void GLWidget::removePrimitive(Primitive *primitive)
  {
    qDebug() << "GLWidget::removePrimitive";
    // clear our engine queues
    for( int i=0; i < d->queues.size(); i++ ) {
      d->queues[i].removePrimitive(primitive);
    }
  
    d->defaultQueue.removePrimitive(primitive);
  
    updateGL();
  }
  
  void GLWidget::setDefaultEngine(int i) 
  {
    //qDebug() << "GLWidget::setDefaultEngine";
    setDefaultEngine(d->engines.at(i));
  }
  
  void GLWidget::setDefaultEngine(Engine *engine) 
  {
    //qDebug() << "GLWidget::setDefaultEngine";
    if(engine)
    {
      d->defaultEngine = engine;
      updateGL();
    }
  }
  
  
  void GLWidget::loadEngines()
  {
    QStringList pluginPaths;
    pluginPaths << "/usr/lib/avogadro" << "/usr/local/lib/avogadro";

    if(getenv("AVOGADRO_ENGINES") != NULL)
    {
      pluginPaths += QString(getenv("AVOGADRO_ENGINES")).split(':');
    }

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
  
    foreach (QString path, pluginPaths)
    {
      QDir dir(path); 
      qDebug() << "SearchPath:" << dir.absolutePath() << endl;
      foreach (QString fileName, dir.entryList(QDir::Files)) {
        QPluginLoader loader(dir.absoluteFilePath(fileName));
        //       qDebug() << "File: " << fileName;
        EngineFactory *factory = qobject_cast<EngineFactory *>(loader.instance());
        if (factory) {
          Engine *engine = factory->createInstance();
          qDebug() << "Found Engine: " << engine->name() << " - " << engine->description(); 
          if (!d->defaultEngine)
          {
            qDebug() << "Setting Default Engine: " << engine->name() << " - " << engine->description(); 
            d->defaultEngine = engine;
          }
          d->engines.append(engine);
        }
      }
    }
  }
  
  #define GL_SEL_BUF_SIZE 512
  
  QList<GLHit> GLWidget::hits(int x, int y, int w, int h) const
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
          //X       if (j == 0)  /*  set row and column  */
          //X         ii = *ptr;
          //X       else if (j == 1)
          //X         jj = *ptr;
          //X       ptr++;
        }
        if (name)
        {
//dc:           printf ("%d(%d) ", name,type);
          hits.append(GLHit(name,type,minZ,maxZ));
        }
      }
//dc:       printf ("\n");
      qSort(hits);
    }
  
    return(hits);
  }

  QSize GLWidget::sizeHint() const
  {
    return minimumSizeHint();
  }

  QSize GLWidget::minimumSizeHint() const
  {
    return QSize(200,200);
  }


}
