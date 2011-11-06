/*
 *    wiiusecppdata.cpp
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
 */

#include "wiiusecppdata.h"


CButtonBaseData::CButtonBaseData() :
  mpBtns(0), mpBtnsHeld(0), mpBtnsReleased(0)
{}

CButtonBaseData::CButtonBaseData( const CButtonBase &cbb ) :
  mpBtns(0), mpBtnsHeld(0), mpBtnsReleased(0)
{
  if( cbb.mpBtnsPtr != NULL ) mpBtns = *(cbb.mpBtnsPtr) ;
  if( cbb.mpBtnsHeldPtr != NULL ) mpBtnsHeld = *(cbb.mpBtnsHeldPtr) ;
  if( cbb.mpBtnsReleasedPtr != NULL ) mpBtnsReleased = *(cbb.mpBtnsReleasedPtr) ;
  if( cbb.mpBtnsPreviousPtr != NULL ) mpBtnsPrevious = *(cbb.mpBtnsPreviousPtr) ;
}

CButtonBaseData::~CButtonBaseData(){}
short CButtonBaseData::getBtnsState()          { return mpBtns ; }
bool CButtonBaseData::isPressed(int Button)    { return (mpBtns & Button) == Button; }
bool CButtonBaseData::isHeld(int Button)       { return (mpBtnsHeld & Button) == Button; }
bool CButtonBaseData::isReleased(int Button)   { return (mpBtnsReleased & Button) == Button; }
bool CButtonBaseData::isJustPressed(int Button){ return (((mpBtns & Button)==Button) && ((mpBtnsHeld & Button)!=Button)) ; }
bool CButtonBaseData::isJustChanged()          { return mpBtns != mpBtnsPrevious ; }

CButtonsData::CButtonsData() : CButtonBaseData() {}
CButtonsData::CButtonsData( const CButtons &cb ) : CButtonBaseData(cb) {}
CButtonsData::~CButtonsData() {}

CNunchukButtonsData::CNunchukButtonsData() : CButtonBaseData() {}
CNunchukButtonsData::CNunchukButtonsData( const CNunchukButtons &cb ) : CButtonBaseData(cb) {}
CNunchukButtonsData::~CNunchukButtonsData() {}

CClassicButtonsData::CClassicButtonsData() : CButtonBaseData() {}
CClassicButtonsData::CClassicButtonsData( const CClassicButtons &cb ) : CButtonBaseData(cb) {}
CClassicButtonsData::~CClassicButtonsData() {}

CGH3ButtonsData::CGH3ButtonsData() : CButtonBaseData() {}
CGH3ButtonsData::CGH3ButtonsData( const CGH3Buttons &cb ) : CButtonBaseData(cb) {}
CGH3ButtonsData::~CGH3ButtonsData() {}

CJoystickData::CJoystickData()
{
  wiiuse_init_joystick_t( &mpJoystick ) ;
}
CJoystickData::CJoystickData(const CJoystick &cj)
{ 
  if( cj.mpJoystickPtr != NULL )
    mpJoystick = *(cj.mpJoystickPtr) ;
  else
    wiiuse_init_joystick_t( &mpJoystick ) ;
}

CJoystickData::~CJoystickData() {}
void CJoystickData::GetMaxCal(int &X, int &Y)    { X = mpJoystick.max.x; Y = mpJoystick.max.y; };
void CJoystickData::GetMinCal(int &X, int &Y)    { X = mpJoystick.min.x; Y = mpJoystick.min.y; };
void CJoystickData::GetCenterCal(int &X, int &Y) { X = mpJoystick.center.x; Y = mpJoystick.center.y; };
void CJoystickData::GetPosition(float &Angle, float &Magnitude){ Angle = mpJoystick.ang; Magnitude = mpJoystick.mag; };

CAccelerometerData::CAccelerometerData() :
  mpAccelThreshold(0), mpOrientThreshold(0), mpValues()
{
  wiiuse_init_accel_t( &mpAccelCalib ) ;
  wiiuse_init_vec3b_t( &mpAccel ) ;
  wiiuse_init_orient_t( &mpOrient ) ;
}

CAccelerometerData::CAccelerometerData( const CAccelerometer &ca )
{
  if( ca.mpAccelCalibPtr != NULL )
    mpAccelCalib = *(ca.mpAccelCalibPtr) ;
  else
    wiiuse_init_accel_t( &mpAccelCalib ) ;

  if( ca.mpAccelPtr != NULL )
    mpAccel = *(ca.mpAccelPtr) ;
  else
    wiiuse_init_vec3b_t( &mpAccel ) ;

  if( ca.mpAccelThresholdPtr != NULL )
    mpAccelThreshold = *(ca.mpAccelThresholdPtr) ;
  else
    mpAccelThreshold = 0 ;

  if( ca.mpOrientPtr != NULL )
    mpOrient = *(ca.mpOrientPtr) ;
  else
    wiiuse_init_orient_t( &mpOrient ) ;
  
  if( ca.mpOrientThresholdPtr != NULL )
    mpOrientThreshold = *(ca.mpOrientThresholdPtr) ;
  else
    mpOrientThreshold = 0 ;

  mpValues = ca.mpValuesInTime[0] ;
  mpGForceElapse = ca.mpValuesInTime[0].gForce-ca.mpValuesInTime[1].gForce ;
}

CAccelerometerData::~CAccelerometerData(){}

float CAccelerometerData::GetOrientThreshold()
{ return mpOrientThreshold ; };
int CAccelerometerData::GetAccelThreshold()
{ return mpAccelThreshold ; };

void CAccelerometerData::GetOrientation(float &Pitch, float &Roll, float &Yaw)
{
  Pitch = mpOrient.pitch ;
  Roll = mpOrient.roll ;
  Yaw = mpOrient.yaw ;
}
void CAccelerometerData::GetRawOrientation(float &Pitch, float &Roll)
{
  Pitch = mpOrient.a_pitch;
  Roll = mpOrient.a_roll;
}
void CAccelerometerData::GetAccCalOne(float &X, float &Y, float &Z)
{
  X = (float)(mpAccelCalib.cal_g.x + mpAccelCalib.cal_zero.x) ;
  Y = (float)(mpAccelCalib.cal_g.y + mpAccelCalib.cal_zero.y) ;
  Z = (float)(mpAccelCalib.cal_g.z + mpAccelCalib.cal_zero.z) ;
}
void CAccelerometerData::GetAccCalZero(float &X, float &Y, float &Z)
{
  X = (float)mpAccelCalib.cal_zero.x ;
  Y = (float)mpAccelCalib.cal_zero.y ;
  Z = (float)mpAccelCalib.cal_zero.z ;
}

double CAccelerometerData::GetGForceElapse(){ return mpGForceElapse ; }
double CAccelerometerData::GetGForceInG(){ return mpValues.gForce/WIIUSECPP_GUNIT_TO_MS2UNIT ; }
double CAccelerometerData::GetGForceInMS2(){ return mpValues.gForce; }
void CAccelerometerData::GetGForceInG(double &X, double &Y, double &Z)
{
  X = mpValues.gForceX/WIIUSECPP_GUNIT_TO_MS2UNIT ;
  Y = mpValues.gForceY/WIIUSECPP_GUNIT_TO_MS2UNIT ;
  Z = mpValues.gForceZ/WIIUSECPP_GUNIT_TO_MS2UNIT ;
}
void CAccelerometerData::GetGForceInMS2(double &X, double &Y, double &Z)
{
  X = mpValues.gForceX ; Y = mpValues.gForceY ; Z = mpValues.gForceZ ;
}

double CAccelerometerData::GetAcceleration(){ return mpValues.acceleration; }
void CAccelerometerData::GetAcceleration(double &X, double &Y, double &Z)
{
  X = mpValues.accelerationX ; Y = mpValues.accelerationY ; Z = mpValues.accelerationZ ;
}

double CAccelerometerData::GetJerkInMS3(){ return mpValues.jerk ; }
void CAccelerometerData::GetJerkInMS3(double &X, double &Y, double &Z)
{
  X = mpValues.jerkX ; Y = mpValues.jerkY ; Z = mpValues.jerkZ ;
}

double CAccelerometerData::GetVelocity(){ return mpValues.velocity ; }
void CAccelerometerData::GetVelocity(double &X, double &Y, double &Z) 
{
  X = mpValues.velocityX ; Y = mpValues.velocityY ; Z = mpValues.velocityZ ;
}

double CAccelerometerData::GetDistance(){ return mpValues.distance ; }
void CAccelerometerData::GetDistance(double &X, double &Y, double &Z)
{
  X = mpValues.distanceX ; Y = mpValues.distanceY ; Z = mpValues.distanceZ ; 
}

void CAccelerometerData::GetPosition( double &X, double &Y, double &Z )
{
  X = mpValues.positionX ; Y = mpValues.positionY ; Z = mpValues.positionZ ;
}

void CAccelerometerData::GetTime( double &time, double &elapseTime )
{
  time = mpValues.time ; elapseTime = mpValues.diffTime ;
}

void CAccelerometerData::GetTmp( double &tmp0, double &tmp1, double &tmp2)
{
  tmp0 = mpValues.tmp1 ;tmp1 = mpValues.tmp2 ;tmp2 = mpValues.tmp3 ;
}

CIRDotData::CIRDotData()
{
  mpDotPtr.order = 0 ;
  mpDotPtr.rx = 0 ;
  mpDotPtr.ry = 0 ;
  mpDotPtr.size = 0 ;
  mpDotPtr.visible = 0 ;
  mpDotPtr.x = 0 ;
  mpDotPtr.y = 0 ;
}
CIRDotData::CIRDotData( const ir_dot_t &ird ) : mpDotPtr(ird) {}
CIRDotData::~CIRDotData(){}
int CIRDotData::isVisible(){ return mpDotPtr.visible ; }
int CIRDotData::GetSize(){ return mpDotPtr.size ; }
int CIRDotData::GetOrder(){ return mpDotPtr.order ; }
void CIRDotData::GetCoordinate(int &X, int &Y)
{
    X = mpDotPtr.x;
    Y = mpDotPtr.y;
}
void CIRDotData::GetRawCoordinate(int &X, int &Y)
{
  X = mpDotPtr.rx ;
  Y = mpDotPtr.ry ;
}


CIRData::CIRData() {}
CIRData::CIRData( const CIR &ci )
{
  if( ci.mpWiimotePtr != NULL )
  {
    mpState = ci.mpWiimotePtr->state ;
    mpIr = ci.mpWiimotePtr->ir ;
    for( int i=0 ; i < (int)ci.mpIRDotsVector.size() ; i++ )
      mpIRDotsVector.push_back( CIRDotData(mpIr.dot[i]) ) ;
  }
  else
  {
    mpState = 0 ;
    mpIRDotsVector.clear() ;
  }
}

CIRData::~CIRData(){}

CIR::BarPositions CIRData::GetBarPositionSetting(){ return (CIR::BarPositions) mpIr.pos ; }
CIR::AspectRatioSelections CIRData::GetAspectRatioSetting(){ return (CIR::AspectRatioSelections) mpIr.aspect ; }

int CIRData::GetSensitivity()
{ 
  int level=0 ;
  if( mpState & 0x0200 )      level = 1;
  else if( mpState & 0x0400 ) level = 2;
  else if( mpState & 0x0800 ) level = 3;
  else if( mpState & 0x1000 ) level = 4;
  else if( mpState & 0x2000 ) level = 5;
  
  return level;
}
int CIRData::GetNumDots(){ return mpIr.num_dots ; }
int CIRData::GetNumSources(){ return mpIr.nb_source_detect ; }
vector<CIRDotData>& CIRData::GetDots(){ return mpIRDotsVector ; }
void CIRData::GetOffset(int &X, int &Y)
{
  X = mpIr.offset[0];
  Y = mpIr.offset[1];
}
int CIRData::GetState(){return mpIr.state ; }
void CIRData::GetCursorPositionAbsolute(int &X, int &Y)
{
  X = mpIr.ax;
  Y = mpIr.ay;
}
void CIRData::GetCursorPosition(int &X, int &Y)
{
  X = mpIr.x ;
  Y = mpIr.y ;
}
void CIRData::GetCursorDelta(double &X, double &Y, double &Z)
{
  X = mpIr.deltax ;
  Y = mpIr.deltay ;
  Z = mpIr.deltaz ;
}

float CIRData::GetPixelDistance(){ return mpIr.distance ; }
float CIRData::GetDistance(){ return mpIr.z ; }
bool CIRData::IsInPrecisionMode(){ return (mpIr.isInPrecisionMode==0?false:true) ; }


CNunchukData::CNunchukData(){}
CNunchukData::CNunchukData( const CNunchuk &cn )
{
  if( cn.mpNunchukPtr != NULL )
  {
    Buttons = cn.Buttons ;
    Joystick = cn.Joystick ;
    Accelerometer = cn.Accelerometer ;
  }
}
CNunchukData::~CNunchukData(){}


CClassicData::CClassicData() : mpLShoulderButton(0), mpRShoulderButton(0){}
CClassicData::CClassicData( const CClassic &cc )
{
  if( cc.mpClassicPtr != NULL )
  {
    mpLShoulderButton = cc.mpClassicPtr->l_shoulder ;
    mpRShoulderButton = cc.mpClassicPtr->r_shoulder ;
    Buttons = cc.Buttons ;
    LeftJoystick = cc.LeftJoystick ;
    RightJoystick = cc.RightJoystick ;
  }
  else
  {
    mpLShoulderButton = 0 ;
    mpRShoulderButton = 0 ;
  }
}
CClassicData::~CClassicData() {}
float CClassicData::GetLShoulderButton(){ return mpLShoulderButton ; }
float CClassicData::GetRShoulderButton(){ return mpRShoulderButton ; }


CGuitarHero3Data::CGuitarHero3Data() : mpWhammyBar(0) {}
CGuitarHero3Data::CGuitarHero3Data( const CGuitarHero3 &cgh )
{
  if( cgh.mpGH3Ptr != NULL )
  {
    mpWhammyBar = cgh.mpGH3Ptr->whammy_bar ;
    Buttons = cgh.Buttons ;
    Joystick = cgh.Joystick ;
  }
  else
  {
    mpWhammyBar = 0 ;
  }
}
CGuitarHero3Data::~CGuitarHero3Data() {}
float CGuitarHero3Data::GetWhammyBar(){ return mpWhammyBar ; }


CExpansionDeviceData::CExpansionDeviceData() : mpExpTypes(0) {}
CExpansionDeviceData::CExpansionDeviceData( const CExpansionDevice &ced )
{
  if( ced.mpExpansionPtr != NULL )
  {
    mpExpTypes = ced.mpExpansionPtr->type ;
    Nunchuk = ced.Nunchuk ;
    Classic = ced.Classic ;
    GuitarHero3 = ced.GuitarHero3 ;
  }
  else
  {
    mpExpTypes = 0 ;
  }
}
CExpansionDeviceData::~CExpansionDeviceData(){}
CExpansionDevice::ExpTypes CExpansionDeviceData::GetType(){ return (CExpansionDevice::ExpTypes)mpExpTypes ; }


CWiimoteData::CWiimoteData() :
  mpUnid(0), mpLEDs(0), mpBattery(0), 
  mpEvent(CWiimote::EVENT_NONE), mpState(0), mpFlags(0),
  mpIsConnected(false), mpIsPolled(false), 
  mpIsPolledButton(false),
  mpIsPolledAcc(false), 
  mpIsPolledIR(false)
{}

CWiimoteData::CWiimoteData( const CWiimote &cw ) :
  IR(cw.IR), Buttons(cw.Buttons), 
  Accelerometer(cw.Accelerometer), ExpansionDevice(cw.ExpansionDevice)
{
  if( cw.mpWiimotePtr != NULL )
  {
    mpUnid = cw.mpWiimotePtr->unid ;
    mpLEDs = cw.mpWiimotePtr->leds ; 
    mpBattery = cw.mpWiimotePtr->battery_level ;
    mpEvent = (CWiimote::EventTypes)cw.mpWiimotePtr->event ;
    mpState = cw.mpWiimotePtr->state ;
    mpFlags = cw.mpWiimotePtr->flags ;
    mpIsConnected = cw.mpIsConnected ;
    mpIsPolled = cw.mpIsPolled ;
    mpIsPolledButton = cw.mpIsPolledButton ;
    mpIsPolledAcc = cw.mpIsPolledAcc ;
    mpIsPolledIR = cw.mpIsPolledIR ;
  }
  else
  {
    mpUnid = 0 ;
    mpLEDs = 0 ; 
    mpBattery = 0 ;
    mpEvent = CWiimote::EVENT_NONE ;
    mpState = 0 ;
    mpFlags = 0 ;
    mpIsConnected = false ;
    mpIsPolled = false ;
    mpIsPolledButton = false ;
    mpIsPolledAcc = false ;
    mpIsPolledIR = false ;
  }
}

CWiimoteData::~CWiimoteData(){}
int CWiimoteData::GetLEDs(){ return mpLEDs ; }
float CWiimoteData::GetBatteryLevel(){ return mpBattery ; }
CWiimote::EventTypes CWiimoteData::GetEvent(){ return mpEvent ; }
int CWiimoteData::GetID(){ return mpUnid ; }
int CWiimoteData::GetState(){ return mpState ; }
int CWiimoteData::GetFlags(){ return mpFlags ; }
bool CWiimoteData::isUsingACC(){ return (mpState & 0x0020) != 0 ; }
bool CWiimoteData::isUsingEXP(){ return (mpState & 0x0040) != 0 ; }
bool CWiimoteData::isUsingIR(){ return (mpState & 0x0080) != 0 ; }
bool CWiimoteData::isUsingSpeaker(){ return (mpState & 0x0100) != 0 ; }
bool CWiimoteData::isLEDSet(int LEDNum)
{
  bool result=false ;

  switch(LEDNum)
  {
    case 1:
      result = (mpLEDs & CWiimote::LED_1) != 0 ;
      break;
    case 2:
      result = (mpLEDs & CWiimote::LED_2) != 0 ;
      break;
    case 3:
      result = (mpLEDs & CWiimote::LED_3) != 0 ;
      break;
    case 4:
      result = (mpLEDs & CWiimote::LED_4) != 0 ;
    default:
      result = 0;
  }
  return result;
}

bool CWiimoteData::isConnected()
{
  return mpIsConnected ;
}

bool CWiimoteData::isPolled()
{
  return mpIsPolled ;
}

bool CWiimoteData::isPolledByIR()
{
  return mpIsPolledIR ;
}

bool CWiimoteData::isPolledByAcc()
{
  return mpIsPolledAcc ;
}

bool CWiimoteData::isPolledByButton()
{
  return mpIsPolledButton ;
}
