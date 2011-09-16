
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

#include "drawobject.h"


DrawObject::DrawObject( Avogadro::GLWidget *widget ) 
  : DrawSimpleObject(widget),
    m_moleculeManip(NULL),
    m_cursorPos(QPoint(0,0)),
    m_rectPos1(QPoint(0,0)), m_rectPos2(QPoint(0,0)), m_activeRect(false),
    m_drawBeginAtom(false), m_drawEndAtom(false), m_drawBond(false),
    m_beginAtom(m_vect3d0), m_endAtom(m_vect3d0)
{
}


DrawObject::~DrawObject()
{
}


/**
  * Initiate/(des)activate the selection rectangle in the render zone.
  * @param active Display or not the rectangle.
  * @param p1 The up/left position of the rectangle.
  * @param p2 The bottom/right position of the rectangle.
  */
void DrawObject::setSelectRect( bool active, const QPoint &p1, const QPoint &p2 )
{
  m_activeRect = active ;
  m_rectPos1 = p1 ;
  m_rectPos2 = p2 ;
}


void DrawObject::setCursorPos( const QPoint &cursor )
{
  m_cursorPos = cursor ;
}


/**
  * Set the position of the atoms to display them.
  * The name of the method is confused, but it more comprehensible when an other class
  * connects it to realize the desired goal : draw the atoms and bond.
  * @param beginAtom The position of the 1st atom to draw.
  * @param endAtom The position of the 2nd atom to draw.
  * @param drawBeginAtom A boolean to draw or not the 1st atom.
  * @param drawEndAtom A boolean to draw or not the 2nd atom.
  * @param drawBond A boolean to draw or not the bond.
  */
void DrawObject::setAtomBondToDraw( const Eigen::Vector3d& beginAtom, const Eigen::Vector3d& endAtom, bool drawBeginAtom, bool drawEndAtom, bool drawBond )
{
  /*
  cout << "atom begin:[" << beginAtom[0] << ";" << beginAtom[1] << ";" << beginAtom[2]  << "]" << endl ;
  cout << "atom end:[" << endAtom[0] << ";" << endAtom[1] << ";" << endAtom[2]  << "]" << endl ;
  cout << "drawBeginAtom:" << drawBeginAtom << endl ;
  cout << "drawEndAtom:" << drawEndAtom << endl ;
  cout << "drawBond:" << drawBond << endl ;
  */

  m_beginAtom = beginAtom ;
  m_endAtom = endAtom ;
  m_drawBeginAtom = drawBeginAtom ;
  m_drawEndAtom = drawEndAtom ;
  m_drawBond = drawBond ;

  // The display is updated by wmextension->wmActions():
  // (isCreateAtom) updates the fake cursor.
}


/**
  * Quick render an atom in the render zone.
  * @param radius Radius of the atom.
  * @param from The position of the atom.
  */
void DrawObject::drawAtom( float radius, const Eigen::Vector3d& from )
{
  QColor color(0,0,255) ;
  drawSphereLine( radius, from, color ) ;
}



/**
  * Quick render the "bond" between 2 positions.
  * @param begin 1st end of the "bond".
  * @param end 2nd end of the "bond".
  */
void DrawObject::drawBond( const Eigen::Vector3d& begin, const Eigen::Vector3d& end )
{
  QColor colorBegin(0,0,0), colorEnd(0,0,127) ;
  drawLine( begin, end, colorBegin, colorEnd, 8.0 ) ;
}


/**
  * Quick render the atom and the bond currently in construction.
  */
void DrawObject::drawBondAtom()
{
  if( m_drawBeginAtom )
    drawAtom( 0.4f, m_beginAtom ) ;

  if( m_drawEndAtom )
    drawAtom( 0.4f, m_endAtom ) ;

  if( m_drawBond )
    drawBond( m_beginAtom, m_endAtom ) ;
}


/**
  * Draw a circle of the 0-origin of the OpenGL universe.
  */
void DrawObject::drawCenter()
{
  /*
  GLdouble pos3D_x, pos3D_y, pos3D_z;

  // Arrays to hold matrix information.
  GLdouble model_view[16] ;
  glGetDoublev(GL_MODELVIEW_MATRIX, model_view) ;

  GLdouble projection[16] ;
  glGetDoublev(GL_PROJECTION_MATRIX, projection) ;

  GLint viewport[4] ;
  glGetIntegerv(GL_VIEWPORT, viewport) ;

  GLint params[4] ;
  glGetIntegerv( GL_VIEWPORT, params ) ;
  GLdouble width=params[2] ;
  GLdouble height=params[3] ;

  // Get 2D coordinates based on opengl coordinates.
  gluProject( width/2, height/2, 0.01,
      model_view, projection, viewport,
      &pos3D_x, &pos3D_y, &pos3D_z ) ;
  */

  QColor color( 50, 255, 50 ) ;
  float radius=0.1f ;
  drawSphereFill( radius, m_vect3d0, color ) ;
}


/**
  * Draw a circle of the barycenter of the molecule.
  */
void DrawObject::drawBarycenter()
{
  if( m_moleculeManip != NULL )
  {
    QColor color( 50, 255, 100 ) ;
    float radius=0.1f ;
    drawSphereFill( radius, m_moleculeManip->getBarycenterMolecule(), color ) ;
  }
}


/**
  * Draw some 
  */
void DrawObject::drawCursor()
{
  // The 3 next ideas are destined to try to fix the size of the circle
  // which change according to the camera or the movement of a atom ...

  // Idea 1 : Load the matrix modelview of the camera 
  // Not solve : the objects are positioned differently and are not face to face at the camera.
  //Eigen::Transform3d modelviewTransf=m_widget->camera()->modelview() ;
  //float modelviewTab[16]= { modelviewTransf(0,0), modelviewTransf(0,1), modelviewTransf(0,2),
  //                          modelviewTransf(1,0), modelviewTransf(1,1), modelviewTransf(1,2),
  //                          modelviewTransf(2,0), modelviewTransf(2,1), modelviewTransf(2,2),
  //                          modelviewTransf(3,0), modelviewTransf(3,1), modelviewTransf(3,2) } ;                        
  //glMatrixMode(GL_MODELVIEW) ;
  //glLoadMatrixf( modelviewTab ) ;

  // Idea 2 : verify the modelview/projection matrix.
  // After check the state of the modelview/projection matrix, we can see that the projection matrix
  // is always recalculated when there is a movement on the z axis (object or camera).
  // This is a problem when we want draw an objet calculated by the right, up and direction vectors 
  // (get by the modelview). The problem is the projection does not render the same visual distance ...

  // Idea 3 : Load my projection matrix.
  glMatrixMode( GL_PROJECTION ) ;
  glPushMatrix() ; //1 projection
  glLoadMatrixf( m_projectionMatrix ) ;
  glMatrixMode( GL_MODELVIEW ) ;
  glPushMatrix() ; //2 modelview
  glPushAttrib(GL_ALL_ATTRIB_BITS) ; //3 attrib
  
  double rightX, rightY, rightZ ;
  double upX, upY, upZ ;
  double directionX, directionY, directionZ ;
  getRightUpDirectionCamera( rightX, rightY, rightZ, upX, upY, upZ, directionX, directionY, directionZ ) ;
    
  double openGLX, openGLY, openGLZ ;
  getPosScreenToPosOpenGL( m_cursorPos, openGLX, openGLY, openGLZ ) ;
  
  //Will be transparent
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  //glDisable(GL_LIGHTING);
  //glDisable(GL_CULL_FACE);

  
  float r=0.02f ;
  QColor color( 191, 191, 191, 191 ) ;

  //glBegin( GL_TRIANGLES ) ;
  //    glVertex3d( posX, posY, posZ ) ;
  //    glVertex3d( posX+r*rightX, posY+r*rightY, posZ+r*rightZ ) ;
  //    glVertex3d( posX+r*upX, posY+r*upY, posZ+r*upZ ) ;
  //glEnd() ;

  drawCircle( openGLX,  openGLY,  openGLZ,
              rightX,   rightY,   rightZ,
              upX,      upY,      upZ,
              directionX, directionY, directionZ,
              r, color ) ;

  drawCircle( openGLX,  openGLY,  openGLZ,
              1,        0,        -1,
              0,        1,        -1,
              1,        1,        -1,
              r, color ) ;
  
  glPopAttrib() ; //3 attrib
  glPopMatrix() ; //2 modelview
  glMatrixMode( GL_PROJECTION ) ;
  glPopMatrix() ; //1 projection
  glMatrixMode( GL_MODELVIEW ) ;
}


void DrawObject::drawSelectRect()
{
  if( m_activeRect )
    drawRect( m_rectPos1, m_rectPos2 ) ;
}

