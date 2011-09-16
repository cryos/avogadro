
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

#pragma once
#ifndef __DRAWSIMPLEOBJECT_H__
#define __DRAWSIMPLEOBJECT_H__

#ifndef NOMINMAX
#define NOMINMAX
// >C:\src\eigen-2.0.15\Eigen/Core(77) : fatal error C1189: #error :  
// The preprocessor symbols 'min' or 'max' are defined. If you are compiling on Windows, 
// do #define NOMINMAX to prevent windows.h from defining these symbols.
#endif

#include "warning_disable_begin.h"
#include "wmavo_const.h"
#include "variousfeatures.h"

// Before #include <QObject> <QGLWidget>
#include "avogadro/glwidget.h" 
// else : In file included from /usr/include/avogadro/glwidget.h:33,
//  from /home/.../src/tool/src/rendering/drawsimpleobject.h:43,
//  from /home/.../src/tool/src/rendering/drawobject.h:30,
//  from /home/.../src/tool/src/rendering/drawobject.cpp:24:
//  /usr/include/GL/glew.h:84:2: error: #error gl.h included before glew.h
//  /usr/include/GL/glew.h:87:2: error: #error glext.h included before glew.h

#include "avogadro/painter.h"
#include "avogadro/atom.h"
#include "avogadro/camera.h"

#include <QObject>
#include <QGLWidget>
#include <QColor>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include "warning_disable_end.h"


// See to realize some drawing in glOrtho mode.

/**
  * The class is specific to draw in Avogadro. There are some particularities in
  * the rendering works.
  */
class DrawSimpleObject : public QObject
{
  Q_OBJECT

public :
  DrawSimpleObject( Avogadro::GLWidget *widget ) ;
  virtual ~DrawSimpleObject() ;

  void getRightUpDirectionCamera( double &rightX_out, double &rightY_out, double &rightZ_out, 
                                  double &upX_out,    double &upY_out,    double &upZ_out,
                                  double &dirX_out,   double &dirY_out,   double &dirZ_out ) ;
  void getPosScreenToPosOpenGL( const QPoint &p_in, double &X_out, double &Y_out, double &Z_out ) ;

  void printOpenGLError() ;
  void printModelviewAndProjectionMatrix() ;
  void print3DStereoPossibilities() ;

  void drawLine( const Eigen::Vector3d& begin, const Eigen::Vector3d& end, const QColor &colorBegin=QColor(), const QColor &colorEnd=QColor(), float lineWidth=8.0f ) ;
  void drawRect( const QPoint &p1, const QPoint &p2, const QColor* color=NULL ) ;
  void drawRect( float sx, float sy, float ex, float ey, const QColor* color=NULL ) ;
  void drawCircle( double posx,   double posy,   double posz, 
                   double rightX, double rightY, double rightZ, 
                   double upX,    double upY,    double upZ,
                   double dirX,   double dirY,   double dirZ,
                   float radius, const QColor color=QColor() ) ;
  void drawSphere( GLUquadric* type, float radius, const Eigen::Vector3d& from, const QColor &color=QColor(), int slices=10, int stacks=10 ) ;
  void drawSphereLine( float radius, const Eigen::Vector3d& from, const QColor &color=QColor() ) ;
  void drawSphereFill( float radius, const Eigen::Vector3d& from, const QColor &color=QColor() ) ;

  void drawTextOnXYZ( const QPoint& pos, const QString &msg, const QFont &font, const QColor &color=QColor() ) ;
  void drawTextOnXYZ( const Eigen::Vector3d& pos, const QString &msg, const QFont &font, const QColor &color=QColor() ) ;
  void drawTextOnXY( const QPoint& pos, const QString &msg, const QFont &font, const QColor &color=QColor() ) ;
  void drawTextOnXY( const Eigen::Vector3d& pos, const QString &msg, const QFont &font, const QColor &color=QColor() ) ;

  void drawTestAllMethod() ;
  
protected :
  Avogadro::GLWidget *m_widget ;  ///< Avogadro object.
  Avogadro::Painter *m_painter ;    ///< Avogadro object.
  QGLWidget *m_qglwidget ; ///< Qt object.
  float m_projectionMatrix[16] ; ///< My projection matrix for some object.
  static const Eigen::Vector3d m_vect3d0 ;
  static const double m_PI ; // PI approximation.
  // OR
  //#define _USE_MATH_DEFINES // M_PI
  //#include <math.h> // M_PI

private :
  GLUquadric* m_gluQuadricGluLine ; // m_gluQuadricParams
  GLUquadric* m_gluQuadricGluFill ; // m_gluQuadricParamsCenter

};

#endif
