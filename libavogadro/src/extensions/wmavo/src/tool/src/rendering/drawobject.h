
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
#ifndef __DRAWOBJECT_H__
#define __DRAWOBJECT_H__

#include "warning_disable_begin.h"
#include "wmavo_const.h"
#include "drawsimpleobject.h"
#include "moleculemanipulation.h"

#include <avogadro/extension.h>

#include "warning_disable_end.h"


class DrawObject : public DrawSimpleObject
{
  Q_OBJECT

  // Public slots.
  public slots :
    void setCursorPos( const QPoint &cursor ) ;
    void setSelectRect( bool active, const QPoint &p1, const QPoint &p2 ) ;

    // renderAtomBond
    void setAtomBondToDraw( const Eigen::Vector3d& beginAtom, const Eigen::Vector3d& endAtom, bool drawBeginAtom, bool drawEndAtom, bool drawBond ) ;
    

  public :

    DrawObject( Avogadro::GLWidget *widget ) ;
    ~DrawObject() ;

    /**
      * @name Draw something in the render zone
      * @{ */
    void drawAtom( float rayon, const Eigen::Vector3d& from ) ;
    void drawBond( const Eigen::Vector3d& begin, const Eigen::Vector3d& end ) ;
    void drawBondAtom() ;
    
    void drawCenter() ;
    void drawBarycenter() ;
    void drawCursor() ;
    void drawSelectRect() ;
    // @}

  private :

    /**
      * @name Main objects
      * @{ */
    Avogadro::MoleculeManipulation *m_moleculeManip ;
    // @}

    /**
      * @name Miscellaneous.
      * @{ */
    QPoint m_cursorPos ;
    QPoint m_rectPos1, m_rectPos2 ;
    bool m_activeRect ;
    // @}

    /**
      * @name Need objects to render/draw something in the render zone.
      * @{ */
    bool m_drawBeginAtom, m_drawEndAtom, m_drawBond ;
    Eigen::Vector3d m_beginAtom, m_endAtom ;
    // @}

  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
      // http://eigen.tuxfamily.org/dox/StructHavingEigenMembers.html

} ;

#endif
