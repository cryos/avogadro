/*
 *      Copyright (c) 2011 Mickael Gadroy
 *
 *	This file is part of example_qt of the wiiusecpp library.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


 #include <QtGui>
 #include <QtOpenGL>

 #include <math.h>

 #include "glwidget.h"
 #include "qtlogo.h"

#include "window.h"

 #ifndef GL_MULTISAMPLE
 #define GL_MULTISAMPLE  0x809D
 #endif


 GLWidget::GLWidget(Window *parent)
     : QGLWidget(QGLFormat(QGL::SampleBuffers), parent),
       m_window(parent),
       m_xRot(0), m_yRot(0), m_zRot(0),
       m_xTransl(0), m_yTransl(0), m_zTransl(0),
       m_xCursor(0),  m_yCursor(0), m_zCursor(0)
 {
     m_purple = QColor::fromCmykF(0.39, 0.39, 0.0, 0.0);

     m_gluQuadricParams1 = gluNewQuadric() ;
     gluQuadricDrawStyle( m_gluQuadricParams1, GLU_FILL ) ;

     this->setMouseTracking( true ) ;

     connect( this, SIGNAL(xyzPositionsChanged(double,double,double,double,double,double,double,double,double)),
              m_window, SLOT(setWmTable(double,double,double,double,double,double,double,double,double)) ) ;
 }

 GLWidget::~GLWidget()
 {
     gluDeleteQuadric( m_gluQuadricParams1 ) ;
 }

 QSize GLWidget::minimumSizeHint() const
 {
     return QSize(100, 100);
 }

 QSize GLWidget::sizeHint() const
 {
     return QSize(400, 400);
 }

 static void qNormalizeAngle(int &angle)
 {
     while (angle < 0)
         angle += 360 * SLIDER_ROT_CONST;
     while (angle > 360 * SLIDER_ROT_CONST)
         angle -= 360 * SLIDER_ROT_CONST;
 }

 void GLWidget::setXRotation(int angle)
 {
     qNormalizeAngle(angle);
     if (angle != m_xRot)
     {
         m_xRot = angle;
         emit xRotationChanged(angle);
         updateGL();
     }
 }

 void GLWidget::setYRotation(int angle)
 {
     qNormalizeAngle(angle);
     if (angle != m_yRot)
     {
         m_yRot = angle;
         emit yRotationChanged(angle);
         updateGL();
     }
 }

 void GLWidget::setZRotation(int angle)
 {
     qNormalizeAngle(angle);
     if (angle != m_zRot)
     {
         m_zRot = angle;
         emit zRotationChanged(angle);
         updateGL();
     }
 }

 static float qNormalizeDistance( float distance )
 {
     if( distance < 0)
         distance = 0 ;
     if( distance > SLIDER_TRANS_MAX )
         distance = SLIDER_TRANS_MAX ;

     return distance / SLIDER_TRANS_CONST ;
 }

 void GLWidget::setXTranslation( int distance )
 {
     float dist=qNormalizeDistance( distance ) ;

     if( dist != m_xTransl )
     {
         m_xTransl = dist ;
         updateGL() ;
     }
 }

 void GLWidget::setYTranslation( int distance )
 {
     float dist=qNormalizeDistance( distance ) ;

     if( dist != m_yTransl )
     {
         m_yTransl = dist ;
         updateGL() ;
     }
 }

 void GLWidget::setZTranslation( int distance )
 {
     float dist=qNormalizeDistance( distance ) ;

     if( dist != m_zTransl )
     {
         m_zTransl = dist ;
         updateGL() ;
     }
 }

 void GLWidget::setTranslation( float x, float y, float z )
 {
    m_zTransl = x ;
    m_yTransl = y ;
    m_zTransl = z ;
    updateGL() ;
 }

 void GLWidget::initializeGL()
 {
     qglClearColor(m_purple.dark());

     glEnable(GL_DEPTH_TEST);
     glEnable(GL_CULL_FACE);
     glShadeModel(GL_SMOOTH);
     glEnable(GL_LIGHTING);
     glEnable(GL_LIGHT0);
     glEnable(GL_MULTISAMPLE);
     static GLfloat lightPosition[4] = { 0.5, 5.0, 7.0, 1.0 };
     glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

     // PI/2
     glViewport(0, 0, width(), height()) ;

     glMatrixMode( GL_PROJECTION ) ;
     glLoadIdentity() ;

     GLdouble aspect=(double)width()/(double)height() ;
     GLdouble fov=3.14*0.50 ;
     gluPerspective( fov, aspect, 1, 1000 );
 }

 void GLWidget::paintGL()
 {
     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
     glMatrixMode( GL_MODELVIEW ) ;
     glLoadIdentity() ;

     // fonction pour fixer la matrice de vue
     // prend en paramètre les coordonnées de la position de la "caméra"
     // les coordonnées du point visé et le vecteur haut de la caméra
     gluLookAt( 10, 0, 10, 0, 0, 0, 0, 1, 0 );

     // Movement feature.
     /*
     glPushMatrix() ;
     //if( m_extension != NULL )
        //m_extension->translateObject() ; // Traitement lié à mon extension.
     glTranslatef(m_xTransl, m_yTransl, m_zTransl) ;
     glRotatef(m_xRot / SLIDER_ROT_CONST, 1.0, 0.0, 0.0) ;
     glRotatef(m_yRot / SLIDER_ROT_CONST, 0.0, 1.0, 0.0) ;
     glRotatef(m_zRot / SLIDER_ROT_CONST, 0.0, 0.0, 1.0) ;

     drawSphere( 0.1f, 0.8f, 0.8f, 0.8f ) ;
     glPopMatrix();
     */

     // Constant sphere.
     /*
     glPushMatrix() ;
     drawSphere( 0.09f, 0.8f, 0.8f, 0.8f ) ;
     glPopMatrix();
     */

     // Cursor
     // Draw in ortho projection.
     glPushMatrix() ;
     drawCursor( m_xCursor, m_yCursor, m_zCursor ) ;
     glPopMatrix() ;
 }

 void GLWidget::resizeGL(int width, int height)
 {
     glViewport(0, 0, width, height) ;

     glMatrixMode( GL_PROJECTION ) ;
     glLoadIdentity();

     GLdouble aspect=(double)width/(double)height ;
     GLdouble fov=3.14*0.50 ;
     gluPerspective( fov, aspect, 1, 1000 );

     emit xyzPositionsChanged( width, height, this->width(), this->height(), 0, 0, 0, 0, 0 ) ;
 }

 void GLWidget::mousePressEvent(QMouseEvent *event)
 {
     // Dedicated to the rotation.
     m_lastPosRotation = event->pos();
 }

 void GLWidget::mouseMoveEvent(QMouseEvent *event)
 {
     // Dedicated to the rotation.
     int dx = event->x() - m_lastPosRotation.x() ;
     int dy = event->y() - m_lastPosRotation.y() ;

     if (event->buttons() & Qt::LeftButton)
     {
         setXRotation(m_xRot + 8 * dy) ;
         setYRotation(m_yRot + 8 * dx) ;
     }
     else if (event->buttons() & Qt::RightButton)
     {
         setXRotation(m_xRot + 8 * dy) ;
         setZRotation(m_zRot + 8 * dx) ;
     }
     m_lastPosRotation = event->pos() ;

     // Dedicated to the cursor.
     double xOpenGL=0, yOpenGL=0, zOpenGL=0 ;
     calculateOpenGLPositionFromScreenCoordinate( event->pos(), xOpenGL, yOpenGL, zOpenGL ) ;
     m_xCursor = xOpenGL ;
     m_yCursor = yOpenGL ;
     m_zCursor = zOpenGL ;

     updateGL() ;
 }

 void GLWidget::calculateOpenGLPositionFromScreenCoordinate( const QPoint &pos_in, double &x_out, double &y_out, double &z_out )
 {
     GLint viewport[4] ;       // Where The Viewport Values Will Be Stored
     GLdouble modelview[16] ;  // Where The 16 Doubles Of The Modelview Matrix Are To Be Stored
     GLdouble projection[16] ; // Where The 16 Doubles Of The Projection Matrix Are To Be Stored
     GLfloat winX=0, winY=0, winZ=0 ;
     GLdouble posX, posY, posZ ;

     glGetDoublev( GL_MODELVIEW_MATRIX, modelview ) ;   // Retrieve The Modelview Matrix
     glGetDoublev( GL_PROJECTION_MATRIX, projection ) ; // Retrieve The Projection Matrix
     glGetIntegerv( GL_VIEWPORT, viewport ) ;           // Retrieves The Viewport Values (X, Y, Width, Height)

     winX = (float)pos_in.x() ;
     winY = (float)viewport[3] - (float)pos_in.y() ;

     // Get the 1st intersection with the openGL object.
     //glReadPixels( x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );

     // Widget position to openGL position.
     gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

     x_out = posX ;
     y_out = posY ;
     z_out = posZ ;
 }

 void GLWidget::drawSphere( float radius, float r, float v, float b  )
 {
     // Init & Save.
     glPushAttrib( GL_ALL_ATTRIB_BITS ) ;

     glColor3f( r, v, b ) ;
     gluSphere( m_gluQuadricParams1, radius, 10, 10) ;
     // params : GLUquadric = options
     // 0.75 : radius : rayon de la sphère.
     // 20 : slices : lontitudes.
     // 20 : stacks : latitudes.

     glPopAttrib();
 }

 void GLWidget::drawCursor( float x, float y, float z )
 {
     GLdouble modelview[16] ;  // Where The 16 Doubles Of The Modelview Matrix Are To Be Stored
     glGetDoublev( GL_MODELVIEW_MATRIX, modelview ) ;   // Retrieve The Modelview Matrix

     GLdouble rightX=modelview[0] ;
     GLdouble rightY=modelview[4] ;
     GLdouble rightZ=modelview[8] ;

     GLdouble upX=modelview[1] ;
     GLdouble upY=modelview[5] ;
     GLdouble upZ=modelview[9] ;

     //emit xyzPositionsChanged( rightX, rightY, rightZ, upX, upY, upZ, x, y, z ) ;

     //glTranslatef( 0.0f, 0.0f, 0.0f ) ;

    // Will be transparent
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    // Draw the cursor at the current mouse pos

    glBegin( GL_TRIANGLES ) ;
        glColor4f( 0.75, 0.75, 0.75, 0.75 ) ;
        glVertex3f( x, y, z ) ;
        glVertex3f( x+0.001f*rightX, y+0.001*rightY, z+0.001*rightZ ) ;
        glVertex3f( x+0.001f*upX, y+0.001f*upY, z+0.001f*upZ ) ;
    glEnd() ;

    // avoir right et up comme pour le triangle
    float r=0.001 ;
    float a=0, b=0 ;
    glBegin( GL_LINE_LOOP );
    for( float i=0 ; i<2*M_PI ; i+=(float)M_PI/10.0f )
    {
        a = r * cosf(i) ;
        b = r * sin(i) ;
        float xf = x + rightX * a + upX * b ;
        float yf = y + rightY * a + upY * b ;
        float zf = z + rightZ * a + upZ * b ;
        glVertex3f( xf, yf, zf ) ;
    }
    glEnd();

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
 }
