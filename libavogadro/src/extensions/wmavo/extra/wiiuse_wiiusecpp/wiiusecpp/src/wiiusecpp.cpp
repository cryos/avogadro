/*
 *wiiusecpp.cpp
 *
 *Written By:
 *James Thomas
 *Email: jt@missioncognition.org
 *
 *Copyright 2009
 *
 *Copyright (c) 2010 Mickael Gadroy
 *
 *This file is part of wiiusecpp.
 *
 *This program is free software; you can redistribute it and/or modify
 *it under the terms of the GNU General Public License as published by
 *the Free Software Foundation; either version 3 of the License, or
 *(at your option) any later version.
 *
 *This program is distributed in the hope that it will be useful,
 *but WITHOUT ANY WARRANTY; without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *GNU General Public License for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * Note:  This C++ library is a (very) thin wrapper of the the wiiuse library.
 *        See http://www.wiiuse.net to get the wiiuse library which is required
 *        to build this package.  A SWIG based Python wrapper for this C++ library
 *        is available from http://www.missioncognition.org.
 * Note:  The current version of wiiusecpp does not work with the previous wrapper
 *        Check the available version in the official website of http://www.missioncognition.org.
 */

#include "wiiusecpp.h"

// 1 1 1 1 1 1 2 4 8 4 2 = 26
double gausTab[]={1, 1, 1, 1, 1, 1, 2, 4, 8, 4, 2} ;
int gausTabSum = 26 ;


/*
 * CButtonBase class methods.
 */
CButtonBase::CButtonBase() :
  mpBtnsPtr(NULL), mpBtnsHeldPtr(NULL), 
  mpBtnsReleasedPtr(NULL)
{}
    
CButtonBase::CButtonBase(short *ButtonsPtr, short *ButtonsHeldPtr, short *ButtonsReleasedPtr, short *ButtonsPreviousPtr) :
  mpBtnsPtr( ButtonsPtr ), mpBtnsHeldPtr( ButtonsHeldPtr ), 
  mpBtnsReleasedPtr( ButtonsReleasedPtr ), mpBtnsPreviousPtr(ButtonsPreviousPtr)
{}
  
CButtonBase::CButtonBase( const CButtonBase& cbb ) :
  mpBtnsPtr(cbb.mpBtnsPtr), mpBtnsHeldPtr(cbb.mpBtnsHeldPtr), 
  mpBtnsReleasedPtr(cbb.mpBtnsReleasedPtr), mpBtnsPreviousPtr(cbb.mpBtnsPreviousPtr)
{}

CButtonBase::~CButtonBase()
{
  // All data desallocate by wiiuse.
  mpBtnsPtr = NULL ;
  mpBtnsHeldPtr = NULL ;
  mpBtnsReleasedPtr = NULL ;
  mpBtnsPreviousPtr = 0 ;
}


bool CButtonBase::isPressed(int Button)
{
  // wiiuse -> events.c :
  // buttons pressed now
	// wm->btns = now ;
  return ((*mpBtnsPtr) & Button) == Button;
}

bool CButtonBase::isHeld(int Button)
{
  // wiiuse -> events.c :
  // pressed now & were pressed, then held
  // wm->btns_held = (now & wm->btnsPrevious);
  return ((*mpBtnsHeldPtr) & Button) == Button;
}

bool CButtonBase::isReleased(int Button)
{
  // wiiuse -> events.c :
  // were pressed or were held & not pressed now, then released
	// wm->btns_released = ((wm->btns | wm->btns_held) & ~now);
  return ((*mpBtnsReleasedPtr) & Button) == Button;
}

bool CButtonBase::isJustPressed(int Button)
{
  // wiiuse -> events.c :
  // isPressed && !isHeld
  return (((*mpBtnsPtr) & Button) == Button) 
         && (((*mpBtnsHeldPtr) & Button) != Button) ;
}


bool CButtonBase::isJustChanged()
{
  // wiiuse -> events.c :
  // wm->btns != wm->btnsPrevious)
  return (*mpBtnsPtr) != (*mpBtnsPreviousPtr) ;
}

/*
 * Initializers for classes derrived from CButtonBase.
 */
CButtons::CButtons() : CButtonBase()
{}
    
CButtons::CButtons( short *ButtonsPtr, short *ButtonsHeldPtr, short *ButtonsReleasedPtr, short *ButtonsPreviousPtr ) :
  CButtonBase(ButtonsPtr, ButtonsHeldPtr, ButtonsReleasedPtr, ButtonsPreviousPtr)
{}

CButtons::CButtons( const CButtons& cb ) : CButtonBase( cb )
{}

CButtons::~CButtons() // See ~CButtonBase
{}


CNunchukButtons::CNunchukButtons() : CButtonBase()
{}

CNunchukButtons::CNunchukButtons(short *ButtonsPtr, short *ButtonsHeldPtr, short *ButtonsReleasedPtr, short *ButtonsPreviousPtr ) :
    CButtonBase(ButtonsPtr, ButtonsHeldPtr, ButtonsReleasedPtr, ButtonsPreviousPtr)
{}

CNunchukButtons::CNunchukButtons( const CNunchukButtons& cnb ) : CButtonBase( cnb )
{}

CNunchukButtons::~CNunchukButtons() // See ~CButtonBase
{}


CClassicButtons::CClassicButtons() : CButtonBase()
{}

CClassicButtons::CClassicButtons(short *ButtonsPtr, short *ButtonsHeldPtr, short *ButtonsReleasedPtr, short *ButtonsPreviousPtr) :
    CButtonBase(ButtonsPtr, ButtonsHeldPtr, ButtonsReleasedPtr, ButtonsPreviousPtr)
{}

CClassicButtons::CClassicButtons( const CClassicButtons& ccb ) : CButtonBase( ccb )
{}

CClassicButtons::~CClassicButtons() // See ~CButtonBase
{}


CGH3Buttons::CGH3Buttons() : CButtonBase()
{}
    
CGH3Buttons::CGH3Buttons(short *ButtonsPtr, short *ButtonsHeldPtr, short *ButtonsReleasedPtr, short *ButtonsPreviousPtr) :
    CButtonBase(ButtonsPtr, ButtonsHeldPtr, ButtonsReleasedPtr, ButtonsPreviousPtr)
{}

CGH3Buttons::CGH3Buttons( const CGH3Buttons& cgb ) : CButtonBase( cgb )
{}

CGH3Buttons::~CGH3Buttons() // See ~CButtonBase
{}


/*
 * CJoystick class methods.
 */

CJoystick::CJoystick() : mpJoystickPtr( NULL )
{}
    
CJoystick::CJoystick(struct joystick_t *JSPtr) : mpJoystickPtr( JSPtr )
{}

CJoystick::CJoystick( const CJoystick& cj ) : mpJoystickPtr( cj .mpJoystickPtr )
{}

CJoystick::~CJoystick()
{
  // All data desallocate by wiiuse.
  mpJoystickPtr = NULL ;
}

void CJoystick::GetMaxCal(int &X, int &Y)
{
  if( mpJoystickPtr != NULL )
  {
    X = mpJoystickPtr->max.x;
    Y = mpJoystickPtr->max.y;
  }
}

void CJoystick::SetMaxCal(int X, int Y)
{
  if( mpJoystickPtr != NULL )
  {
    mpJoystickPtr->max.x = X;
    mpJoystickPtr->max.y = Y;
  }
}

void CJoystick::GetMinCal(int &X, int &Y)
{
  if( mpJoystickPtr != NULL )
  {
    X = mpJoystickPtr->min.x;
    Y = mpJoystickPtr->min.y;
  }
}

void CJoystick::SetMinCal(int X, int Y)
{
  if( mpJoystickPtr != NULL )
  {
    mpJoystickPtr->min.x = X;
    mpJoystickPtr->min.y = Y;
  }
}

void CJoystick::GetCenterCal(int &X, int &Y)
{
  if( mpJoystickPtr != NULL )
  {
    X = mpJoystickPtr->center.x;
    Y = mpJoystickPtr->center.y;
  }
}

void CJoystick::SetCenterCal(int X, int Y)
{
  if( mpJoystickPtr != NULL )
  {
    mpJoystickPtr->center.x = X;
    mpJoystickPtr->center.y = Y;
  }
}

void CJoystick::GetPosition(float &Angle, float &Magnitude)
{
  if( mpJoystickPtr != NULL )
  {
    Angle = mpJoystickPtr->ang;
    Magnitude = mpJoystickPtr->mag;
  }
}

/*
 * CAccelerometer class methods.
 */

CAccelerometer::CAccelerometer() :
  mpAccelCalibPtr(NULL), mpAccelPtr(NULL), mpAccelThresholdPtr(NULL),
  mpOrientPtr(NULL), mpOrientThresholdPtr(NULL), mpGForcePtr(NULL),
  mpUpdateOneMore(false)
{
  initValuesInTime() ;
}

CAccelerometer::CAccelerometer( struct accel_t *AccelCalPtr, struct vec3b_t *AccelerationPtr, int *AccelThresholdPtr,
                                struct orient_t *OrientationPtr, float *OrientationThresholdPtr,
                                struct gforce_t *GForcePtr ) :
  mpAccelCalibPtr(AccelCalPtr), mpAccelPtr(AccelerationPtr),  mpAccelThresholdPtr(AccelThresholdPtr),
  mpOrientPtr(OrientationPtr), mpOrientThresholdPtr(OrientationThresholdPtr),
  mpGForcePtr(GForcePtr), mpUpdateOneMore(false)
{
  initValuesInTime() ;
}

CAccelerometer::CAccelerometer( const CAccelerometer& ca ) :
  mpAccelCalibPtr(ca.mpAccelCalibPtr), mpAccelPtr(ca.mpAccelPtr),
  mpAccelThresholdPtr(ca.mpAccelThresholdPtr), 
  mpOrientPtr(ca.mpOrientPtr), mpOrientThresholdPtr(ca.mpOrientThresholdPtr),
  mpGForcePtr(ca.mpGForcePtr), mpUpdateOneMore(false)
{
  initValuesInTime() ;
}

CAccelerometer::~CAccelerometer()
{
  // All data desallocate by wiiuse.
  mpAccelCalibPtr = NULL ;
  mpAccelPtr = NULL ;
  mpOrientPtr = NULL ;
  mpGForcePtr = NULL ;
  mpAccelThresholdPtr = NULL ;
  mpOrientThresholdPtr = NULL ;
}


float CAccelerometer::SetSmoothAlpha(float Alpha)
{
  float old_value=0.0 ;

  if( mpAccelCalibPtr != NULL )
  {
    old_value = mpAccelCalibPtr->st_alpha ;
    mpAccelCalibPtr->st_alpha = Alpha ;
  }

  return old_value;
}

float CAccelerometer::GetOrientThreshold()
{
  return (mpOrientThresholdPtr!=NULL ? *mpOrientThresholdPtr : 0.0f ) ;
}

void CAccelerometer::SetOrientThreshold(float Threshold)
{
  if( mpOrientThresholdPtr != NULL )
    *mpOrientThresholdPtr = Threshold ;
}

int CAccelerometer::GetAccelThreshold()
{
  return (mpAccelThresholdPtr!=NULL ? *mpAccelThresholdPtr : 0 ) ;
}

void CAccelerometer::SetAccelThreshold(int Threshold)
{
  if( mpAccelThresholdPtr != NULL )
    *mpAccelThresholdPtr = Threshold ;
}

void CAccelerometer::GetOrientation(float &Pitch, float &Roll, float &Yaw)
{
  if( mpOrientPtr != NULL )
  {
    Pitch = mpOrientPtr->pitch ;
    Roll = mpOrientPtr->roll ;
    Yaw = mpOrientPtr->yaw ;
  }
  else
  {
    Pitch = 0.0f ;
    Roll = 0.0f ;
    Yaw = 0.0f ;
  }
}
void CAccelerometer::GetRawOrientation(float &Pitch, float &Roll)
{
  if( mpOrientPtr != NULL )
  {
    Pitch = mpOrientPtr->a_pitch;
    Roll = mpOrientPtr->a_roll;
  }
  else
  {
    Pitch = 0.0 ;
    Roll = 0.0 ;
  }
}

void CAccelerometer::GetAccCalOne(float &X, float &Y, float &Z)
{
  if( mpAccelCalibPtr != NULL )
  {
    X = (float)(mpAccelCalibPtr->cal_g.x + mpAccelCalibPtr->cal_zero.x) ;
    Y = (float)(mpAccelCalibPtr->cal_g.y + mpAccelCalibPtr->cal_zero.y) ;
    Z = (float)(mpAccelCalibPtr->cal_g.z + mpAccelCalibPtr->cal_zero.z) ;
  }
  else
  { X = 0.0 ; Y = 0.0 ; Z = 0.0 ; }
}

void CAccelerometer::GetAccCalZero(float &X, float &Y, float &Z)
{
  if( mpAccelCalibPtr != NULL )
  {
    X = (float)mpAccelCalibPtr->cal_zero.x;
    Y = (float)mpAccelCalibPtr->cal_zero.y;
    Z = (float)mpAccelCalibPtr->cal_zero.z;
  }
  else
  { X = 0.0 ; Y = 0.0 ; Z = 0.0 ; }
}

void CAccelerometer::GetGForceInG(double &X, double &Y, double &Z)
{
  X = mpValuesInTime[0].gForceX/WIIUSECPP_GUNIT_TO_MS2UNIT ;
  Y = mpValuesInTime[0].gForceY/WIIUSECPP_GUNIT_TO_MS2UNIT ;
  Z = mpValuesInTime[0].gForceZ/WIIUSECPP_GUNIT_TO_MS2UNIT ;
}

double CAccelerometer::GetGForceInG()
{
  return mpValuesInTime[0].gForce/WIIUSECPP_GUNIT_TO_MS2UNIT ;
}

void CAccelerometer::GetGForceInMS2(double &X, double &Y, double &Z)
{
  X = mpValuesInTime[0].gForceX ;
  Y = mpValuesInTime[0].gForceY ;
  Z = mpValuesInTime[0].gForceZ ;
}

double CAccelerometer::GetGForceInMS2()
{
  return mpValuesInTime[0].gForce ;
}

double CAccelerometer::GetGForceElapse()
{
  return mpValuesInTime[0].gForce - mpValuesInTime[1].gForce ;
}

void CAccelerometer::GetJerkInMS3(double &X, double &Y, double &Z)
{
  X = mpValuesInTime[0].jerkX ;
  Y = mpValuesInTime[0].jerkY ;
  Z = mpValuesInTime[0].jerkZ ;
}

double CAccelerometer::GetJerkInMS3()
{
  return mpValuesInTime[0].jerk ;
}

void CAccelerometer::GetVelocity(double &X, double &Y, double &Z)
{
  X = mpValuesInTime[1].velocityX ;
  Y = mpValuesInTime[1].velocityY ;
  Z = mpValuesInTime[1].velocityZ ;
}

double CAccelerometer::GetVelocity()
{
  return mpValuesInTime[1].velocity ;
}

void CAccelerometer::GetDistance(double &X, double &Y, double &Z)
{
  X = mpValuesInTime[0].distanceX ;
  Y = mpValuesInTime[0].distanceY ;
  Z = mpValuesInTime[0].distanceZ ;
}

double CAccelerometer::GetDistance()
{
  return mpValuesInTime[0].distance ;
}

void CAccelerometer::GetPosition( double &X, double &Y, double &Z )
{
  X = mpValuesInTime[0].positionX ;
  Y = mpValuesInTime[0].positionY ;
  Z = mpValuesInTime[0].positionZ ;
}

bool CAccelerometer::isCountdownEnoughAccuracyInMS()
{
  // Test if the number of clock is enough to have a accuracy in ms
  // to use the clock() method.
  // CLOCKS_PER_SEC: macro of ctime library.
  //printf( "CLOCKS_PER_SEC: %f, CLOCKS_PER_SEC:%f\n", CLOCKS_PER_SEC, CLOCKS_PER_SEC/1000 ) ;
  return (((float)CLOCKS_PER_SEC)*0.001f)>=1.0f ;
}

double CAccelerometer::getElapsedTime()
{
  return ((double)clock()) / ((double)CLOCKS_PER_SEC) ;
}

void CAccelerometer::initValuesInTime()
{
  for( int i=0 ; i<WIIUSECPP_ACC_NB_SAVED_VALUES ; i++ )
    mpValuesInTime.push_front( values_t() ) ;
}


bool CAccelerometer::calculateValues( bool mustBeCalculated )
{
  double timeNow=getElapsedTime() ;
  bool valuesIsCalculated=false ;
  
  if( false //!(mustBeCalculated //|| mpUpdateOneMore )
      || (mpGForcePtr==NULL
          || !(mpGForcePtr!=NULL && (mpGForcePtr->x > -20 && mpGForcePtr->x < 20)))
          // To avoid a bug with the 1st get values.
    )
  {
    valuesIsCalculated = false ;
  }
  else
  { 
    valuesIsCalculated = true ;

    //if( !mustBeCalculated && mpUpdateOneMore ) 
    //  mpUpdateOneMore = false ;
    //else
    //  mpUpdateOneMore = true ;
    
    values_t t ;
    t.time = timeNow ;
    t.diffTime = timeNow - mpValuesInTime[0].time ;
    t.diffTimeSquarred = t.diffTime * t.diffTime ;

    //mpValuesInTime[0].diffTime = 1 ; //0.01 // 0.005
    //mpValuesInTime[0].diffTimeSquarred = 1 ; // 0.0001 // 0.000025
    // When the Wiimote is not used, no communication is realized.
    // When the Wiimote is moving, there is communication.
    // The acceleration value get after a "non-used", it is a acceleration during
    // all "non-used" time, or just the last millisecond.
    // So the hypothesis is : the acceleration is for the 5 last millisecond.

    t.gForceX = mpGForcePtr->x * WIIUSECPP_GUNIT_TO_MS2UNIT ;
    t.gForceY = mpGForcePtr->y * WIIUSECPP_GUNIT_TO_MS2UNIT ;
    t.gForceZ = mpGForcePtr->z * WIIUSECPP_GUNIT_TO_MS2UNIT ;
    t.gForce = sqrt( t.gForceX*t.gForceX +
                     t.gForceY*t.gForceY + 
                     t.gForceZ*t.gForceZ ) ;

    //double elapseX=t.gForceX-mpValuesInTime[0].gForceX ;
    //double elapseY=t.gForceY-mpValuesInTime[0].gForceY ;
    //double elapseZ=t.gForceZ-mpValuesInTime[0].gForceZ ;
    //double elapseNorm=sqrt( elapseX*elapseX + elapseY*elapseY + elapseZ*elapseZ ) ;

   
    if( //elapseNorm<0.04 // Base values to init at zero.
        (timeNow-mpValuesInTime[0].time)>=0.1 // The elapse time is non contant, so useless to init each time even without calculation. 
        || mpValuesInTime[0].diffTime==0 ) // To avoid a divide by zero.    
    {
      // Caution during constant movement ...

      t.jerkX=0 ; t.jerkY=0 ; t.jerkZ=0 ; t.jerk=0 ;
      t.accelerationX=0 ; t.accelerationY=0 ; t.accelerationZ=0 ; t.acceleration=0 ;
      t.velocityX=0 ; t.velocityY=0 ; t.velocityZ=0 ; t.velocity=0 ;
      t.distanceX=0 ; t.distanceY=0 ; t.distanceZ=0 ; t.distance=0 ;

      // The current position must be unchanged.
      t.positionX=mpValuesInTime[0].positionX ;
      t.positionY=mpValuesInTime[0].positionY ;
      t.positionZ=mpValuesInTime[0].positionZ ;
    }
    else
    
    {
      /*
      // Limit with max values (jerk?, acc?, velocity?)
      // Average values to reduce parasit and jerk variation.
      for( int i=0 ; i<WIIUSECPP_ACC_NB_SAVED_VALUES ; i++ )
      {
        t.gForceX += (mpValuesInTime[i].gForceX * gausTab[i]) ;
        t.gForceY += (mpValuesInTime[i].gForceY * gausTab[i]) ;
        t.gForceZ += (mpValuesInTime[i].gForceZ * gausTab[i]) ;
      }

      t.gForceX /= gausTabSum ; //(WIIUSECPP_ACC_NB_SAVED_VALUES+1) ;
      t.gForceY /= gausTabSum ; //(WIIUSECPP_ACC_NB_SAVED_VALUES+1) ;
      t.gForceZ /= gausTabSum ; //(WIIUSECPP_ACC_NB_SAVED_VALUES+1) ;
      */

      

      // Distance values.
      if( false /*&& hasElapsedTime*/ )
        calculateFromTailorFormulaWithConstantTime_p(t) ;
      else if( true )
        calculateFromTailorFormulaWithNonConstantTime_p(t) ;
      else if( false )
        calculateByPhysicBasicFormula_p(t) ;
      
      t.acceleration = sqrt( t.accelerationX*t.accelerationX +
                             t.accelerationY*t.accelerationY +
                             t.accelerationZ*t.accelerationZ ) ;
      t.velocity = sqrt( t.velocityX*t.velocityX +
                         t.velocityY*t.velocityY +
                         t.velocityZ*t.velocityZ ) ;
      t.distance = sqrt( t.distanceX*t.distanceX +
                         t.distanceY*t.distanceY +
                         t.distanceZ*t.distanceZ ) ;
    }

    mpValuesInTime.push_front(t) ;
    mpValuesInTime.pop_back() ;
  }

  return valuesIsCalculated ;
}


void CAccelerometer::calculateFromTailorFormulaWithConstantTime_p( values_t &t )
{
  // Based on Tailor formula :
  // f(x) = f(x0) + f'(x0).(x-x0) + 1/(2!) . (x-x0)².f''(x0) + 1/(3!) . (x-x0)^3.f'''(x0) ...
  // x = t = time
  // f(x) = x(t) = distance at the t time
  // f''(x) = x''(t) = a(t) = acceleration at the t time
  
  // x(t+dt) = x(t) + x'(t).dt    + 1/2 . dt² . x''(t)
  // x(t-dt) = x(t) + x'(t).(-dt) + 1/2 . dt² . x''(t)
  // <=> x(t+dt) + x(t-dt) = 2.x(t) + 1/2 . dt² . x''(t)
  // <=> x(t+dt) = 2.x(t) - x(t-dt) + 1/2 . dt² . a(t)

  mpValuesInTime[0].velocityX = 0 ;
  mpValuesInTime[0].velocityY = 0 ;
  mpValuesInTime[0].velocityZ = 0 ;

  t.positionX = 2*mpValuesInTime[0].positionX - mpValuesInTime[1].positionX
                           + t.diffTimeSquarred * t.gForceX ;
  t.positionY = 2*mpValuesInTime[0].positionY - mpValuesInTime[1].positionY
                           + t.diffTimeSquarred * t.gForceY ;
  t.positionZ = 2*mpValuesInTime[0].positionZ - mpValuesInTime[1].positionZ 
                           + t.diffTimeSquarred * (t.gForceZ-1.0f) ;

  t.distanceX = t.positionX - mpValuesInTime[0].positionX ;
  t.distanceY = t.positionY - mpValuesInTime[0].positionY ;
  t.distanceZ = t.positionZ - mpValuesInTime[0].positionZ ;
}

void CAccelerometer::calculateFromTailorFormulaWithNonConstantTime_p( values_t &t )
{
  // Based on Tailor formula : see calculateFromTailorFormulaWithNonConstantTime().
  // With the integration of the non constant delta time.

  double dt1=t.time-mpValuesInTime[0].time ;
  double dt2=mpValuesInTime[0].time-mpValuesInTime[1].time ;
  double dt1Sqr=dt1*dt1 ;
  double dt2Sqr=dt2*dt2 ;
  //double dt1Cube=dt1*dt1*dt1 ;
  //double dt2Cube=dt2*dt2*dt2 ;
  double dt1Plusdt2=dt1+dt2 ;

  // Jerk values (derivative of the GForce).
  mpValuesInTime[0].jerkX = (t.gForceX - mpValuesInTime[1].gForceX) / dt1Plusdt2 ;
  mpValuesInTime[0].jerkY = (t.gForceY - mpValuesInTime[1].gForceY) / dt1Plusdt2 ;
  mpValuesInTime[0].jerkZ = (t.gForceZ - mpValuesInTime[1].gForceZ) / dt1Plusdt2 ;
  mpValuesInTime[0].jerk = sqrt( mpValuesInTime[0].jerkX*mpValuesInTime[0].jerkX 
                                  + mpValuesInTime[0].jerkY*mpValuesInTime[0].jerkY 
                                  + mpValuesInTime[0].jerkZ*mpValuesInTime[0].jerkZ ) ;

  // The velocity is non null, so it must be calculated.
  mpValuesInTime[0].velocityX = mpValuesInTime[1].velocityX                // v(t-dt2)
                                + dt2 * mpValuesInTime[0].gForceX          // dt2 . a(t)
                                - 0.5 * dt2Sqr * mpValuesInTime[0].jerkX ; // -1/2 . dt2² . a'(t)
  mpValuesInTime[0].velocityY = mpValuesInTime[1].velocityY
                                + dt2 * mpValuesInTime[0].gForceY
                                - 0.5 * dt2Sqr * mpValuesInTime[0].jerkY ;
  mpValuesInTime[0].velocityZ = mpValuesInTime[1].velocityZ
                                + dt2 * mpValuesInTime[0].gForceZ
                                - 0.5 * dt2Sqr * mpValuesInTime[0].jerkZ ;

  if( fabs(mpValuesInTime[0].gForceX) > 1.0 )
  { 
    t.positionX = 2*mpValuesInTime[0].positionX - mpValuesInTime[1].positionX // 2.x(t) - x(t-dt2)
                  //+ mpValuesInTime[0].velocityX * (dt1-dt2)                   // x'(t) . (dt1-dt2)
                  + mpValuesInTime[0].gForceX * 0.5 * (dt1Sqr + dt2Sqr) ;      // x''(t) . 1/2 . (dt1²+dt2²)
                  //- mpValuesInTime[0].jerkX * 1/6 * (dt1Cube + dt1Cube) ;     // x'''(t) . 1/6 . (dt1^3+dt2^3)

    t.tmp1 = 2*mpValuesInTime[0].positionX - mpValuesInTime[1].positionX ;
    t.tmp2 = mpValuesInTime[0].velocityX * (dt1-dt2)  ;
    t.tmp3 = mpValuesInTime[0].gForceX * 0.5 * (dt1Sqr + dt2Sqr) ;

    t.positionY = 2*mpValuesInTime[0].positionY - mpValuesInTime[1].positionY
                   //+ mpValuesInTime[0].velocityY * (dt1-dt2)
                   + mpValuesInTime[0].gForceY * 0.5 * (dt1Sqr + dt2Sqr) ;
                   //- mpValuesInTime[0].jerkY * 1/6 * (dt1Cube + dt1Cube) ;
    t.positionZ = 2*mpValuesInTime[0].positionZ - mpValuesInTime[1].positionZ
                   //+ mpValuesInTime[0].velocityZ * (dt1-dt2)
                   + mpValuesInTime[0].gForceZ * 0.5 * (dt1Sqr + dt2Sqr) ;
                   //- mpValuesInTime[0].jerkZ * 1/6 * (dt1Cube + dt1Cube) ;
  }
  else
  {
     t.positionX = mpValuesInTime[0].positionX ;

    t.tmp1 = 2*mpValuesInTime[0].positionX - mpValuesInTime[1].positionX ;
    t.tmp2 = mpValuesInTime[0].velocityX * (dt1-dt2)  ;
    t.tmp3 = mpValuesInTime[0].gForceX * 0.5 * (dt1Sqr + dt2Sqr) ;

    t.positionY = mpValuesInTime[0].positionY ;
    t.positionZ = mpValuesInTime[0].positionZ ;
  }

  t.distanceX = t.positionX - mpValuesInTime[0].positionX ;
  t.distanceY = t.positionY - mpValuesInTime[0].positionY ;
  t.distanceZ = t.positionZ - mpValuesInTime[0].positionZ ;
}

void CAccelerometer::calculateByPhysicBasicFormula_p( values_t &t )
{
  // Based on :
  // v = v0 +a.t ; d = d0 + v.t + 1/2.a.t
  // t = time interval ; a, v, d => vector

  // Jerk values (derivative of the GForce).
  t.jerkX = (t.gForceX - mpValuesInTime[0].gForceX) / t.diffTime ;
  t.jerkY = (t.gForceY - mpValuesInTime[0].gForceY) / t.diffTime ;
  t.jerkZ = (t.gForceZ - mpValuesInTime[0].gForceZ) / t.diffTime ;
  t.jerk = sqrt( t.jerkX*t.jerkX + t.jerkY*t.jerkY + t.jerkZ*t.jerkZ ) ;

  double elapsedTime = t.diffTime ;
	double elapsedTime2 = elapsedTime * elapsedTime;
	double elapsedTime3 = elapsedTime * elapsedTime * elapsedTime;

	const double half  = 1.0/2.0;
	const double third = 1.0/3.0;

  // at = j0 * (t-t0) + a0
  t.accelerationX = mpValuesInTime[0].jerkX * elapsedTime + mpValuesInTime[0].accelerationX;
  t.accelerationY = mpValuesInTime[0].jerkY * elapsedTime + mpValuesInTime[0].accelerationY;
  t.accelerationZ = mpValuesInTime[0].jerkZ * elapsedTime + mpValuesInTime[0].accelerationZ;

  // vt = 1/2 * j0 * (t-t0)^2 + a0 * (t-t0) + v0
  t.velocityX = half * mpValuesInTime[0].jerkX * elapsedTime2 + mpValuesInTime[0].accelerationX * elapsedTime + mpValuesInTime[0].velocityX;
  t.velocityY = half * mpValuesInTime[0].jerkY * elapsedTime2 + mpValuesInTime[0].accelerationY * elapsedTime + mpValuesInTime[0].velocityY;
  t.velocityZ = half * mpValuesInTime[0].jerkZ * elapsedTime2 + mpValuesInTime[0].accelerationZ * elapsedTime + mpValuesInTime[0].velocityZ;

  // xt = 1/2 * 1/3 * j0 * (t-t0)^3 + 1/2 * a0 * (t-t0)^2 + v0 * (t-t0) + x0
  t.positionX = half * third * mpValuesInTime[0].jerkX * elapsedTime3 + half * mpValuesInTime[0].accelerationX * elapsedTime2 + mpValuesInTime[0].velocityX * elapsedTime + mpValuesInTime[0].positionX;
  t.positionY = half * third * mpValuesInTime[0].jerkY * elapsedTime3 + half * mpValuesInTime[0].accelerationY * elapsedTime2 + mpValuesInTime[0].velocityY * elapsedTime + mpValuesInTime[0].positionY;
  t.positionZ = half * third * mpValuesInTime[0].jerkZ * elapsedTime3 + half * mpValuesInTime[0].accelerationZ * elapsedTime2 + mpValuesInTime[0].velocityZ * elapsedTime + mpValuesInTime[0].positionZ;
}


/*
 * CIRDot class methods.
 */

CIRDot::CIRDot() : mpDotPtr( NULL )
{}

CIRDot::CIRDot( struct ir_dot_t *DotPtr ) : mpDotPtr(DotPtr)
{}

// Copy constructor to handle pass by value.
CIRDot::CIRDot(const CIRDot &copyin) : mpDotPtr( copyin.mpDotPtr )
{}

CIRDot::~CIRDot()
{
  // All data desallocate by wiiuse.
  mpDotPtr = NULL ;
}

int CIRDot::isVisible()
{
  return (mpDotPtr!=NULL ? mpDotPtr->visible : 0 ) ;
}

int CIRDot::GetSize()
{
  return (mpDotPtr!=NULL ? mpDotPtr->size : 0 ) ;
}

int CIRDot::GetOrder()
{
  return (mpDotPtr!=NULL ? mpDotPtr->order : 0 ) ;
}

void CIRDot::GetCoordinate(int &X, int &Y)
{
  if( mpDotPtr != NULL )
  {
    X = mpDotPtr->x;
    Y = mpDotPtr->y;
  }
  else
  {
    X = 0 ;
    Y = 0 ;
  }
}

void CIRDot::GetRawCoordinate(int &X, int &Y)
{
  if( mpDotPtr != NULL )
  {
    X = mpDotPtr->rx ;
    Y = mpDotPtr->ry ;
  }
  else
  {
    X = 0 ;

    Y = 0 ;
  }
}

/*
 * CIR class methods.
 */

CIR::CIR() : mpWiimotePtr( NULL )
{
  //mpIRDotsVector.clear() ;
}

CIR::CIR(struct wiimote_t *wmPtr) : mpWiimotePtr( NULL )
{
  cout << mpIRDotsVector.capacity() << endl ;
  cout << mpIRDotsVector.empty() << endl ;
  cout << mpIRDotsVector.max_size() << endl ;
  cout << mpIRDotsVector.size() << endl ;
	mpIRDotsVector.push_back(0) ;
	mpIRDotsVector.clear() ;

  init( wmPtr ) ;
}

CIR::CIR( const CIR& ci ) : mpWiimotePtr( NULL )
{
  init( ci.mpWiimotePtr ) ;
}

CIR::~CIR()
{
  // mpWiimotePtr : delete by ~CWii.
  mpIRDotsVector.clear() ;
  mpWiimotePtr = NULL ;
}

void CIR::init( struct wiimote_t *wmPtr )
{
  mpWiimotePtr = wmPtr ;

  if( mpWiimotePtr != NULL )
  {
    CIRDot *dot=NULL ;
    for( int i=0 ; i<WIIUSECPP_NUM_IRDOTS ; i++ )
    {
      dot = new CIRDot( (struct ir_dot_t *) (&(mpWiimotePtr->ir.dot[i])) ) ;
      mpIRDotsVector.push_back( dot ) ;
    }
  }
}

void CIR::SetMode(CIR::OnOffSelection State)
{
    wiiuse_set_ir(mpWiimotePtr, State);
}

void CIR::SetVres(unsigned int x, unsigned int y)
{
    wiiuse_set_ir_vres(mpWiimotePtr, x, y);
}

CIR::BarPositions CIR::GetBarPositionSetting()
{
  if( mpWiimotePtr != NULL )
    return (CIR::BarPositions) mpWiimotePtr->ir.pos ;
  else
    return CIR::BAR_ABOVE ;
}

void CIR::SetBarPosition(CIR::BarPositions PositionSelection)
{
    wiiuse_set_ir_position( mpWiimotePtr, (ir_position_t) PositionSelection ) ;
}

CIR::AspectRatioSelections CIR::GetAspectRatioSetting()
{
  if( mpWiimotePtr != NULL )
    return (CIR::AspectRatioSelections) mpWiimotePtr->ir.aspect ;
  else
    return CIR::ASPECT_4_3 ;
}

void CIR::SetAspectRatio( CIR::AspectRatioSelections AspectRatioSelection )
{
    wiiuse_set_aspect_ratio( mpWiimotePtr, (enum aspect_t) AspectRatioSelection ) ;
}

void CIR::SetSensitivity(int Level)
{
    wiiuse_set_ir_sensitivity( mpWiimotePtr, Level ) ;
}

int CIR::GetSensitivity()
{
  int level=0 ;

  if( mpWiimotePtr != NULL )
  {
    if( mpWiimotePtr->state & 0x0200 )
      level = 1;
    else if( mpWiimotePtr->state & 0x0400 )
      level = 2;
    else if( mpWiimotePtr->state & 0x0800 )
      level = 3;
    else if( mpWiimotePtr->state & 0x1000 )
      level = 4;
    else if( mpWiimotePtr->state & 0x2000 )
      level = 5;
  }
  
  return level;
}

int CIR::GetNumDots()
{
  return (mpWiimotePtr!=NULL ? mpWiimotePtr->ir.num_dots : 0 ) ;
}

int CIR::GetNumSources()
{
  return (mpWiimotePtr!=NULL ? mpWiimotePtr->ir.nb_source_detect : 0 ) ;
}

std::vector<CIRDot*>& CIR::GetDots()
{
  /*
    int index;

    // Empty the array of irdots before reloading
    mpIRDotsVector.clear();

    for(index = 0; index < mpWiimotePtr->ir.num_dots; index++)
    {
        CIRDot dot((struct ir_dot_t *) (&(mpWiimotePtr->ir.dot[index])));
        mpIRDotsVector.push_back(dot);
    }

    //return numConnected;
    */
    return mpIRDotsVector;
}

void CIR::GetOffset(int &X, int &Y)
{
  if( mpWiimotePtr != NULL )
  {
    X = mpWiimotePtr->ir.offset[0];
    Y = mpWiimotePtr->ir.offset[1];
  }
  else
  {
    X = 0 ;
    Y = 0 ;
  }
}

int CIR::GetState()
{
  return (mpWiimotePtr!=NULL ? mpWiimotePtr->ir.state : 0 ) ;
}

/**
  * Average of the IR dots (0->1024, 0->768)
  * @param X Output value on X-axe.
  * @param Y Output value on Y-axe.
  */
void CIR::GetCursorPositionAbsolute(int &X, int &Y)
{
  if( mpWiimotePtr != NULL )
  {
    X = mpWiimotePtr->ir.ax;
    Y = mpWiimotePtr->ir.ay;
  }
  else
  {
    X = 0 ;
    Y = 0 ;
  }
}

/**
  * The position = last ir position + delta ir.
  * Begin from (0,0) to ((-oo,+oo),(-oo,+oo)).
  * Tip : use GetCursorDelta() instead.
  * @param X Output value on X-axe.
  * @param Y Output value on Y-axe.
  */
void CIR::GetCursorPosition(int &X, int &Y)
{
  if( mpWiimotePtr != NULL )
  {
    X = mpWiimotePtr->ir.x ;
    Y = mpWiimotePtr->ir.y ;
  }
  else
  {
    X = 0 ;
    Y = 0 ;
  }
}

/**
  * Difference between the current and the last ir dots.
  * @param X Output value on X-axe.
  * @param Y Output value on Y-axe.
  * @param Z Output value on Z-axe.
  */
void CIR::GetCursorDelta(double &X, double &Y, double &Z)
{
  if( mpWiimotePtr != NULL )
  {
    X = mpWiimotePtr->ir.deltax ;
    Y = mpWiimotePtr->ir.deltay ;
    Z = mpWiimotePtr->ir.deltaz ;
  }
  else
  {
    X = 0 ;
    Y = 0 ;
    Z = 0 ;
  }
}

/**
  * This is the distance between 2 points on the IR cam.
  * Tip : use GetCursorDelta() instead.
  * @return An unreliable data about the distance.
  */
float CIR::GetPixelDistance()
{
  return (mpWiimotePtr!=NULL ? mpWiimotePtr->ir.distance : 0.0f ) ;
}

/**
  * This is the distance between 2 points on the IR cam.
  * Tip : use GetCursorDelta() instead.
  * @return An unreliable data about the distance.
  */
float CIR::GetDistance()
{
  return (mpWiimotePtr!=NULL ? mpWiimotePtr->ir.z : 0.0f ) ;
}

/**
  * If the values are below at (const value in code), that can be 
  * considered like a movement which search to be precise.
  * @return TRUE the movement is 
  */
bool CIR::IsInPrecisionMode()
{
  if( mpWiimotePtr->ir.isInPrecisionMode == 0 )
    return false ;
  else
    return true ;
}


/*
 * CExpansionDevice class methods.  This is a container class so there is not much.
 */
CExpansionDevice::CExpansionDevice() : 
  Nunchuk(NULL), Classic(NULL), GuitarHero3(NULL), mpExpansionPtr( NULL )
{}

CExpansionDevice::CExpansionDevice(struct expansion_t *ExpPtr) :
  Nunchuk(ExpPtr), Classic(ExpPtr), GuitarHero3(ExpPtr), mpExpansionPtr(ExpPtr)
{}
    
CExpansionDevice::CExpansionDevice( const CExpansionDevice& ced ) :
  Nunchuk(ced.mpExpansionPtr), Classic(ced.mpExpansionPtr), GuitarHero3(ced.mpExpansionPtr), mpExpansionPtr(ced.mpExpansionPtr)
{}

CExpansionDevice::~CExpansionDevice()
{
  // All data desallocate by wiiuse.
  mpExpansionPtr = NULL ;
}

CExpansionDevice::ExpTypes CExpansionDevice::GetType()
{
  return (mpExpansionPtr!=NULL ? (CExpansionDevice::ExpTypes) (mpExpansionPtr->type) : CExpansionDevice::TYPE_NONE ) ;
}

/*
 * CNunchuk class methods.
 */

CNunchuk::CNunchuk() : Buttons(), Joystick(), Accelerometer(), mpNunchukPtr(NULL)
{}

CNunchuk::CNunchuk( struct expansion_t *ExpPtr ) :
   Buttons(), Joystick(), Accelerometer(), mpNunchukPtr(NULL)
{
  if( ExpPtr != NULL )
    mpNunchukPtr = &(ExpPtr->nunchuk) ;
  else
    mpNunchukPtr = NULL ;

  init( mpNunchukPtr ) ;
}

CNunchuk::CNunchuk( const CNunchuk& cn ) :
   Buttons(), Joystick(), Accelerometer(), mpNunchukPtr(NULL)
{
  mpNunchukPtr = cn.mpNunchukPtr ;
  init( mpNunchukPtr ) ;
}

CNunchuk::~CNunchuk()
{
  mpNunchukPtr = NULL ;
}

void CNunchuk::init( struct nunchuk_t *n )
{
  if( n != NULL )
  {
    Buttons = CNunchukButtons( &(n->btns.btns), &(n->btns.btns_held), &(n->btns.btns_released), &(n->btns.btns_previous) ) ;
    Joystick = CJoystick( &(n->js) ) ;
    Accelerometer = CAccelerometer( &(n->accel_calib), &(n->accel), &(n->accel_threshold), 
                                    &(n->orient), &(n->orient_threshold), &(n->gforce) ) ;
  }
  else
  {
    Buttons = CNunchukButtons() ;
    Joystick = CJoystick() ;
    Accelerometer = CAccelerometer() ;
  }
}


/*
 * CClassic class methods.
 */

CClassic::CClassic() : Buttons(), LeftJoystick(), RightJoystick(), mpClassicPtr(NULL)
{}

CClassic::CClassic(struct expansion_t *ExpPtr) :
  Buttons(), LeftJoystick(), RightJoystick(), mpClassicPtr(NULL)
{
  if( ExpPtr != NULL )
    mpClassicPtr = &(ExpPtr->classic) ;
  else
    mpClassicPtr = NULL ;

  init( mpClassicPtr ) ;
}

CClassic::CClassic( const CClassic& cc ) :
  Buttons(), LeftJoystick(), RightJoystick(), mpClassicPtr(NULL)
{
  mpClassicPtr = cc.mpClassicPtr ;
  init( mpClassicPtr ) ;
}

CClassic::~CClassic()
{
  mpClassicPtr = NULL ;
}

void CClassic::init( struct classic_ctrl_t *c )
{
  if( c != NULL )
  {
    Buttons = CClassicButtons( &(c->btns.btns), &(c->btns.btns_held), &(c->btns.btns_released), &(c->btns.btns_previous) ) ;
    LeftJoystick = CJoystick( &(c->ljs) ) ;
    RightJoystick = CJoystick( &(c->rjs) ) ;
  }
  else
  {
    Buttons = CClassicButtons() ;
    LeftJoystick = CJoystick() ;
    RightJoystick = CJoystick() ;
  }
}

float CClassic::GetLShoulderButton()
{
  if( mpClassicPtr != NULL )
    return mpClassicPtr->l_shoulder ;
  else
    return 0.0f ;
}

float CClassic::GetRShoulderButton()
{   
  if( mpClassicPtr != NULL )
    return mpClassicPtr->r_shoulder ;
  else
    return 0.0f ;
}

/*
 * CGuitarHero3 class methods.
 */

CGuitarHero3::CGuitarHero3() : Buttons(), Joystick(), mpGH3Ptr(NULL)
{}

CGuitarHero3::CGuitarHero3( struct expansion_t *ExpPtr ) : Buttons(), Joystick(), mpGH3Ptr(NULL)
{
  if( ExpPtr != NULL )
    mpGH3Ptr = &(ExpPtr->gh3) ;
  else
    mpGH3Ptr = NULL ;

  init( mpGH3Ptr ) ;
}

CGuitarHero3::CGuitarHero3( const CGuitarHero3& cgh ) : Buttons(), Joystick(), mpGH3Ptr(NULL)
{
  mpGH3Ptr = cgh.mpGH3Ptr ;
  init( mpGH3Ptr ) ;
}

void CGuitarHero3::init( struct guitar_hero_3_t *g )
{
  if( g != NULL )
  {
    Buttons = CGH3Buttons( &(g->btns.btns), &(g->btns.btns_held), &(g->btns.btns_released), &(g->btns.btns_previous) ) ;
    Joystick = CJoystick( &(g->js) ) ;
  }
  else
  {
    Buttons = CGH3Buttons() ;
    Joystick = CJoystick() ;
  }
}

float CGuitarHero3::GetWhammyBar()
{
  if( mpGH3Ptr != NULL )
    return mpGH3Ptr->whammy_bar ;
  else
    return 0.0f ;
} 

/*
 * CWiimote class methods.
 */

CWiimote::CWiimote() : // SWIG insisted it exist for the vectors. Hopefully it will only be used for copy.
  IR(), Buttons(), Accelerometer( NULL, NULL, &mpTempInt, NULL, &mpTempFloat, NULL ),
  ExpansionDevice(), mpWiimotePtr(NULL), mpTempInt(0), mpTempFloat(0),
  mpIsConnected(false), mpIsPolled(false), mpIsPolledButton(false),
  mpIsPolledAcc(false), mpIsPolledIR(false)
{
  puts( "Constructor CWiimote par défaut" ) ;
}

CWiimote::CWiimote(struct wiimote_t *wmPtr) :
  IR(), Buttons(), Accelerometer(), ExpansionDevice(), 
  mpWiimotePtr(NULL), mpTempInt(0), mpTempFloat(0),
  mpIsConnected(false), mpIsPolled(false), mpIsPolledButton(false),
  mpIsPolledAcc(false), mpIsPolledIR(false)
{
  puts( "Constructor CWiimote par argument" ) ;
  if( wmPtr != NULL )
    mpWiimotePtr = wmPtr ;
  else
    mpWiimotePtr = NULL ;

  init( mpWiimotePtr  ) ;
}

CWiimote::CWiimote(const CWiimote &copyin) : // Copy constructor to handle pass by value.
  IR(), Buttons(), Accelerometer(), ExpansionDevice(), 
  mpWiimotePtr(NULL), mpTempInt(0), mpTempFloat(0),
  mpIsConnected(false), mpIsPolled(false), mpIsPolledButton(false),
  mpIsPolledAcc(false), mpIsPolledIR(false)
{
  puts( "Constructor CWiimote par copie" ) ;
  mpWiimotePtr = copyin.mpWiimotePtr ;
  init( mpWiimotePtr ) ;
}

CWiimote::~CWiimote()
{
  puts( "Destructor CWiimote" ) ;
  // mpWiimotePtr : delete by ~CWii.
  mpWiimotePtr = NULL ;
}

void CWiimote::init( struct wiimote_t *wmPtr )
{
  mpTempInt = 0 ;
  mpTempFloat = 0 ;

  if( wmPtr != NULL )
  {
    IR = CIR(wmPtr) ;
    Buttons = CButtons( const_cast<short*>(&(wmPtr->btns.btns)), const_cast<short*>(&(wmPtr->btns.btns_held)), 
                        const_cast<short*>(&(wmPtr->btns.btns_released)), const_cast<short*>(&(wmPtr->btns.btns_previous)) ) ;
    Accelerometer = CAccelerometer( (accel_t*) &(wmPtr->accel_calib), (vec3b_t*) &(wmPtr->accel),
                                    (int*) &(wmPtr->accel_threshold), (orient_t*) &(wmPtr->orient),
                                    (float*) &(wmPtr->orient_threshold), (gforce_t*) &(wmPtr->gforce) ) ;
    ExpansionDevice = CExpansionDevice( (struct expansion_t*) &(wmPtr->exp) ) ;
  }
  else
  {
    IR = CIR() ;
    Buttons = CButtons() ;
    Accelerometer = CAccelerometer( NULL, NULL, &mpTempInt, NULL, &mpTempFloat, NULL ) ;
    ExpansionDevice = CExpansionDevice() ;
  }
}

void CWiimote::Disconnected()
{
    wiiuse_disconnected(mpWiimotePtr);
}

void CWiimote::SetRumbleMode(CWiimote::OnOffSelection State)
{
    wiiuse_rumble(mpWiimotePtr, State);
}

void CWiimote::ToggleRumble()
{
    wiiuse_toggle_rumble(mpWiimotePtr);
}

int CWiimote::GetLEDs()
{
  return (mpWiimotePtr!=NULL ? mpWiimotePtr->leds : 0 ) ;
}

void CWiimote::SetLEDs(int LEDs)
{
    wiiuse_set_leds(mpWiimotePtr, LEDs);
}

float CWiimote::GetBatteryLevel()
{
  return (mpWiimotePtr!=NULL ? mpWiimotePtr->battery_level : 0.0f ) ;
}

int CWiimote::GetHandshakeState()
{
  return (mpWiimotePtr!=NULL ? mpWiimotePtr->handshake_state : 0 ) ;
}

CWiimote::EventTypes CWiimote::GetEvent()
{
  return (mpWiimotePtr!=NULL ? (CWiimote::EventTypes) mpWiimotePtr->event : CWiimote::EVENT_NONE) ;
}

const unsigned char *CWiimote::GetEventBuffer()
{
  return (mpWiimotePtr!=NULL ? mpWiimotePtr->event_buf : NULL ) ;
}

void CWiimote::SetMotionSensingMode(CWiimote::OnOffSelection State)
{
    wiiuse_motion_sensing(mpWiimotePtr, State);
}

void CWiimote::ReadData(unsigned char *Buffer, unsigned int Offset, unsigned int Length)
{
    wiiuse_read_data(mpWiimotePtr, Buffer, Offset, Length);
}

void CWiimote::WriteData(unsigned int Address, unsigned char *Data, unsigned int Length)
{
    wiiuse_write_data(mpWiimotePtr, Address, Data, Length);
}

void CWiimote::UpdateStatus()
{
    wiiuse_status(mpWiimotePtr);
}

int CWiimote::GetID()
{
  return (mpWiimotePtr!=NULL ? mpWiimotePtr->unid : 0 ) ;
}

int CWiimote::GetState()
{
  return (mpWiimotePtr!=NULL ? mpWiimotePtr->state : 0 ) ;
}

int CWiimote::GetFlags()
{
  return (mpWiimotePtr!=NULL ? mpWiimotePtr->flags : 0 ) ;
}

int CWiimote::SetFlags(int Enable, int Disable)
{
    return wiiuse_set_flags(mpWiimotePtr, Enable, Disable);
}

void CWiimote::Resync()
{
    wiiuse_resync(mpWiimotePtr);
}

void CWiimote::Disconnect()
{
    wiiuse_disconnect(mpWiimotePtr);
}

int CWiimote::isUsingACC()
{
  if( mpWiimotePtr != NULL )
    return (mpWiimotePtr->state & 0x0020) != 0 ;
  else
    return 0 ;
}

int CWiimote::isUsingEXP()
{
  if( mpWiimotePtr != NULL )
    return (mpWiimotePtr->state & 0x0040) != 0 ;
  else
    return 0 ;
}

int CWiimote::isUsingIR()
{
  if( mpWiimotePtr != NULL )
    return (mpWiimotePtr->state & 0x0080) != 0 ;
  else
    return 0 ;
}

int CWiimote::isUsingSpeaker()
{
  if( mpWiimotePtr != NULL )
    return (mpWiimotePtr->state & 0x0100) != 0 ;
  else
    return 0 ;
}

int CWiimote::isLEDSet(int LEDNum)
{
  int result = 0;

  if( mpWiimotePtr != NULL )
  {
    switch(LEDNum)
    {
      case 1:
        result = (mpWiimotePtr->leds & LED_1) != 0;
        break;
      case 2:
        result = (mpWiimotePtr->leds & LED_2) != 0;
        break;
      case 3:
        result = (mpWiimotePtr->leds & LED_3) != 0;
        break;
      case 4:
        result = (mpWiimotePtr->leds & LED_4) != 0;
      default:
        result = 0;
    }
  }
  return result;
}

bool CWiimote::isConnected()
{
  return mpIsConnected ;
}

bool CWiimote::isPolled()
{
  return mpIsPolled ;
}

bool CWiimote::isPolledByIR()
{
  return mpIsPolledIR ;
}

bool CWiimote::isPolledByAcc()
{
  return mpIsPolledAcc ;
}

bool CWiimote::isPolledByButton()
{
  return mpIsPolledButton ;
}

CWiimoteData* CWiimote::copyData()
{
  CWiimoteData *wm=NULL ;
 
  if( mpWiimotePtr != NULL )
  {
    wm = new CWiimoteData(*this) ;
  }

  return wm ;
}

/*
 * Wii Class Methods
 */

CWii::CWii() : mpWiimoteArray(NULL), mpWiimoteArraySize(0)
{
  puts( "Constructor CWii by default" ) ;
  init( WIIUSECPP_DEFAULT_NUM_WM ) ;
}

CWii::CWii(int MaxNumWiimotes ) : mpWiimoteArray(NULL), mpWiimoteArraySize(0)
{
  puts( "Constructor CWii by arguments" ) ;
  init( MaxNumWiimotes ) ;
}

CWii::CWii( const CWii& cw ) : mpWiimoteArray(NULL), mpWiimoteArraySize(0)
{
  mpWiimoteArraySize = cw.mpWiimoteArraySize ;
  mpWiimoteArray = cw.mpWiimoteArray ;
  mpWiimotesVector = cw.mpWiimotesVector ;
}

CWii::~CWii()
{
  puts( "Destructor CWii" ) ;
  wiiuse_cleanup((struct wiimote_t**) mpWiimoteArray, mpWiimoteArraySize);

  CWiimote *wm=NULL ;
  for( int i=0 ; i<mpWiimoteArraySize ; i++ )
  {
    wm = mpWiimotesVector.at(i) ;
    delete( wm ) ;
    wm = NULL ;
    mpWiimotesVector.at(i) = NULL ;
  }

  mpWiimotesVector.clear() ;
}

void CWii::init( int nbWm )
{
  if( nbWm>0 && nbWm<=WIIUSECPP_MAX_NUM_WM )  
    mpWiimoteArraySize = nbWm ;
  else
    mpWiimoteArraySize = WIIUSECPP_DEFAULT_NUM_WM ;

  mpWiimoteArray = wiiuse_init(mpWiimoteArraySize);
  //mpWiimotesVector.clear() ;
  //mpWiimotesVector.reserve(mpWiimoteArraySize) ; // Useless to use more memories.
  //mpWiimotesVector.assign( 1, CWiimote(mpWiimoteArray[0]) ) ;

  CWiimote *wm=NULL ;
  for( int i=0 ; i<mpWiimoteArraySize ; i++ )
  {
    wm = new CWiimote(mpWiimoteArray[i]) ;
    mpWiimotesVector.push_back( wm ) ;
  }

  cout << "wiiusecpp / pywii by Jim Thomas (jt@missioncognition.net)" << endl ;
  cout << "Download from http://missioncognition.net/" << endl ;
}

void CWii::RefreshWiimotes()
{
  if( mpWiimoteArray!=NULL && mpWiimoteArraySize>1 )
  {
    // This approach is a bit wasteful but it will work.  The other
    // option is to force the user to handle disconnect events to remove
    // wiimotes from the array.
    mpWiimotesVector.clear();

    for( int i=0 ; i<mpWiimoteArraySize ; i++ )
    {
        if( (mpWiimoteArray[i]->state & 0x0008) != 0 )
        {
          CWiimote *wm = new CWiimote( mpWiimoteArray[i] ) ;
          mpWiimotesVector.push_back(wm);
        }
    }
  }
}

int CWii::GetNumConnectedWiimotes()
{
  int count=0 ;
  if( mpWiimoteArray!=NULL && mpWiimoteArraySize>1 )
  {
    for( int i=0 ; i<mpWiimoteArraySize ; i++ )
    {
      if( (mpWiimoteArray[i]->state & 0x0008) != 0 )
        count++;
    }
  }
  return count;
}

CWiimote* CWii::GetByID( int UnID, int Refresh )
{
  if( mpWiimoteArray!=NULL && mpWiimoteArraySize>1 )
  {
    if( Refresh )
        RefreshWiimotes();

    //int cpt=0 ;
    for( std::vector<CWiimote*>::iterator i=mpWiimotesVector.begin() ; i!=mpWiimotesVector.end() ; ++i )
		{
			if( (*i)->GetID() == UnID )
			{
        //cout << "GetByID:cpt:" << cpt << endl ;
				return *i;
			}
			//cpt ++ ;
    }

    return *mpWiimotesVector.begin(); // Return the first one if it was not found.
  }
  else
    return NULL ;
}

std::vector<CWiimote*>& CWii::GetWiimotes(int Refresh)
{
    if(Refresh)
        RefreshWiimotes();

    return mpWiimotesVector ;
}

void CWii::SetBluetoothStack(CWii::BTStacks Type)
{
    wiiuse_set_bluetooth_stack((struct wiimote_t**) mpWiimoteArray, mpWiimoteArraySize, (win_bt_stack_t) Type);
}

void CWii::SetTimeout(int NormalTimeout, int ExpTimeout)
{
    wiiuse_set_timeout((struct wiimote_t**) mpWiimoteArray, mpWiimoteArraySize, NormalTimeout, ExpTimeout);
}

int CWii::Find(int Timeout)
{
    return wiiuse_find((struct wiimote_t**) mpWiimoteArray, mpWiimoteArraySize, Timeout);
}

std::vector<CWiimote*>& CWii::Connect()
{
    wiiuse_connect((struct wiimote_t**) mpWiimoteArray, mpWiimoteArraySize) ;
  
    for( int i=0 ; i<mpWiimoteArraySize ; i++ ) // mpWiimotesVector.size()
      mpWiimotesVector.at(i)->mpIsConnected = true ;
      
    return mpWiimotesVector;
}


int CWii::Poll()
{
  bool a,b,c ;
  int d ;
  d = Poll(a,b,c) ; 
  return (a || b || c || d) ;
}

int CWii::Poll( bool &updateButton_out, bool &updateAccelerometerData_out, bool &updateIRData_out )
{
    bool isUpdated=false ;
    double x, y, z ;
    int poll=wiiuse_poll((struct wiimote_t**) mpWiimoteArray, mpWiimoteArraySize) ;

    updateButton_out = (poll==0?false:true) ;
    updateAccelerometerData_out = false ;
    updateIRData_out = false ;

    for( int i=0 ; i<mpWiimoteArraySize ; i++ )
    {
      if( mpWiimotesVector[i]->GetEvent() == CWiimote::EVENT_DISCONNECT
          || mpWiimotesVector[i]->GetEvent() == CWiimote::EVENT_UNEXPECTED_DISCONNECT )
      {
        mpWiimotesVector[i]->mpIsConnected = false ;
      }
      else
      {
        isUpdated = false ;

        // Buttons updated.
        isUpdated = mpWiimotesVector[i]->Buttons.isJustChanged() ;
        if( isUpdated )
          mpWiimotesVector[i]->mpIsPolledButton = true ;
        else
          mpWiimotesVector[i]->mpIsPolledButton = false ;

        // Acc Updated.
        if( poll )
          isUpdated = mpWiimotesVector[i]->Accelerometer.calculateValues( true ) ;
        else
          isUpdated = mpWiimotesVector[i]->Accelerometer.calculateValues( false ) ;

        if( isUpdated )
        {
          updateAccelerometerData_out = true ;
          mpWiimotesVector[i]->mpIsPolledAcc = true ;
        }
        else
          mpWiimotesVector[i]->mpIsPolledAcc = false ;

        // IR Updated.
        mpWiimotesVector[i]->IR.GetCursorDelta(x,y,z) ;
        if( x!=0 || y!=0 || z!=0 )
        {
          updateIRData_out = true ;
          mpWiimotesVector[i]->mpIsPolledIR = true ;
        }
        else
          mpWiimotesVector[i]->mpIsPolledIR = false ;

        if( mpWiimotesVector[i]->mpIsPolledButton
            || mpWiimotesVector[i]->mpIsPolledAcc 
            || mpWiimotesVector[i]->mpIsPolledIR )
          mpWiimotesVector[i]->mpIsPolled = true ;
        else
          mpWiimotesVector[i]->mpIsPolled = false ;
       }
    }

    return poll ;
}
