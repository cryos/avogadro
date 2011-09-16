
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
 
#include "distanceanglediedre.h"


const double DistanceAngleDiedre::m_PI180=m_PI/180.0 ;
const double DistanceAngleDiedre::m_180PI=180.0/m_PI ;

const QChar DistanceAngleDiedre::m_angstromChar = 0x00C5 ; // Å
const QChar DistanceAngleDiedre::m_degreeChar = 0x00B0 ; // °
const QString DistanceAngleDiedre::m_angstromStr=QString( DistanceAngleDiedre::m_angstromChar ) ;
const QString DistanceAngleDiedre::m_degreeStr=QString( DistanceAngleDiedre::m_degreeChar ) ;
//const QString DistanceAngleDiedre::m_angstromStr=QString::fromUtf8( DistanceAngleDiedre::m_degreeChar. ) ;
//const QString DistanceAngleDiedre::m_degreeStr=QString::fromUtf8( "" ) ;


DistanceAngleDiedre::DistanceAngleDiedre( Avogadro::GLWidget *widget )
  : DrawSimpleObject(widget),
    m_isCalculDistDiedre(false), m_nbAtomForDistDiedre(0),

    m_nbHPixelDist(WMTOOL_SPACING_LEFT_WORDGRP ), m_nbHPixelAngle(WMTOOL_SPACING_LEFT_WORDGRP),
    m_nbHPixelDihedral(WMTOOL_SPACING_LEFT_WORDGRP), m_nbVPixelDist(WMTOOL_SPACING_DOWN_WORDGRP),
    m_nbVPixelAngle(0), m_nbVPixelDihedral(0),
    m_ratioFontSize(WMTOOL_POINTSIZE_RATIO_DEFAULT)
{
  m_fontDistDiedreInfo.setFamily( WMTOOL_FONT_FAMILY_DISTDIEDREINFO ) ;
  m_fontDistDiedreInfo.setPointSizeF( m_ratioFontSize * WMTOOL_FONT_POINTSIZE_DISTDIEDREINFO ) ;
  m_fontDistDiedreInfo.setWeight( WMTOOL_FONT_WEIGHT_DISTDIEDREINFO ) ;

  m_fontDistDiedreAtom.setFamily( WMTOOL_FONT_FAMILY_DISTDIEDREATOM ) ;
  m_fontDistDiedreAtom.setPointSizeF( m_ratioFontSize * WMTOOL_FONT_POINTSIZE_DISTDIEDREATOM ) ;
  m_fontDistDiedreAtom.setWeight( WMTOOL_FONT_WEIGHT_DISTDIEDREATOM ) ;

  for( int i=0 ; i<6 ; i++ )
    m_lastMeasurement[i] = 0.0 ;
}


DistanceAngleDiedre::~DistanceAngleDiedre()
{}


/**
  * Set the size of displayed text.
  * @param ratio Ratio between (WMTOOL_POINTSIZE_RATIO_MIN) and (WMTOOL_POINTSIZE_RATIO_MAX).
  */
void DistanceAngleDiedre::setSizeRatioFont( float ratio )
{
  if( m_ratioFontSize != ratio )
  {
    m_ratioFontSize = ratio ;

    if( ratio>=WMTOOL_POINTSIZE_RATIO_MIN && ratio<=WMTOOL_POINTSIZE_RATIO_MAX )
    {
      m_fontDistDiedreAtom.setPointSizeF( ratio * WMTOOL_FONT_POINTSIZE_DISTDIEDREATOM ) ;
      m_fontDistDiedreInfo.setPointSizeF( ratio * WMTOOL_FONT_POINTSIZE_DISTDIEDREINFO ) ;
    }
  }
}


/**
  * Initiate the need to calculate the distance & co between atoms.
  * @param what Number of desired atoms to realize all the calculations (2 : distance, 3 : angle&distance, 4 : diedre angle&angle&distance).
  * @see WmTool::calculDistDiedre( Avo::Atom *atom )
  */
void DistanceAngleDiedre::setCalculDistDiedre( int what )
{
  if( what == -1 )
  { // Clear everything which looks like "calcul distance, diedre, dihedral".

    clearDistDiedre() ;
  }
  else if( what>=2 && what<=4 )
  { // Init. or re-init. wmExtension & wmTool.

    clearDistDiedre() ;

    m_isCalculDistDiedre = true ;
    m_nbAtomForDistDiedre = what ;

    emit askDistDiedre() ;
  }
}


/**
  * Get the atoms to realize the "calculation of the distance, angle, diedre angle" feature.
  * @param atom An added atom in the calculation.
  */
void DistanceAngleDiedre::addAtom( Avogadro::Atom *atom )
{
  if( m_isCalculDistDiedre && m_nbAtomForDistDiedre>0 )
  {
    int indexOfAtom=m_atomForDistDiedre.indexOf(atom) ;

    if( indexOfAtom != -1 )
    { // Delete the atom.

      m_atomForDistDiedre.removeAt(indexOfAtom) ;
      m_nbAtomForDistDiedre++ ;
    }
    else
    { // Add the atom.

      m_nbAtomForDistDiedre-- ;
      m_atomForDistDiedre.append( atom ) ;

      // Realize into paint().
      //if( m_atomForDistDiedre.size() >= 2 ) calculateParameters() ;

    }

    if( m_nbAtomForDistDiedre > 0 )
      emit askDistDiedre() ;
  }
}


/**
  * Erase all informations used by the "calculation of the distance, angle, diedre angle" feature.
  */
void DistanceAngleDiedre::clearDistDiedre()
{
  m_atomForDistDiedre.clear() ;
  m_isCalculDistDiedre = false ;
  m_nbAtomForDistDiedre = 0 ;
  //m_initDisplayDistDiedre = false ;

  for( int i=0 ; i<6 ; i++ )
    m_lastMeasurement[i] = 0.0 ;
}


/**
  * Sub-method used to calculate all informations for the "calculation of the distance, angle, diedre angle" feature.
  */
void DistanceAngleDiedre::calculateParameters()
{
  int i=0 ;
  double norm=0 ;
  QString distanceString, angleString, dihedralString ;

  // Check if no atom is erased.
  while( i < m_atomForDistDiedre.size() )
  {
    if( m_atomForDistDiedre[i].isNull() )
    {
      //cout << " Atome effacé" << endl ;
      m_atomForDistDiedre.removeAt(i) ;

      if( m_nbAtomForDistDiedre > 0 )
        m_nbAtomForDistDiedre ++ ;
      else
        clearDistDiedre() ;
    }
    else
      i++ ;
  }

  // Calculate all parameters and store them in member variables.

  if( m_atomForDistDiedre.size() >= 2 )
  { // Two atoms selected - distance measurement only

    m_vector[0] = *(m_atomForDistDiedre[1]->pos()) - *(m_atomForDistDiedre[0]->pos()) ;
    norm = m_vector[0].norm() ;

    // Check whether we have already sent this out...
    if( m_lastMeasurement[0] != norm ) // coeff(0)~=[0] : coef more performant.
    {
      m_lastMeasurement[0] = norm ;

      /*distanceString = tr("Distance (1->2): %L1 %2", "%L1 is distance, %2 is Angstrom symbol")
        .arg(norm)
        .arg(QString::fromUtf8("Å")) ;

      emit message(distanceString) ;*/
    }
  }

  if( m_atomForDistDiedre.size() >= 3 )
  { // Two distances and the angle between the three selected atoms

    m_vector[1] = *(m_atomForDistDiedre[1]->pos()) - *(m_atomForDistDiedre[2]->pos()) ;
    norm = m_vector[1].norm() ;

    // Calculate the angle between the atoms
    m_angle[0] = acos(m_vector[0].normalized().dot(m_vector[1].normalized()));
    m_angle[0] *= m_180PI ; // To degree.

    // Check whether we have already sent this out
    if( m_lastMeasurement[1] != norm )
    {
      m_lastMeasurement[1] = norm ;

      /*distanceString = tr("Distance (2->3): %L1 %2", "%L1 is distance, %2 is Angstrom symbol")
        .arg(norm)
        .arg(QString::fromUtf8("Å")) ;

      emit message(distanceString) ;*/
    }

    if( m_lastMeasurement[3] != m_angle[0] )
    {
      m_lastMeasurement[3] = m_angle[0] ;

      /*angleString = tr("Angle 1: %L1 °").arg(m_angle[0]) ;
      emit message(angleString) ;*/
    }
  }

  if( m_atomForDistDiedre.size() >= 4 )
  { // Three distances, bond angle and dihedral angle

    m_vector[2] = *m_atomForDistDiedre[2]->pos() - *m_atomForDistDiedre[3]->pos() ;
    norm = m_vector[2].norm() ;

    // Calculate the angle between the atoms.
    Eigen::Vector3d v1=*(m_atomForDistDiedre[2]->pos()) - *(m_atomForDistDiedre[1]->pos()) ;
    Eigen::Vector3d v2=*(m_atomForDistDiedre[2]->pos()) - *(m_atomForDistDiedre[3]->pos()) ;
    m_angle[1] = acos(v1.normalized().dot(v2.normalized()));
    m_angle[1] *= m_180PI ;

    /*
    double aSq=m_vector[1].norm()*m_vector[1].norm() ;
    double cSq=m_vector[2].norm()*m_vector[2].norm() ;


    Eigen::Vector3d v=*(m_atomForDistDiedre[1]->pos()) - *(m_atomForDistDiedre[3]->pos()) ;
    double bSq=v.norm()*v.norm() ;

    cout << "norm:" << v.norm() << endl ;
    cout << "aSq:" << aSq << " bSq:" << bSq  << "cSq:" << cSq << endl ;

    m_angle[1] = acos( (cSq+aSq-bSq)/(2*cSq*aSq) ) ;
    m_angle[1] *= m_180PI ; // To degree.
    */

    m_dihedral=OpenBabel::CalcTorsionAngle
                (
                  OpenBabel::vector3( m_atomForDistDiedre[0]->pos()->x(),
                                      m_atomForDistDiedre[0]->pos()->y(),
                                      m_atomForDistDiedre[0]->pos()->z()),
                  OpenBabel::vector3( m_atomForDistDiedre[1]->pos()->x(),
                                      m_atomForDistDiedre[1]->pos()->y(),
                                      m_atomForDistDiedre[1]->pos()->z()),
                  OpenBabel::vector3( m_atomForDistDiedre[2]->pos()->x(),
                                      m_atomForDistDiedre[2]->pos()->y(),
                                      m_atomForDistDiedre[2]->pos()->z()),
                  OpenBabel::vector3( m_atomForDistDiedre[3]->pos()->x(),
                                      m_atomForDistDiedre[3]->pos()->y(),
                                      m_atomForDistDiedre[3]->pos()->z())
                  );



    // Check whether these measurements have been sent already
    if( m_lastMeasurement[2] != norm )
    {
      m_lastMeasurement[2] = norm ;

      /*distanceString = tr("Distance (3->4): %L1 %2", "%L1 is distance, %2 is Angstrom symbol")
        .arg(norm)
        .arg(QString::fromUtf8("Å")) ;

      emit message(distanceString) ;*/
    }

    if( m_lastMeasurement[4] != m_angle[1] )
    {
      m_lastMeasurement[4] = m_angle[1] ;

      /*angleString = tr("Angle 2: %L1 °").arg(m_angle[1]) ;
      emit message(angleString) ;*/
    }

    if( m_lastMeasurement[5] != m_dihedral )
    {
      m_lastMeasurement[5] = m_dihedral ;

      /*dihedralString = tr("Dihedral Angle: %1 °").arg(m_dihedral) ;
      emit message(dihedralString) ;*/
    }
  }
}


/**
  * Render all informations for the "calculation of the distance, angle, diedre angle" feature.
  */
void DistanceAngleDiedre::drawDistDiedre()
{
  if( m_atomForDistDiedre.size() > 0 )
  {
    calculateParameters() ;
    //puts( "After calculateParameters() ;" ) ;

    Eigen::Vector3d btza=m_widget->camera()->backTransformedZAxis() ;
    int wh=m_widget->height() ;
    
    QString msg ;
    std::string tmp1, tmp2 ;
    std::ostringstream oss ;
    Eigen::Vector3d textRelPos ;
    int r,g,b ;

    
    int maxV = qMax( m_nbVPixelDist, m_nbVPixelAngle ) ;
    maxV = qMax( maxV, m_nbVPixelDihedral ) ;
    drawRect( 10.f, float(wh-maxV), float(m_nbHPixelDist), float(wh-3) ) ;

    //m_initDisplayDistDiedre = true ;
    QFontMetrics fontMetricDisDiedreInfo(m_fontDistDiedreInfo) ;

    m_nbHPixelDist = WMTOOL_SPACING_LEFT_WORDGRP ;
    m_nbHPixelAngle = WMTOOL_SPACING_LEFT_WORDGRP ;
    m_nbHPixelDihedral = WMTOOL_SPACING_LEFT_WORDGRP ;
    m_nbVPixelDist = WMTOOL_SPACING_DOWN_WORDGRP ;
    m_nbVPixelAngle = 0 ;
    m_nbVPixelDihedral = 0 ;


    glPushAttrib( GL_ALL_ATTRIB_BITS ) ;
    // Save OpenGL attribute stack (color, lightning, texturing ...)

    for( int i=0 ; i<m_atomForDistDiedre.size() ; i++ )
    {
      oss.str("") ;
      oss << "." << (i+1) ; tmp1 = oss.str() ;
      oss << " is a number." ; tmp2 = oss.str() ;
      textRelPos = (0.18 + m_widget->radius(m_atomForDistDiedre[i])) * btza ;

      switch( i )
      {
      case 0 : r=255;   g=0;    b=0; break ;
      case 1 : r=0;     g=255;  b=0; break ;
      case 2 : r=0;     g=0;    b=255; break ;
      case 3 : r=0;     g=255;  b=255; break ;
      default : r=0;    g=0;    b=0; 
      }

      msg = tr(tmp1.c_str(), tmp2.c_str()) ;
      textRelPos += *m_atomForDistDiedre[i]->pos() ;
      drawTextOnXYZ( textRelPos, msg, m_fontDistDiedreAtom, QColor(r,g,b) ) ;
    }


    if( m_atomForDistDiedre.size() >= 2 )
    {
      // Try to put the labels in a reasonable place on the display.

      QString format("%L1"); // For localized measurements, e.g. 1,02 in Europe.

      // Text position for distance.
      msg = tr("Distance(s) :") ;
      drawTextOnXY( QPoint(m_nbHPixelDist, wh-m_nbVPixelDist),
                    msg, m_fontDistDiedreInfo, QColor(255,255,255) ) ;
      m_nbHPixelDist += fontMetricDisDiedreInfo.width(msg) ;
      m_nbVPixelAngle += (m_nbVPixelDist + fontMetricDisDiedreInfo.lineSpacing() + WMTOOL_SPACING_V_WORDGRP) ;

      // Text position for the 1st distance.
      msg = format.arg(m_vector[0].norm(), 0, 'f', 3) + m_angstromStr ;
      drawTextOnXY( QPoint(m_nbHPixelDist, wh-m_nbVPixelDist), 
                    msg, m_fontDistDiedreInfo, QColor(0,255,0) ) ;
      m_nbHPixelDist += (fontMetricDisDiedreInfo.width(msg)+WMTOOL_SPACING_H_WORDGRP) ;


      if( m_atomForDistDiedre.size() >= 3 )
      {
        // Text position for the 2nd distance.
        msg = format.arg(m_vector[1].norm(), 0, 'f', 3) + m_angstromStr ;
        drawTextOnXY( QPoint(m_nbHPixelDist, wh-m_nbVPixelDist), 
                      msg, m_fontDistDiedreInfo, QColor(0,0,255) ) ;
        m_nbHPixelDist += (fontMetricDisDiedreInfo.width(msg)+WMTOOL_SPACING_H_WORDGRP) ;

        // Text position for angle.
        msg = QString(tr("Angle(s)      :")) ;
        drawTextOnXY( QPoint(m_nbHPixelAngle, wh-m_nbVPixelAngle), 
                      msg, m_fontDistDiedreInfo, QColor(255,255,255) ) ;
        m_nbHPixelAngle += (fontMetricDisDiedreInfo.width(msg)+WMTOOL_SPACING_H_WORDGRP) ;
        m_nbVPixelDihedral = (m_nbVPixelAngle + fontMetricDisDiedreInfo.lineSpacing() + WMTOOL_SPACING_V_WORDGRP) ;

        // Text position for the 1st angle.
        msg = format.arg(m_angle[0], 0, 'f', 1) + m_degreeStr ;
        drawTextOnXY( QPoint(m_nbHPixelAngle, wh-m_nbVPixelAngle), 
                      msg, m_fontDistDiedreInfo, QColor(0,255,0) ) ;
        m_nbHPixelAngle += (fontMetricDisDiedreInfo.width(msg)+WMTOOL_SPACING_H_WORDGRP) ;


        // Draw the angle.
        const Eigen::Vector3d *origin=m_atomForDistDiedre[1]->pos() ;
        double radius=m_widget->radius(m_atomForDistDiedre[1])+0.2 ;

        glEnable(GL_BLEND);
        glDepthMask(GL_FALSE);

        m_painter->setColor(0, 1.0, 0, 0.3f);
        m_painter->drawShadedSector( *origin, *m_atomForDistDiedre[0]->pos(),
                                      *m_atomForDistDiedre[2]->pos(), radius ) ;
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        m_painter->setColor(1.0, 1.0, 1.0, 1.0);
        m_painter->drawArc( *origin, *m_atomForDistDiedre[0]->pos(),
                             *m_atomForDistDiedre[2]->pos(), radius, true ) ;


        if(m_atomForDistDiedre.size() >= 4)
        {
          // Text position for the 3rd distance.
          msg = format.arg(m_vector[2].norm(), 0, 'f', 3) + m_angstromStr ;
          drawTextOnXY( QPoint(m_nbHPixelDist, wh-m_nbVPixelDist), 
                        msg, m_fontDistDiedreInfo, QColor(0,255,255) ) ;
          m_nbHPixelDist += (fontMetricDisDiedreInfo.width(msg)+WMTOOL_SPACING_H_WORDGRP) ;

          // Text position for the 2nd angle.
          msg = format.arg(m_angle[1], 0, 'f', 1)+m_degreeStr ;
          drawTextOnXY( QPoint(m_nbHPixelAngle, wh-m_nbVPixelAngle),
                        msg, m_fontDistDiedreInfo, QColor(0,0,255)  ) ;

          // Text position for dihetral.
          msg = QString(tr("Dihedral    :")) ;
          drawTextOnXY( QPoint(m_nbHPixelDihedral, wh-m_nbVPixelDihedral), 
                        msg, m_fontDistDiedreInfo, QColor(255,255,255) ) ;
          m_nbHPixelDihedral += (fontMetricDisDiedreInfo.width(msg) + WMTOOL_SPACING_H_WORDGRP) ;


          msg = format.arg(m_dihedral, 0, 'f', 1)+m_degreeStr ;
          drawTextOnXY( QPoint(m_nbHPixelDihedral , wh-m_nbVPixelDihedral), 
                        msg, m_fontDistDiedreInfo, QColor(0,255,0) ) ;
          m_nbVPixelDihedral += (fontMetricDisDiedreInfo.lineSpacing() + (WMTOOL_SPACING_V_WORDGRP/2)) ;
        }
      }
    }

    glPopAttrib() ;      
  }
}
