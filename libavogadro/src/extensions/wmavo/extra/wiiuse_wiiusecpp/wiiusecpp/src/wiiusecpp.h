/*
 *    wiiusecpp.cpp
 *
 *    Written By:
 *        James Thomas
 *        Email: jt@missioncognition.org
 *
 *    Copyright 2009
 *
 *    Copyright (c) 2010 Mickael Gadroy
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

#ifndef WIIUSECPP_H_
#define WIIUSECPP_H_

#define WIIUSECPP_DEFAULT_NUM_WM 1 /**< Number of Wiimote used by default */
#define WIIUSECPP_MAX_NUM_WM 4 /**< Max number of Wiimote usable */
#define WIIUSECPP_NUM_IRDOTS 4 /**< Number ir dots seen by the Wiimote (4 is the max) */
#define WIIUSECPP_GUNIT_TO_MS2UNIT 9.80665 /**< Constant to convert g unit to m.s^-2 unit. */
#define WIIUSECPP_ACC_NB_SAVED_VALUES 10 //< Number saved values in the mpValuesInTime attribut (CAccelerometer class) (3 is a minimum!).

// If you want change <wiiusecpp.h> <-> "wiiusecpp.h"
// do not forget to do the same in the makefile.
//#ifdef _WIN32
#include "wiiuse.h"
//#else
//#include <wiiuse.h>
//#endif

#include <vector>
#include <deque> // ~list
#include <iostream>
#include <ctime>
#include <cmath>
using namespace std;

class CWiimoteData ;

class CButtonBase
{
  friend class CButtonBaseData ;
  friend class CWii ;

public:
    CButtonBase(short *ButtonsPtr, short *ButtonsHeldPtr, short *ButtonsReleasedPtr, short *ButtonsPreviousPtr );
    virtual ~CButtonBase() ;

    inline short getBtnsState(){ return *mpBtnsPtr ; } ;
    bool isPressed(int Button);
    bool isHeld(int Button);
    bool isReleased(int Button);
    bool isJustPressed(int Button);
    bool isJustChanged() ;

protected:
    CButtonBase() ;
    CButtonBase( const CButtonBase& bb ) ;

private:
    //virtual inline short Cast(void *Ptr) {return *((short *)(Ptr));}

    short *mpBtnsPtr;
    short *mpBtnsHeldPtr;
    short *mpBtnsReleasedPtr;
    short *mpBtnsPreviousPtr;
};

class CButtons : public CButtonBase
{
  friend class CWiimote ;

public:
    enum ButtonDefs
    {
        BUTTON_TWO = WIIMOTE_BUTTON_TWO,
        BUTTON_ONE = WIIMOTE_BUTTON_ONE,
        BUTTON_B = WIIMOTE_BUTTON_B,
        BUTTON_A = WIIMOTE_BUTTON_A,
        BUTTON_MINUS = WIIMOTE_BUTTON_MINUS,
        BUTTON_HOME = WIIMOTE_BUTTON_HOME,
        BUTTON_LEFT = WIIMOTE_BUTTON_LEFT,
        BUTTON_RIGHT = WIIMOTE_BUTTON_RIGHT,
        BUTTON_DOWN = WIIMOTE_BUTTON_DOWN,
        BUTTON_UP = WIIMOTE_BUTTON_UP,
        BUTTON_PLUS = WIIMOTE_BUTTON_PLUS,
        BUTTON_UNKNOWN = WIIMOTE_BUTTON_UNKNOWN,
        BUTTON_ALL = WIIMOTE_BUTTON_ALL
    };

    CButtons(short *ButtonsPtr, short *ButtonsHeldPtr, short *ButtonsReleasedPtr, short *ButtonsPreviousPtr );
    virtual ~CButtons() ;

private :
    CButtons() ;
    CButtons( const CButtons& cb ) ;
};

class CNunchukButtons : public CButtonBase
{
  friend class CNunchuk ;

public:
    enum ButtonDefs
    {
        BUTTON_Z = NUNCHUK_BUTTON_Z,
        BUTTON_C = NUNCHUK_BUTTON_C,
        BUTTON_ALL = NUNCHUK_BUTTON_ALL
    };

    CNunchukButtons(short *ButtonsPtr, short *ButtonsHeldPtr, short *ButtonsReleasedPtr, short *ButtonsPreviousPtr );
    virtual ~CNunchukButtons() ;

protected :
    CNunchukButtons() ;
    CNunchukButtons( const CNunchukButtons& cnb ) ;

private:
    //short Cast(void *Ptr) {return (short)(*((byte *)(Ptr)));} // Inlined using the different type.
};

class CClassicButtons : public CButtonBase
{
  friend class CClassic ;
  friend class CClassicData ;

public:
    enum ButtonDefs
    {
        BUTTON_X = CLASSIC_CTRL_BUTTON_X,
        BUTTON_Y = CLASSIC_CTRL_BUTTON_Y,
        BUTTON_B = CLASSIC_CTRL_BUTTON_B,
        BUTTON_A = CLASSIC_CTRL_BUTTON_A,
        BUTTON_MINUS = CLASSIC_CTRL_BUTTON_MINUS,
        BUTTON_HOME = CLASSIC_CTRL_BUTTON_HOME,
        BUTTON_LEFT = CLASSIC_CTRL_BUTTON_LEFT,
        BUTTON_RIGHT = CLASSIC_CTRL_BUTTON_RIGHT,
        BUTTON_DOWN = CLASSIC_CTRL_BUTTON_DOWN,
        BUTTON_UP = CLASSIC_CTRL_BUTTON_UP,
        BUTTON_PLUS = CLASSIC_CTRL_BUTTON_PLUS,
        BUTTON_ZR = CLASSIC_CTRL_BUTTON_ZR,
        BUTTON_ZL = CLASSIC_CTRL_BUTTON_ZL,
        BUTTON_FULL_R = CLASSIC_CTRL_BUTTON_FULL_R,
        BUTTON_FULL_L = CLASSIC_CTRL_BUTTON_FULL_L,
        BUTTON_ALL = CLASSIC_CTRL_BUTTON_ALL
    };

    CClassicButtons(short *ButtonsPtr, short *ButtonsHeldPtr, short *ButtonsReleasedPtr, short *ButtonsPreviousPtr );
    virtual ~CClassicButtons() ;

private :
    CClassicButtons() ;
    CClassicButtons( const CClassicButtons& ccb ) ;
};

class CGH3Buttons : public CButtonBase
{
  friend class CGuitarHero3 ;

public:
    enum ButtonDefs
    {
        BUTTON_STRUM_UP = GUITAR_HERO_3_BUTTON_STRUM_UP,
        BUTTON_STRUM_DOWN = GUITAR_HERO_3_BUTTON_STRUM_DOWN,
        BUTTON_YELLOW = GUITAR_HERO_3_BUTTON_YELLOW,
        BUTTON_GREEN = GUITAR_HERO_3_BUTTON_GREEN,
        BUTTON_BLUE = GUITAR_HERO_3_BUTTON_BLUE,
        BUTTON_RED = GUITAR_HERO_3_BUTTON_RED,
        BUTTON_ORANGE = GUITAR_HERO_3_BUTTON_ORANGE,
        BUTTON_MINUS = GUITAR_HERO_3_BUTTON_MINUS,
        BUTTON_PLUS = GUITAR_HERO_3_BUTTON_PLUS,
        BUTTON_ALL = GUITAR_HERO_3_BUTTON_ALL
    };

    CGH3Buttons(short *ButtonsPtr, short *ButtonsHeldPtr, short *ButtonsReleasedPtr, short *ButtonsPreviousPtr);
    virtual ~CGH3Buttons() ;

private :
    CGH3Buttons() ;
    CGH3Buttons( const CGH3Buttons& cgb ) ;
};

class CJoystick
{
  friend class CNunchuk ;
  friend class CClassic ;
  friend class CGuitarHero3 ;
  friend class CJoystickData ;
  friend class CClassicData ;

public:
    CJoystick(struct joystick_t *JSPtr);
    ~CJoystick() ;

    void GetMaxCal(int &X, int &Y);
    void SetMaxCal(int X, int Y);

    void GetMinCal(int &X, int &Y);
    void SetMinCal(int X, int Y);

    void GetCenterCal(int &X, int &Y);
    void SetCenterCal(int X, int Y);

    void GetPosition(float &Angle, float &Magnitude);

private:
    CJoystick() ;
    CJoystick( const CJoystick& cj ) ;

    struct joystick_t *mpJoystickPtr;
};

class CAccelerometer
{
  friend class CWiimote ;
  friend class CNunchuk ;
  friend class CAccelerometerData ;

public:
    CAccelerometer( struct accel_t *AccelCalPtr, struct vec3b_t *AccelerationPtr,
                    int *AccelThresholdPtr, struct orient_t *OrientationPtr, 
                    float *OrientationThresholdPtr, struct gforce_t *GForcePtr );
    /* Initiate like that :
      Accelerometer( (accel_t*) &(wmPtr->accel_calib), (vec3b_t*) &(wmPtr->accel),
                     (int*) &(wmPtr->accel_threshold), (orient_t*) &(wmPtr->orient),

                     (float*) &(wmPtr->orient_threshold), (gforce_t*) &(wmPtr->gforce) )

    */

    ~CAccelerometer() ;

    /**

      * Enable the calculation of values contained in the mpValuesInTime attribut.

      * The calculation is realised when some getter method is called like GetGForce*, GetJerk*, GetDistance* ...
      */
    void setNeedUpdateValuesInTime( bool update ) ;

    /**
      * wiiuse.accel_t.st_alpha

      * Use to smooth the angle values when flags FLAGS_SMOOTHING is on.

      * @return Old value of alpha.

      */
    float SetSmoothAlpha(float Alpha);

    /**

      * wiiuse.wiimote_t.orient_threshold

      * Threshold ((fr:)seuil) for orient to generate an event.
      * @{
      */
    float GetOrientThreshold();
    void SetOrientThreshold(float Threshold);
    // @}

    /**

      * wiiuse.wiimote_t.accel_threshold
      * Threshold for accel to generate an event.

      * @{*/
    int GetAccelThreshold();
    void SetAccelThreshold(int Threshold);
    // @}

    /**

      * wiiuse.orient_t.roll .pitch .yaw
      * Get value for pitch, roll (and yaw when it is possible).

      * This value can be smooth when flags FLAGS_SMOOTHING is on.

      */
    void GetOrientation(float &Pitch, float &Roll, float &Yaw);
    
    /**

      * wiiuse.orient_t.roll .pitch

      * Get value for pitch, roll.

      * This value are unsmoothed.

      */
    void GetRawOrientation(float &Pitch, float &Roll);

    /** wiiuse.accel_t.cal_zero.x .y .z : the "zero g"
      * wiiuse.accel_t.cal_g.x .y .z    : the result of ("one g" - "zero g")

      *
      * Represent the accel calibrated values get from the Wiimote.
      * This data are necessary to get acc values in g unit from the hex values 

      * provide ((fr:)fournie) of the Wiimote.

      * Here, it provide just for informations.
      * The acc calibrated zero values represent where are the "zero g" position.
      * The acc calibrated one values represent where are the "one g" position.
      * @{ */
    void GetAccCalOne(float &X, float &Y, float &Z);
    void GetAccCalZero(float &X, float &Y, float &Z);
    // @}

    /*

     * wiiuse.gforce_t.x .y .z
     * Get GForce on 3 axes.

     * @{*/
    void GetGForceInG(double &X, double &Y, double &Z) ; //< in g unit.
    double GetGForceInG() ; //< in g unit.
    void GetGForceInMS2(double &X, double &Y, double &Z) ; //< in m.s^-2 unit.
    double GetGForceInMS2() ; //< in m.s^-2 unit.
    double GetGForceElapse() ; //< in g unit.
    // @}
    
    /**

      * Difference between the last and the current gForce values.
      * It lets to know the user movement in theory ...

      * This acceleration represents the derivative of the GForce => a jerk (m.s^-3).
      * jerk == the rate of change of acceleration => da/dt

      * http://en.wikipedia.org/wiki/Jerk_%28physics%29
      * Nota Bene: this method need be called twice before to get "good" values.
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
    void getTmp( double &tmp1, double &tmp2, double &tmp3 )
    {
      tmp1 = mpValuesInTime[0].tmp1 ;
      tmp2 = mpValuesInTime[0].tmp2 ;
      tmp3 = mpValuesInTime[0].tmp3 ;
    };

    // Calculate all things we can.
    bool calculateValues( bool mustBeCalculated ) ;

// Public attributs.
public :
  struct values_t
    {
      double time, diffTime, diffTimeSquarred ; // in s.
      double gForceX, gForceY, gForceZ ; // in m.s^-2.
      double gForce ; // in m.s^-2.
      double accelerationX, accelerationY, accelerationZ ; // in m.s^-2.
      double acceleration ; // in m.s^-2.
      double jerkX, jerkY, jerkZ ; // in m.s^-3.
      double jerk ; // in m.s^-3.
      double velocityX, velocityY, velocityZ ; // in m.s^-1.
      double velocity ; // in m.s^-1.
      double distanceX, distanceY, distanceZ ; // in m.
      double distance ; // in m.
      double positionX, positionY, positionZ ; // in m.
      double tmp1, tmp2, tmp3 ;

      values_t()      
      { time=0; diffTime=0; diffTimeSquarred=0;
        gForceX=0; gForceY=0; gForceZ=0; gForce=0; 
        accelerationX=0; accelerationY=0; accelerationZ=0; acceleration=0; 
        jerkX=0; jerkY=0; jerkZ=0; jerk=0; 
        velocityX=0; velocityY=0; velocityZ=0; velocity=0; 
        distanceX=0; distanceY=0; distanceZ=0; distance=0; 
        positionX=0; positionY=0; positionZ=0; 
        tmp1=0; tmp2=0; tmp3=0;
      }
    };

// Private methods.
private:
    CAccelerometer() ;
    CAccelerometer( const CAccelerometer& ca ) ;

    void initValuesInTime() ;

    /**
      * Time methods.
      * @{ */
    bool isCountdownEnoughAccuracyInMS() ; //< Test if the time system is accuracy in ms unit.
    double getElapsedTime() ; //< In s.
    // @}

    /**

      * Methods to calculate velocity, distance and position.

      * @{ */
    void calculateByPhysicBasicFormula_p( values_t &t ) ;
    void calculateFromTailorFormulaWithNonConstantTime_p( values_t &t ) ;
    void calculateFromTailorFormulaWithConstantTime_p( values_t &t ) ;
    // @}

// Private attributs.
private:
    struct accel_t *mpAccelCalibPtr ;
    struct vec3b_t *mpAccelPtr ;
    int *mpAccelThresholdPtr ;

    struct orient_t *mpOrientPtr ;
    float *mpOrientThresholdPtr ;

    struct gforce_t *mpGForcePtr ;
    deque<values_t> mpValuesInTime ; // ~lists
    bool mpUpdateOneMore ;
};

class CIRDot
{
  friend class CIR ; // For default/copy constructor.
  friend class CIRDotData ;

public:
    CIRDot(struct ir_dot_t *DotPtr) ;
    ~CIRDot() ;

    int isVisible();
    int GetSize();
    int GetOrder();
    void GetCoordinate(int &X, int &Y);
    void GetRawCoordinate(int &X, int &Y);

private:
    CIRDot() ;
    CIRDot(const CIRDot & copyin) ;

    struct ir_dot_t *mpDotPtr ;
};

class CIR
{
  friend class CWiimote ;
  friend class CIRData ;

public:
    enum BarPositions
    {
        BAR_ABOVE = WIIUSE_IR_ABOVE,
        BAR_BELOW = WIIUSE_IR_BELOW
    };

    enum AspectRatioSelections
    {
        ASPECT_4_3 = WIIUSE_ASPECT_4_3,
        ASPECT_16_9 = WIIUSE_ASPECT_16_9
    };

    enum OnOffSelection
    {
        OFF = 0,
        ON = 1
    };

    CIR(struct wiimote_t *wmPtr);
    ~CIR() ;

    void SetMode(OnOffSelection State);
    void SetVres(unsigned int x, unsigned int y);

    BarPositions GetBarPositionSetting();
    void SetBarPosition(BarPositions PositionSelection);

    AspectRatioSelections GetAspectRatioSetting();
    void SetAspectRatio(AspectRatioSelections AspectRatioSelection );

    void SetSensitivity(int Level);
    int GetSensitivity();

    int GetNumDots();
    int GetNumSources() ;
    std::vector<CIRDot*>& GetDots();

    void GetOffset(int &X, int &Y);
    int GetState();
    void GetCursorPositionAbsolute(int &X, int &Y);
    void GetCursorPosition(int &X, int &Y);
    void GetCursorDelta(double &X, double &Y, double &Z);
    float GetPixelDistance();
    float GetDistance();

    // If the delta are 
    bool IsInPrecisionMode() ;

//private:
public :
    CIR() ;
    CIR( const CIR& ci ) ;
private:
    void init( struct wiimote_t *wmPtr ) ;

    struct wiimote_t *mpWiimotePtr;
    std::vector<CIRDot*> mpIRDotsVector;
};

class CNunchuk
{
  friend class CExpansionDevice ; // For default/copy constructor.
  friend class CNunchukData ;

public:
    CNunchuk( struct expansion_t *ExpPtr ) ;
    ~CNunchuk() ;

    CNunchukButtons Buttons;
    CJoystick Joystick;
    CAccelerometer Accelerometer;

private:
    CNunchuk() ;
    CNunchuk( const CNunchuk& cn ) ;

    void init( struct nunchuk_t *ExpPtr ) ;
    struct nunchuk_t *mpNunchukPtr;
};

class CClassic
{
  friend class CClassicData ;

public:
    CClassic(struct expansion_t *ExpPtr);
    ~CClassic() ;

    float GetLShoulderButton();
    float GetRShoulderButton();

    CClassicButtons Buttons;
    CJoystick LeftJoystick;
    CJoystick RightJoystick;

private:
    CClassic() ;
    CClassic( const CClassic& cc ) ;

    void init( struct classic_ctrl_t *ExpPtr ) ;

    struct classic_ctrl_t *mpClassicPtr;
};

class CGuitarHero3
{
  friend class CGuitarHero3Data ;
public:
    CGuitarHero3( struct expansion_t *ExpPtr ) ;

    float GetWhammyBar();

    CGH3Buttons Buttons;
    CJoystick Joystick;

private:
    CGuitarHero3() ;
    CGuitarHero3( const CGuitarHero3& cgh ) ;

    void init( struct guitar_hero_3_t *g ) ;

    struct guitar_hero_3_t *mpGH3Ptr;
};

class CExpansionDevice
{
  friend class CWiimote ;
  friend class CExpansionDeviceData ;

public:
    enum ExpTypes
    {
        TYPE_NONE = EXP_NONE,
        TYPE_NUNCHUK = EXP_NUNCHUK,
        TYPE_CLASSIC = EXP_CLASSIC,
        TYPE_GUITAR_HERO_3 = EXP_GUITAR_HERO_3
    };

    CExpansionDevice(struct expansion_t *ExpPtr);
    ~CExpansionDevice() ;

    ExpTypes GetType();

    CNunchuk Nunchuk;
    CClassic Classic;
    CGuitarHero3 GuitarHero3;

private:
    CExpansionDevice() ;
    CExpansionDevice( const CExpansionDevice& ced ) ;

    struct expansion_t *mpExpansionPtr;
};


class CWiimote
{
  friend class CWii ;
  friend class CWiimoteData ;
public:
    enum LEDS
    {
        LED_NONE = WIIMOTE_LED_NONE,
        LED_1 = WIIMOTE_LED_1,
        LED_2 = WIIMOTE_LED_2,
        LED_3 = WIIMOTE_LED_3,
        LED_4 = WIIMOTE_LED_4

    };

    enum Flags
    {
        FLAG_SMOOTHING = WIIUSE_SMOOTHING,
        FLAG_CONTINUOUS = WIIUSE_CONTINUOUS,
        FLAG_ORIENT_THRESH = WIIUSE_ORIENT_THRESH,
        FLAG_INIT_FLAGS = WIIUSE_INIT_FLAGS
    };

    enum EventTypes
    {
        EVENT_NONE = WIIUSE_NONE,
        EVENT_EVENT = WIIUSE_EVENT,
        EVENT_STATUS = WIIUSE_STATUS,
        EVENT_CONNECT = WIIUSE_CONNECT,
        EVENT_DISCONNECT = WIIUSE_DISCONNECT,
        EVENT_UNEXPECTED_DISCONNECT = WIIUSE_UNEXPECTED_DISCONNECT,
        EVENT_READ_DATA = WIIUSE_READ_DATA,
        EVENT_NUNCHUK_INSERTED = WIIUSE_NUNCHUK_INSERTED,
        EVENT_NUNCHUK_REMOVED = WIIUSE_NUNCHUK_REMOVED,
        EVENT_CLASSIC_CTRL_INSERTED = WIIUSE_CLASSIC_CTRL_INSERTED,
        EVENT_CLASSIC_CTRL_REMOVED = WIIUSE_CLASSIC_CTRL_REMOVED,
        EVENT_GUITAR_HERO_3_CTRL_INSERTED = WIIUSE_GUITAR_HERO_3_CTRL_INSERTED,
        EVENT_GUITAR_HERO_3_CTRL_REMOVED = WIIUSE_GUITAR_HERO_3_CTRL_REMOVED
    };

    enum OnOffSelection
    {
        OFF = 0,
        ON = 1
    };

    //static const int EVENT_BUFFER_LENGTH = MAX_PAYLOAD;
    //static const float ORIENT_PRECISION = WIIUSE_ORIENT_PRECISION;

    CWiimote(struct wiimote_t *wmPtr) ;
    ~CWiimote() ;

    int SetFlags(int Enable, int Disable);
    void SetLEDs(int LEDs);
    void SetRumbleMode(OnOffSelection State);
    void ToggleRumble();
    void SetMotionSensingMode(OnOffSelection State);

    int GetID();
    int GetState();
    int GetFlags();
    int GetLEDs();
    float GetBatteryLevel();
    int GetHandshakeState();
    EventTypes GetEvent();
    const unsigned char *GetEventBuffer();

    int isUsingACC();
    int isUsingEXP();
    int isUsingIR();
    int isUsingSpeaker();
    int isLEDSet(int LEDNum);

    bool isConnected() ;
    bool isPolled() ;
    bool isPolledByIR() ;
    bool isPolledByAcc() ;
    bool isPolledByButton() ;

    void ReadData(unsigned char *Buffer, unsigned int Offset, unsigned int Length);
    void WriteData(unsigned int Address, unsigned char *Data, unsigned int Length);

    void UpdateStatus();
    void Resync();
    void Disconnect();
    void Disconnected();

    /**
      * Method to copy all usefull data. The returned CWiimote object cannot be used to manipulated
      * the Wiimote (!). Do not forget to delete the object after using.
      */
    CWiimoteData* copyData() ;

    CIR IR;
    CButtons Buttons;
    CAccelerometer Accelerometer;
    CExpansionDevice ExpansionDevice;

private:
    CWiimote() ;
    CWiimote(const CWiimote & copyin) ;

    void init( struct wiimote_t *wmPtr ) ;

    struct wiimote_t *mpWiimotePtr; /* The pointer to the wm structure */
    int mpTempInt ; // Ref in the default constructor.
    float mpTempFloat ; // Ref in the default constructor.

    bool mpIsConnected ;
    bool mpIsPolled ;

    bool mpIsPolledButton ;
    bool mpIsPolledAcc ;
    bool mpIsPolledIR ;
};

class CWii
{
public:
    enum BTStacks
    {
        STACK_UNKNOWN = WIIUSE_STACK_UNKNOWN,
        STACK_MS = WIIUSE_STACK_MS,
        STACK_BLUESOLEIL = WIIUSE_STACK_BLUESOLEIL
    };

    CWii() ;
    CWii( int MaxNumCWiimotes ) ;
    ~CWii() ;

    int GetNumConnectedWiimotes();

    void RefreshWiimotes();

    CWiimote* GetByID(int ID, int Refresh=1);
		//CWiimote& GetByID(int ID); // Without pass by mpWiimoteVector.
    std::vector<CWiimote*>& GetWiimotes(int Refresh=1);

    void SetBluetoothStack(BTStacks Type);
    void SetTimeout(int NormalTimeout, int ExpTimeout);

    int Find(int timeout) ;
    std::vector<CWiimote*>& Connect() ;

    int Poll() ; //< Return !0 if a button is just changed state.
    int Poll( bool &updateButton_out, bool &updateAccelerometerData_out, bool &updateIRData_out ) ;
      //< Return !0 if a button is just changed state, if accelemeter/ir data have changed.

private:
    CWii( const CWii& cw ) ;

    void init( int MaxNumCWiimotes ) ;

    struct wiimote_t **mpWiimoteArray ;
    int mpWiimoteArraySize ;
    std::vector<CWiimote*> mpWiimotesVector ;
};


#include "wiiusecppdata.h"

#endif /* WIIUSECPP_H_ */
