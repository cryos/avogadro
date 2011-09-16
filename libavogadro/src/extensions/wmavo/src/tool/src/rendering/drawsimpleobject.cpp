
/*******************************************************************************
  Copyright (C) 2011 Mickael Gadroy

  This file is part of WmAvo (WiiChem project)
  WmAvo - Integrate the Wiimote and the Nunchuk in Avogadro software for the
  handling of the atoms and the camera.
  For more informations, see the README file.

  WmAvo is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  WmAvo is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with WmAvo. If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

#include "drawsimpleobject.h"


const Eigen::Vector3d DrawSimpleObject::m_vect3d0(0,0,0) ;
const double DrawSimpleObject::m_PI=103993.0/33102.0 ; // PI approximation.


DrawSimpleObject::DrawSimpleObject( Avogadro::GLWidget *widget )
  : m_widget(widget), m_painter(NULL), m_qglwidget(NULL)
{
  if( m_widget != NULL )
  {
    m_painter = widget->painter() ;
    m_qglwidget = dynamic_cast<QGLWidget*>(m_widget) ;

    // Initiate my projection matrix.
    double w=(double)(m_widget->width()) ;
    double h=(double)(m_widget->height()) ;
    GLdouble fovy=m_widget->camera()->angleOfViewY() ;
    GLdouble aspect=w/h ;
    GLdouble nearPlan=1.0 ;
    GLdouble farPlan=100.0 ;

    glMatrixMode( GL_PROJECTION ) ;
    glPushMatrix() ;
      glLoadIdentity() ;
      gluPerspective( fovy, aspect, nearPlan, farPlan ) ;
      glGetFloatv( GL_MODELVIEW_MATRIX, m_projectionMatrix ) ;
    glPopMatrix() ;
    glMatrixMode(GL_MODELVIEW) ;
  }
  else
  {
    for( int i=0 ; i<16 ; i++ )
      m_projectionMatrix[i] = 0 ;
  }

  m_gluQuadricGluLine = gluNewQuadric() ;
  gluQuadricDrawStyle( m_gluQuadricGluLine, GLU_LINE ) ;

  m_gluQuadricGluFill = gluNewQuadric() ;
  gluQuadricDrawStyle( m_gluQuadricGluFill, GLU_FILL ) ;
}

DrawSimpleObject::~DrawSimpleObject()
{
  gluDeleteQuadric( m_gluQuadricGluLine ) ;
  gluDeleteQuadric( m_gluQuadricGluFill ) ;
}


/**
  * Get the orientation directions of XYZ-axis.
  * @param rightX_out X component of X-axis.
  * @param rightY_out Y component of X-axis.
  * @param rightZ_out Z component of X-axis.
  * @param upX_out X component of Y-axis. 
  * @param upY_out Y component of Y-axis.
  * @param upZ_out Z component of Y-axis.
  * @param dirX_out X component of Z-axis.
  * @param dirY_out Y component of Z-axis.
  * @param dirZ_out Z component of Z-axis.
  */
void DrawSimpleObject::getRightUpDirectionCamera
      ( double &rightX_out, double &rightY_out, double &rightZ_out, 
        double &upX_out,    double &upY_out,    double &upZ_out,
        double &dirX_out,   double &dirY_out,   double &dirZ_out )
{
  GLdouble modelview[16] ;  // Where The 16 Doubles Of The Modelview Matrix Are To Be Stored
  glGetDoublev( GL_MODELVIEW_MATRIX, modelview ) ;   // Retrieve The Modelview Matrix

  rightX_out = modelview[0] ;
  rightY_out = modelview[4] ;
  rightZ_out = modelview[8] ;

  upX_out = modelview[1] ;
  upY_out = modelview[5] ;
  upZ_out = modelview[9] ;

  dirX_out = -modelview[2] ;
  dirY_out = -modelview[6] ;
  dirZ_out = -modelview[10] ;
}


/**
  * Convert a screen position to an openGl position on the near plan.
  * @param p_in The screen position in input.
  * @param X_out The X component position in ouput.
  * @param Y_out The Y component position in ouput.
  * @param Z_out The Z component position in ouput.
  */
void DrawSimpleObject::getPosScreenToPosOpenGL
        ( const QPoint &p_in, double &X_out, double &Y_out, double &Z_out )
{
  GLint viewport[4] ;       // Where The Viewport Values Will Be Stored
  GLdouble modelview[16] ;  // Where The 16 Doubles Of The Modelview Matrix Are To Be Stored
  GLdouble projection[16] ; // Where The 16 Doubles Of The Projection Matrix Are To Be Stored

  glGetDoublev( GL_MODELVIEW_MATRIX, modelview ) ;   // Retrieve The Modelview Matrix
  glGetDoublev( GL_PROJECTION_MATRIX, projection ) ; // Retrieve The Projection Matrix
  glGetIntegerv( GL_VIEWPORT, viewport ) ;           // Retrieves The Viewport Values (X, Y, Width, Height)

  QPoint p=m_widget->mapFromGlobal(p_in) ;

  GLfloat winX=(float)p.x() ;
  GLfloat winY=(float)viewport[3] - (float)p.y() ;
  GLfloat winZ=0 ;

  // Get the 1st intersection with the openGL object.
  //glReadPixels( x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );

  // Widget position to openGL position.
  gluUnProject( winX, winY, winZ, modelview, projection, viewport, &X_out, &Y_out, &Z_out);
}


/**
  * Print the last openGL error in a terminal.
  */
void DrawSimpleObject::printOpenGLError()
{
  GLenum errCode ;
  const GLubyte *errString ;
  if( (errCode=glGetError()) != GL_NO_ERROR )
  {
    errString = gluErrorString( errCode ) ;
    fprintf (stderr, "OpenGL Error: %s\n", errString);
  }
}

/**
  * Print the modelview and projection matrix in a terminal.
  */
void DrawSimpleObject::printModelviewAndProjectionMatrix()
{
  GLdouble modelview[16] ;  // Where The 16 Doubles Of The Modelview Matrix Are To Be Stored
  GLdouble projection[16] ; // Where The 16 Doubles Of The Projection Matrix Are To Be Stored
  glGetDoublev( GL_MODELVIEW_MATRIX, modelview ) ;   // Retrieve The Modelview Matrix
  glGetDoublev( GL_PROJECTION_MATRIX, projection ) ; // Retrieve The Projection Matrix
  printf( "modelview: " ) ;
  for( int i=0 ; i<16 ; i++ )
    printf( " %.3lf", modelview[i] ) ;
  printf( "\n" ) ;
  printf( "projection:" ) ;
  for( int i=0 ; i<16 ; i++ )
    printf( " %.3lf", projection[i] ) ;
  printf( "\n" ) ;
}



/**
  * Print the 3D stereoscopic possibilities of the software and the hardware.
  */
void DrawSimpleObject::print3DStereoPossibilities()
{
  // 1
  QGLFormat fmt ;
  fmt.setStereo( true ) ;
  printf( "3D Stereo by QGLFormat:%d\n", (int)(fmt.stereo()) ) ;

  // 2
  GLboolean hasStereo[1] ;
  glGetBooleanv(GL_STEREO, hasStereo) ;
  printf( "3D Stereo by glGetBooleanv:%d\n", (int)(hasStereo[0]) ) ;

  printOpenGLError() ;

  // 3
  if( m_widget == NULL )
  {
    printf( "DrawSimpleObject::print3DStereoPossibilities: m_widget==NULL !\n" ) ;
  }
  else
  {
    QGLFormat fmt2=m_widget->format() ;
    fmt2.setStereo(true) ;
    m_widget->setFormat(fmt) ;
    bool hasStereo2=m_widget->format().stereo() ;
    printf( "3D Stereo by m_widget.QGLFormat:%d\n", (int)(hasStereo2) ) ;

    if( !hasStereo2 )
    {
      // ok, goggles off
      if (!m_widget->format().hasOverlay())
        printf( "Cool hardware required !!!\n" ) ;
      else
        printf( "Cool hardware OK !\n" ) ;
    }
  }
}


/**
  * Draw a line.
  * @param begin Position on one side.
  * @param end Position on the other side.
  * @param colorBegin Color of the line (on one side).
  * @param colorEnd Color of the line (on the other side).
  * @param lineWidth Width of the line.
  */
void DrawSimpleObject::drawLine( const Eigen::Vector3d& begin, const Eigen::Vector3d& end, const QColor &colorBegin, const QColor &colorEnd, float lineWidth )
{
  unsigned char r, g, b, a ;
  unsigned char r2, g2, b2, a2 ;

  r = (unsigned char)( colorBegin.red() ) ;
  g = (unsigned char)( colorBegin.green() ) ;
  b = (unsigned char)( colorBegin.blue() ) ;
  a = (unsigned char)( colorBegin.alpha() ) ;
  
  r2 = (unsigned char)( colorEnd.red() ) ;
  g2 = (unsigned char)( colorEnd.green() ) ;
  b2 = (unsigned char)( colorEnd.blue() ) ;
  a2 = (unsigned char)( colorEnd.alpha() ) ;


  glPushAttrib( GL_ALL_ATTRIB_BITS ) ;

  // If active, do not paint the atom !!!
  // glLoadIdentity();

  // If no active, do not change color !!!
  glDisable( GL_LIGHTING ) ;

  glLineWidth( lineWidth ) ;

  glBegin( GL_LINES ) ;
    glColor4ub( r, g, b, a ) ;
    glVertex3d( begin[0], begin[1], begin[2] ) ;
    glColor4ub( r2, g2, b2, a2 ) ;
    glVertex3d( end[0], end[1], end[2] ) ;
  glEnd() ;

  glPopAttrib() ;
}


/**
  * Draw a rectangle in the render zone. If color==NULL, the inverse color of the 
  * background is used.
  * @param p1 The up/left position of the rectangle.
  * @param p2 The bottom/right position of the rectangle.
  * @param color RGBA components.
  */
void DrawSimpleObject::drawRect( const QPoint &p1, const QPoint &p2, const QColor* color )
{
  drawRect( (float)p1.x(), (float)p1.y(), (float)p2.x(), (float)p2.y(), color ) ;
}


/**
  * Draw a rectangle in the render zone. If color==NULL, the inverse color of the 
  * background is used.
  * @param sx The up/left position in x of the rectangle.
  * @param sy The up/left position in y of the rectangle.
  * @param ex The bottom/right position in x of the rectangle.
  * @param ey The bottom/right position in y of the rectangle.
  * @param color RGBA components.
  */
void DrawSimpleObject::drawRect( float sx, float sy, float ex, float ey, const QColor* color )
{

  unsigned char rBg=0, gBg=0, bBg=0, aBg=0 ;
  unsigned char rLimit=0, gLimit=0, bLimit=0 ;
  QColor bgColor=m_widget->background() ;
  unsigned char rWBg=0, gWBg=0, bWBg=0 ;

  rWBg = (unsigned char)(bgColor.red()) ;
  gWBg = (unsigned char)(bgColor.green()) ;
  bWBg = (unsigned char)(bgColor.blue()) ;

  if( color == NULL )
  { // Calculate the inverse color.
    
    rBg = 255 - rWBg ;
    gBg = 255 - gWBg ;
    bBg = 255 - bWBg ;
    aBg = 150 ;
  }
  else
  {
    rBg=(unsigned char)(color->red()) ; bBg=(unsigned char)(color->blue()) ; 
    gBg=(unsigned char)(color->green()) ; aBg=(unsigned char)(color->alpha()) ;
  }

  //printf( "%d %d %d - %d %d %d\n", rBg, gBg, bBg, bgColor.red(), bgColor.green(), bgColor.blue() ) ;

  rLimit = ( rBg>=rWBg ? rBg-rWBg : rWBg-rBg ) >> 1 ; // /2
  gLimit = ( gBg>=gWBg ? gBg-gWBg : gWBg-gBg ) >> 1 ;
  bLimit = ( bBg>=bWBg ? bBg-bWBg : bWBg-bBg ) >> 1 ;

  glPushMatrix();
  glLoadIdentity();

  GLdouble projection[16];
  glGetDoublev(GL_PROJECTION_MATRIX,projection);
  GLdouble modelview[16];
  glGetDoublev(GL_MODELVIEW_MATRIX,modelview);
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT,viewport);

  GLdouble startPos[3], endPos[3] ;

  gluUnProject( double(sx), double(viewport[3])-double(sy), 0.1, 
                modelview, projection, viewport, 
                &startPos[0], &startPos[1], &startPos[2] ) ;
  gluUnProject( double(ex), double(viewport[3])-double(ey), 0.1, 
                modelview, projection, viewport, 
                &endPos[0], &endPos[1], &endPos[2] ) ;

  glPushAttrib(GL_ALL_ATTRIB_BITS) ;
  glPushMatrix() ;
  glLoadIdentity() ;
  glEnable(GL_BLEND) ;
  glDisable(GL_LIGHTING) ;
  glDisable(GL_CULL_FACE) ;

  glColor4ub( rBg, gBg, bBg, aBg ) ;

  glBegin(GL_POLYGON);
    glVertex3d(startPos[0],startPos[1],startPos[2]);
    glVertex3d(startPos[0],endPos[1],startPos[2]);
    glVertex3d(endPos[0],endPos[1],startPos[2]);
    glVertex3d(endPos[0],startPos[1],startPos[2]);
  glEnd();

  startPos[2] += 0.0001;
  glDisable(GL_BLEND);

  glColor3ub( rLimit, gLimit, bLimit ) ;
  glBegin(GL_LINE_LOOP);
    glVertex3d(startPos[0],startPos[1],startPos[2]);
    glVertex3d(startPos[0],endPos[1],startPos[2]);
    glVertex3d(endPos[0],endPos[1],startPos[2]);
    glVertex3d(endPos[0],startPos[1],startPos[2]);
  glEnd();

  glPopMatrix();
  glPopAttrib();
  glPopMatrix();
}


/**
  * Draw a circle.
  * param posX X-axis position component
  * param posY Y-axis position component
  * param posZ Z-axis position component
  * param rightX X-axis right component
  * param rightY Y-axis right component
  * param rightZ Z-axis right component
  * param upX X-axis up component
  * param upY Y-axis up component
  * param upZ Z-axis up component
  * param dirX X-axis direction component
  * param dirY Y-axis direction component
  * param dirZ Z-axis direction component
  * param radius The radius of the circle
  * param color The color of the circle
  */
void DrawSimpleObject::drawCircle( double posX,   double posY,   double posZ, 
                                   double rightX, double rightY, double rightZ, 
                                   double upX,    double upY,    double upZ,
                                   double dirX,   double dirY,   double dirZ,
                                   float radius, const QColor color )
{
  float maxCircle=(float)(2.0*m_PI);
  float step=(float)(m_PI/10.0) ;
  double a=0, b=0, x=0, y=0, z=0 ;
  unsigned char red, green, blue , alpha ;

  red = (unsigned char)( color.red() ) ;
  green = (unsigned char)( color.green() ) ;
  blue = (unsigned char)( color.blue() ) ;
  alpha = (unsigned char)( color.alpha() ) ;
  
  glPushAttrib( GL_ALL_ATTRIB_BITS ) ;
  glColor4ub( red, green, blue, alpha ) ;

  glBegin( GL_LINE_LOOP ) ;
  for( float i=0 ; i<maxCircle ; i+=step )
  {
    a = radius * cosf(i) ;
    b = radius * sinf(i) ;
    x = posX + rightX * a + upX * b + -1.0*dirX ;
    y = posY + rightY * a + upY * b + -1.0*dirY ;
    z = posZ + rightZ * a + upZ * b + -1.0*dirZ ;
    glVertex3d( x, y, z ) ;
  }
  glEnd() ;
  glPopAttrib();
}


/**
  * Draw a "line sphere" in the render zone.
  * @param radius Radius of the sphere.
  * @param from The position of the sphere.
  * @param color RGBA components.
  */
void DrawSimpleObject::drawSphereLine( float radius, const Eigen::Vector3d& from, const QColor &color )
{
  this->drawSphere( m_gluQuadricGluLine, radius, from, color ) ;
}


/**
  * Draw a "fill sphere" in the render zone.
  * @param radius Radius of the sphere.
  * @param from The position of the sphere.
  * @param color RGBA components.
  */
void DrawSimpleObject::drawSphereFill( float radius, const Eigen::Vector3d& from, const QColor &color )
{
  this->drawSphere( m_gluQuadricGluFill, radius, from, color, 5, 5 ) ;
}


/**
  * Draw a sphere in the render zone.
  * @param radius Radius of the sphere.
  * @param from The position of the sphere.
  * @param color RGBA components.
  * @param slices Number of sphere slices
  * @param stacks Number of sphere stacks
  */
void DrawSimpleObject::drawSphere( GLUquadric* type, float radius, const Eigen::Vector3d& from, const QColor &color, int slices, int stacks )
{
  unsigned char r, g, b, a ;

  r = (unsigned char)( color.red() ) ;
  g = (unsigned char)( color.green() ) ;
  b = (unsigned char)( color.blue() ) ;
  a = (unsigned char)( color.alpha() ) ;
  
  // Init & Save.
  glPushMatrix() ;
  glPushAttrib( GL_ALL_ATTRIB_BITS ) ;

  // If active, do not paint the atom !!!
  // glLoadIdentity();

  // If no active, do not change color !!!
  glDisable(GL_LIGHTING) ;
  glColor4ub( r, g, b, a ) ;

  glTranslated( from[0], from[1], from[2] ) ;
  glRotatef( 22, 1, 0 , 0 ) ; // Just not to see the sphere at the bottom ...

  gluSphere( type, radius, slices, stacks ) ;
  // type : GLUquadric = options
  // radius : radius : rayon de la sphère.
  // 10 : slices : lontitudes.
  // 10 : stacks : latitudes.

  glPopAttrib();
  glPopMatrix() ;
}


/**
  * Draw a text in the render zone on XYZ-axis (based on OpenGL-coordinate).
  * @param pos The position of text
  * @param msg Message to display
  * @param font Font ...
  * @param color RGBA components.
  */
void DrawSimpleObject::drawTextOnXYZ( const QPoint& pos, const QString &msg, const QFont &font, const QColor &color )
{
  if( m_widget!=NULL )
  {
    unsigned char r, g, b, a ;

    r = (unsigned char)( color.red() ) ;
    g = (unsigned char)( color.green() ) ;
    b = (unsigned char)( color.blue() ) ;
    a = (unsigned char)( color.alpha() ) ;
    
    glPushAttrib( GL_ALL_ATTRIB_BITS ) ;
    glColor4ub( r, g, b, a ) ;

    if( m_qglwidget != NULL )
      m_qglwidget->renderText( pos.x(), pos.y(), 0, msg, font ) ;
    else
      m_painter->drawText( pos, msg ) ;

    glPopAttrib() ;
  }
}


/**
  * Draw a sphere in the render zone on XYZ-axis (based on OpenGL-coordinate).
  * @param pos The position of text.
  * @param msg Message to display
  * @param font Font ...
  * @param color RGBA components.
  */
void DrawSimpleObject::drawTextOnXYZ( const Eigen::Vector3d& pos, const QString &msg, const QFont &font, const QColor &color )
{
  if( m_widget!=NULL )
  {
    unsigned char r, g, b, a ;

    r = (unsigned char)( color.red() ) ;
    g = (unsigned char)( color.green() ) ;
    b = (unsigned char)( color.blue() ) ;
    a = (unsigned char)( color.alpha() ) ;
    
    glPushAttrib( GL_ALL_ATTRIB_BITS ) ;
    glColor4ub( r, g, b, a ) ;

    if( m_qglwidget != NULL )
      m_qglwidget->renderText( pos[0], pos[1], pos[2], msg, font ) ;
    else
      m_painter->drawText( pos, msg ) ;

    glPopAttrib() ;  
  }
}


/**
  * Draw a sphere in the render zone on XY-axis (based on screen-coordinate).
  * Using the Qt method, change the size with the font size.
  * @param pos The position of text.
  * @param msg Message to display
  * @param font Font ...
  * @param color RGBA components.
  */ 
void DrawSimpleObject::drawTextOnXY( const QPoint& pos, const QString &msg, const QFont &font, const QColor &color )
{
  if( m_widget != NULL )
  {
    unsigned char r, g, b, a ;

    r = (unsigned char)( color.red() ) ;
    g = (unsigned char)( color.green() ) ;
    b = (unsigned char)( color.blue() ) ;
    a = (unsigned char)( color.alpha() ) ;
    
    glPushAttrib( GL_ALL_ATTRIB_BITS ) ;
    glColor4ub( r, g, b, a ) ;

    if( m_qglwidget != NULL )
      m_qglwidget->renderText( pos.x(), pos.y(), msg, font ) ;
    else
      m_painter->drawText( pos, msg ) ;

    glPopAttrib() ;
  }
}


/**
  * Draw a sphere in the render zone on XY-axis (based on screen-coordinate).
  * @param pos The position of text.
  * @param msg Message to display.
  * @param font Font ...
  * @param color RGBA components.
  */
void DrawSimpleObject::drawTextOnXY( const Eigen::Vector3d& pos, const QString &msg, const QFont &font, const QColor &color )
{
  if( m_widget!=NULL )
  {
    unsigned char r, g, b, a ;

    r = (unsigned char)( color.red() ) ;
    g = (unsigned char)( color.green() ) ;
    b = (unsigned char)( color.blue() ) ;
    a = (unsigned char)( color.alpha() ) ;
    
    glPushAttrib( GL_ALL_ATTRIB_BITS ) ;
    glColor4ub( r, g, b, a ) ;

    if( m_qglwidget!=NULL )
    {
      Eigen::Vector3d proj=m_widget->camera()->project(pos) ;
      QPoint p( (int)proj[0], (int)proj[1] ) ;
      m_qglwidget->renderText( (int)pos.x(), (int)pos.y(), msg, font ) ;
    }
    else
      m_painter->drawText( pos, msg ) ;

    glPopAttrib() ;
  }
}


/**
  * All method to render/draw text in the render zone.
  */
void DrawSimpleObject::drawTestAllMethod()
{
  if( m_widget!=NULL )
  {
    QString msg ;
    QFont myFont( "Times", 32, QFont::Bold ) ;

    // 1, Origin : Up/left screen. Nothing change during a camera movement.
    Avogadro::Painter *painter=m_widget->painter() ;
    glPushAttrib( GL_ALL_ATTRIB_BITS ) ;
    glColor3f( 1.0, 1.0, 1.0 ) ;
    msg="Painter : DrawText() 1" ;
    painter->drawText( 150, 10, msg ) ;
    painter->drawText( QPoint(150,30), msg ) ;
    glPopAttrib() ; 

    // 2 Change with the camera like a point in the 3D-space
    // ! Works sometime ... Realize 2 calls of this method => don't work ...
    //glPushAttrib( GL_ALL_ATTRIB_BITS ) ;
    //glColor3f( 1.0, 1.0, 1.0 ) ;
    //msg="Painter : DrawText() 2" ;
    // Origin : center screen.
    //painter->drawText( Eigen::Vector3d(0,0,0), msg, myFont ) ;
    //glPopAttrib() ;
   
    // 3.0 ! Painter class isn't a derived class of QPainter.
    //QPainter *myQPainter=dynamic_cast<QPainter*>(painter) ;

    // 3.1, Display, but in black ... Be carefull of the background color.
    glPushAttrib( GL_ALL_ATTRIB_BITS ) ;
    glPushMatrix() ;
    glColor3f( 1.0, 1.0, 1.0 ) ;
    QPainter myQPainter ;
    myQPainter.begin(m_widget) ;
    msg = "Qt::QPainter : DrawText() 3.1" ;
    myQPainter.drawText( QPoint(150,70), msg ) ;
    msg = "Qt::QPainter : DrawText() 3.2" ;
    myQPainter.drawText( 150, 80, 150, 150, Qt::AlignLeft, msg ) ;
    myQPainter.end() ;
    glPopMatrix() ;
    glPopAttrib() ;

    // 3.2 No tested ... I do not understand when use it to "gain" performance.
    //QStaticText test ;
    //myQPainter.drawStaticText( 150, 130, ) ;
    //myQPainter.end() ;
    //glPopAttrib() ; 

    // 4 Works sometime, The font size change with zoom.
    // Problem with others visual feature like the blue selection ...
    //glPushAttrib( GL_ALL_ATTRIB_BITS ) ;
    //glColor3f( 1.0, 1.0, 1.0 ) ;
    //msg="QWidget : renderText() 4" ;
    //m_widget->renderText( 40, 200, 0, msg, myFont ) ;
    //glPopAttrib() ;
    

    // 5 Nothing change during a camera movement.
    glPushAttrib( GL_ALL_ATTRIB_BITS ) ;
    glColor3f( 1.0, 1.0, 1.0 ) ;

    QGLWidget *myWidget=dynamic_cast<QGLWidget*>(m_widget) ;
    if( myWidget != NULL )
    {
      msg="Qt::QWidget : renderText() 5" ;
      myWidget->renderText( 40, 40, msg, myFont ) ;
      msg="Qt::QWidget : renderText() 5.1" ;
      // Sometime below the previous message, sometime in the 3D-space coordonate.
      myWidget->renderText( 80, 80, 0, msg, myFont ) ;
      msg="Qt::QWidget : renderText() 5.2" ;
      myWidget->renderText( -6.879, 0.298, 0.51, msg, myFont ) ;
    }
    else
    {
      msg="Qt::QWidget : renderText() 5 FALSE" ;
      painter->drawText( 100, 140, msg ) ;
    }

    glPopAttrib() ;
  }
}

