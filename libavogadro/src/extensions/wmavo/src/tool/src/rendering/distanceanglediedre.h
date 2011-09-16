
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
#ifndef __DISTANCEANGLEDIEDRE_H__
#define __DISTANCEANGLEDIEDRE_H__

#include "warning_disable_begin.h"

#include "drawsimpleobject.h"
#include "avogadro/atom.h"
#include <openbabel/math/vector3.h>
#include <sstream>

#include "warning_disable_end.h"


class DistanceAngleDiedre : public DrawSimpleObject
{
  Q_OBJECT

  // Public slots.
  public slots :
    void setSizeRatioFont( float ratio ) ;
    void setCalculDistDiedre( int what ) ;
    void addAtom( Avogadro::Atom *atom ) ; // calculDistDiedre

  // Signals.
  signals :
    void askDistDiedre() ; ///< Element to the calculation of the distance feature.
    void changedIrSensitive( int ) ;


  // Public methods.
  public :
    DistanceAngleDiedre( Avogadro::GLWidget *widget ) ;
    ~DistanceAngleDiedre() ;

    void drawDistDiedre() ;

  // Private methods.
  private :
    void clearDistDiedre() ;
    void calculateParameters() ;

  // Private attributs.
  private :

    /**
      * @name To calcul distance and diedre
      * @{ */
    bool m_isCalculDistDiedre ;
    int m_nbAtomForDistDiedre ;
    QList<QPointer<Avogadro::Atom> > m_atomForDistDiedre ;

    Eigen::Vector3d m_vector[3] ;
    double m_angle[2] ;
    double m_dihedral ;
    // Need to store the previous values of all variables in order to only send
    // an event to the information pane once
    double m_lastMeasurement[6] ;
    // @}

    /**
      * @name To display distance and diedre
      * @{ */
    int m_nbHPixelDist, m_nbHPixelAngle, m_nbHPixelDihedral ;
    int m_nbVPixelDist, m_nbVPixelAngle, m_nbVPixelDihedral ;
    // @}

    /**
      * @name Fonts definition.
      * @{ */
    float m_ratioFontSize ; //< The user can modify this ratio to resize the messages.
    QFont m_fontDistDiedreAtom ;
    QFont m_fontDistDiedreInfo ;
    // @}

    /**
      * @name Mathematical needs.
      * @{ */
    static const double m_PI180 ; // 3.14/180 for degree to radian.
    static const double m_180PI ; // 180/3.14 for radian to degree.
    static const QString m_angstromStr, m_degreeStr ;
    static const QChar m_angstromChar, m_degreeChar ;
    // @}

  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
      // http://eigen.tuxfamily.org/dox/StructHavingEigenMembers.html*

} ;

#endif
