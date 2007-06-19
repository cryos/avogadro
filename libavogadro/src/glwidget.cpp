/**********************************************************************
  GLWidget - general OpenGL display

  Copyright (C) 2006,2007 Geoffrey R. Hutchison
  Copyright (C) 2006,2007 Donald Ephraim Curtis
  Copyright (C) 2007      Benoit Jacob

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  Avogadro is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Avogadro is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#include <config.h>

#include <avogadro/glwidget.h>
#include <avogadro/camera.h>
#include <avogadro/toolgroup.h>
#include <avogadro/primitivelist.h>

#include <openbabel/generic.h>

#include <QDir>
#include <QMouseEvent>
#include <QPluginLoader>
#include <QUndoStack>

#include <stdio.h>
#include <vector>

using namespace OpenBabel;
namespace Avogadro {

  bool engineLessThan(const Engine* lhs, const Engine* rhs)
  {
    Engine::EngineFlags lhsFlags = lhs->flags();
    Engine::EngineFlags rhsFlags = rhs->flags();

    if(!(lhsFlags & Engine::Overlay) && rhsFlags & Engine::Overlay)
    {
      return true;
    }
    else if (lhsFlags & Engine::Overlay && rhsFlags & Engine::Overlay)
    {
      return lhs->transparencyDepth() < rhs->transparencyDepth();
    }
    else if (lhsFlags & Engine::Overlay && !(rhsFlags & Engine::Overlay))
    {
      return false;
    }
    else if(!(lhsFlags & Engine::Molecules) && rhsFlags & Engine::Molecules)
    {
      return true;
    }
    else if (lhsFlags & Engine::Molecules && rhsFlags & Engine::Molecules)
    {
      return lhs->transparencyDepth() < rhs->transparencyDepth();
    }
    else if (lhsFlags & Engine::Molecules && !(rhsFlags & Engine::Molecules))
    {
      return false;
    }
    else if(!(lhsFlags & Engine::Atoms) && rhsFlags & Engine::Atoms)
    {
      return true;
    }
    else if (lhsFlags & Engine::Atoms && rhsFlags & Engine::Atoms)
    {
      return lhs->transparencyDepth() < rhs->transparencyDepth();
    }
    else if (lhsFlags & Engine::Atoms && !(rhsFlags & Engine::Atoms))
    {
      return false;
    }
    else if(!(lhsFlags & Engine::Bonds) && rhsFlags & Engine::Bonds)
    {
      return true;
    }
    else if (lhsFlags & Engine::Bonds && rhsFlags & Engine::Bonds)
    {
      return lhs->transparencyDepth() < rhs->transparencyDepth();
    }
    else if (lhsFlags & Engine::Bonds && !(rhsFlags & Engine::Bonds))
    {
      return false;
    }
    return false;
  }

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
    qDebug() << "SortHit";
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
      GLWidgetPrivate() : background(Qt::black),
                          aCells(1), bCells(1), cCells(1),
                          molecule(0),
                          camera(new Camera),
                          tool(0),
                          toolGroup(0),
                          selectBuf(0),
                          selectBufSize(-1),
                          sharedPainter(false),
                          painter(new Painter)
                          {}

      ~GLWidgetPrivate()
      {
        if(!sharedPainter) {
          delete painter;
        }
        if(selectBuf) delete[] selectBuf;
        delete camera;
      }

      QList<Engine *>        engines;

      QList<GLuint>          displayLists;

      QColor                 background;

      Eigen::Vector3d        normalVector;
      Eigen::Vector3d        center;
      double                 radius;
      const Atom             *farthestAtom;

      //! number of unit cells in a, b, and c crystal directions
      unsigned char          aCells;
      unsigned char          bCells;
      unsigned char          cCells;

      Molecule               *molecule;

      Camera                 *camera;

      Tool                   *tool;
      ToolGroup              *toolGroup;

      GLuint                 *selectBuf;
      int                    selectBufSize;

      PrimitiveList          selectedPrimitives;
      PrimitiveList          primitives;

      QUndoStack             *undoStack;

      bool                   stable;

      bool                   sharedPainter;
      Painter                *painter;
  };


  GLWidget::GLWidget(QWidget *parent )
  : QGLWidget(parent), d(new GLWidgetPrivate)
  {
    constructor();
  }

  GLWidget::GLWidget(const QGLFormat &format, QWidget *parent,
      const QGLWidget *shareWidget )
  : QGLWidget(format, parent, shareWidget), d(new GLWidgetPrivate)
  {
    constructor();
  }

  GLWidget::GLWidget(Molecule *molecule,
      const QGLFormat &format, QWidget *parent,
      const QGLWidget *shareWidget )
  : QGLWidget(format, parent, shareWidget), d(new GLWidgetPrivate)
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
    d->camera->setParent(this);
    qDebug() << "Sorting";
  }

  void GLWidget::initializeGL()
  {
    qglClearColor ( d->background );

    glShadeModel( GL_SMOOTH );
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );
    glEnable( GL_CULL_FACE );
    glEnable( GL_COLOR_SUM_EXT );

    // Used to display semi-transparent selection rectangle
    //  glBlendFunc(GL_ONE, GL_ONE);
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glEnable( GL_NORMALIZE );

    GLfloat ambientLight[] = { 0.2, 0.2, 0.2, 1.0 };
    GLfloat diffuseLight[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat specularLight[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat position[] = { 0.8, 0.7, 1.0, 0.0 };

    glLightModeli( GL_LIGHT_MODEL_COLOR_CONTROL_EXT,
        GL_SEPARATE_SPECULAR_COLOR_EXT );

    // Due to the bug found with Mesa 6.5.3 in the Radeon DRI driver
    // in radeon_state.c in radeonUpdateSpecular(),
    // it is important to set GL_SEPARATE_SPECULAR_COLOR_EXT
    // _before_ enabling lighting
    glEnable( GL_LIGHTING );

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
    OBUnitCell *uc = NULL;
    std::vector<vector3> cellVectors;
    vector3 offset;

    if (d->molecule && d->molecule->HasData(OBGenericDataType::UnitCell))
      uc = dynamic_cast<OBUnitCell*>(d->molecule->GetData(OBGenericDataType::UnitCell));

    if (!uc) { // a plain molecule, no crystal cell
      foreach(Engine *engine, d->engines)
        {
          if(engine->isEnabled()) {
            engine->renderOpaque(this);
          }
        }
        foreach(Engine *engine, d->engines)
        {
          if(engine->isEnabled() && engine->flags() & Engine::Transparent) {
            engine->renderTransparent(this);
          }
        }
    } else { // render a crystal
      cellVectors = uc->GetCellVectors();

      // render opaque parts of crystal
      for (int a = 0; a < d->aCells; a++) {
        for (int b = 0; b < d->bCells; b++)  {
          for (int c = 0; c < d->cCells; c++) {
            foreach(Engine *engine, d->engines)
            {
              if(engine->isEnabled()) {
                engine->renderOpaque(this);
              }
            } // end rendering loop
            glTranslatef(cellVectors[2].x(), cellVectors[2].y(), cellVectors[2].z());
          } // end c
          glTranslatef(cellVectors[2].x() * -d->cCells,
                       cellVectors[2].y() * -d->cCells,
                       cellVectors[2].z() * -d->cCells);
          glTranslatef(cellVectors[1].x(), cellVectors[1].y(), cellVectors[1].z());
        } // end b
        glTranslatef(cellVectors[1].x() * -d->bCells,
                     cellVectors[1].y() * -d->bCells,
                     cellVectors[1].z() * -d->bCells);
        glTranslatef(cellVectors[0].x(), cellVectors[0].y(), cellVectors[0].z());
      } // end a

      // render transparent parts of crystal
      for (int a = 0; a < d->aCells; a++) {
        for (int b = 0; b < d->bCells; b++)  {
          for (int c = 0; c < d->cCells; c++) {
            foreach(Engine *engine, d->engines)
            {
              if(engine->isEnabled() && engine->flags() & Engine::Transparent) {
                engine->renderTransparent(this);
              }
            }
            glTranslatef(cellVectors[2].x(), cellVectors[2].y(), cellVectors[2].z());
          } // end c
          glTranslatef(cellVectors[2].x() * -d->cCells,
                      cellVectors[2].y() * -d->cCells,
                                        cellVectors[2].z() * -d->cCells);
          glTranslatef(cellVectors[1].x(), cellVectors[1].y(), cellVectors[1].z());
        } // end b
        glTranslatef(cellVectors[1].x() * -d->bCells,
                    cellVectors[1].y() * -d->bCells,
                                      cellVectors[1].z() * -d->bCells);
        glTranslatef(cellVectors[0].x(), cellVectors[0].y(), cellVectors[0].z());
      } // end a
    } // end rendering crystal

    foreach(GLuint dl, d->displayLists)
      glCallList(dl);

    if(d->toolGroup) {
      QList<Tool *> tools = d->toolGroup->tools();
      foreach(Tool *tool, tools)
      {
        if(tool != d->tool) {
          tool->paint(this);
        }
      }
    }

    if(d->tool) {
      d->tool->paint(this);
    }

    // shouldn't we have this disabled?
    // glFlush();

  }

  void GLWidget::glDraw()
  {
    QGLWidget::glDraw();
  }

  void GLWidget::paintGL()
  {
    assert(QGLContext::currentContext() == context());

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // setup the OpenGL projection matrix using the camera
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    d->camera->applyPerspective();

    // setup the OpenGL modelview matrix using the camera
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    d->camera->applyModelview();

    render();
  }

  void GLWidget::mousePressEvent( QMouseEvent * event )
  {
    if(d->tool) {
      QUndoCommand *command = 0;
      command = d->tool->mousePress(this, event);

      if(command && d->undoStack) {
        d->undoStack->push(command);
      } else if (command) {
        delete command;
      }
    }
  }

  void GLWidget::mouseReleaseEvent( QMouseEvent * event )
  {
    if(d->tool) {
      QUndoCommand *command = d->tool->mouseRelease(this, event);

      if(command && d->undoStack) {
        d->undoStack->push(command);
      }
    }
  }

  void GLWidget::mouseMoveEvent( QMouseEvent * event )
  {
    if(d->tool) {
      QUndoCommand *command = d->tool->mouseMove(this, event);
      if(command && d->undoStack) {
        d->undoStack->push(command);
      }
    }
  }

  void GLWidget::wheelEvent( QWheelEvent * event )
  {
    if(d->tool) {
      QUndoCommand *command = d->tool->wheel(this, event);
      if(command && d->undoStack) {
        d->undoStack->push(command);
      }
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
    if(!molecule) { return; }

    // disconnect from our old molecule
    if(d->molecule) {
      QObject::disconnect(d->molecule, 0, this, 0);
    }

    d->molecule = molecule;

    // clear our engine queues
    for( int i=0; i < d->engines.size(); i++ ) {
      d->engines.at(i)->clearPrimitives();
    }
    d->primitives.clear();

    // add the atoms to the default queue
    std::vector<OpenBabel::OBNodeBase*>::iterator i;
    for(Atom *atom = (Atom*)d->molecule->BeginAtom(i);
        atom; atom = (Atom*)d->molecule->NextAtom(i))
    {
      for( int i=0; i < d->engines.size(); i++ ) {
        d->engines.at(i)->addPrimitive(atom);
      }
      d->primitives.append(atom);
    }

    // add the bonds to the default queue
    std::vector<OpenBabel::OBEdgeBase*>::iterator j;
    for(Bond *bond = (Bond*)d->molecule->BeginBond(j);
        bond; bond = (Bond*)d->molecule->NextBond(j))
    {
      for( int i=0; i < d->engines.size(); i++ ) {
        d->engines.at(i)->addPrimitive(bond);
      }
      d->primitives.append(bond);
    }

    // add the residues to the default queue
    std::vector<OpenBabel::OBResidue*>::iterator k;
    for(Residue *residue = (Residue*)d->molecule->BeginResidue(k);
        residue; residue = (Residue *)d->molecule->NextResidue(k))
    {
      for( int i=0; i < d->engines.size(); i++ ) {
        d->engines.at(i)->addPrimitive(residue);
      }
      d->primitives.append(residue);
    }

    // add the molecule to the default queue
    for( int i=0; i < d->engines.size(); i++ ) {
      d->engines.at(i)->addPrimitive(d->molecule);
    }
    d->primitives.append(d->molecule);

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
    d->camera->initializeViewPoint();

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
    // Needs to handle unit cells PR#1739844
    
    OBUnitCell *uc = NULL;

    if (d->molecule && d->molecule->HasData(OBGenericDataType::UnitCell))
      uc = dynamic_cast<OBUnitCell*>(d->molecule->GetData(OBGenericDataType::UnitCell));

    if (!uc) { // a plain molecule, no crystal cell
      d->center = d->molecule->center();
      d->normalVector = d->molecule->normalVector();
      d->radius = d->molecule->radius();
      d->farthestAtom = d->molecule->farthestAtom();
    } else { 
      // render a crystal (so most geometry comes from the cell vectors)
      // Origin at 0.0, 0.0, 0.0
      // a = <x0, y0, z0>
      // b = <x1, y1, z1>
      // c = <x2, y2, z2>
      std::vector<vector3> cellVectors = uc->GetCellVectors();
      Eigen::Vector3d a, b, c;
      a = *reinterpret_cast<Eigen::Vector3d *>(&cellVectors[0]);
      a *= d->aCells;
      b = *reinterpret_cast<Eigen::Vector3d *>(&cellVectors[1]);
      b *= d->bCells;
      c = *reinterpret_cast<Eigen::Vector3d *>(&cellVectors[2]);
      c *= d->cCells;

      d->center = (a + b + c) / 2.0;
      // Radius should be the magnitude of the center vector
      d->radius = sqrt(d->center.x() * d->center.x() + 
                       d->center.y() * d->center.y() +
                       d->center.z() * d->center.z());
      // GH: Not sure about these
      d->normalVector = d->molecule->normalVector();
      d->farthestAtom = d->molecule->farthestAtom();
    }
  }

  Camera * GLWidget::camera() const
  {
    return d->camera;
  }

  QList<Engine *> GLWidget::engines() const
  {
    return d->engines;
  }

  PrimitiveList GLWidget::primitives() const
  {
    return d->primitives;
  }

  void GLWidget::addPrimitive(Primitive *primitive)
  {
    if(primitive) {
      // add the molecule to the default queue
      for( int i=0; i < d->engines.size(); i++ ) {
        d->engines.at(i)->addPrimitive(primitive);
      }
      d->primitives.append(primitive);
    }
  }

  void GLWidget::updatePrimitive(Primitive *primitive)
  {
    for( int i=0; i< d->engines.size(); i++) {
      d->engines.at(i)->updatePrimitive(primitive);
    }
    updateGeometry();
  }

  void GLWidget::removePrimitive(Primitive *primitive)
  {
    if(primitive) {
      // add the molecule to the default queue
      for( int i=0; i < d->engines.size(); i++ ) {
        d->engines.at(i)->removePrimitive(primitive);
      }
      d->selectedPrimitives.removeAll(primitive);
      d->primitives.removeAll(primitive);
    }
  }

  void GLWidget::loadEngines()
  {
    QString prefixPath = QString(INSTALL_PREFIX) + "/lib/avogadro/engines";
    QStringList pluginPaths;
    pluginPaths << prefixPath;

#ifdef WIN32
	pluginPaths << "./engines";
#endif

    if(getenv("AVOGADRO_ENGINES") != NULL)
    {
      pluginPaths = QString(getenv("AVOGADRO_ENGINES")).split(':');
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
          connect(engine, SIGNAL(changed()), this, SLOT(update()));
          // FIXME: below is a ugly hack so that the text-painting engines are
          // at the END of the engines list, so that text is painted last.
          if(engine->name() == "Dynamic Ball and Stick") {
            engine->setEnabled(true);
          }
          d->engines.append(engine);
        }
      }
    }
    qSort(d->engines.begin(), d->engines.end(), engineLessThan);
    foreach(Engine* e, d->engines)
    {
      qDebug() << e->name();
    }
  }

  void GLWidget::setTool(Tool *tool)
  {
    if(tool) {
      d->tool = tool;
    }
  }

  void GLWidget::setPainter(Painter *painter)
  {
    Painter *old = d->painter;
    d->painter = painter;

    if(!d->sharedPainter) {
      delete old;
    }

    d->sharedPainter = true;
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


  void GLWidget::setUndoStack(QUndoStack *undoStack)
  {
    d->undoStack = undoStack;
  }

  QUndoStack *GLWidget::undoStack() const
  {
    return d->undoStack;
  }

  Tool* GLWidget::tool() const
  {
    return d->tool;
  }

  ToolGroup *GLWidget::toolGroup() const
  {
    return d->toolGroup;
  }

  Painter *GLWidget::painter() const
  {
    return d->painter;
  }

  QList<GLHit> GLWidget::hits(int x, int y, int w, int h)
  {
    QList<GLHit> hits;

    if(!molecule()) return hits;

    GLint viewport[4];
    unsigned int hit_count;

    int cx = w/2 + x;
    int cy = h/2 + y;

    // setup the selection buffer
    int requiredSelectBufSize = (d->molecule->NumAtoms() + d->molecule->NumBonds()) * 8;
    if( requiredSelectBufSize > d->selectBufSize )
    {
      //resize selection buffer
      if(d->selectBuf) delete[] d->selectBuf;
      // add some margin so that resizing doesn't occur every time an atom is added
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
    d->camera->applyPerspective();

    // now load the modelview matrix from the camera
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    d->camera->applyModelview();

    // now actually render
    render();

    // returning to normal rendering mode
    hit_count = glRenderMode(GL_RENDER);

    // if no error occurred and there are hits, process them
    if (hit_count > 0)
    {
      unsigned int i, j;
      GLuint names, type, *ptr;
      GLuint minZ, maxZ, name;

      //X   printf ("hits = %d\n", hits);
      ptr = (GLuint *) d->selectBuf;
      // for all hits and not past end of buffer
      for (i = 0; i < hit_count && !(ptr > d->selectBuf + d->selectBufSize); i++) {
        names = *ptr++;
        // make sure that we won't be passing the end of bufer
        if( ptr + names + 2 > d->selectBuf + d->selectBufSize )
        {
          break;
        }
        minZ = *ptr++;
        maxZ = *ptr++;
        name = 0;
        for (j = 0; j < names/2; j++) { /*  for each name */
          type = *ptr++;
          name = *ptr++;
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

  bool GLWidget::isStable() const
  {
    return d->stable;
  }

  void GLWidget::setStable(bool stable)
  {
    d->stable = stable;
  }

  void GLWidget::setSelected(QList<Primitive *> primitives, bool select)
  {
    foreach (Primitive *item, primitives) {
      if (select) {
        if (!d->selectedPrimitives.contains(item)) {
          d->selectedPrimitives.append(item);
        }
      } else {
        d->selectedPrimitives.removeAll(item);
      }
      item->update();
    }
  }

  QList<Primitive *> GLWidget::selectedPrimitives() const
  {
    return d->selectedPrimitives.list();
  }

void GLWidget::toggleSelected(QList<Primitive*> primitives)
  {
    foreach (Primitive *item, primitives) {
      if (d->selectedPrimitives.contains(item))
        d->selectedPrimitives.removeAll(item);
      else
        d->selectedPrimitives.append(item);
    }
  }

  void GLWidget::clearSelection()
  {
    d->selectedPrimitives.clear();
  }

  bool GLWidget::isSelected(const Primitive *p)
  {
    // Return true if the item is selected
    return d->selectedPrimitives.contains(const_cast<Primitive *>(p));
  }

  void GLWidget::setUnitCells(int a, int b, int c)
  {
    d->aCells = a;
    d->bCells = b;
    d->cCells = c;
  }

 int GLWidget::aCells()
 {
    return d->aCells;
 }

 int GLWidget::bCells()
 {
    return d->bCells;
 }

 int GLWidget::cCells()
 {
    return d->cCells;
 }

}

#include "glwidget.moc"
