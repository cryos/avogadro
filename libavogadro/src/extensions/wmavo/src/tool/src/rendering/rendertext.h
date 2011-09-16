
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
#ifndef __RENDERTEXT_H__
#define __RENDERTEXT_H__

#include "warning_disable_begin.h"

#include "drawsimpleobject.h"
#include "wmdevice.h"

#include <QTime>

#include <avogadro/glwidget.h>
#include <avogadro/painter.h>

#include "warning_disable_end.h"


class RenderText : public DrawSimpleObject
{
  Q_OBJECT

  public slots :
    void setSizeRatioFont( int ratio ) ; //< Change the size of all the font.
    void setSizeRatioFont( float ratio ) ;
    void setWmInfo( const InputDevice::WmDeviceData_from &wmDataFrom ) ;
    void setAtomicNumberCurrent( int atomicNumber ) ;
    void setMsg( const QList<QString> &strList, const QPoint &pos, int msTime=0 ) ;
   
  public :
    RenderText( Avogadro::GLWidget *widget ) ;
    ~RenderText() ;

    /**
      * @name Display text in the render zone
      * @{ */
    void drawWmInfo() ;
    void drawMsg() ;
    // @}


  private :

    InputDevice::WmDeviceData_from m_wmDataFrom ; 

    /**
      * @name Miscellaneous.
      * @{ */
    QTime m_time ;
    int m_atomicNumberCur ;
    // @}

    /**
      * @name Fonts definition.
      * @{ */
    float m_ratioFontSize ; //< The user can modify this ratio to resize the messages.
    QFont m_fontWmInfo ;
    QFont m_fontMsg ;
    // @}

     /**
      * @name To display a message in rendering area & Co.
      * @{ */
    QList<QString> m_displayList ;
    QPoint m_displayPos ;
    bool m_displayMsg ;
    int m_displayTimeBegin ;
    int m_displayTimeDuring ;
    // @}

} ;

#endif
