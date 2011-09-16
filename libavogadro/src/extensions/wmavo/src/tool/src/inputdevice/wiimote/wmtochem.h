
/*******************************************************************************
  Copyright (C) 2010,2011 Mickael Gadroy, University of Reims Champagne-Ardenne (Fr)
                          Project managers: Eric Henon and Michael Krajecki
                          Financial support: Region Champagne-Ardenne (Fr)

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

#pragma once
#ifndef __WMAVO_H__
#define __WMAVO_H__

#ifndef NOMINMAX
#define NOMINMAX
// To avoid a conflit with a macro definition in Eigen/Core.
// If you are compiling on Windows, to prevent windows.h from defining these symbols.
#endif

#include "warning_disable_begin.h"
#include "wmavo_const.h"
#include "wmrumble.h"
#include "wiwo.h"

#include <Eigen/Core>

#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qpoint.h>
#include <QTimer>
#include <QTime>

#include <time.h>
#include <math.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#if !defined WIN32 && !defined _WIN32
  #include <unistd.h>
#endif
#include "warning_disable_end.h"

class CWiimoteData ;

/**
  * @class WmToChem
  * @brief The class which intanciate the Wiiuse library to use the Wiimote
  *
  * It is more complicated ... In fact, this class represents plus a wrapper that
  * just a Wiimote class. This wrapper is destined to translate the Wiimote action
  * to the "actions necessary to build a molecule".
  * <br/>This means that the Avogadro plugin "know" what it must do (action to create atom,
  * action to rotate the camera ...). After, more than 1 actions can be realised,
  * so Avogadro can interpret a combinaison of actions :
  * ex. : "move the mouse cursor" + "select one atom" =
  *       = "select all atoms include in a selection rectangle"
  * This is the 1st part of the wrapper. The 2nd (WMAVO_thread) manage the thread part and communication with other
  * application.
  */
class WmToChem
{

  //
  // Static attributs.
  public:

    static const double m_PI ; ///< Pi approximation.
    static const double m_PI180 ; ///< 3.14/180 for degree to radian.
    static const double m_180PI ; ///< 180/3.14 for radian to degree.
    static const Eigen::Vector3d m_refPoint0 ; ///< Point(0,0,0).
    static const int m_XScreen, m_YScreen ; ///< Resolution of the screen.


  //
  // Public methods.
  public :

    /**
      * @name Heart methods
      * @{ */
    WmToChem( int operatingMode=0 ) ; ///< Constructor
    ~WmToChem() ; ///< Destructor
    bool convert( CWiimoteData *data ) ;
    // @}
    

    /**
      * @name Getter/setter methods.
      * Getter/setter methods for various need.
      * @{ */
    void setOperatingMode(int opMode) ;
    void setMenuMode( bool menuMode ) ;
    void setActionMode( bool actionMode ) ;
    void setSizeWidget( int x, int y, int width, int height ) ;
    void setIrSensitive( int irSensitive ) ;

    int getActions() ;
    int getOperatingMode() ;

    QPoint getPosCursor() ;
    Eigen::Vector3d getPos3dCurrent() ;
    Eigen::Vector3d getPos3dLast() ;
    double getDistCamZoom() ;
    double getAngleCamRotateXDeg() ;	double getAngleCamRotateYDeg() ;
    double getDistCamTranslateX() ;	double getDistCamTranslateY() ;
    
    bool isConnected() ; bool isConnectedNc() ;
    bool isMovedCursor() ; bool isSelected() ;
    bool isCreated() ; bool isDeleted() ;
    bool isMovedAtom() ; ///< Indicate if wmAtomIsRotate() or wmAtomIsTranslate() == true
    bool isRotatedAtom() ;	bool isTranslatedAtom() ;/* bool wmAtomIsZoom() ;*/
    bool isRotatedCam() ; bool isTranslatedCam() ; bool isZoomedCam() ;
    bool isInitializedCam() ;
    bool isActivedMenu() ;
    // @}

  //
  // Protected methods.
  protected:

    /**
      * @name Various operating mode and sub-methods for the choice of the operating mode
      * Various global methods which interpret the Wiimote signal to "the construction of the
      * molecule" paradigm,
      * Decomposition of wmOperatingMode*() methods according to the Wiimote or
      * the Nunchuk.
      * @{ */
    void operatingMode1() ;
    void operatingMode2() ;
    void operatingMode3() ;
    void operatingModeWm2() ;
    void operatingModeNc1() ;
    void operatingModeNc2() ;
    // @}

    /**
      * @name Transform Wiimote actions to something.
      * Decomposition of wmOperatingModeWm*() methods.
      * @{ */
    bool transformWmAction1ToMouseMovement() ;
    bool transformWmAction1ToSelectOrTranslateAtom() ;
    bool transformWmAction1ToCreateAtom() ;
    bool transformWmAction1ToDeleteAtom() ;
    bool transformWmAction1ToInitCamera() ;
    bool transformWmAction1ToOKMenu() ;
    bool transformWmAction1ToRotateAtomOrActivateMenu() ;
    bool transformWmAction1ToMoveInMenu() ;
    // @}

    /**
      * @name Transform Nunchuk actions to something.
      * Decomposition of wmOperatingModeNc*() methods.
      * @{ */
    bool transformNcAction1ToZoomIn() ;
    bool transformNcAction1ToZoomOut() ;
    bool transformNcAction1ToTranslateCamera() ;
    bool transformNcAction1ToRotateCameraOrFinalizeTranslateCamera( float angle, float magnitude ) ;

    bool transformNcAction2ToZoom() ;
    bool transformNcAction2ToTranslateCamera() ;
    bool transformNcAction2ToRotateCamera() ;
    bool transformNcAction2ToNothingForCamera() ;
    bool transformNcAction2ToCalculCameraMovement( float angle, float magnitude ) ;
    // @}


    /**
      * @name Calculate some informations
      * This informations are the get data from the Wiimote which must be
      * calibrated/calculated/adjusted.
      */
    Eigen::Vector3d getIrCursor() ; ///< Update m_wmCurrentIrCursorPos and m_wmLastIrCursorPos.
    bool getSmoothedCursor() ; ///< Update m_currentPosSmooth and m_lastPosSmooth.
    float getAcc() ;

    void calibrateCamRotateAngles( float angle, float magnitude ) ;
        ///< Update m_angleNcJoystickCosDeg, m_angleNcJoystickSinDeg
    void calibrateCamTranslateDist( float angle, float magnitude ) ;
        ///< Update m_distCamXTranslate, m_distCamYTranslate
    void calibrateCamZoomDist( float angle, float magnitude ) ;
        ///< Update m_distCamZoom

    void translateCursorToScreen() ;
        ///< Update m_posCursor
    bool actionZoom( bool active, bool pause=false ) ;
        ///< Update m_activeZoom & m_nbZoom
    // @}


  //
  // Private attributs
  private:

    CWiimoteData *m_wmData ;
    CNunchukData *m_ncData ;
    bool m_hasWm, m_hasNc ;
    bool m_otherPoll ;

    int m_operatingMode ; ///< A mode defines "what buttons for what actions".
    int m_irSensitive ;

    /**
      * @name Wiimote/Chemical states
      * @{ */
    int m_isWhat ;
    // @}

    /**
      * @name Differents positions of the IR cursor points.
      * @{ */
    QPoint m_posCursor ;
    Eigen::Vector3d m_wmCurrentIrCursorPos, m_wmLastIrCursorPos ;
    // @}

    /**
      * @name Movements informations
      * @{ */
    double m_angleNcJoystickCosDeg, m_angleNcJoystickSinDeg ;
    double m_distCamZoom, m_distCamXTranslate, m_distCamYTranslate ;
    // @}

    /**
      * @name Manage smoothing
      * @{ */
    WIWO<Eigen::Vector3d> *m_smoothXY ;
    Eigen::Vector3d m_lastSmooth, m_diffSmooth, m_diffPos ;
    Eigen::Vector3d m_lastPosSmooth, m_currentPosSmooth ;

    WIWO<double> *m_smoothAcc ;
    double m_lastAccSmooth, m_diffAccSmooth ;
    // @}

    /**
      * @name Calibration & Co
      * @{ */
    bool m_activeZoom ;
    int m_nbZoom ;
    QTime m_time ;
    int m_t1, m_t2 ;
    int m_timeFirst, m_timeSecond ; // In ms
    int m_timeTmp1, m_timeTmp2 ;
    // @}

    /**
      * @name Use to translate ...
      * ... The Wiimote actions to "the construction of the molecule" paradigm.
      * Some attributs let to decrease number of signal,
      * other, let an action to finish itself, etc.
      * @{ */
    bool m_doWork ; ///< If the cursor has really moved.
    bool m_pressedButton ;
    bool m_selectRelease, m_homeRelease ;
    bool m_addRelease, m_delRelease ;
    bool m_crossRelease, m_crossReleaseEnd ;
    int m_lastCrossAction ; ///< 0:Nothing, 1:Right, 2:Down ...
    bool m_crossMenuRelease, m_crossMenuTimeOut ;
    bool m_okMenuRelease, m_okMenuReleaseEnd ;
    bool m_delAllAlready ;
    // @}

    /**
      * @name Size of the widget
      * @{ */
    int m_xWidget, m_yWidget, m_widthWidget, m_heightWidget ;
    // @}

  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
      ///< http://eigen.tuxfamily.org/dox/StructHavingEigenMembers.html
      ///< http://eigen2.sourcearchive.com/documentation/2.0.10/StructHavingEigenMembers.html

};


#include "wmtochem.inl"

#endif
