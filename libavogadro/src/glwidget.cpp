/**********************************************************************
  GLWidget - general OpenGL display

  Copyright (C) 2006,2007 Geoffrey R. Hutchison
  Copyright (C) 2006,2007 Donald Ephraim Curtis
  Copyright (C) 2007      Benoit Jacob
  Copyright (C) 2007,2008 Marcus D. Hanwell

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
#include <avogadro/glpainter.h>
#include <avogadro/painterdevice.h>
#include <avogadro/toolgroup.h>

#include "elementcolor.h"

#include <QUndoStack>
#include <QDir>
#include <QPluginLoader>
#include <QTime>

#ifdef ENABLE_THREADED_GL
#include <QWaitCondition>
#include <QMutex>
#endif

#include <QDebug>

#include <stdio.h>
#include <vector>

using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro {

  bool engineLessThan( const Engine* lhs, const Engine* rhs )
  {
    Engine::EngineFlags lhsFlags = lhs->flags();
    Engine::EngineFlags rhsFlags = rhs->flags();

    if ( !( lhsFlags & Engine::Overlay ) && rhsFlags & Engine::Overlay ) {
      return true;
    } else if (( lhsFlags & Engine::Overlay ) && ( rhsFlags & Engine::Overlay ) ) {
      return lhs->transparencyDepth() < rhs->transparencyDepth();
    } else if (( lhsFlags & Engine::Overlay ) && !( rhsFlags & Engine::Overlay ) ) {
      return false;
    } else if ( !( lhsFlags & Engine::Molecules ) && rhsFlags & Engine::Molecules ) {
      return true;
    } else if (( lhsFlags & Engine::Molecules ) && ( rhsFlags & Engine::Molecules ) ) {
      return lhs->transparencyDepth() < rhs->transparencyDepth();
    } else if (( lhsFlags & Engine::Molecules ) && !( rhsFlags & Engine::Molecules ) ) {
      return false;
    } else if ( !( lhsFlags & Engine::Atoms ) && rhsFlags & Engine::Atoms ) {
      return true;
    } else if (( lhsFlags & Engine::Atoms ) && ( rhsFlags & Engine::Atoms ) ) {
      return lhs->transparencyDepth() < rhs->transparencyDepth();
    } else if (( lhsFlags & Engine::Atoms ) && !( rhsFlags & Engine::Atoms ) ) {
      return false;
    } else if ( !( lhsFlags & Engine::Bonds ) && rhsFlags & Engine::Bonds ) {
      return true;
    } else if (( lhsFlags & Engine::Bonds ) && ( rhsFlags & Engine::Bonds ) ) {
      return lhs->transparencyDepth() < rhs->transparencyDepth();
    } else if (( lhsFlags & Engine::Bonds ) && !( rhsFlags & Engine::Bonds ) ) {
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

  GLHit::GLHit() : d( new GLHitPrivate ) {}

  GLHit::GLHit( const GLHit &other ) : d( new GLHitPrivate )
  {
    GLHitPrivate *e = other.d;
    d->type = e->type;
    d->name = e->name;
    d->minZ = e->minZ;
    d->maxZ = e->maxZ;
  }

  GLHit::GLHit( GLuint type, GLuint name, GLuint minZ, GLuint maxZ ) : d( new GLHitPrivate )
  {
    d->name = name;
    d->type = type;
    d->minZ = minZ;
    d->maxZ = maxZ;
  }

  GLHit &GLHit::operator=( const GLHit &other )
  {
    GLHitPrivate *e = other.d;
    d->type = e->type;
    d->name = e->name;
    d->minZ = e->minZ;
    d->maxZ = e->maxZ;
    return *this;
  }

  GLHit::~GLHit()
  {
    delete d;
  }

  bool GLHit::operator<( const GLHit &other ) const
  {
    GLHitPrivate *e = other.d;
    return d->minZ < e->minZ;
  }

  bool GLHit::operator==( const GLHit &other ) const
  {
    GLHitPrivate *e = other.d;
    return (( d->type == e->type ) && ( d->name == e->name ) );
  }

  GLuint GLHit::name() const { return d->name; }
  GLuint GLHit::type() const { return d->type; }
  GLuint GLHit::minZ() const { return d->minZ; }
  GLuint GLHit::maxZ() const { return d->maxZ; }

  void GLHit::setName( GLuint name ) { d->name = name; }
  void GLHit::setType( GLuint type ) { d->type = type; }
  void GLHit::setMinZ( GLuint minZ ) { d->minZ = minZ; }
  void GLHit::setMaxZ( GLuint maxZ ) { d->maxZ = maxZ; }

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
    Color *colorMap() const { return widget->colorMap();  }

    int width() { return widget->width(); }
    int height() { return widget->height(); }

  private:
    GLWidget *widget;
  };

  class GLWidgetPrivate
  {
  public:
    GLWidgetPrivate() : background( Qt::black ),
                        aCells( 1 ), bCells( 1 ), cCells( 1 ),
                        uc (0),
                        molecule( 0 ),
                        camera( new Camera ),
                        tool( 0 ),
                        toolGroup( 0 ),
                        selectBuf( 0 ),
                        selectBufSize( -1 ),
#ifdef ENABLE_THREADED_GL
                        thread( 0 ),
#else
                        initialized( false ),
#endif
                        painter( 0 ),
                        map( 0),
                        defaultMap( new ElementColor ),
                        updateCache(true),
                        quickRender(false),
                        renderAxes(false),
                        renderDebug(true),
                        dlistQuick(0), dlistOpaque(0), dlistTransparent(0),
                        pd(0)
    {
      loadEngineFactories();
    }

    ~GLWidgetPrivate()
    {
      if ( selectBuf ) delete[] selectBuf;
      delete camera;
      delete defaultMap;

      // free the display lists
      if (dlistQuick)
        glDeleteLists(dlistQuick, 1);
      if (dlistOpaque)
        glDeleteLists(dlistOpaque, 1);
      if (dlistTransparent)
        glDeleteLists(dlistTransparent, 1);
    }

    void updateListQuick();
    static void loadEngineFactories();
    static QList<EngineFactory *> engineFactories;
    static QHash<QString, EngineFactory *> engineClassFactory;

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

    OBUnitCell            *uc;

    Molecule              *molecule;

    Camera                *camera;

    Tool                  *tool;
    ToolGroup             *toolGroup;

    GLuint                *selectBuf;
    int                    selectBufSize;

    PrimitiveList          selectedPrimitives;
    PrimitiveList          primitives;

    QUndoStack            *undoStack;

    bool                   stable;

#ifdef ENABLE_THREADED_GL
    QWaitCondition         paintCondition;
    QMutex                 renderMutex;

    GLThread              *thread;
#else
    bool                   initialized;
#endif

    GLPainter             *painter;
    Color                 *map; // global color map
    Color *defaultMap;  // default fall-back coloring (i.e., by elements)
    bool                   updateCache; // Update engine caches in quick render?
    bool                   quickRender; // Are we using quick render?
    bool                   renderAxes;  // Should the x, y, z axes be rendered?
    bool                   renderDebug; // Should the debug information be shown?

    GLuint                 dlistQuick;
    GLuint                 dlistOpaque;
    GLuint                 dlistTransparent;

    /**
      * Member GLPainterDevice which is passed to the engines.
      */
    GLPainterDevice *pd;
  };

  QList<EngineFactory *> GLWidgetPrivate::engineFactories;
  QHash<QString, EngineFactory *> GLWidgetPrivate::engineClassFactory;

  void GLWidgetPrivate::loadEngineFactories()
  {
    static bool enginesLoaded = false;
    if(!enginesLoaded)
      {
        QString prefixPath = QString( INSTALL_PREFIX ) + "/lib/avogadro/engines";
        QStringList pluginPaths;
        pluginPaths << prefixPath;

#ifdef WIN32
        pluginPaths << "./engines";
#endif

        if ( getenv( "AVOGADRO_ENGINES" ) != NULL ) {
          pluginPaths = QString( getenv( "AVOGADRO_ENGINES" ) ).split( ':' );
        }

        // load static plugins first

        // now load plugins from paths
        foreach( QString path, pluginPaths ) {
          QDir dir( path );
          foreach( QString fileName, dir.entryList( QDir::Files ) ) {
            QPluginLoader loader( dir.absoluteFilePath( fileName ) );
            QObject *instance = loader.instance();
            EngineFactory *factory = qobject_cast<EngineFactory *>( instance );
            if ( factory )
              {
                engineFactories.append(factory);
                engineClassFactory[factory->className()] = factory;
              }
          }
        }
        enginesLoaded = true;
      }
  }

  void GLWidgetPrivate::updateListQuick()
  {
    // Create a display list cache
    if (updateCache) {
      qDebug() << "Making new quick display lists...";
      if (dlistQuick == 0)
        dlistQuick = glGenLists(1);

      // Don't use dynamic scaling when rendering quickly
      painter->setDynamicScaling(false);

      glNewList(dlistQuick, GL_COMPILE);
      foreach(Engine *engine, engines)
        if(engine->isEnabled())
          engine->renderQuick(pd);
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
      {
        delete d->painter;
      }
    else
      {
        d->painter->decrementShare();
      }

#ifdef ENABLE_THREADED_GL
    // cleanup our thread
    d->thread->stop();
    d->paintCondition.wakeAll();
    d->thread->wait();
#endif

    // delete the engines
    foreach( Engine *engine, d->engines ) {
      delete engine;
    }

    delete( d );
  }

  void GLWidget::constructor(const GLWidget *shareWidget)
  {
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

    setSizePolicy( QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding );
    d->camera->setParent( this );
    setAutoBufferSwap( false );
#ifdef ENABLE_THREADED_GL
    qDebug() << "Threaded GL enabled.";
    d->thread = new GLThread( this, this );
    doneCurrent();
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

  void GLWidget::initializeGL()
  {
    qDebug() << "GLWidget initialisation...";
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

    GLfloat ambientLight[] = { 0.2, 0.2, 0.2, 1.0 };
    GLfloat diffuseLight[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat diffuseLight2[] = { 0.3, 0.3, 0.3, 1.0 };
    GLfloat specularLight[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat specularLight2[] = { 0.5, 0.5, 0.5, 1.0 };
    GLfloat position[] = { 0.8, 0.7, 1.0, 0.0 };
    GLfloat position2[] = { -0.8, 0.7, -0.5, 0.0 };

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
    // Create a second light source to illuminate those shadows a little better
    glLightfv( GL_LIGHT1, GL_AMBIENT, ambientLight );
    glLightfv( GL_LIGHT1, GL_DIFFUSE, diffuseLight2 );
    glLightfv( GL_LIGHT1, GL_SPECULAR, specularLight2 );
    glLightfv( GL_LIGHT1, GL_POSITION, position2 );
    glEnable( GL_LIGHT1 );
    qDebug() << "GLWidget initialised...";

  }

  void GLWidget::resizeEvent( QResizeEvent *event )
  {
    //qDebug() << "resizeEvent";
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
    resizeGL( event->size().width(), event->size().height() );
#endif
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
#ifdef ENABLE_THREADED_GL
    d->renderMutex.unlock();
#endif
  }

  QColor GLWidget::background() const
  {
    return d->background;
  }

  void GLWidget::setColorMap(Color *map)
  {
    d->map = map;
  }

  Color *GLWidget::colorMap() const
  {
    if (d->map)
      return d->map;
    else
      return d->defaultMap;
  }

  void GLWidget::setQuality(int quality)
  {
    d->painter->setQuality(quality);
  }

  int GLWidget::quality() const
  {
    return d->painter->quality();
  }

  void GLWidget::setRenderAxes(bool renderAxes)
  {
    d->renderAxes = renderAxes;
  }

  bool GLWidget::renderAxes()
  {
    return d->renderAxes;
  }

  void GLWidget::setRenderDebug(bool renderDebug)
  {
    d->renderDebug = renderDebug;
  }

  bool GLWidget::renderDebug()
  {
    return d->renderDebug;
  }

  void GLWidget::render()
  {
    d->painter->begin(this);

    // Use renderQuick if the view is being moved, otherwise full render
    if (d->quickRender) {

      d->updateListQuick();
      qDebug() << "Calling quick display lists...";
      glCallList(d->dlistQuick);
      if (d->uc) renderCrystal(d->dlistQuick);

    }
    else {
      qDebug() << "Normal rendering...";

      // we save a display list if we're doing a crystal
      if (d->dlistOpaque == 0)
        d->dlistOpaque = glGenLists(1);
      if (d->dlistTransparent == 0)
        d->dlistTransparent = glGenLists(1);

      if (d->uc) glNewList(d->dlistOpaque, GL_COMPILE);
      foreach(Engine *engine, d->engines)
        if(engine->isEnabled())
          engine->renderOpaque(d->pd);
      if (d->uc) { // end the main list and render the opaque crystal
        glEndList();
        renderCrystal(d->dlistOpaque);
      }

      glDepthMask(GL_FALSE);
      if (d->uc) glNewList(d->dlistTransparent, GL_COMPILE);
      foreach(Engine *engine, d->engines)
        if(engine->isEnabled() && engine->flags() & Engine::Transparent)
          engine->renderTransparent(d->pd);
      if (d->uc) { // end the main list and render the transparent bits
        glEndList();
        renderCrystal(d->dlistTransparent);
      }
      glDepthMask(GL_TRUE);
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

    // Render the active tool
    if ( d->tool ) {
      d->tool->paint( this );
    }

    // If enabled draw the axes
    if (d->renderAxes) renderAxesOverlay();

    // If enabled show debug information
    if (d->renderDebug) renderDebugOverlay();

    d->painter->end();
  }

  void GLWidget::renderCrystal(GLuint displayList)
  {
    std::vector<vector3> cellVectors = d->uc->GetCellVectors();

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
    std::vector<vector3> cellVectors = d->uc->GetCellVectors();
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
    glColor4f(1.0, 1.0, 1.0, 0.7);
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
    MatrixP3d axisTranslation;
    axisTranslation.loadTranslation(d->pd->camera()->transformedXAxis() * 0.04);
    Vector3d aXa = axisTranslation * origin;
    axisTranslation.loadTranslation(d->pd->camera()->transformedXAxis() * 0.06);
    Vector3d aX = axisTranslation * origin;
    axisTranslation.loadTranslation(d->pd->camera()->transformedYAxis() * 0.04);
    Vector3d aYa = axisTranslation * origin;
    axisTranslation.loadTranslation(d->pd->camera()->transformedYAxis() * 0.06);
    Vector3d aY = axisTranslation * origin;
    axisTranslation.loadTranslation(d->pd->camera()->transformedZAxis() * 0.04);
    Vector3d aZa = axisTranslation * origin;
    axisTranslation.loadTranslation(d->pd->camera()->transformedZAxis() * 0.06);
    Vector3d aZ = axisTranslation * origin;

    // Turn off dynamic scaling in the painter (cylinders don't render correctly)
    d->painter->setDynamicScaling(false);

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
    QList<Primitive *> list;

    // Draw all text in while
    d->pd->painter()->setColor(1.0, 1.0, 1.0);

    int x = 5, y = 5;
    y += d->pd->painter()->drawText(x, y, "---- " + tr("Debug Information") + " ----");
    y += d->pd->painter()->drawText(x, y, tr("FPS") + ": " + QString::number(computeFramesPerSecond(), 'g', 3));

    y += d->pd->painter()->drawText(x, y, tr("View Size") + ": "
                                 + QString::number(d->pd->width())
                                 + " x "
                                 + QString::number(d->pd->height()) );

    list = primitives().subList(Primitive::AtomType);
    y += d->pd->painter()->drawText(x, y, tr("Atoms") + ": " + QString::number(list.size()));

    list = primitives().subList(Primitive::BondType);
    y += d->pd->painter()->drawText(x, y, tr("Bonds") + ": " + QString::number(list.size()));
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

  void GLWidget::paintEvent( QPaintEvent * )
  {
    //qDebug() << "paintEvent";
    makeCurrent();
#ifdef ENABLE_THREADED_GL
    // tell our thread to paint
    d->paintCondition.wakeAll();
    doneCurrent();
#else
    if(!d->initialized)
      {
        d->initialized = true;
        initializeGL();
      }
    qglClearColor(d->background);
    paintGL();
    swapBuffers();
#endif
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
    if ( d->tool ) {
      QUndoCommand *command = 0;
      command = d->tool->mousePress( this, event );

      if ( command && d->undoStack ) {
        d->undoStack->push( command );
      } else if ( command ) {
        delete command;
      }
    }
#ifdef ENABLE_THREADED_GL
    d->renderMutex.lock();
#endif
    // Use quick render while the mouse is down
    d->quickRender = true;
#ifdef ENABLE_THREADED_GL
    d->renderMutex.unlock();
#endif
  }

  void GLWidget::mouseReleaseEvent( QMouseEvent * event )
  {
    if ( d->tool ) {
      QUndoCommand *command = d->tool->mouseRelease( this, event );

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
  }

  void GLWidget::mouseMoveEvent( QMouseEvent * event )
  {
    if ( d->tool ) {
      QUndoCommand *command = d->tool->mouseMove( this, event );
      if ( command && d->undoStack ) {
        d->undoStack->push( command );
      }
    }
  }

  void GLWidget::wheelEvent( QWheelEvent * event )
  {
    if ( d->tool ) {
      QUndoCommand *command = d->tool->wheel( this, event );
      if ( command && d->undoStack ) {
        d->undoStack->push( command );
      }
    }
  }

  void GLWidget::setMolecule( Molecule *molecule )
  {
    if ( !molecule ) { return; }

    // disconnect from our old molecule
    if ( d->molecule ) {
      QObject::disconnect( d->molecule, 0, this, 0 );
    }

    d->molecule = molecule;

    // clear our engine queues
    for ( int i=0; i < d->engines.size(); i++ ) {
      d->engines.at( i )->clearPrimitives();
    }
    d->primitives.clear();

    // add the atoms to the default queue
    std::vector<OpenBabel::OBNodeBase*>::iterator i;
    for ( Atom *atom = ( Atom* )d->molecule->BeginAtom( i );
          atom; atom = ( Atom* )d->molecule->NextAtom( i ) ) {
      d->primitives.append( atom );
    }

    // add the bonds to the default queue
    std::vector<OpenBabel::OBEdgeBase*>::iterator j;
    for ( Bond *bond = ( Bond* )d->molecule->BeginBond( j );
          bond; bond = ( Bond* )d->molecule->NextBond( j ) ) {
      d->primitives.append( bond );
    }

    // add the residues to the default queue
    std::vector<OpenBabel::OBResidue*>::iterator k;
    for ( Residue *residue = ( Residue* )d->molecule->BeginResidue( k );
          residue; residue = ( Residue * )d->molecule->NextResidue( k ) ) {
      d->primitives.append( residue );
    }

    d->primitives.append( d->molecule );

    std::cout << "SetMolecule Called!" << std::endl;
    // Now set the primitives for the engines
    for (int i = 0; i < d->engines.size(); i++)
      d->engines.at(i)->setPrimitives(d->primitives);

    // connect our signals so if the molecule gets updated
    connect( d->molecule, SIGNAL( primitiveAdded( Primitive* ) ),
             this, SLOT( addPrimitive( Primitive* ) ) );
    connect( d->molecule, SIGNAL( primitiveUpdated( Primitive* ) ),
             this, SLOT( updatePrimitive( Primitive* ) ) );
    connect( d->molecule, SIGNAL( primitiveRemoved( Primitive* ) ),
             this, SLOT( removePrimitive( Primitive* ) ) );

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

  const Vector3d & GLWidget::center() const
  {
    return d->center;
  }

  const Vector3d & GLWidget::normalVector() const
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
    if (d->molecule->HasData(OBGenericDataType::UnitCell))
      d->uc = dynamic_cast<OBUnitCell*>(d->molecule->GetData(OBGenericDataType::UnitCell));

    if ( !d->uc ) { // a plain molecule, no crystal cell
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
      std::vector<vector3> cellVectors = d->uc->GetCellVectors();
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
      if(d->molecule->NumAtoms() == 0 ) {
        d->farthestAtom = 0;
      }
      // Second case: there is no repetition of the molecule
      else if(d->aCells <= 1 && d->bCells <= 1 && d->cCells <= 1) {
        d->farthestAtom = d->molecule->farthestAtom();
      }
      // General case: the farthest atom is the one that is located the
      // farthest in the direction pointed to by centerOffset.
      else {
        std::vector<OBAtom*>::iterator atom_iterator;
        Atom *atom;
        double x, max_x;
        for(
            atom = static_cast<Atom*>(d->molecule->BeginAtom(atom_iterator)),
              max_x = centerOffset.dot(atom->pos()),
              d->farthestAtom = atom;
            atom;
            atom = static_cast<Atom*>(d->molecule->NextAtom(atom_iterator))
            ) {
          x = centerOffset.dot(atom->pos());
          if(x > max_x)
            {
              max_x = x;
              d->farthestAtom = atom;
            }
        }
      }
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

  QList<EngineFactory *> GLWidget::engineFactories() const
  {
    return d->engineFactories;
  }

  PrimitiveList GLWidget::primitives() const
  {
    return d->primitives;
  }

  void GLWidget::addPrimitive( Primitive *primitive )
  {
    if ( primitive ) {
      // add the molecule to the default queue
      for ( int i=0; i < d->engines.size(); i++ ) {
        d->engines.at( i )->addPrimitive( primitive );
      }
      d->primitives.append( primitive );
      // The engine caches must be invalidated
      d->updateCache = true;
    }
  }

  void GLWidget::updatePrimitive( Primitive *primitive )
  {
    for ( int i=0; i< d->engines.size(); i++ ) {
      d->engines.at( i )->updatePrimitive( primitive );
    }
    // The engine caches must be invalidated
    d->updateCache = true;
    updateGeometry();
  }

  void GLWidget::removePrimitive( Primitive *primitive )
  {
    if ( primitive ) {
      // add the molecule to the default queue
      for ( int i=0; i < d->engines.size(); i++ ) {
        d->engines.at( i )->removePrimitive( primitive );
      }
      d->selectedPrimitives.removeAll( primitive );
      d->primitives.removeAll( primitive );
      // The engine caches must be invalidated
      d->updateCache = true;
    }
  }

  void GLWidget::addEngine(Engine *engine)
  {
    connect( engine, SIGNAL( changed() ), this, SLOT( update() ) );
    d->engines.append( engine );
    qSort( d->engines.begin(), d->engines.end(), engineLessThan );
    emit engineAdded(engine);
    update();
  }

  void GLWidget::removeEngine(Engine *engine)
  {
    connect( engine, SIGNAL( changed() ), this, SLOT( update() ) );
    d->engines.removeAll( engine );
    emit engineRemoved(engine);
    engine->deleteLater();
    update();
  }

  void GLWidget::setTool( Tool *tool )
  {
    if ( tool ) {
      d->tool = tool;
    }
  }

  void GLWidget::setToolGroup( ToolGroup *toolGroup )
  {
    if ( d->toolGroup ) {
      disconnect( d->toolGroup, 0, this, 0 );
    }

    if ( toolGroup ) {
      d->toolGroup = toolGroup;
      d->tool = toolGroup->activeTool();
      connect( toolGroup, SIGNAL( toolActivated( Tool* ) ),
               this, SLOT( setTool( Tool* ) ) );
    }
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
    //    QReadLocker(d->molecule->lock());
    QList<GLHit> hits;

    if ( !molecule() ) return hits;

    GLint viewport[4];
    unsigned int hit_count;

    int cx = w/2 + x;
    int cy = h/2 + y;

    // setup the selection buffer
    int requiredSelectBufSize = ( d->molecule->NumAtoms() + d->molecule->NumBonds() ) * 8;
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
    makeCurrent();
#endif
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

    // now actually render using low quality a.k.a. "quickrender"
    bool oldQuickRender = d->quickRender;
    d->quickRender = true;
    render();
    d->quickRender = oldQuickRender;

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
      for ( i = 0; i < hit_count && !( ptr > d->selectBuf + d->selectBufSize ); i++ ) {
        names = *ptr++;
        // make sure that we won't be passing the end of bufer
        if ( ptr + names + 2 > d->selectBuf + d->selectBufSize ) {
          break;
        }
        minZ = *ptr++;
        maxZ = *ptr++;

        // allow names of 0
        name = -1;
        for ( j = 0; j < names/2; j++ ) { /*  for each name */
          type = *ptr++;
          name = *ptr++;
        }
        if ( name > -1 ) {
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
    foreach( GLHit hit, chits )
      {
        //qDebug() << "Hit: " << hit.name();
        if(hit.type() == Primitive::AtomType) {
          return static_cast<Atom *>(molecule()->GetAtom(hit.name()));
        } else if(hit.type() == Primitive::BondType) {
          return static_cast<Bond *>(molecule()->GetBond(hit.name()));
        }
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
    foreach( GLHit hit, chits )
      {
        if(hit.type() == Primitive::AtomType)
          {
            return static_cast<Atom *>(molecule()->GetAtom(hit.name()));
          }
      }
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
    foreach( GLHit hit, chits )
      {
        if(hit.type() == Primitive::BondType)
          {
            return static_cast<Bond *>(molecule()->GetBond(hit.name()));
          }
      }
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
    double radius = 0.0;
    foreach( Engine *engine, d->engines ) {
      if ( engine->isEnabled() ) {
        double engineRadius = engine->radius( d->pd, p );
        if ( engineRadius > radius ) {
          radius = engineRadius;
        }
      }
    }

    return radius;
  }

  bool GLWidget::isStable() const
  {
    return d->stable;
  }

  void GLWidget::setStable( bool stable )
  {
    d->stable = stable;
  }

  void GLWidget::setSelected(PrimitiveList primitives, bool select)
  {
    foreach( Primitive *item, primitives )
    {
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

  void GLWidget::clearSelected()
  {
    d->selectedPrimitives.clear();
    // The engine caches must be invalidated
    d->updateCache = true;
  }

  bool GLWidget::isSelected( const Primitive *p ) const
  {
    // Return true if the item is selected
    return d->selectedPrimitives.contains( const_cast<Primitive *>( p ) );
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
    settings.setValue("renderAxes", d->renderAxes);
    settings.setValue("renderDebug", d->renderDebug);

    int count = d->engines.size();
    settings.beginWriteArray("engines");
    for(int i = 0; i< count; i++)
      {
        settings.setArrayIndex(i);
        Engine *engine = d->engines.at(i);
        settings.setValue("engineClass", engine->metaObject()->className());
        engine->writeSettings(settings);
      }
    settings.endArray();
  }

  void GLWidget::readSettings(QSettings &settings)
  {
    // Make sure to provide some default values for any settings.value("", DEFAULT) call
    d->painter->setQuality(settings.value("quality", 2).toInt());
    d->background = settings.value("background", QColor(0,0,0)).value<QColor>();
    d->renderAxes = settings.value("renderAxes", 1).value<bool>();
    d->renderDebug = settings.value("renderDebug", 0).value<bool>();

    int count = settings.beginReadArray("engines");
    for(int i=0; i<count; i++)
      {
        settings.setArrayIndex(i);
        QString engineClass = settings.value("engineClass", QString()).toString();

        if(!engineClass.isEmpty() && d->engineClassFactory.contains(engineClass))
          {
            EngineFactory *factory = d->engineClassFactory.value(engineClass);
            Engine *engine = factory->createInstance(this);
            engine->readSettings(settings);

            // eventually settings will store which has what but
            // for now we ignore this.  (will need this when we
            // copy the selected primitives also).
            if(!engine->primitives().size())
              {
                engine->setPrimitives(primitives());
              }

            addEngine(engine);
          }
      }
    settings.endArray();

    if(!d->engines.count())
      {
        loadDefaultEngines();
      }
  }

  void GLWidget::loadDefaultEngines()
  {
    QList<Engine *> engines = d->engines;

    d->engines.clear();
    foreach(Engine *engine, engines)
      {
        delete engine;
      }

    foreach(EngineFactory *factory, d->engineFactories)
      {
        Engine *engine = factory->createInstance(this);
        if ( engine->name() == tr("Ball and Stick") ) {
          engine->setEnabled( true );
        }
        engine->setPrimitives(primitives());
        addEngine(engine);
      }
  }
}

#include "glwidget.moc"
