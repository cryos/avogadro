/**********************************************************************
  GLWidget - general OpenGL display

  Copyright (C) 2006-2009 Geoffrey R. Hutchison
  Copyright (C) 2006,2007 Donald Ephraim Curtis
  Copyright (C) 2007      Benoit Jacob
  Copyright (C) 2007-2009 Marcus D. Hanwell
  Copyright (C) 2011      David C. Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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

// krazy:excludeall=includes
#include "config.h"

#include "camera.h"
#include "glwidget.h"
#include "glpainter_p.h"
#include "glhit.h"

#include <QtGui/QMessageBox>
#include <QtGui/QPen>
#include <QtGui/QPainter>
#include <QtGui/QPaintEngine>
#include <QtGui/QUndoStack>

#ifdef ENABLE_PYTHON
  #include "pythonthread_p.h"
  #include "pythonextension_p.h"
#endif

#include <avogadro/painterdevice.h>
#include <avogadro/tool.h>
#include <avogadro/toolgroup.h>
#include <avogadro/extension.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/residue.h>
#include <avogadro/molecule.h>
#include <avogadro/color.h>

// Include static engine headers
#include "engines/bsdyengine.h"

#include "pluginmanager.h"

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QPluginLoader>
#include <QtCore/QPointer>
#include <QtCore/QReadWriteLock>
#include <QtCore/QTime>
#include <QtCore/QMutex>

#ifdef ENABLE_THREADED_GL
  #include <QtCore/QWaitCondition>
  #include <QtCore/QThread>
#endif

#ifdef ENABLE_GLSL
  #include <GL/glew.h>
#endif

#include <cstdio>
#include <vector>
#include <cstdlib>

#include <openbabel/mol.h>

using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro {

  bool engineLessThan( const Engine* lhs, const Engine* rhs )
  {
    Engine::Layers lhsLayers = lhs->layers();
    Engine::Layers rhsLayers = rhs->layers();
    Engine::PrimitiveTypes lhsPrimitives = lhs->primitiveTypes();
    Engine::PrimitiveTypes rhsPrimitives = rhs->primitiveTypes();

    if ( !( lhsLayers & Engine::Overlay ) && rhsLayers & Engine::Overlay ) {
      return true;
    } else if (( lhsLayers & Engine::Overlay ) && ( rhsLayers & Engine::Overlay ) ) {
      return lhs->transparencyDepth() < rhs->transparencyDepth();
    } else if (( lhsLayers & Engine::Overlay ) && !( rhsLayers & Engine::Overlay ) ) {
      return false;

    } else if ( !( lhsPrimitives & Engine::Molecules ) && rhsPrimitives & Engine::Molecules ) {
      return true;
    } else if (( lhsPrimitives & Engine::Molecules ) && ( rhsPrimitives & Engine::Molecules ) ) {
      return lhs->transparencyDepth() < rhs->transparencyDepth();
    } else if (( lhsPrimitives & Engine::Molecules ) && !( rhsPrimitives & Engine::Molecules ) ) {
      return false;

    } else if ( !( lhsPrimitives & Engine::Atoms ) && rhsPrimitives & Engine::Atoms ) {
      return true;
    } else if (( lhsPrimitives & Engine::Atoms ) && ( rhsPrimitives & Engine::Atoms ) ) {
      return lhs->transparencyDepth() < rhs->transparencyDepth();
    } else if (( lhsPrimitives & Engine::Atoms ) && !( rhsPrimitives & Engine::Atoms ) ) {
      return false;

    } else if ( !( lhsPrimitives & Engine::Bonds ) && rhsPrimitives & Engine::Bonds ) {
      return true;
    } else if (( lhsPrimitives & Engine::Bonds ) && ( rhsPrimitives & Engine::Bonds ) ) {
      return lhs->transparencyDepth() < rhs->transparencyDepth();
    } else if (( lhsPrimitives & Engine::Bonds ) && !( rhsPrimitives & Engine::Bonds ) ) {
      return false;
    }
    return false;
  }

  class GLPainterDevice : public PainterDevice
  {
  public:
    GLPainterDevice(GLWidget *gl) { widget = gl; }
    ~GLPainterDevice() {}

    Painter *painter() const { return widget->painter(); }
    Camera *camera() const { return widget->camera(); }
    bool isSelected( const Primitive *p ) const { return widget->isSelected(p); }
    double radius( const Primitive *p ) const { return widget->radius(p); }
    const Molecule *molecule() const { return widget->molecule(); }
    Color *colorMap() const { return widget->colorMap(); }

    int width() { return widget->width(); }
    int height() { return widget->height(); }

  private:
    GLWidget *widget;
  };

  class GLWidgetPrivate
  {
  public:
    GLWidgetPrivate() : background( 0,0,0,0 ),
                        aCells( 1 ), bCells( 1 ), cCells( 1 ),
                        cellColor( 255,255,255 ),
                        molecule( 0 ),
                        camera( new Camera ),
                        tool( 0 ),
                        toolGroup( 0 ),
                        selectBuf( 0 ),
                        selectBufSize( -1 ),
                        undoStack(0),
#ifdef ENABLE_THREADED_GL
                        thread( 0 ),
#else
                        initialized( false ),
#endif
                        painter( 0 ),
                        colorMap( 0),
                        defaultColorMap( 0),
                        updateCache(true),
                        quickRender(false),
                        allowQuickRender(true),
                        renderUnitCellAxes(false),
                        fogLevel(0),
                        renderAxes(false),
                        renderDebug(false),
                        dlistQuick(0), dlistOpaque(0), dlistTransparent(0),
                        pd(0)
    {
    }

    ~GLWidgetPrivate()
    {
      if ( selectBuf ) delete[] selectBuf;
      delete camera;

      // free the display lists
      if (dlistQuick)
        glDeleteLists(dlistQuick, 1);
      if (dlistOpaque)
        glDeleteLists(dlistOpaque, 1);
      if (dlistTransparent)
        glDeleteLists(dlistTransparent, 1);
    }

    void updateListQuick();

    QList<Engine *>        engines;

    QColor                 background;

    Vector3d               normalVector;
    Vector3d               center;
    double                 radius;
    const Atom            *farthestAtom;

    //! number of unit cells in a, b, and c crystal directions
    unsigned char          aCells;
    unsigned char          bCells;
    unsigned char          cCells;

    QColor                 cellColor;

    Molecule              *molecule;

    Camera                *camera;

    Tool                  *tool;
    ToolGroup             *toolGroup;
    QList<Extension*>     extensions;

    GLuint                *selectBuf;
    int                    selectBufSize;

    QList<QPair<QString, QPair<QList<unsigned int>, QList<unsigned int> > > > namedSelections;
    PrimitiveList          selectedPrimitives;

    QUndoStack            *undoStack;

#ifdef ENABLE_THREADED_GL
    QWaitCondition         paintCondition;
    QMutex                 renderMutex;

    GLThread              *thread;
#else
    bool                   initialized;
#endif

    GLPainter             *painter;
    Color                 *colorMap; // global color map
    Color                 *defaultColorMap;  // default fall-back coloring (i.e., by elements)
    bool                   updateCache; // Update engine caches in quick render?
    bool                   quickRender; // Are we using quick render?
    bool                   allowQuickRender; // Are we allowed to use quick render?
    bool                   renderUnitCellAxes; // Do we render the unit cell axes
    int                    fogLevel;    // The level of fog to use (0=none, 9=max)
    bool                   renderAxes;  // Should the x, y, z axes be rendered?
    bool                   renderDebug; // Should the debug information be shown?

    GLuint                 dlistQuick;
    GLuint                 dlistOpaque;
    GLuint                 dlistTransparent;

    QMutex                 textOverlayMutex; // Protects textOverlayLabels
    QList<QPointer<QLabel> > textOverlayLabels; // List of labels to render
    /**
      * Member GLPainterDevice which is passed to the engines.
      */
    GLPainterDevice *pd;
  };

  void GLWidgetPrivate::updateListQuick()
  {
    // Create a display list cache
    if (updateCache) {
//      qDebug() << "Making new quick display lists...";
      if (dlistQuick == 0) {
        dlistQuick = glGenLists(1);
      }

      // Don't use dynamic scaling when rendering quickly
      painter->setDynamicScaling(false);

      glNewList(dlistQuick, GL_COMPILE);
      foreach(Engine *engine, engines)
      {
        if(engine->isEnabled())
        {
          molecule->lock()->lockForRead();
          engine->renderQuick(pd);
          molecule->lock()->unlock();
        }
      }
      glEndList();

      updateCache = false;
      painter->setDynamicScaling(true);
    }
  }


#ifdef ENABLE_THREADED_GL
  class GLThread : public QThread
  {
  public:
    GLThread( GLWidget *widget, QObject *parent );

    void run();
    void resize( int width, int height );
    void stop();

  private:
    GLWidget *m_widget;
    QGLContext *m_context;
    bool m_running;
    bool m_resize;
    bool m_initialized;

    int m_width;
    int m_height;

  };

  GLThread::GLThread( GLWidget *widget, QObject *parent ) : QThread( parent ),
                                                            m_widget( widget ), m_running( true ), m_resize( false ), m_initialized( false )
  {}

  void GLThread::run()
  {
    GLWidgetPrivate *d = m_widget->d;

    while ( true ) {
      // lock the mutex
      d->renderMutex.lock();

      // unlock and wait
      d->paintCondition.wait( &( d->renderMutex ) );
      if ( !m_running ) {
        d->renderMutex.unlock();
        break;
      }
      m_widget->makeCurrent();

      if ( !m_initialized ) {
        m_widget->initializeGL();
        m_initialized = true;
      }

      if ( m_resize ) {
        m_widget->resizeGL( m_width, m_height );
        m_resize=false;
      }

      d->background.setAlphaF(0.0);
      m_widget->qglClearColor(d->background);
      m_widget->paintGL();
      m_widget->swapBuffers();
      m_widget->doneCurrent();
      d->renderMutex.unlock();
    }
  }

  void GLThread::resize( int width, int height )
  {
    m_resize = true;
    m_width = width;
    m_height = height;
  }

  void GLThread::stop()
  {
    m_running = false;
  }
#endif

  GLWidget::GLWidget( QWidget *parent )
    : QGLWidget( parent ), d( new GLWidgetPrivate )
  {
    constructor();
  }

  GLWidget::GLWidget( const QGLFormat &format, QWidget *parent,
                      const GLWidget *shareWidget )
    : QGLWidget( format, parent, shareWidget ), d( new GLWidgetPrivate )
  {
    constructor(shareWidget);
  }

  GLWidget::GLWidget( Molecule *molecule,
                      const QGLFormat &format, QWidget *parent,
                      const GLWidget *shareWidget )
    : QGLWidget( format, parent, shareWidget ), d( new GLWidgetPrivate )
  {
    constructor(shareWidget);
    setMolecule( molecule );
  }

  GLWidget::~GLWidget()
  {
    if(!d->painter->isShared())
      delete d->painter;
    else
      d->painter->decrementShare();

#ifdef ENABLE_THREADED_GL
    // cleanup our thread
    d->thread->stop();
    d->paintCondition.wakeAll();
    d->thread->wait();
#endif

#ifdef ENABLE_PYTHON
    // Creating the PythonThread object in Engine destructor doesn't seem
    // to work so we do it here
    PythonThread pt;
#endif

    // delete the engines
    foreach(Engine *engine, d->engines)
      delete engine;

    delete( d );
  }

  void GLWidget::constructor(const GLWidget *shareWidget)
  {
    // Make sure we get keyboard events
    setFocusPolicy(Qt::StrongFocus);

    // New PainterDevice
    d->pd = new GLPainterDevice(this);
    if(shareWidget && isSharing()) {
      // we are sharing contexts
      d->painter = static_cast<GLPainter *>(shareWidget->painter());
    }
    else
    {
      d->painter = new GLPainter();
    }
    d->painter->incrementShare();

    setAutoFillBackground( false );
    setSizePolicy( QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding );
    d->camera->setParent( this );
    setAutoBufferSwap( false );
    m_glslEnabled = false;
    m_navigateTool = 0;

#ifdef ENABLE_THREADED_GL
    qDebug() << "Threaded GL enabled.";
    d->thread = new GLThread( this, this );
    //doneCurrent();
    d->thread->start();
#endif
  }

  GLWidget *GLWidget::m_current = 0;

  GLWidget *GLWidget::current()
  {
    return m_current;
  }

  void GLWidget::setCurrent(GLWidget *current)
  {
    m_current = current;
  }

  void GLWidget::renderNow()
  {
    paintGL();
  }

  void GLWidget::initializeGL()
  {
    qDebug() << "GLWidget initialisation...";
    if(!context()->isValid())
    {
      // this should never happen, as we checked for availability of features that we requested in
      // the default OpenGL format. However it happened to a user who had a very broken setting with
      // a proprietary nvidia driver.
      const QString error_msg = tr("Invalid OpenGL context.\n"
                                   "Either something is completely broken in your OpenGL setup "
                                   "(can you run any OpenGL application?), "
                                   "or you found a bug.");
      qDebug() << error_msg;
      QMessageBox::critical(0, tr("OpenGL error"), error_msg);
      abort();
    }

    // Try to initialise GLEW if GLSL was enabled, test for OpenGL 2.0 support
    #ifdef ENABLE_GLSL
    GLenum err = glewInit();
    if (err != GLEW_OK) {
      qDebug() << "GLSL support enabled but GLEW could not initialise!";
      m_glslEnabled = false;
    }
    if (GLEW_VERSION_2_0) {
      qDebug() << "GLSL support enabled, OpenGL 2.0 support confirmed.";
      m_glslEnabled = true;
    }
    else if (GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader) {
      qDebug() << "GLSL support enabled, no OpenGL 2.0 support.";
      m_glslEnabled = true;
    }
    else {
      qDebug() << "GLSL support disabled, OpenGL 2.0 support not present.";
      m_glslEnabled = false;
    }
    #endif

    qglClearColor( d->background );

    glShadeModel( GL_SMOOTH );
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );
    glEnable( GL_CULL_FACE );
    glEnable( GL_COLOR_SUM_EXT );

    // Used to display semi-transparent selection rectangle
    //  glBlendFunc(GL_ONE, GL_ONE);
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glEnable( GL_NORMALIZE );

    glLightModeli( GL_LIGHT_MODEL_COLOR_CONTROL_EXT,
                   GL_SEPARATE_SPECULAR_COLOR_EXT );

    // Due to the bug found with Mesa 6.5.3 in the Radeon DRI driver
    // in radeon_state.c in radeonUpdateSpecular(),
    // it is important to set GL_SEPARATE_SPECULAR_COLOR_EXT
    // _before_ enabling lighting
    glEnable( GL_LIGHTING );

    glLightfv( GL_LIGHT0, GL_AMBIENT, LIGHT_AMBIENT );
    glLightfv( GL_LIGHT0, GL_DIFFUSE, LIGHT0_DIFFUSE );
    glLightfv( GL_LIGHT0, GL_SPECULAR, LIGHT0_SPECULAR );
    glLightfv( GL_LIGHT0, GL_POSITION, LIGHT0_POSITION );
    glEnable( GL_LIGHT0 );

    // Create a second light source to illuminate those shadows a little better
    glLightfv( GL_LIGHT1, GL_AMBIENT, LIGHT_AMBIENT );
    glLightfv( GL_LIGHT1, GL_DIFFUSE, LIGHT1_DIFFUSE );
    glLightfv( GL_LIGHT1, GL_SPECULAR, LIGHT1_SPECULAR );
    glLightfv( GL_LIGHT1, GL_POSITION, LIGHT1_POSITION );
    glEnable( GL_LIGHT1 );

    qDebug() << "GLWidget initialised...";
  }

  void GLWidget::paintGL()
  {
    resizeGL(width(), height()); // fix for bug #1797069. don't remove!
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // setup the OpenGL projection matrix using the camera
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    d->camera->applyPerspective();

    // setup the OpenGL modelview matrix using the camera
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    d->camera->applyModelview();

    render();
  }

  void GLWidget::paintGL2()
  {
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // setup the OpenGL projection matrix using the camera
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    d->camera->applyPerspective();

    // setup the OpenGL modelview matrix using the camera
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();
    d->camera->applyModelview();

    glEnable( GL_CULL_FACE );
    glEnable( GL_LIGHTING );
    glShadeModel( GL_SMOOTH );
    glEnable( GL_DEPTH_TEST );

    render();

    // Restore the OpenGL modelview matrix for the GLGraphicsView painter
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();

    glDisable( GL_CULL_FACE );
    glDisable( GL_LIGHTING );
  }

  void GLWidget::paintEvent( QPaintEvent * )
  {
    if(updatesEnabled())
    {
#ifdef ENABLE_THREADED_GL
      // tell our thread to paint
      d->paintCondition.wakeAll();
#else
      makeCurrent();
      if(!d->initialized) {
        d->initialized = true;
        initializeGL();
      }
      qglClearColor(d->background);
      paintGL();
      swapBuffers();
#endif
    }
  }

  void GLWidget::resizeEvent( QResizeEvent *event )
  {
#ifdef ENABLE_THREADED_GL
    d->thread->resize( event->size().width(), event->size().height() );
#else
    if (!isValid())
      return;
    makeCurrent();
    if(!d->initialized)
    {
      d->initialized = true;
      initializeGL();
    }
    // GLXWaitX() is called by the TT resizeEvent on Linux... We may need
    // specific functions here - need to look at Mac and Windows code.
    resizeGL(event->size().width(), event->size().height());
#endif
    emit resized();
  }

  void GLWidget::resizeGL( int width, int height )
  {
    glViewport( 0, 0, width, height );
  }

  void GLWidget::setBackground( const QColor &background )
  {
#ifdef ENABLE_THREADED_GL
    d->renderMutex.lock();
#endif
    d->background = background;
        d->background.setAlphaF(0.0);
#ifdef ENABLE_THREADED_GL
    d->renderMutex.unlock();
#endif
  }

  QColor GLWidget::background() const
  {
    return d->background;
  }

  void GLWidget::setColorMap(Color *colorMap)
  {
    d->colorMap = colorMap;
  }

  Color *GLWidget::colorMap() const
  {
    if (d->colorMap) {
      return d->colorMap;
    }
    else {
      if(!d->defaultColorMap) {
        PluginManager *plugins = PluginManager::instance();
        d->defaultColorMap = static_cast<Color*>(plugins->factories(Plugin::ColorType).at(0)->createInstance());
      }
      return d->defaultColorMap;
    }
  }

  void GLWidget::setQuality(int quality)
  {
    // Invalidate the display lists and change the painter quality level
    invalidateDLs();
    d->painter->setQuality(quality);
  }

  int GLWidget::quality() const
  {
    return d->painter->quality();
  }

  void GLWidget::setFogLevel(int level)
  {
    d->fogLevel = level;
  }

  int GLWidget::fogLevel() const
  {
    return d->fogLevel;
  }

  void GLWidget::setRenderAxes(bool renderAxes)
  {
    d->renderAxes = renderAxes;
    update();
  }

  bool GLWidget::renderAxes()
  {
    return d->renderAxes;
  }

  void GLWidget::setRenderDebug(bool renderDebug)
  {
    d->renderDebug = renderDebug;
    update();
  }

  bool GLWidget::renderDebug()
  {
    return d->renderDebug;
  }

  void GLWidget::render()
  {
    if (!d->molecule) {
      qDebug() << "GLWidget::render(): No molecule set.";
      return;
    }
    if (!d->molecule->lock()->tryLockForRead()) {
      qDebug() << "GLWidget::render(): Could not get read lock on molecule.";
      return;
    }

    d->painter->begin(this);

    if (d->painter->quality() >= 3) {
      glEnable(GL_LIGHT1);
    }
    else {
      glDisable(GL_LIGHT1);
    }
    bool hasUnitCell = (d->molecule->OBUnitCell() != NULL);

    if (d->fogLevel) {
      glFogi(GL_FOG_MODE, GL_LINEAR);
      GLfloat fogColor[4]= {d->background.redF(), d->background.greenF(),
                            d->background.blueF(), d->background.alphaF()};
      glFogfv(GL_FOG_COLOR, fogColor);
      Vector3d distance = camera()->modelview() * d->center;
      double distanceToCenter = distance.norm();
      glFogf(GL_FOG_DENSITY, 1.0);
      glHint(GL_FOG_HINT, GL_NICEST);
      glFogf(GL_FOG_START, distanceToCenter - (d->fogLevel / 8.0) * d->radius);
      glFogf(GL_FOG_END, distanceToCenter + ((10-d->fogLevel)/8.0 * 2.0) * d->radius);
      glEnable(GL_FOG);
    }
    else {
      glDisable(GL_FOG);
    }

    // Use renderQuick if the view is being moved, otherwise full render
    if (d->quickRender) {
      d->updateListQuick();
      glCallList(d->dlistQuick);
      if (hasUnitCell) {
        renderCrystal(d->dlistQuick);
      }
      // Render the active tool
      if ( d->tool ) {
        d->tool->paint( this );
      }
    }
    else {
      // we save a display list if we're doing a crystal
      if (d->dlistOpaque == 0)
        d->dlistOpaque = glGenLists(1);
      if (d->dlistTransparent == 0)
        d->dlistTransparent = glGenLists(1);

      // Opaque engine elements rendered first
      if (hasUnitCell) glNewList(d->dlistOpaque, GL_COMPILE);
      foreach(Engine *engine, d->engines)
        if(engine->isEnabled()) {
#ifdef ENABLE_GLSL
          if (m_glslEnabled) glUseProgramObjectARB(engine->shader());
#endif
          engine->renderOpaque(d->pd);
        }
#ifdef ENABLE_GLSL
          if (m_glslEnabled) glUseProgramObjectARB(0);
#endif
      if (hasUnitCell) { // end the main list and render the opaque crystal
        glEndList();
        renderCrystal(d->dlistOpaque);
      }

      // Render the active tool
      if ( d->tool ) {
        d->tool->paint( this );
      }

#ifdef ENABLE_PYTHON
      // Render the extensions (for now: python only)
      foreach (Extension *extension, d->extensions) {
        PythonExtension *pyext = qobject_cast<PythonExtension*>(extension);
        if (pyext)
          pyext->paint(this);
      }
#endif


      // Now render transparent
      glEnable(GL_BLEND);
      if (hasUnitCell)
        glNewList(d->dlistTransparent, GL_COMPILE);
      foreach(Engine *engine, d->engines) {
        if(engine->isEnabled() && engine->layers() & Engine::Transparent) {
#ifdef ENABLE_GLSL
          if (m_glslEnabled) glUseProgramObjectARB(engine->shader());
#endif
          engine->renderTransparent(d->pd);
        }
      }
      glDisable(GL_BLEND);
#ifdef ENABLE_GLSL
          if (m_glslEnabled) glUseProgramObjectARB(0);
#endif
      if (hasUnitCell) { // end the main list and render the transparent bits
        glEndList();
        renderCrystal(d->dlistTransparent);
      }
    }
    // Render all the inactive tools
    if ( d->toolGroup ) {
      QList<Tool *> tools = d->toolGroup->tools();
      foreach( Tool *tool, tools ) {
        if ( tool != d->tool ) {
          tool->paint( this );
        }
      }
    }

    // If enabled draw the axes
    if (d->renderAxes) renderAxesOverlay();

    // Render text overlay
    renderTextOverlay();

    d->painter->end();
    d->molecule->lock()->unlock();
  }

  void GLWidget::renderCrystal(GLuint displayList)
  {
    std::vector<vector3> cellVectors = d->molecule->OBUnitCell()->GetCellVectors();

    for (int a = 0; a < d->aCells; a++) {
      for (int b = 0; b < d->bCells; b++)  {
        for (int c = 0; c < d->cCells; c++)  {
          glPushMatrix();
          glTranslated(
                       cellVectors[0].x() * a
                       + cellVectors[1].x() * b
                       + cellVectors[2].x() * c,
                       cellVectors[0].y() * a
                       + cellVectors[1].y() * b
                       + cellVectors[2].y() * c,
                       cellVectors[0].z() * a
                       + cellVectors[1].z() * b
                       + cellVectors[2].z() * c );

          glCallList(displayList);
          glPopMatrix();
        }
      }
    } // end of for loops

    if (d->renderUnitCellAxes)
      renderCrystalAxes();
  }

  // Render the unit cell axes, indicating the frame of the cell
  //       4---5
  //      /   /|
  //     /   / |    (0 is the "origin" for this unit cell)
  //    3---2  6    (7 is in the back corner = cellVector[2])
  //    |   | /     (3 is cellVector[1])
  //    |   |/      (1 is cellVector[0])
  //    0---1
  void GLWidget::renderCrystalAxes()
  {
    std::vector<vector3> cellVectors = d->molecule->OBUnitCell()->GetCellVectors();
    vector3 v0(0.0, 0.0, 0.0);
    vector3 v1(cellVectors[0]);
    vector3 v3(cellVectors[1]);
    vector3 v7(cellVectors[2]);
    vector3 v2, v4, v5, v6;
    v2 = v1 + v3;
    v4 = v3 + v7;
    v6 = v1 + v7;
    v5 = v4 + v1;

    glDisable(GL_LIGHTING);
    glColor4f(d->cellColor.redF(), d->cellColor.greenF(), d->cellColor.blueF(), 0.7);
    //glColor4f(1.0, 1.0, 1.0, 0.7);
    glLineWidth(2.0);
    for (int a = 0; a < d->aCells; a++) {
      for (int b = 0; b < d->bCells; b++)  {
        for (int c = 0; c < d->cCells; c++)  {
          glPushMatrix();
          glTranslated(
                       cellVectors[0].x() * a
                       + cellVectors[1].x() * b
                       + cellVectors[2].x() * c,
                       cellVectors[0].y() * a
                       + cellVectors[1].y() * b
                       + cellVectors[2].y() * c,
                       cellVectors[0].z() * a
                       + cellVectors[1].z() * b
                       + cellVectors[2].z() * c );

          glBegin(GL_LINE_STRIP);
          glVertex3dv(v0.AsArray());
          glVertex3dv(v1.AsArray());
          glEnd();

          glBegin(GL_LINE_STRIP);
          glVertex3dv(v0.AsArray());
          glVertex3dv(v3.AsArray());
          glEnd();

          glBegin(GL_LINE_STRIP);
          glVertex3dv(v0.AsArray());
          glVertex3dv(v7.AsArray());
          glEnd();

          glBegin(GL_LINE_STRIP);
          glVertex3dv(v1.AsArray());
          glVertex3dv(v2.AsArray());
          glEnd();

          glBegin(GL_LINE_STRIP);
          glVertex3dv(v3.AsArray());
          glVertex3dv(v2.AsArray());
          glEnd();

          glBegin(GL_LINE_STRIP);
          glVertex3dv(v3.AsArray());
          glVertex3dv(v4.AsArray());
          glEnd();

          glBegin(GL_LINE_STRIP);
          glVertex3dv(v5.AsArray());
          glVertex3dv(v4.AsArray());
          glEnd();

          glBegin(GL_LINE_STRIP);
          glVertex3dv(v5.AsArray());
          glVertex3dv(v2.AsArray());
          glEnd();

          glBegin(GL_LINE_STRIP);
          glVertex3dv(v5.AsArray());
          glVertex3dv(v6.AsArray());
          glEnd();

          glBegin(GL_LINE_STRIP);
          glVertex3dv(v1.AsArray());
          glVertex3dv(v6.AsArray());
          glEnd();

          glBegin(GL_LINE_STRIP);
          glVertex3dv(v6.AsArray());
          glVertex3dv(v7.AsArray());
          glEnd();

          glBegin(GL_LINE_STRIP);
          glVertex3dv(v4.AsArray());
          glVertex3dv(v7.AsArray());
          glEnd();

          glPopMatrix();
        }
      }
    } // end of for loops
    glEnable(GL_LIGHTING);
  }

  void GLWidget::renderAxesOverlay()
  {
    // Render x, y, z axes as an overlay on the widget
    // Save the opengl projection matrix and set up an orthogonal projection
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    // Ensure the axes are of the same length
    double aspectRatio = static_cast<double>(d->pd->width())/static_cast<double>(d->pd->height());
    glOrtho(0, aspectRatio, 0, 1, 0, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Set the origin and calculate the positions of the axes
    Vector3d origin = Vector3d(0.07, 0.07, -.07);
    Vector3d aXa = d->pd->camera()->transformedXAxis() * 0.04 + origin;
    Vector3d aX = d->pd->camera()->transformedXAxis() * 0.06 + origin;
    Vector3d aYa = d->pd->camera()->transformedYAxis() * 0.04 + origin;
    Vector3d aY = d->pd->camera()->transformedYAxis() * 0.06 + origin;
    Vector3d aZa = d->pd->camera()->transformedZAxis() * 0.04 + origin;
    Vector3d aZ = d->pd->camera()->transformedZAxis() * 0.06 + origin;

    // Turn off dynamic scaling in the painter (cylinders don't render correctly)
    d->painter->setDynamicScaling(false);

    // Circle next to the axes
    painter()->setColor(1.0, 0.0, 0.0);
    painter()->drawSphere(&origin, 0.005);
    // x axis
    painter()->setColor(1.0, 0.0, 0.0);
    painter()->drawCylinder(origin, aXa, 0.005);
    painter()->drawCone(aXa, aX, 0.01);
    // y axis
    painter()->setColor(0.0, 1.0, 0.0);
    painter()->drawCylinder(origin, aYa, 0.005);
    painter()->drawCone(aYa, aY, 0.01);
    // y axis
    painter()->setColor(0.0, 0.0, 1.0);
    painter()->drawCylinder(origin, aZa, 0.005);
    painter()->drawCone(aZa, aZ, 0.01);

    // Turn dynamic scaling back on (default state)
    d->painter->setDynamicScaling(true);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
  }

  void GLWidget::renderDebugOverlay()
  {
    qDebug() << Q_FUNC_INFO << "is deprecated."
             << "Please use renderTextOverlay() instead.";
    return renderTextOverlay();
  }

  void GLWidget::renderTextOverlay()
  {
    // Draw all text in white
    d->pd->painter()->setColor(1.0, 1.0, 1.0);

    int x = 5, y = 5;

    if (d->renderDebug) {
      // Title
      y += d->pd->painter()->drawText
        (x, y, "---- " + tr("Debug Information") + " ----");

      // FPS
      y += d->pd->painter()->drawText
        (x, y, tr("FPS: %L1").arg(computeFramesPerSecond(), 0, 'g', 3));

      // pd size
      y += d->pd->painter()->drawText
        (x, y, tr("View Size: %L1 x %L2")
         .arg(d->pd->width()).arg(d->pd->height()) );

      // Molecule info
      if (!d->molecule) {
        y += d->pd->painter()->drawText(x, y, tr("No molecule set"));
      }
      else {
        // numAtoms
        y += d->pd->painter()->drawText
          (x, y, tr("Atoms: %L1").arg(d->molecule->numAtoms()));

        // numBonds
        y += d->pd->painter()->drawText
          (x, y, tr("Bonds: %L1").arg(d->molecule->numBonds()));
      }
    } // end debug

    // textOverlay stuff
    if (d->textOverlayLabels.size()) {
      // Lock mutex
      d->textOverlayMutex.lock();

      // For null pointers:
      QList<int> deadLabelIndices;

      // Draw text

      for (int i = 0; i < d->textOverlayLabels.size(); ++i) {
        QPointer<QLabel> &label = d->textOverlayLabels[i];

        // Check that QPointer is valid
        if (label == 0) {
          deadLabelIndices.append(i);
          continue;
        }

        // Draw text
        y += d->pd->painter()->drawText(x, y, label->text());
      }

      // Remove dead entries in reverse order
      for (int i = deadLabelIndices.size()-1; i >= 0; --i) {
        d->textOverlayLabels.removeAt(i);
      }

      // Release mutex
      d->textOverlayMutex.unlock();
    }
  }

  bool GLWidget::event( QEvent *event )
  {
    if(event->type() == QEvent::Show)
      {
        GLWidget::setCurrent(this);
      }

    return QGLWidget::event(event);
  }

  void GLWidget::mousePressEvent( QMouseEvent * event )
  {
    // Set the event to ignored, check whether any tools accept it
    event->ignore();

    if ( d->tool ) {
      QUndoCommand *command = 0;
      command = d->tool->mousePressEvent( this, event );
      // If the mouse event is not accepted, pass it to the navigate tool
      if (!event->isAccepted() && m_navigateTool) {
        command = m_navigateTool->mousePressEvent(this, event);
      }

      if ( command && d->undoStack ) {
        d->undoStack->push( command );
      } else if ( command ) {
        delete command;
      }
    }
    emit activated(this);
    emit mousePress(event);
  }

  void GLWidget::mouseReleaseEvent( QMouseEvent * event )
  {
    // Set the event to ignored, check whether any tools accept it
    event->ignore();

    if ( d->tool ) {
      QUndoCommand *command;
      command = d->tool->mouseReleaseEvent( this, event );
      // If the mouse event is not accepted, pass it to the navigate tool
      if (!event->isAccepted() && m_navigateTool) {
        command = m_navigateTool->mouseReleaseEvent(this, event);
      }

      if ( command && d->undoStack ) {
        d->undoStack->push( command );
      }
    }
#ifdef ENABLE_THREADED_GL
    d->renderMutex.lock();
#endif
    // Stop using quickRender
    d->quickRender = false;
#ifdef ENABLE_THREADED_GL
    d->renderMutex.unlock();
#endif
    // Render the scene at full quality now the mouse button has been released
    update();
    emit mouseRelease(event);
  }

  void GLWidget::mouseMoveEvent( QMouseEvent * event )
  {
    // Set the event to ignored, check whether any tools accept it
    event->ignore();

#ifdef ENABLE_THREADED_GL
    d->renderMutex.lock();
#endif
    // Use quick render while the mouse is down
    if (d->allowQuickRender)
      d->quickRender = true;
#ifdef ENABLE_THREADED_GL
    d->renderMutex.unlock();
#endif
    if ( d->tool ) {
      QUndoCommand *command;
      command = d->tool->mouseMoveEvent( this, event );
      // If the mouse event is not accepted, pass it to the navigate tool
      if (!event->isAccepted() && m_navigateTool) {
        command = m_navigateTool->mouseMoveEvent(this, event);
      }
      if ( command && d->undoStack ) {
        d->undoStack->push( command );
      }
    }
    emit mouseMove(event);
  }

  void GLWidget::wheelEvent( QWheelEvent * event )
  {
    // Set the event to ignored, check whether any tools accept it
    event->ignore();
    if ( d->tool ) {
      QUndoCommand *command;
      command = d->tool->wheelEvent( this, event );
      // If the mouse event is not accepted, pass it to the navigate tool
      if (!event->isAccepted() && m_navigateTool) {
        command = m_navigateTool->wheelEvent(this, event);
      }
      if ( command && d->undoStack ) {
        d->undoStack->push( command );
      }
    }
    emit wheel(event);
  }

  void GLWidget::keyPressEvent(QKeyEvent *event)
  {
    event->ignore();
    if (d->tool) {
      QUndoCommand *command;
      command = d->tool->keyPressEvent(this, event);
      // If the key event is not accepted, pass it to the navigate tool
      if (!event->isAccepted() && m_navigateTool) {
        command = m_navigateTool->keyPressEvent(this, event);
      }
      if ( command && d->undoStack ) {
        d->undoStack->push( command );
      }
    }
    update();
  }

  void GLWidget::keyReleaseEvent(QKeyEvent *event)
  {
    event->ignore();
    if (d->tool) {
      QUndoCommand *command;
      command = d->tool->keyReleaseEvent(this, event);
      // If the mouse event is not accepted, pass it to the navigate tool
      if (!event->isAccepted() && m_navigateTool) {
        command = m_navigateTool->keyReleaseEvent(this, event);
      }
      if ( command && d->undoStack ) {
        d->undoStack->push( command );
      }
    }
  }

  void GLWidget::setMolecule(Molecule *molecule)
  {
    if (!molecule)
      return;

    // disconnect from our old molecule
    if (d->molecule)
      disconnect(d->molecule, 0, this, 0);

    // Emit the molecule changed signal
    emit moleculeChanged(molecule);

    d->molecule = molecule;

    // Clear the selection list
    d->selectedPrimitives.clear();

    // compute the molecule's geometric info
    updateGeometry();
    invalidateDLs();

    // When the molecule is updated, the display lists become invalid, we should
    // also render the updated molecule. This should be much simpler than before.
    connect(d->molecule, SIGNAL(updated()), this, SLOT(invalidateDLs()));
    connect(d->molecule, SIGNAL(updated()), this, SLOT(updateGeometry()));
    connect(d->molecule, SIGNAL(updated()), this, SLOT(update()));

    // If primitives, atoms, or bonds are removed, we need to delete them from the selected list
    connect(d->molecule, SIGNAL(primitiveRemoved(Primitive*)),
            this, SLOT(unselectPrimitive(Primitive*)));
    connect(d->molecule, SIGNAL(atomRemoved(Atom*)),
            this, SLOT(unselectAtom(Atom*)));
    connect(d->molecule, SIGNAL(bondRemoved(Bond*)),
            this, SLOT(unselectBond(Bond*)));

    // setup the camera to have a nice viewpoint on the molecule
    d->camera->initializeViewPoint();

    update();
  }

  void GLWidget::unselectPrimitive(Primitive *p)
  {
    d->selectedPrimitives.removeAll( p );
    // The engine caches must be invalidated
    d->updateCache = true;

    // TODO: remove also from named selections
  }

  void GLWidget::unselectAtom(Atom *a)
  {
    unselectPrimitive(a);
  }

  void GLWidget::unselectBond(Bond *b)
  {
    unselectPrimitive(b);
  }

  const Molecule* GLWidget::molecule() const
  {
    return d->molecule;
  }

  Molecule* GLWidget::molecule()
  {
    return d->molecule;
  }

  const Vector3d & GLWidget::center() const
  {
    return d->center;
  }

  const Vector3d & GLWidget::normalVector() const
  {
    return d->normalVector;
  }

  double GLWidget::radius() const
  {
    return d->radius;
  }

  const Atom *GLWidget::farthestAtom() const
  {
    return d->farthestAtom;
  }

  void GLWidget::updateGeometry()
  {
    if (!d->molecule)
      return;

    // Try to get a read lock for the molecule
    if (!d->molecule->lock()->tryLockForRead())
      return;

    if (!d->molecule->OBUnitCell()) {
      // Plain molecule, no crystal cell
      d->center = d->molecule->center();
      d->normalVector = d->molecule->normalVector();
      d->radius = d->molecule->radius();
      d->farthestAtom = d->molecule->farthestAtom();
    }
    else {
      // render a crystal (so most geometry comes from the cell vectors)
      // Origin at 0.0, 0.0, 0.0
      // a = <x0, y0, z0>
      // b = <x1, y1, z1>
      // c = <x2, y2, z2>
      std::vector<vector3> cellVectors = d->molecule->OBUnitCell()->GetCellVectors();
      Vector3d a(cellVectors[0].AsArray());
      Vector3d b(cellVectors[1].AsArray());
      Vector3d c(cellVectors[2].AsArray());
      Vector3d centerOffset = ( a * (d->aCells - 1)
                              + b * (d->bCells - 1)
                              + c * (d->cCells - 1) ) / 2.0;
      // the center is the center of the molecule translated by centerOffset
      d->center = d->molecule->center() + centerOffset;
      // the radius is the length of centerOffset plus the molecule radius
      d->radius = d->molecule->radius() + centerOffset.norm();
      // for the normal vector, we just ask for the molecule's normal vector,
      // crossing our fingers hoping that it will give a nice viewpoint not only
      // with respect to the molecule but also with respect to the cells.
      d->normalVector = d->molecule->normalVector();
      // Computation of the farthest atom.
      // First case: the molecule is empty
      if(d->molecule->numAtoms() == 0)
      d->farthestAtom = 0;
      // Second case: there is no repetition of the molecule
      else if(d->aCells <= 1 && d->bCells <= 1 && d->cCells <= 1)
        d->farthestAtom = d->molecule->farthestAtom();
      // General case: the farthest atom is the one that is located the
      // farthest in the direction pointed to by centerOffset.
      else {
        QList<Atom *> atoms = d->molecule->atoms();
        double x, max_x;

        d->farthestAtom = atoms.at(0);
        max_x = centerOffset.dot(*d->farthestAtom->pos());
        foreach (Atom *atom, atoms) {
          x = centerOffset.dot(*atom->pos());
          if (x > max_x) {
            max_x = x;
            d->farthestAtom = atom;
          }
        } // end foreach
      } // end general repeat (many atoms, multiple cells)
    } // End the case for unit cells
    d->molecule->lock()->unlock();
  }

  Camera * GLWidget::camera() const
  {
    return d->camera;
  }

  QList<Engine *> GLWidget::engines() const
  {
    return d->engines;
  }

  void GLWidget::addEngine(Engine *engine)
  {
    connect(engine, SIGNAL(changed()), this, SLOT(update()));
    connect(engine, SIGNAL(changed()), this, SLOT(invalidateDLs()));
    connect(this, SIGNAL(moleculeChanged(Molecule *)),
            engine, SLOT(setMolecule(Molecule *)));
    d->engines.append(engine);
    qSort(d->engines.begin(), d->engines.end(), engineLessThan);
    engine->setPainterDevice(d->pd);
    emit engineAdded(engine);
    update();
  }

  void GLWidget::removeEngine(Engine *engine)
  {
    disconnect(engine, 0, this, 0);
    disconnect(this, 0, engine, 0);
    d->engines.removeAll(engine);
    emit engineRemoved(engine);
    engine->deleteLater();
    update();
  }

  void GLWidget::setTool(Tool *tool)
  {
    if ( tool ) {
      d->tool = tool;
    }
  }

  void GLWidget::setToolGroup(ToolGroup *toolGroup)
  {
    if ( d->toolGroup ) {
      disconnect( d->toolGroup, 0, this, 0 );
    }

    if ( toolGroup ) {
      d->toolGroup = toolGroup;
      d->tool = toolGroup->activeTool();
      connect( toolGroup, SIGNAL( toolActivated( Tool* ) ),
               this, SLOT( setTool( Tool* ) ) );
      connect( toolGroup, SIGNAL( toolsDestroyed() ),
               this, SLOT( toolsDestroyed() ) );
    }
    // Find the navigate tool and set it
    foreach (Tool *tool, d->toolGroup->tools()) {
      if (tool->identifier() == "Navigate") {
        m_navigateTool = tool;
      }
    }
  }

  void GLWidget::toolsDestroyed()
  {
    d->tool = 0;
    m_navigateTool = 0;
  }

  void GLWidget::addTextOverlay(QLabel* str)
  {
    d->textOverlayMutex.lock();
    d->textOverlayLabels.append(QPointer<QLabel>(str));
    d->textOverlayMutex.unlock();
  }

  void GLWidget::addTextOverlay(const QList<QLabel*> &strs)
  {
    d->textOverlayMutex.lock();
    for (QList<QLabel*>::const_iterator
           it = strs.constBegin(),
           it_end = strs.constEnd();
         it != it_end; ++it) {
      d->textOverlayLabels.append(QPointer<QLabel>(*it));
    }
    d->textOverlayMutex.unlock();
  }

  void GLWidget::setExtensions(QList<Extension*> extensions)
  {
    d->extensions = extensions;
  }


  void GLWidget::setUndoStack( QUndoStack *undoStack )
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

  QList<GLHit> GLWidget::hits( int x, int y, int w, int h )
  {
    QList<GLHit> hits;

    if ( !molecule() ) return hits;

    GLint viewport[4];
    unsigned int hit_count;

    int cx = w/2 + x;
    int cy = h/2 + y;

    // setup the selection buffer
    int requiredSelectBufSize = (d->molecule->numAtoms() + d->molecule->numBonds()) * 8;
    if ( requiredSelectBufSize > d->selectBufSize ) {
      //resize selection buffer
      if ( d->selectBuf ) delete[] d->selectBuf;
      // add some margin so that resizing doesn't occur every time an atom is added
      d->selectBufSize = requiredSelectBufSize + SEL_BUF_MARGIN;
      if ( d->selectBufSize > SEL_BUF_MAX_SIZE ) {
        d->selectBufSize = SEL_BUF_MAX_SIZE;
      }
      d->selectBuf = new GLuint[d->selectBufSize];
    }

#ifdef ENABLE_THREADED_GL
    d->renderMutex.lock();
#endif
    makeCurrent();
    //X   hits.clear();

    glSelectBuffer( d->selectBufSize, d->selectBuf );
    glRenderMode( GL_SELECT );
    glInitNames();

    // Setup a projection matrix for picking in the zone delimited by (x,y,w,h).
    glGetIntegerv( GL_VIEWPORT, viewport );
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    gluPickMatrix( cx,viewport[3]-cy, w, h,viewport );

    // now multiply that projection matrix with the perspective of the camera
    d->camera->applyPerspective();

    // now load the modelview matrix from the camera
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();
    d->camera->applyModelview();

    // now actually render using low quality, "pickrender"
    d->painter->begin(this);
#ifdef ENABLE_GLSL
        if (m_glslEnabled) glUseProgramObjectARB(0);
#endif
    foreach(Engine *engine, d->engines) {
      if(engine->isEnabled()) {
        engine->renderPick(d->pd);
      }
    }
    d->painter->end();

    // returning to normal rendering mode
    hit_count = glRenderMode( GL_RENDER );
    
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();

#ifdef ENABLE_THREADED_GL
    doneCurrent();
    d->renderMutex.unlock();
#endif

    // if no error occurred and there are hits, process them
    if ( hit_count > 0 ) {
      unsigned int i, j;
      GLuint names, type, *ptr;
      GLuint minZ, maxZ;
      long name;

      //X   printf ("hits = %d\n", hits);
      ptr = ( GLuint * ) d->selectBuf;
      // for all hits and not past end of buffer
      for (i = 0; i < hit_count && !(ptr > d->selectBuf + d->selectBufSize); ++i) {
        names = *ptr++;
        // make sure that we won't be passing the end of bufer
        if ( ptr + names + 2 > d->selectBuf + d->selectBufSize ) {
          break;
        }
        minZ = *ptr++;
        maxZ = *ptr++;

        // allow names of 0
        name = -1;
        for (j = 0; j < names/2; ++j) { /*  for each name */
          type = *ptr++;
          name = *ptr++;
        }
        if (name > -1) {
          //            printf ("%ld(%d) ", name,type);
          hits.append( GLHit( type,name,minZ,maxZ ) );
        }
      }
      //      printf ("\n");
      qSort( hits );
    }

    return( hits );
  }

  Primitive* GLWidget::computeClickedPrimitive(const QPoint& p)
  {
    QList<GLHit> chits;

    // Perform an OpenGL selection and retrieve the list of hits.
    chits = hits(p.x()-SEL_BOX_HALF_SIZE,
                 p.y()-SEL_BOX_HALF_SIZE,
                 SEL_BOX_SIZE, SEL_BOX_SIZE);

    // Find the first atom or bond (if any) in hits - this will be the closest
    foreach(const GLHit& hit, chits)
    {
      //qDebug() << "Hit: " << hit.name();
      if(hit.type() == Primitive::AtomType)
        return molecule()->atom(hit.name());
      else if(hit.type() == Primitive::BondType)
        return molecule()->bond(hit.name());
    }
    return 0;
  }

  Atom* GLWidget::computeClickedAtom(const QPoint& p)
  {
    QList<GLHit> chits;

    // Perform an OpenGL selection and retrieve the list of hits.
    chits = hits(p.x()-SEL_BOX_HALF_SIZE,
                 p.y()-SEL_BOX_HALF_SIZE,
                 SEL_BOX_SIZE, SEL_BOX_SIZE);

    // Find the first atom (if any) in hits - this will be the closest
    foreach(const GLHit& hit, chits)
      if(hit.type() == Primitive::AtomType)
        return molecule()->atom(hit.name());

    return 0;
  }

  Bond* GLWidget::computeClickedBond(const QPoint& p)
  {
    QList<GLHit> chits;

    // Perform an OpenGL selection and retrieve the list of hits.
    chits = hits(p.x()-SEL_BOX_HALF_SIZE,
                 p.y()-SEL_BOX_HALF_SIZE,
                 SEL_BOX_SIZE, SEL_BOX_SIZE);

    // Find the first bond (if any) in hits - this will be the closest
    foreach(const GLHit& hit, chits)
      if(hit.type() == Primitive::BondType)
        return static_cast<Bond *>(molecule()->bond(hit.name()));

    return 0;
  }

  QSize GLWidget::sizeHint() const
  {
    return minimumSizeHint();
  }

  QSize GLWidget::minimumSizeHint() const
  {
    return QSize( 200,200 );
  }

  double GLWidget::radius( const Primitive *p ) const
  {
    if (!p)
      return 0.0;
    double radius = 0.0;
    foreach(Engine *engine, d->engines)
    {
      if (engine->isEnabled())
      {
        double engineRadius = engine->radius( d->pd, p );
        if ( engineRadius > radius )
          radius = engineRadius;
      }
    }

    return radius;
  }

  void GLWidget::setSelected(PrimitiveList primitives, bool select)
  {
    foreach(Primitive *item, primitives) {
      if (select && !d->selectedPrimitives.contains(item))
          d->selectedPrimitives.append( item );
      else if (!select)
        d->selectedPrimitives.removeAll( item );
      // The engine caches must be invalidated
      d->updateCache = true;
      item->update();
    }
  }

  PrimitiveList GLWidget::selectedPrimitives() const
  {
    return d->selectedPrimitives.list();
  }

  void GLWidget::toggleSelected( PrimitiveList primitives )
  {
    foreach(Primitive *item, primitives)
    {
      if (d->selectedPrimitives.contains(item))
        d->selectedPrimitives.removeAll( item );
      else
        d->selectedPrimitives.append(item);
    }
    // The engine caches must be invalidated
    d->updateCache = true;
  }

  void GLWidget::toggleSelected()
  {
    if (!d->molecule) return;
    // Currently handle atoms and bonds
    foreach(Atom *a, d->molecule->atoms()) {
      Primitive *p = static_cast<Primitive *>(a);
      if (d->selectedPrimitives.contains(p))
        d->selectedPrimitives.removeAll(p);
      else
        d->selectedPrimitives.append(p);
    }
    foreach(Bond *b, d->molecule->bonds()) {
      Primitive *p = static_cast<Primitive *>(b);
      if (d->selectedPrimitives.contains(p))
        d->selectedPrimitives.removeAll(p);
      else
        d->selectedPrimitives.append(p);
    }
    // The engine caches must be invalidated
    d->updateCache = true;
  }

  void GLWidget::clearSelected()
  {
    d->selectedPrimitives.clear();
    // The engine caches must be invalidated
    d->updateCache = true;
  }

  bool GLWidget::isSelected( const Primitive *p ) const
  {
    // Return true if the item is selected
    return d->selectedPrimitives.contains(const_cast<Primitive *>(p));
  }

  bool GLWidget::addNamedSelection(const QString &name, PrimitiveList &primitives)
  {
    // make sure the name is unique
    for (int i = 0; i < d->namedSelections.size(); ++i)
      if (d->namedSelections.at(i).first == name)
        return false;

    QList<unsigned int> atomIds;
    QList<unsigned int> bondIds;
    foreach(Primitive *item, primitives) {
      if (item->type() == Primitive::AtomType)
        atomIds.append(item->id());
      if (item->type() == Primitive::BondType)
        bondIds.append(item->id());
    }

    QPair<QList<unsigned int>,QList<unsigned int> > pair(atomIds, bondIds);
    QPair<QString, QPair<QList<unsigned int>,QList<unsigned int> > > namedSelection(name, pair);
    d->namedSelections.append(namedSelection);

    emit namedSelectionsChanged();
    return true;
  }

  void GLWidget::removeNamedSelection(const QString &name)
  {
    for (int i = 0; i < d->namedSelections.size(); ++i)
      if (d->namedSelections.at(i).first == name) {
        d->namedSelections.removeAt(i);
        emit namedSelectionsChanged();
        return;
      }
  }

  void GLWidget::removeNamedSelection(int index)
  {
    d->namedSelections.removeAt(index);
  }

  void GLWidget::renameNamedSelection(int index, const QString &name)
  {
    if (name.isEmpty())
      return;

    QPair<QString, QPair<QList<unsigned int>,QList<unsigned int> > > pair = d->namedSelections.takeAt(index);
    pair.first = name;
    d->namedSelections.insert(index, pair);
    emit namedSelectionsChanged();
  }

  QList<QString> GLWidget::namedSelections()
  {
    QList<QString> names;
    for (int i = 0; i < d->namedSelections.size(); ++i)
      names.append(d->namedSelections.at(i).first);

    return names;
  }

  PrimitiveList GLWidget::namedSelectionPrimitives(const QString &name)
  {
    for (int i = 0; i < d->namedSelections.size(); ++i)
      if (d->namedSelections.at(i).first == name) {
    return namedSelectionPrimitives(i);
    }

    return PrimitiveList();
  }

  PrimitiveList GLWidget::namedSelectionPrimitives(int index)
  {
    PrimitiveList list;
    if (!d->molecule)
      return list;

    for (int j = 0; j < d->namedSelections.at(index).second.first.size(); ++j) {
      Atom *atom = d->molecule->atomById(d->namedSelections.at(index).second.first.at(j));
      if (atom)
        list.append(atom);
    }

    for (int j = 0; j < d->namedSelections.at(index).second.second.size(); ++j) {
      Bond *bond = d->molecule->bondById(d->namedSelections.at(index).second.second.at(j));
      if (bond)
        list.append(bond);
    }

    return list;
  }

  void GLWidget::setUnitCells( int a, int b, int c )
  {
    d->aCells = a;
    d->bCells = b;
    d->cCells = c;
    updateGeometry();
    d->camera->initializeViewPoint();
    update();
  }

  void GLWidget::setUnitCellColor(const QColor c)
  {
#ifdef ENABLE_THREADED_GL
    d->renderMutex.lock();
#endif
    d->cellColor = c;
#ifdef ENABLE_THREADED_GL
    d->renderMutex.unlock();
#endif
  }

  void GLWidget::clearUnitCell()
  {
    updateGeometry();
    d->camera->initializeViewPoint();
    update();
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

  inline double GLWidget::computeFramesPerSecond()
  {
    static QTime time;
    static bool firstTime = true;
    static int old_time, new_time;
    static int frames;
    static double fps;

    if( firstTime )
    {
      time.start();
      firstTime = false;
      old_time = time.elapsed();
      frames = 0;
      fps = 0;
    }

    new_time = time.elapsed();
    frames++;

    if( new_time - old_time > 200 )
    {
      fps = 1000.0 * frames / double( new_time - old_time );
      frames = 0;
      time.restart();
      old_time = time.elapsed();
    }

    return fps;
  }

  void GLWidget::writeSettings(QSettings &settings) const
  {
    settings.setValue("background", d->background);
    settings.setValue("quality", d->painter->quality());
    settings.setValue("fogLevel", d->fogLevel);
    settings.setValue("renderAxes", d->renderAxes);
    settings.setValue("renderDebug", d->renderDebug);
    settings.setValue("allowQuickRender", d->allowQuickRender);
    settings.setValue("renderUnitCellAxes", d->renderUnitCellAxes);

    int count = d->engines.size();
    settings.beginWriteArray("engines");
    // Clear the array before writing settings - fixes issue when index changes
    settings.remove("");
    for(int i = 0; i< count; i++) {
      settings.setArrayIndex(i);
      d->engines.at(i)->writeSettings(settings);
    }
    settings.endArray();
  }

  void GLWidget::readSettings(QSettings &settings)
  {
    // Make sure to provide some default values for any settings.value("", DEFAULT) call
    setQuality(settings.value("quality", 2).toInt());
    setFogLevel(settings.value("fogLevel", 0).toInt());
    d->background = settings.value("background", QColor(0,0,0,0)).value<QColor>();
    d->renderAxes = settings.value("renderAxes", 1).value<bool>();
    d->renderDebug = settings.value("renderDebug", 0).value<bool>();
    d->allowQuickRender = settings.value("allowQuickRender", 1).value<bool>();
    d->renderUnitCellAxes = settings.value("renderUnitCellAxes", 1).value<bool>();

    int count = settings.beginReadArray("engines");
    for(int i=0; i<count; i++) {
      settings.setArrayIndex(i);
      QString engineClass = settings.value("engineID", QString()).toString();
      PluginManager *plugins = PluginManager::instance();
      PluginFactory *factory = plugins->factory(engineClass,
                                                Plugin::EngineType);
      if(!engineClass.isEmpty() && factory) {
        Engine *engine = static_cast<Engine *>(factory->createInstance(this));
        engine->readSettings(settings);
        addEngine(engine);
      }
    }
    settings.endArray();

    if(!d->engines.count())
      loadDefaultEngines();
  }

  void GLWidget::loadDefaultEngines()
  {
    QList<Engine *> engines = d->engines;

    foreach(Engine *engine, engines)
      this->removeEngine(engine);

    PluginManager *plugins = PluginManager::instance();
    foreach(PluginFactory *factory, plugins->factories(Plugin::EngineType)) {
      Engine *engine = static_cast<Engine *>(factory->createInstance(this));
      if (engine->name() == tr("Ball and Stick"))
        engine->setEnabled(true);
      addEngine(engine);
    }
  }

  void GLWidget::reloadEngines()
  {
    QSettings settings;

    // save engine settings
    int count = d->engines.size();
    settings.beginWriteArray("engines");
    for(int i = 0; i< count; i++) {
      settings.setArrayIndex(i);
      d->engines.at(i)->writeSettings(settings);
    }
    settings.endArray();

    // delete engines
    foreach(Engine *engine, d->engines) {
      delete engine;
    }

    // clear the engine list
    d->engines.clear();

    // read settings and create required engines
    count = settings.beginReadArray("engines");
    for(int i=0; i<count; i++) {
      settings.setArrayIndex(i);
      QString engineClass = settings.value("engineID", QString()).toString();
      PluginManager *plugins = PluginManager::instance();
      PluginFactory *factory = plugins->factory(engineClass,
                                                Plugin::EngineType);
      if(!engineClass.isEmpty() && factory) {
        Engine *engine = static_cast<Engine *>(factory->createInstance(this));
        engine->readSettings(settings);
        addEngine(engine);
      }
    }
    settings.endArray();
  }


  void GLWidget::setQuickRender(bool enabled)
  {
    d->allowQuickRender = enabled;
  }

  bool GLWidget::quickRender() const
  {
    return d->allowQuickRender;
  }

  void GLWidget::setRenderUnitCellAxes(bool enabled)
  {
    if (d->renderUnitCellAxes != enabled) {
      d->renderUnitCellAxes = enabled;
      emit unitCellAxesRenderChanged(enabled);
    }
  }

  bool GLWidget::renderUnitCellAxes() const
  {
    return d->renderUnitCellAxes;
  }

  void GLWidget::invalidateDLs()
  {
    // Something changed and we need to invalidate the display lists
    d->updateCache = true;
  }

// Copied from current sources of Qt 4.7  
#ifndef QT_OPENGL_ES

static void save_gl_state()
{
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glShadeModel(GL_FLAT);
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

static void restore_gl_state()
{
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopAttrib();
    glPopClientAttrib();
}

  void gl_draw_text(QPainter *p, int x, int y, const QString &str, const QFont &font, double zoom)
  {
    GLfloat color[4];
    glGetFloatv(GL_CURRENT_COLOR, &color[0]);

    QFont fore_font = font;
    qreal size = font.pointSizeF() * zoom;
    fore_font.setPointSizeF(size);

    QColor col, outline;
    col.setRgbF(color[0], color[1], color[2],color[3]);
    outline.setRgbF((1-color[0])/2,(1-color[1])/2,(1-color[2])/2); // use opposite color, but make it darker
    if(!p) return;  // prevent segfaults
    
    QPen old_pen = p->pen();
    QFont old_font = p->font();

    // Outline
    p->setPen(outline);
    //QFont out_font = fore_font;
    //out_font.setWeight(font.weight()+10);
    p->setFont(fore_font);
   // p->drawText(x+2, y, str);
    p->drawText(x+1, y, str);
    p->drawText(x-1, y, str);
   // p->drawText(x-2, y, str);
    p->drawText(x, y+1, str);
   // p->drawText(x, y+2, str);
    p->drawText(x, y-1, str);
   // p->drawText(x, y-2, str);

    // Foreground
    p->setPen(col);
    p->setFont(fore_font);
    p->drawText(x, y, str);

    p->setPen(old_pen);
    p->setFont(old_font);
  }

static inline void transform_point(GLdouble out[4], const GLdouble m[16], const GLdouble in[4])
{
#define M(row,col)  m[col*4+row]
    out[0] =
        M(0, 0) * in[0] + M(0, 1) * in[1] + M(0, 2) * in[2] + M(0, 3) * in[3];
    out[1] =
        M(1, 0) * in[0] + M(1, 1) * in[1] + M(1, 2) * in[2] + M(1, 3) * in[3];
    out[2] =
        M(2, 0) * in[0] + M(2, 1) * in[1] + M(2, 2) * in[2] + M(2, 3) * in[3];
    out[3] =
        M(3, 0) * in[0] + M(3, 1) * in[1] + M(3, 2) * in[2] + M(3, 3) * in[3];
#undef M
}

static inline GLint gluProject(GLdouble objx, GLdouble objy, GLdouble objz,
           const GLdouble model[16], const GLdouble proj[16],
           const GLint viewport[4],
           GLdouble * winx, GLdouble * winy, GLdouble * winz)
{
   GLdouble in[4], out[4];

   in[0] = objx;
   in[1] = objy;
   in[2] = objz;
   in[3] = 1.0;
   transform_point(out, model, in);
   transform_point(in, proj, out);

   if (in[3] == 0.0)
      return GL_FALSE;

   in[0] /= in[3];
   in[1] /= in[3];
   in[2] /= in[3];

   *winx = viewport[0] + (1 + in[0]) * viewport[2] / 2;
   *winy = viewport[1] + (1 + in[1]) * viewport[3] / 2;

   *winz = (1 + in[2]) / 2;
   return GL_TRUE;
}

#endif

  // Based on Qt code
  void GLWidget::renderText(double x, double y, double z, const QString &str, const QFont &font, int)
  {
    //QGLWidget::renderText(x,y,z,str,font, i);
    
#ifndef QT_OPENGL_ES
    if (str.isEmpty() || !isValid())
        return;

    bool auto_swap = autoBufferSwap();

    int width = d->pd->width();
    int height = d->pd->height();
    GLdouble model[4][4], proj[4][4];
    GLint view[4];
    glGetDoublev(GL_MODELVIEW_MATRIX, &model[0][0]);
    glGetDoublev(GL_PROJECTION_MATRIX, &proj[0][0]);
    glGetIntegerv(GL_VIEWPORT, &view[0]);
    GLdouble win_x = 0, win_y = 0, win_z = 0;
    gluProject(x, y, z, &model[0][0], &proj[0][0], &view[0],
                &win_x, &win_y, &win_z);
    win_y = height - win_y; // y is inverted

    //QPaintEngine::Type oldEngineType = qgl_engine_selector()->preferredPaintEngine();
    //QPaintEngine::Type oldEngineType = QGL::preferredPaintEngine();
    QPaintEngine *engine = paintEngine();

    //if (engine && (oldEngineType == QPaintEngine::OpenGL2) && engine->isActive()) {
    //    qWarning("QGLWidget::renderText(): Calling renderText() while a GL 2 paint engine is"
    //             " active on the same device is not allowed.");
    //    return;
    //}

    // this changes what paintEngine() returns
    //QGL::setPreferredPaintEngine(QPaintEngine::OpenGL);
    //qgl_engine_selector()->setPreferredPaintEngine(QPaintEngine::OpenGL);
    engine = paintEngine();
    QPainter *p;
    bool reuse_painter = false;
    bool use_depth_testing = glIsEnabled(GL_DEPTH_TEST);
    bool use_scissor_testing = glIsEnabled(GL_SCISSOR_TEST);

    if (engine->isActive()) {
        reuse_painter = true;
        p = engine->painter();
        save_gl_state();
    } else {
        setAutoBufferSwap(false);
        // disable glClear() as a result of QPainter::begin()
        //d->disable_clear_on_painter_begin = true;
        p = new QPainter(this);
    }

    QRect viewport(view[0], view[1], view[2], view[3]);
    if (!use_scissor_testing && viewport != rect()) {
        glScissor(view[0], view[1], view[2], view[3]);
        glEnable(GL_SCISSOR_TEST);
    } else if (use_scissor_testing) {
        glEnable(GL_SCISSOR_TEST);
    }
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, width, height);
    glOrtho(0, width, height, 0, 0, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glAlphaFunc(GL_GREATER, 0.0);
    glEnable(GL_ALPHA_TEST);
    if (use_depth_testing)
        glEnable(GL_DEPTH_TEST);
    glTranslated(0, 0, -win_z);
    
    gl_draw_text(p, qRound(win_x), qRound(win_y), str, font, 10.0/camera()->distance(Vector3d(0,0,0)));

    if (reuse_painter) {
        restore_gl_state();
    } else {
        p->end();
        delete p;
        setAutoBufferSwap(auto_swap);
      //  d->disable_clear_on_painter_begin = false;
    }
    //qgl_engine_selector()->setPreferredPaintEngine(oldEngineType);
#else // QT_OPENGL_ES
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(z);
    Q_UNUSED(str);
    Q_UNUSED(font);
    qWarning("QGLWidget::renderText is not supported under OpenGL/ES");
#endif
  }
}
