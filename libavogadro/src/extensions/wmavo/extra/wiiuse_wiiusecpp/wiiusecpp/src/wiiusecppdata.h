/*******************************************************************************
 *    wiiusecppdata.h
 *
 *    Copyright (c) 2011 Mickael Gadroy
 *
 *    This file is part of wiiusecpp.
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *******************************************************************************/

#ifndef WIIUSECPPDATA_H_
#define WIIUSECPPDATA_H_

// If you want change <wiiusecpp.h> <-> "wiiusecpp.h"
// do not forget to do the same in the makefile.
//#ifdef _WIN32
#include "wiiusecpp.h"
//#else
//#include <wiiusecpp.h>
//#endif

class CButtonBaseData
{
public:
  CButtonBaseData() ;
  CButtonBaseData( const CButtonBase &cbb ) ;
  ~CButtonBaseData() ;

  short getBtnsState() ;

  bool isPressed(int Button) ;
  bool isHeld(int Button) ;
  bool isReleased(int Button) ;
  bool isJustPressed(int Button) ;
  bool isJustChanged() ;

private:
  short mpBtns ;				/**< what buttons have just been pressed	*/
  short mpBtnsHeld ;		/**< what buttons are being held down		*/
  short mpBtnsReleased ;/**< what buttons were just released this	*/
  short mpBtnsPrevious ;/**< what previous pressed buttons state	*/
};


class CButtonsData : public CButtonBaseData
{
public:
  CButtonsData() ;
  CButtonsData( const CButtons &cb ) ;
  ~CButtonsData() ;
};


class CNunchukButtonsData : public CButtonBaseData
{
public:
  CNunchukButtonsData() ;
  CNunchukButtonsData( const CNunchukButtons &cnb ) ;
  ~CNunchukButtonsData() ;
};


class CClassicButtonsData : public CButtonBaseData
{
public:
  CClassicButtonsData() ;
  CClassicButtonsData( const CClassicButtons &ccb ) ;
  ~CClassicButtonsData() ;
};


class CGH3ButtonsData : public CButtonBaseData
{
public:
  CGH3ButtonsData() ;
  CGH3ButtonsData( const CGH3Buttons &ccb ) ;
  ~CGH3ButtonsData() ;
};


class CJoystickData
{
public:
  CJoystickData() ;
  CJoystickData(const CJoystick &joystick) ;
  ~CJoystickData() ;
  
  void GetMaxCal(int &X, int &Y) ;
  void GetMinCal(int &X, int &Y) ;
  void GetCenterCal(int &X, int &Y) ;
  void GetPosition(float &Angle, float &Magnitude) ;

private:
  struct joystick_t mpJoystick;
};


class CAccelerometerData
{
public:
  CAccelerometerData() ;
  CAccelerometerData( const CAccelerometer &ca ) ;
  ~CAccelerometerData() ;

  // Threshold ((fr:)seuil) for orient to generate an event.
  float GetOrientThreshold();
  // Threshold for accel to generate an event.
  int GetAccelThreshold() ;
  // Get value for pitch, roll (and yaw when it is possible).
  void GetOrientation(float &Pitch, float &Roll, float &Yaw) ;
  // Get value for pitch, roll (unsmoothed).
  void GetRawOrientation(float &Pitch, float &Roll);

  /*
   * Represent the accel calibrated values get from the Wiimote for the 0g and 1g.
   * @{*/
  void GetAccCalOne(float &X, float &Y, float &Z) ;
  void GetAccCalZero(float &X, float &Y, float &Z);
  // @}

  /*
   * Get GForce on 3 axes.
   * @{*/
  void GetGForceInG(double &X, double &Y, double &Z) ; //< In g unit.
  double GetGForceInG() ; //< In g unit.
  void GetGForceInMS2(double &X, double &Y, double &Z) ; //< in m.s^-2 unit.
  double GetGForceInMS2() ; //< in m.s^-2 unit.
  double GetGForceElapse() ; //< in m.s^-2 unit.
  // @}

  /*
   * Get GForce on 3 axes.
   * @{*/
  void GetAcceleration(double &X, double &Y, double &Z) ; //< in m.s^-2 unit.
  double GetAcceleration() ; //< in m.s^-2 unit.
  // @}
  
  /**
    * Difference between the last and the current gForce values.
    * @{ */
  void GetJerkInMS3(double &X, double &Y, double &Z) ; //< in m.s^-3 unit.
  double GetJerkInMS3() ; //< in m.s^-3 unit.
  // @}

  /*
   * An estimation of the instant velocity in m.s^-1.
   * @{*/
  void GetVelocity(double &X, double &Y, double &Z) ;
  double GetVelocity() ;
  // @}

  /*
   * An estimation of the traveled distance in m.
   * @{*/
  void GetDistance(double &X, double &Y, double &Z) ;
  double GetDistance() ;
  // @}

  // An estimation of the position according to the 1st position of the Wiimote.
  void GetPosition( double &X, double &Y, double &Z ) ;
  void GetTime( double &time, double &elapseTime ) ;
  void GetTmp( double &tmp0, double &tmp1, double &tmp2) ;

private:

  accel_t mpAccelCalib ;
  vec3b_t mpAccel ;
  int mpAccelThreshold ;

  orient_t mpOrient ;
  float mpOrientThreshold ;

  CAccelerometer::values_t mpValues ;
  double mpGForceElapse ;
};

class CIRDotData
{
public:
  CIRDotData() ;
  CIRDotData( const ir_dot_t &cird ) ;
  ~CIRDotData() ;

  int isVisible();
  int GetSize();
  int GetOrder();
  void GetCoordinate(int &X, int &Y);
  void GetRawCoordinate(int &X, int &Y);

private:
  ir_dot_t mpDotPtr ;
};

class CIRData
{
public:
  CIRData() ;
  CIRData( const CIR &ci ) ;
  ~CIRData() ;

  CIR::BarPositions GetBarPositionSetting() ;
  CIR::AspectRatioSelections GetAspectRatioSetting();

  int GetSensitivity();
  int GetNumDots();
  int GetNumSources() ;
  vector<CIRDotData>& GetDots();

  void GetOffset(int &X, int &Y);
  int GetState();
  void GetCursorPositionAbsolute(int &X, int &Y);
  void GetCursorPosition(int &X, int &Y);
  void GetCursorDelta(double &X, double &Y, double &Z);
  float GetPixelDistance();
  float GetDistance();
  bool IsInPrecisionMode();

private:

  int mpState ;
  ir_t mpIr ;
  vector<CIRDotData> mpIRDotsVector;
};

class CNunchukData
{
public:
  CNunchukData();
  CNunchukData( const CNunchuk &cn ) ;
  ~CNunchukData() ;

  CNunchukButtonsData Buttons;
  CJoystickData Joystick;
  CAccelerometerData Accelerometer;
};

class CClassicData
{
public:
  CClassicData() ;
  CClassicData( const CClassic &cc ) ;
  ~CClassicData() ;

  float GetLShoulderButton() ;
  float GetRShoulderButton() ;

  CClassicButtons Buttons ;
  CJoystick LeftJoystick ;
  CJoystick RightJoystick ;

private:
  float mpLShoulderButton ;
  float mpRShoulderButton ;
};

class CGuitarHero3Data
{
public:
  CGuitarHero3Data() ;
  CGuitarHero3Data( const CGuitarHero3 &cgh ) ;
  ~CGuitarHero3Data() ;

  float GetWhammyBar() ;

  CGH3ButtonsData Buttons ;
  CJoystickData Joystick ;

private:
  float mpWhammyBar ;
};

class CExpansionDeviceData
{
public:
  CExpansionDeviceData() ;
  CExpansionDeviceData( const CExpansionDevice &ced ) ;
  ~CExpansionDeviceData() ;

  CExpansionDevice::ExpTypes GetType() ;

  CNunchukData Nunchuk ;
  CClassicData Classic ;
  CGuitarHero3Data GuitarHero3 ;

private:
  int mpExpTypes ;
};


class CWiimoteData
{
public:
  CWiimoteData() ;
  CWiimoteData( const CWiimote &cw ) ;
  ~CWiimoteData() ;

  int GetLEDs();
  float GetBatteryLevel();
  CWiimote::EventTypes GetEvent();

  int GetID();
  int GetState();
  int GetFlags();

  bool isUsingACC();
  bool isUsingEXP();
  bool isUsingIR();
  bool isUsingSpeaker();
  bool isLEDSet(int LEDNum);

  bool isConnected() ;
  bool isPolled() ;
  bool isPolledByIR() ;
  bool isPolledByAcc() ;
  bool isPolledByButton() ;


  CIRData IR;
  CButtonsData Buttons;
  CAccelerometerData Accelerometer;
  CExpansionDeviceData ExpansionDevice;

private:
  int mpUnid ;
  byte mpLEDs ;
  float mpBattery ;
  CWiimote::EventTypes mpEvent ;
  int mpState ;
  int mpFlags ;

  bool mpIsConnected ;
  bool mpIsPolled ;
  bool mpIsPolledButton ;
  bool mpIsPolledAcc ;
  bool mpIsPolledIR ;

};

#endif
