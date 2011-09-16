
/*******************************************************************************
  Copyright (C) 2010,2011 Mickael Gadroy

  This file is part of WmToChem (WiiChem project)
  WmToChem - Integrate the Wiimote and the Nunchuk in Avogadro software for the
  handling of the atoms and the camera.
  For more informations, see the README file.

  WmToChem is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  WmToChem is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with WmToChem. If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/


// Inline method are here.
// Some getter/setter methods, and isSomething() methods.


/**
  * Change the current operating mode.
  * @param opMode Integer which represents the new operating mode.
  */
inline void WmToChem::setOperatingMode(int opMode)
{
  if( opMode>=0 && opMode<WMAVO_NBOPERATINGMODE )
    m_operatingMode = opMode ;
}

/**
  * Activate or not the menu mode : WMAVO_MENU_ACTIVE, WMAVO_MENU_DOWN, WMAVO_MENU_UP ...
  * It means that the wrapper change the sent signals.
  * @param menuMode TRUE to switch to the "menu mode", FALSE to switch to the "action mode"
  */
inline void WmToChem::setMenuMode( bool menuMode )
{
  if( menuMode )
    WMAVO_SETON( WMAVO_MENU_ACTIVE ) ;
  else
    WMAVO_SETOFF( WMAVO_MENU_ACTIVE ) ;
}


/**
  * Activate or not the menu mode : WMAVO_MENU_ACTIVE, WMAVO_MENU_DOWN, WMAVO_MENU_UP ...
  * It means that the wrapper change the sent signals.
  * @param actionMode TRUE to switch to the "action mode", FALSE to switch to the "menu mode".
  */
inline void WmToChem::setActionMode( bool actionMode )
{
  if( actionMode )
    WMAVO_SETOFF( WMAVO_MENU_ACTIVE ) ;
  else
    WMAVO_SETON( WMAVO_MENU_ACTIVE ) ;
}


/**
  * Set the window where the cursor can be moved.
  * @param x The X-axis position (up-left)
  * @param y The Y-axis position (up-left)
  * @param width The width (at right of (x,y))
  * @param height The height (at bottom of (x,y))
  */
inline void WmToChem::setSizeWidget( int x, int y, int width, int height )
{
  m_xWidget = x ;
  m_yWidget = y ;
  m_widthWidget = width ;
  m_heightWidget = height ;
}

/**
  * Set the sensitivity of the Wiimote (15 to 60)
  */
inline void WmToChem::setIrSensitive( int irSensitive )
{
  //cout << "WmToChem::wmSetSensitive" << endl ;
  if( PLUGIN_WM_SENSITIVE_MIN<irSensitive && irSensitive<PLUGIN_WM_SENSITIVE_MAX )
    m_irSensitive = irSensitive ;
}


/**
  * Get the actions realized by the Wiimote.
  * @return The actions realized by the Wiimote.
  */
inline int WmToChem::getActions(){ return m_isWhat ; }


/**
  * Get the current operating mode of the Wiimote (how the wrapper translate the
  * action Wiimote to the "actions to make a chemical structure".
  * @return Integer which represents the current operating mode.
  */
inline int WmToChem::getOperatingMode(){ return m_operatingMode ; }


/**
  * Get the position pointed by the Wiimote.
  * @return A point targeted by the Wiimote.
  */
inline QPoint WmToChem::getPosCursor(){ return m_posCursor ; }

/**
  * Get the current "3D-position" of the Wiimote, by coupling the "position of the cursor"
  * and the distance calculated by the Wiimote.
  * @return A current "3D-position" of the Wiimote.
  */
inline Eigen::Vector3d WmToChem::getPos3dCurrent(){ return m_currentPosSmooth ; }

/**
  * Get the last "3D-position" of the Wiimote, by coupling the "position of the cursor"
  * and the distance calculated by the Wiimote.
  * @return A last "3D-position" of the Wiimote.
  */
inline Eigen::Vector3d WmToChem::getPos3dLast(){ return m_lastPosSmooth ; }


/**
  * Get the angle of rotation for the camera on the X-axis, calculated from the joystick
  * of the Nunchuk.
  * @return The angle of rotation for the camera on the X-axis.
  */
inline double WmToChem::getAngleCamRotateXDeg(){ return m_angleNcJoystickCosDeg ; }

/**
  * Get the angle of rotation for the camera on the Y-axis, calculated from the joystick
  * of the Nunchuk.
  * @return The angle of rotation for the camera on the Y-axis.
  */
inline double WmToChem::getAngleCamRotateYDeg(){ return m_angleNcJoystickSinDeg ; }


/**
  * Get the zoom for the camera, calculated from the Nunchuk.
  * @return The zoom for the camera.
  */
inline double WmToChem::getDistCamZoom(){ return m_distCamZoom ; }


/**
  * Get the distance of translation for the camera on the X-axis, calculated from the joystick
  * of the Nunchuk.
  * @return The distance of translation for the camera on the X-axis.
  */
inline double WmToChem::getDistCamTranslateX(){ return m_distCamXTranslate ; }

/**
  * Get the distance of translation for the camera on the X-axis, calculated from the joystick
  * of the Nunchuk.
  * @return The distance of translation for the camera on the X-axis.
  */
inline double WmToChem::getDistCamTranslateY(){ return m_distCamYTranslate ; }


/**
  * Get if the Wiimote is connected.
  * @return TRUE if the Wiimote is connected ; FALSE else.
  */
inline bool WmToChem::isConnected(){ return m_hasWm ; }


/**
  * Get if the Wiimote is disconnected.
  * @return TRUE if the Wiimote is disconnected ; FALSE else.
  */
inline bool WmToChem::isConnectedNc(){ return m_hasNc ; }

/**
  * Get if the Wiimote is moving.
  * @return TRUE if the Wiimote is moving; FALSE else.
  */
inline bool WmToChem::isMovedCursor(){ return (m_doWork ? WMAVO_IS(WMAVO_CURSOR_MOVE) : false) ; }

/**
  * Get if the action Wiimote is "selection".
  * @return TRUE if the action Wiimote is "selection" ; FALSE else.
  */
inline bool WmToChem::isSelected(){ return (WMAVO_IS(WMAVO_SELECT)==0?false:true) ; }

/**
  * Get if the action Wiimote is "creation".
  * @return TRUE if the action Wiimote is "creation" ; FALSE else.
  */
inline bool WmToChem::isCreated(){ return (WMAVO_IS(WMAVO_CREATE)==0?false:true) ; }

/**
  * Get if the action Wiimote is "delete".
  * @return TRUE if the action Wiimote is "delete" ; FALSE else.
  */
inline bool WmToChem::isDeleted(){ return (WMAVO_IS(WMAVO_DELETE)==0?false:true) ; }

/**
  * Get if the action Wiimote is "moving atoms".
  * @return TRUE if the action Wiimote is "moving atoms" ; FALSE else.
  */
inline bool WmToChem::isMovedAtom(){ return (WMAVO_IS(WMAVO_ATOM_MOVE)==0?false:true); }

/**
  * Get if the action Wiimote is "rotating atoms".
  * @return TRUE if the action Wiimote is "rotating atoms" ; FALSE else.
  */
inline bool WmToChem::isRotatedAtom(){ return (m_doWork ? (WMAVO_IS(WMAVO_ATOM_ROTATE)==0?false:true):false) ; }

/**
  * Get if the action Wiimote is "translating atoms".
  * @return TRUE if the action Wiimote is "translating atoms" ; FALSE else.
  */
inline bool WmToChem::isTranslatedAtom(){ return (m_doWork ? (WMAVO_IS(WMAVO_ATOM_TRANSLATE)==0?false:true):false) ; }

//inline bool WmToChem::wmAtomIsZoom(){ return m_isAtomZoom ; } // Not use.

/**
  * Get if the action Wiimote is "rotating the camera".
  * @return TRUE if the action Wiimote is "rotating the camera" ; FALSE else.
  */
inline bool WmToChem::isRotatedCam(){ return (WMAVO_IS(WMAVO_CAM_ROTATE)==0?false:true) ; }

/**
  * Get if the action Wiimote is "translating the camera".
  * @return TRUE if the action Wiimote is "translating the camera" ; FALSE else.
  */
inline bool WmToChem::isTranslatedCam(){ return (WMAVO_IS(WMAVO_CAM_TRANSLATE)==0?false:true) ; }

/**
  * Get if the action Wiimote is "zooming the camera".
  * @return TRUE if the action Wiimote is "zooming the camera" ; FALSE else.
  */
inline bool WmToChem::isZoomedCam(){ return (WMAVO_IS(WMAVO_CAM_ZOOM)==0?false:true) ; }

/**
  * Get if the action Wiimote is "initialized the camera".
  * @return TRUE if the action Wiimote is "initialized the camera" ; FALSE else.
  */
inline bool WmToChem::isInitializedCam(){ return (WMAVO_IS(WMAVO_CAM_INITIAT)==0?false:true) ; }


/**
  * Get if the action Wiimote is "actived the menu".
  * @return TRUE if the action Wiimote is "actived the menu" ; FALSE else.
  */
inline bool WmToChem::isActivedMenu(){ return (WMAVO_IS(WMAVO_MENU_ACTIVE)==0?false:true) ; }
