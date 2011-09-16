
/*******************************************************************************
  Copyright (C) 2010,2011 Mickael Gadroy, University of Reims Champagne-Ardenne (Fr)
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


/// All constants use by the plugins.


#ifndef __WMAVO_CONST_H__
#define __WMAVO_CONST_H__

#define __WMDEBUG_WMTOOL 0
#define __WMDEBUG_CHEMWRAPPER 0
#define __WMDEBUG_WMDEVICE 0
#define __WMDEBUG_MOLMANIP 0


///**********************************************************
/// Naming plugins, descriptions ...
/// Specific for wmextension.h & wmtool.h.
///**********************************************************

#define PLUGIN_WMTOOL_IDENTIFIER "wm"
#define PLUGIN_WMTOOL_NAME "WmTool"
#define PLUGIN_WMTOOL_DESCR "Use Wiimote in Avogadro"
#define PLUGIN_WMTOOL_WIDGET_TITLE "WmTool Title"

#define PLUGIN_DRAWTOOL_NAME "Draw"

#define WMTOOL_TIME_DISPLAY_MSG 10000

#define PLUGIN_WM_SENSITIVE_DEFAULT 20
#define PLUGIN_WM_SENSITIVE_MIN 15
#define PLUGIN_WM_SENSITIVE_MAX 40

#define PLUGIN_WM_VIBRATION_ONOFF true

#define CIRBUFFER_DEFAULT_SIZE 20

#define PLUGIN_WM_SLEEPTHREAD_ONOFF true 
#define PLUGIN_WM_SLEEPTHREAD_TIME 8 // ms
#define PLUGIN_WM_SLEEPTHREAD_NBTIME_BEFORE_SLEEP 5
#define PLUGIN_WM_TIMEOUT_BEFORE_SENDDATA 17 // ms,  16.6ms ~=> 60frames/second
#define PLUGIN_WM_NBLOSTDATA_BEFORE_COUNTERSTRIKE 5 // To have a threshold 
    // before to send data to WmTool even if it do not respond.


///**********************************************************
/// Specific for wmextension.h.
///**********************************************************

#define WMEX_DISTBEFORE_CREATE 0.6
#define WMEX_MAXBONDNUMBER_BETWEENATOM 3
#define WMEX_CREATEDATOMDEFAULT 6 // Carbon.
#define WMEX_ADJUST_HYDROGEN 1 // true


///**********************************************************
/// Specific for wmavo_thread.h.
///**********************************************************

#define WMAVOTH_SLEEPBEFORE_NEXTROUND 10 //4 // ms
#define WMAVOTH_DIST_MIN 1.5 // Minimum distance before beginning atomic reaction.
#define WMAVOTH_DIST_MAX 6.0 // Maximum distance, after : "no interaction" between atom.


///**********************************************************
/// Specific for wmavo.h and use by wmextension.h.
// Many constants values are got by experience (and they have not unit...).
///**********************************************************

#define WMAVO_CONNECTION_TIMEOUT 5.0 // Computer lets few time to the connection of the Wiimote (in seconde).
#define WMAVO_IRSENSITIVITY 3
#define WMAVO_WIWO_IR_SMOOTH 10
#define WMAVO_ACTIVATEMENU_AFTER 700 //ms // 1000
#define WMAVO_DELETEALL_AFTER 2000 //ms // 3000
#define WMAVO_ZOOM_STEP 0.01 // For zoom, it is a step.

#define WMAVO_CURSOR_CALIBRATION_X 2.4 // 2.5
#define WMAVO_CURSOR_CALIBRATION_Y 2.4 // 3.0
#define WMAVO_CURSOR_CALIBRATION_Z 1.2 // 1.8 // 2.0

#define WMAVO_CAM_CALIBRATION_ZOOM_DISTANCE 0.1
#define WMAVO_CAM_CALIBRATION_ROTATION 9.0 // 10.0
#define WMAVO_CAM_CALIBRATION_TRANSLATION 0.9 // 1.0

#define WMAVO_ATOM_MAX_MOVE_Z 0.08 //0.1
#define WMAVO_ATOM_SMOOTHED_MOVE_XY 4
#define WMAVO_ATOM_MAX_MOVE_FOR_PROHITED_MOVE 0.001 // 0.0
#define WMAVO_ATOM_ROTATION_SPEED 0.2

#define WMAVO_WM_XY_SELECTING 18 // Nb. pixel before activate move of atom. (Not use)
#define WMAVO_WM_XY_MINPOINTING_MOVEALLOWED 0
#define WMAVO_WM_XY_MAXPOINTING_MOVEALLOWED 60 // 40 Normally to avoid "jumping cursor".
  //Remember: cursor moves itself by difference calculate here (wmavo), not by a position gets by wiiuse.

#define WMAVO_WM_Z_MINPOINTING_MOVEALLOWED 0.2 // 0.1

#define WMAVO_NC_MINJOY_MOVEALLOWED 0.1

#define WMAVO_NBOPERATINGMODE 3
#define WMAVO_OPERATINGMODE1 0
#define WMAVO_OPERATINGMODE2 1
#define WMAVO_OPERATINGMODE3 2

// In m_isWhat => the events of the Wiimote for molecular visualisation.
#define WMAVO_IS(action) (m_isWhat & action)
#define WMAVO_IS2(allActions, action) (allActions & action) // For wmextension.cpp
#define WMAVO_SETON(action) (m_isWhat |= action)
#define WMAVO_SETOFF(action) (m_isWhat &= (~action))
#define WMAVO_SETON2(allActions, action) (allActions |= action)
#define WMAVO_SETOFF2(allActions, action) (allActions &= (~action))


#define WMAVO_CURSOR_MOVE       0x000001
#define WMAVO_SELECT            0x000002
#define WMAVO_SELECT_MULTI      0x000004
#define WMAVO_CREATE            0x000008
#define WMAVO_DELETE            0x000010
#define WMAVO_DELETEALL         0x000020
#define WMAVO_ATOM_MOVE         0x000040 // Translate or rotate, the mode is "activate", 
                                          // but it can be "not translating or rotating" 
                                          // to limit the calling of wmextension.
#define WMAVO_ATOM_ROTATE       0x000080
#define WMAVO_ATOM_TRANSLATE    0x000100
#define WMAVO_CAM_ROTATE        0x000200
#define WMAVO_CAM_ROTATE_BYNC   0x000400
#define WMAVO_CAM_ROTATE_BYWM   0x000800
#define WMAVO_CAM_ZOOM          0x001000
#define WMAVO_CAM_TRANSLATE     0x002000
#define WMAVO_CAM_INITIAT       0x004000
#define WMAVO_MENU_ACTIVE       0x008000
#define WMAVO_MENU_RIGHT        0x010000
#define WMAVO_MENU_LEFT         0x020000
#define WMAVO_MENU_UP           0x040000
#define WMAVO_MENU_DOWN         0x080000
#define WMAVO_MENU_OK           0x100000


///**********************************************************
/// Specific for wmavo_rumble.h.
///**********************************************************


#define TIME_TRYLOCK 5 // µs
#define WMRUMBLE_TIME_SLEEP 2 // ms
#define WMRUMBLE_MAX_TIME 60000// ms
#define WMRUMBLE_MIN_TIME 80 // 50 // ms CAUTION : do not enter less, else the Wiimote sinks under Bluetooth connection, and PC blocks because mutex is always activate.

// Use for set gradual => [0;100]
// 0 => disable rumble
// 1 => WMRUMBLE_MIN_DURATION_TREMOR & WMRUMBLE_MAX_DURATION_POSE
// ...
// 99 => WMRUMBLE_MAX_DURATION_TREMOR & WMRUMBLE_MIN_DURATION_POSE
// 100 => active rumble
#define WMRUMBLE_MAX_DURATION_TREMOR 800 // 1000
#define WMRUMBLE_MIN_DURATION_TREMOR WMRUMBLE_MIN_TIME // 150 // 30
#define WMRUMBLE_MAX_DURATION_POSE 1000 // 2000 // ((fr) "prendre un pose")
#define WMRUMBLE_MIN_DURATION_POSE WMRUMBLE_MIN_TIME // 150 //


///**********************************************************
/// Specific for wmtool.h.
///**********************************************************

// Fonts in the render zone.

#define WMTOOL_POINTSIZE_RATIO_MIN 0.7f
#define WMTOOL_POINTSIZE_RATIO_MAX 3.0f
#define WMTOOL_POINTSIZE_RATIO_DEFAULT 1.0f

#define WMTOOL_SPACING_LEFT_WORDGRP 20 //70
#define WMTOOL_SPACING_DOWN_WORDGRP 10 //25
#define WMTOOL_SPACING_H_WORDGRP 10
#define WMTOOL_SPACING_V_WORDGRP 10

// Family : 
// QFont::AnyStyle, QFont::System
// QFont::Helvetica "0"
// QFont::Serif "Times" == QFont::Times
// QFont::TypeWriter "Courier" == QFont::Courier
// QFont::OldEnglish == QFont::Decorative "OldEnglish"
#define WMTOOL_FONT_FAMILY_INFO "0"
#define WMTOOL_FONT_FAMILY_ERROR "Times"
#define WMTOOL_FONT_FAMILY_DISTDIEDREINFO "0"
#define WMTOOL_FONT_FAMILY_DISTDIEDREATOM "0"

// PointSize : default : 12
#define WMTOOL_FONT_POINTSIZE_INFO 12
#define WMTOOL_FONT_POINTSIZE_ERROR 14
#define WMTOOL_FONT_POINTSIZE_DISTDIEDREINFO 12
#define WMTOOL_FONT_POINTSIZE_DISTDIEDREATOM 14

// Weight : QFont::Light, QFont::Normal, QFont::DemiBold, QFont::Bold, QFont::Black
#define WMTOOL_FONT_WEIGHT_INFO QFont::Normal
#define WMTOOL_FONT_WEIGHT_ERROR QFont::Bold
#define WMTOOL_FONT_WEIGHT_DISTDIEDREINFO QFont::Bold
#define WMTOOL_FONT_WEIGHT_DISTDIEDREATOM QFont::DemiBold

#endif

