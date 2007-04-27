/**********************************************************************
  GLWidget - general OpenGL display

  Copyright (C) 2006 by Geoffrey R. Hutchison
  Copyright (C) 2006,2007 by Donald Ephraim Curtis

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
#include <avogadro/camera.h>
#include <avogadro/toolgroup.h>

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

  GLHit::GLHit(GLuint type, GLuint name, GLuint minZ, GLuint maxZ) : d(new GLHitPrivate) { 
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
    return *this;
  }

  GLHit::~GLHit() {
    delete d;
  }

  bool GLHit::operator<(const GLHit &other) const { 
    GLHitPrivate *e = other.d;
    return d->minZ < e->minZ; 
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
      GLWidgetPrivate() : background(Qt::black), molecule(0),
                          tool(0), toolGroup(0), selectBuf(0) {}
      ~GLWidgetPrivate()
      {
        if(selectBuf) delete[] selectBuf;
      }
      
      QList<Engine *>        engines;
    
      Molecule               *molecule;
      QList<GLuint>          displayLists;

      Eigen::Vector3d        normalVector; 
      Eigen::Vector3d        center; 
      double                 radius;
      const Atom             *farthestAtom;

      Tool                   *tool;
      ToolGroup            *toolGroup;
    
      //TODO: convert to pointer
      Camera                 camera;
      QColor                 background;

      int                    selectBufSize;
      GLuint                 *selectBuf;
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

  GLWidget::~GLWidget()
  {
    foreach(Engine *engine, d->engines) {
      delete engine;
    }
    delete(d);
  }
  
  void GLWidget::constructor()
  {
    setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
    loadEngines();
    d->camera.setParent(this);
  }
  
  void GLWidget::initializeGL()
  {
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
  }
  
  void GLWidget::resizeGL(int width, int height)
  {
    glViewport(0, 0, width, height);
  }
  
  void GLWidget::setBackground(const QColor &background) { 
    d->background = background; qglClearColor(background);
  }
  
  QColor GLWidget::background() const {
    return d->background;
  }

  void GLWidget::render()
  {
    int size = 0;
    
    size = d->engines.size();
    for(int i=0; i<size; i++)
    {
      Engine *engine = d->engines.at(i);
      if(engine->isEnabled()) {
        engine->render(this);
      }
    }

    size = d->displayLists.size();
    for(int i=0; i<size; i++) {
      glCallList(d->displayLists.at(i));
    }
  
    glFlush();
  }

  void GLWidget::glDraw()
  {
    if(QGLContext::currentContext() != context())
    {
      return;
    }

    QGLWidget::glDraw();
  }

  void GLWidget::paintGL()
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // setup the OpenGL projection matrix using the camera
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    d->camera.applyPerspective();

    // setup the OpenGL modelview matrix using the camera
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    d->camera.applyModelview();

    render();
  }
  
  void GLWidget::mousePressEvent( QMouseEvent * event )
  {
    if(d->tool) {
      d->tool->mousePress(this, event);
    }
  }
  
  void GLWidget::mouseReleaseEvent( QMouseEvent * event )
  {
    if(d->tool) {
      d->tool->mouseRelease(this, event);
    }
  }
  
  void GLWidget::mouseMoveEvent( QMouseEvent * event )
  {
    if(d->tool) {
      d->tool->mouseMove(this, event);
    }
  }
  
  void GLWidget::wheelEvent( QWheelEvent * event )
  {
    if(d->tool) {
      d->tool->wheel(this, event);
    }
  }
  
  void GLWidget::addDL(GLuint dl)
  {
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
    for( int i=0; i < d->engines.size(); i++ ) {
      d->engines.at(i)->clearQueue();
    }
  
    // add the atoms to the default queue
    std::vector<OpenBabel::OBNodeBase*>::iterator i;
    for(Atom *atom = (Atom*)d->molecule->BeginAtom(i); 
        atom; atom = (Atom*)d->molecule->NextAtom(i))
    {
      for( int i=0; i < d->engines.size(); i++ ) {
        d->engines.at(i)->addPrimitive(atom);
      }
    }
  
    // add the bonds to the default queue
    std::vector<OpenBabel::OBEdgeBase*>::iterator j;
    for(Bond *bond = (Bond*)d->molecule->BeginBond(j); 
        bond; bond = (Bond*)d->molecule->NextBond(j))
    {
      for( int i=0; i < d->engines.size(); i++ ) {
        d->engines.at(i)->addPrimitive(bond);
      }
    }
  
    // add the residues to the default queue
    std::vector<OpenBabel::OBResidue*>::iterator k;
    for(Residue *residue = (Residue*)d->molecule->BeginResidue(k); 
        residue; residue = (Residue *)d->molecule->NextResidue(k))
    {
      for( int i=0; i < d->engines.size(); i++ ) {
        d->engines.at(i)->addPrimitive(residue);
      }
    }
  
    // add the molecule to the default queue
    for( int i=0; i < d->engines.size(); i++ ) {
      d->engines.at(i)->addPrimitive(d->molecule);
    }
  
    // connect our signals so if the molecule gets updated
    connect(d->molecule, SIGNAL(primitiveAdded(Primitive*)), 
        this, SLOT(addPrimitive(Primitive*)));
    connect(d->molecule, SIGNAL(primitiveUpdated(Primitive*)), 
        this, SLOT(updatePrimitive(Primitive*)));
    connect(d->molecule, SIGNAL(primitiveRemoved(Primitive*)), 
        this, SLOT(removePrimitive(Primitive*)));

    // compute the molecule's geometric info
    updateGeometry();

    // setup the camera to have a nice viewpoint on the molecule
    d->camera.initializeViewPoint();

    update();
  }
  
  const Molecule* GLWidget::molecule() const
  {
    return d->molecule;
  }

  Molecule* GLWidget::molecule()
  {
    return d->molecule;
  }

  const Eigen::Vector3d & GLWidget::center() const
  {
    return d->center;
  }

  const Eigen::Vector3d & GLWidget::normalVector() const
  {
    return d->normalVector;
  }

  const double & GLWidget::radius() const
  {
    return d->radius;
  }

  const Atom *GLWidget::farthestAtom() const
  {
    return d->farthestAtom;
  }

  void GLWidget::updateGeometry()
  {
    d->center = d->molecule->center();
    d->normalVector = d->molecule->normalVector();
    d->radius = d->molecule->radius();
    d->farthestAtom = d->molecule->farthestAtom();
  }

  Camera & GLWidget::camera() const
  {
    return d->camera;
  }
  
  QList<Engine *> GLWidget::engines() const
  {
    return d->engines;
  }

  void GLWidget::addPrimitive(Primitive *primitive)
  {
    if(primitive) {
      // add the molecule to the default queue
      for( int i=0; i < d->engines.size(); i++ ) {
        d->engines.at(i)->addPrimitive(primitive);
      }
    }
  }
  
  void GLWidget::updatePrimitive(Primitive *primitive)
  {
    for( int i=0; i< d->engines.size(); i++) {
      d->engines.at(i)->updatePrimitive(primitive);
    }

    update();
  }
  
  void GLWidget::removePrimitive(Primitive *primitive)
  {
    if(primitive) {
      // add the molecule to the default queue
      for( int i=0; i < d->engines.size(); i++ ) {
        d->engines.at(i)->removePrimitive(primitive);
      }
    }
  
    update();
  }
  
  void GLWidget::loadEngines()
  {
    QStringList pluginPaths;
    pluginPaths << "/usr/lib/avogadro" << "/usr/local/lib/avogadro";

#ifdef WIN32
	pluginPaths << "./engines";
#endif

    if(getenv("AVOGADRO_ENGINES") != NULL)
    {
      pluginPaths += QString(getenv("AVOGADRO_ENGINES")).split(':');
    }

    // load static plugins first

    // now load plugins from paths
    foreach (QString path, pluginPaths)
    {
      QDir dir(path); 
      foreach (QString fileName, dir.entryList(QDir::Files)) {
        QPluginLoader loader(dir.absoluteFilePath(fileName));
        QObject *instance = loader.instance();
        EngineFactory *factory = qobject_cast<EngineFactory *>(instance);
        if (factory) {
          Engine *engine = factory->createInstance(this);
          qDebug() << "Found Engine: " << engine->name() << " - " << engine->description();
          
          d->engines.append(engine);
        }
      }
    }
  }

  void GLWidget::setTool(Tool *tool)
  {
    if(tool) {
      d->tool = tool;
    }
  }

  void GLWidget::setToolGroup(ToolGroup *toolGroup)
  {
    if(d->toolGroup) {
      disconnect(d->toolGroup, 0, this, 0);
    }

    if(toolGroup) {
      d->toolGroup = toolGroup;
      d->tool = toolGroup->activeTool();
      connect(toolGroup, SIGNAL(toolActivated(Tool*)),
          this, SLOT(setTool(Tool*)));
    }
  }

  Tool* GLWidget::tool() const
  {
    return d->tool;
  }

  ToolGroup *GLWidget::toolGroup() const
  {
    return d->toolGroup;
  }

  QList<GLHit> GLWidget::hits(int x, int y, int w, int h)
  {
    QList<GLHit> hits;
    GLint viewport[4];
    int hit_count;
  
    int cx = w/2 + x;
    int cy = h/2 + y;

    // setup the selection buffer
    int requiredSelectBufSize = d->molecule->NumAtoms() * 8;
    if( requiredSelectBufSize > d->selectBufSize )
    {
      //resize selection buffer
      if(d->selectBuf) delete[] d->selectBuf;
      // add some margin so that resizing doesn't occur everytime an atom is added
      d->selectBufSize = requiredSelectBufSize + SEL_BUF_MARGIN;
      if( d->selectBufSize > SEL_BUF_MAX_SIZE ) {
        d->selectBufSize = SEL_BUF_MAX_SIZE;
      }
      d->selectBuf = new GLuint[d->selectBufSize];
    }
  
    //X   hits.clear();

    glSelectBuffer(d->selectBufSize, d->selectBuf);
    glRenderMode(GL_SELECT);
    glInitNames();
  
    // Setup a projection matrix for picking in the zone delimited by (x,y,w,h).
    glGetIntegerv(GL_VIEWPORT, viewport);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluPickMatrix(cx,viewport[3]-cy, w, h,viewport);

    // now multiply that projection matrix with the perspective of the camera
    d->camera.applyPerspective();

    // now load the modelview matrix from the camera
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    d->camera.applyModelview();
  
    // now actually render
    render();
  
    // returning to normal rendering mode
    hit_count = glRenderMode(GL_RENDER);

    // if no error occured and there are hits, process them
    if (hit_count > 0)
    {
      unsigned int i, j;
      GLuint names, type, *ptr;
      GLuint minZ, maxZ, name;
  
      //X   printf ("hits = %d\n", hits);
      ptr = (GLuint *) d->selectBuf;
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
          hits.append(GLHit(type,name,minZ,maxZ));
        }
      }
//dc:       printf ("\n");
      qSort(hits);
    }
  
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
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

  Eigen::Vector3d GLWidget::unProject(const Eigen::Vector3d & v) const
  {
    GLdouble projection[16];
    glGetDoublev(GL_PROJECTION_MATRIX,projection);
    GLdouble modelview[16];
    glGetDoublev(GL_MODELVIEW_MATRIX,modelview);
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT,viewport);

    Eigen::Vector3d pos;
    gluUnProject(v.x(), viewport[3] - v.y(), v.z(),
                 modelview, projection, viewport, &pos.x(), &pos.y(), &pos.z());

    return pos;
  }

  Eigen::Vector3d GLWidget::unProject(const QPoint& p, const Eigen::Vector3d& ref) const
  {
    // project the reference point
    Eigen::Vector3d projected = project(ref);
  
    // Now unproject the pixel of coordinates (x,height-y) into a 3D point having the same Z-index
    // as the reference point.
    Eigen::Vector3d pos = unProject( Eigen::Vector3d( p.x(), p.y(), projected.z() ));

    return pos;
  }

  Eigen::Vector3d GLWidget::unProject(const QPoint& p) const
  {
    return unProject(p, center());
  }

  Eigen::Vector3d GLWidget::project(const Eigen::Vector3d & v) const
  {
    GLdouble projection[16];
    glGetDoublev(GL_PROJECTION_MATRIX,projection);
    GLdouble modelview[16];
    glGetDoublev(GL_MODELVIEW_MATRIX,modelview);
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT,viewport);

    Eigen::Vector3d pos;
    gluProject(v.x(), v.y(), v.z(),
               modelview, projection, viewport, &pos.x(), &pos.y(), &pos.z());

    return pos;
  }

}

#include "glwidget.moc"
