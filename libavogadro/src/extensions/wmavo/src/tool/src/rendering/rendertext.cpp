
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

#include "rendertext.h"

RenderText::RenderText( Avogadro::GLWidget *widget )
  : DrawSimpleObject(widget),
    m_atomicNumberCur( WMEX_CREATEDATOMDEFAULT ),
    m_ratioFontSize(WMTOOL_POINTSIZE_RATIO_DEFAULT)
{
  m_time.start() ;

  m_fontWmInfo.setFamily( WMTOOL_FONT_FAMILY_INFO ) ;
  m_fontWmInfo.setPointSizeF( m_ratioFontSize * WMTOOL_FONT_POINTSIZE_INFO ) ;
  m_fontWmInfo.setWeight( WMTOOL_FONT_WEIGHT_INFO ) ;

  m_fontMsg.setFamily( WMTOOL_FONT_FAMILY_ERROR ) ;
  m_fontMsg.setPointSizeF( m_ratioFontSize * WMTOOL_FONT_POINTSIZE_ERROR ) ;
  m_fontMsg.setWeight( WMTOOL_FONT_WEIGHT_ERROR ) ;
}


RenderText::~RenderText()
{}


/**
  * Set the size of displayed text. Specific method for a QSlider : the data is multiplied by 0.1 .
  * @param ratio Ratio between (WMTOOL_POINTSIZE_RATIO_MIN) and (WMTOOL_POINTSIZE_RATIO_MAX).
  */
void RenderText::setSizeRatioFont( int ratio )
{
  float r=(float)(ratio)*0.1f ;
  setSizeRatioFont( r ) ;
}


/**
  * Set the size of displayed text.
  * @param ratio Ratio between (WMTOOL_POINTSIZE_RATIO_MIN) and (WMTOOL_POINTSIZE_RATIO_MAX).
  */
void RenderText::setSizeRatioFont( float ratio )
{
  if( m_ratioFontSize != ratio )
  {
    m_ratioFontSize = ratio ;

    if( ratio>=WMTOOL_POINTSIZE_RATIO_MIN && ratio<=WMTOOL_POINTSIZE_RATIO_MAX )
    {
      m_fontMsg.setPointSizeF( ratio * WMTOOL_FONT_POINTSIZE_ERROR ) ;
      m_fontWmInfo.setPointSizeF( ratio * WMTOOL_FONT_POINTSIZE_INFO ) ;
    }
  }
}


/**
  * Set the informations of the Wiimote.
  * @param connect Wiimote connected ?
  * @param nbDots Number of IR dots detected by the Wiimote.
  * @param nbSources Number of "IR dots" really used detected by the Wiimote.
  * @param distance The "distance" calculated by the Wiimote.
  */
void RenderText::setWmInfo( const InputDevice::WmDeviceData_from &wmData )
//void WmTool::setWmInfo( const QPoint &cursor, bool connect, int nbDots, int nbSources, int distance )
{
  m_wmDataFrom = wmData ;
}


/**
  * Set the current atomic number used for a new atom.
  * @param atomicNumber The new value of the current atomic number.
  */
void RenderText::setAtomicNumberCurrent( int atomicNumber )
{
  m_atomicNumberCur = atomicNumber ;
}


/**
  * Set the message which will be displayed in the render zone.
  * @param strList A QList of QString to diplay message by line (1 QString = 1 line)
  * @param pos The position of the message.
  * @param msTime Time of the message display.
  */
void RenderText::setMsg( const QList<QString> &strList, const QPoint &pos, int msTime )
{
  m_displayList = strList ;
  m_displayPos = pos ;
  m_displayMsg = true ;
  m_displayTimeBegin = m_time.elapsed() ;
  
  if( msTime == 0 )
    m_displayTimeDuring = WMTOOL_TIME_DISPLAY_MSG ;
  else
    m_displayTimeDuring = msTime ;
}


/**
  * Display informations of the Wiimote in the render zone.
  */
void RenderText::drawWmInfo()
{
  if( m_painter != NULL )
  {
    QFontMetrics fontMetric( m_fontWmInfo ) ;
    QColor color( 255, 255, 255 ) ;
    QString msg ;
    CWiimoteData *wm=m_wmDataFrom.getDeviceData() ;
    bool isConnected=(wm!=NULL?wm->isConnected():false) ;
    int border=5 ;
    int height=fontMetric.height() ; //lineSpacing() ;
    int xB=10, yB=10+height ;
    int nbL=0, iH=0 ;

    if( isConnected )
      nbL = 4 ;
    else
      nbL = 2 ;
    
    drawRect( float(xB-border),
              float(yB-height),
              float(xB+fontMetric.width("Current atomic number : 160")+border), 
              float(yB+(nbL-1)*height+border) ) ;
    
    // 1
    msg = "Wiimote connected : " ;
    if( isConnected ) msg += "YES" ;
    else msg += tr("NO") ; //, Press 1+2") ;
    drawTextOnXY( QPoint(xB,yB), msg, m_fontWmInfo, color ) ;
    iH++ ;

    // 2
    msg = tr("Current atomic number : ") + QString::number(m_atomicNumberCur) ;
    drawTextOnXY( QPoint(xB,yB+height*iH), msg, m_fontWmInfo, color ) ;
    iH ++ ;

    if( isConnected )
    {
      int r,g,b ;

      msg = tr("Nb detected LEDs : ") + QString::number(wm->IR.GetNumDots()) ;
      drawTextOnXY( QPoint(xB,yB+height*iH), msg, m_fontWmInfo, color ) ;
      iH++ ; 

      msg = tr("Nb detected sources : ") + QString::number(wm->IR.GetNumSources()) ;
      if( wm->IR.GetNumSources() == 1 ){ r=255; g=127; b=0; }
      else if( wm->IR.GetNumSources() == 0 ){ r=255; g=0; b=0; }
      else{ r=255; g=255; b=255; }

      drawTextOnXY( QPoint(xB,yB+height*iH), msg, m_fontWmInfo, QColor(r, g, b) ) ;
      iH++ ;
    }
  }
}


/**
  * Display a message in the render zone.
  */
void RenderText::drawMsg()
{
  if( m_displayMsg && m_painter!=NULL )
  {
    int i=0 ;
    int width=0, height=0, tmp=0 ;
    int t2=m_time.elapsed() ;
    int border=5 ; // spacing=20

    if( (t2-m_displayTimeBegin) > m_displayTimeDuring )
    {
      m_displayMsg = false ;
    }
    else
    {
      QFontMetrics fontMetric(m_fontMsg) ;
      height = fontMetric.lineSpacing() ;

      // Search the longest width.
      foreach( QString str, m_displayList )
      {
        i++ ;
        tmp = fontMetric.width( str ) ;
        if( tmp > width )
          width = tmp ;
      }

      drawRect( float(m_displayPos.x()-border), 
                float(m_displayPos.y()-(border+fontMetric.lineSpacing())), 
                float(m_displayPos.x()+width+border), float(m_displayPos.y()+i*height+border) ) ;

      i = 0 ;
      foreach( QString str, m_displayList )
      {
        if( i == 0 )
          drawTextOnXY(QPoint(m_displayPos.x(), m_displayPos.y()+i), str, m_fontMsg, QColor(255, 127, 0) ) ;
        else
          drawTextOnXY(QPoint(m_displayPos.x(), m_displayPos.y()+i), str, m_fontMsg, QColor() ) ;
        i += 20 ;
      }
    }
  }
}
