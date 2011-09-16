
/*******************************************************************************
  Copyright (C) 2011 Mickael Gadroy, University of Reims Champagne-Ardenne (Fr)
                          Project managers: Eric Henon and Michael Krajecki
                          Financial support: Region Champagne-Ardenne (Fr)

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
#ifndef __INPUTDEVICE_H__
#define __INPUTDEVICE_H__

#define INPUTDEVICE_ID_NOTHING 0x00000000
#define INPUTDEVICE_ID_WIIMOTE 0x00000001

#include "warning_disable_begin.h"
#include <iostream>
#include <QObject>
#include "warning_disable_end.h"

namespace InputDevice
{
  class DeviceData_from
  {
  public :
    DeviceData_from() ;
    virtual ~DeviceData_from() ;

    virtual unsigned int getDeviceType()=0 ;
  };

  class DeviceData_to
  {
  public :
    DeviceData_to() ;
    virtual ~DeviceData_to() ;

    virtual unsigned int getDeviceType()=0 ;
  };

  class Device : public QObject
  {
    Q_OBJECT 

  public :
    Device() ;
    virtual ~Device() ;

    virtual unsigned int getDeviceType()=0 ;
    virtual DeviceData_from* getDeviceDataFrom()=0 ;
    inline bool isConnected(){return m_isConnected;} ;
    virtual bool hasDeviceDataAvailable()=0 ;

  protected :
    bool m_isConnected ;
  };

}

#endif
