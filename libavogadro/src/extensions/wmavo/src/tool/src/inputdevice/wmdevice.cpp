
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

#include "wmdevice.h"
#include "wmrumble.h"

namespace InputDevice
{
  WmDeviceData_from::WmDeviceData_from()
    : DeviceData_from(), m_data(NULL)
  {
  }

  WmDeviceData_from::WmDeviceData_from( CWiimoteData *wmData )
    : DeviceData_from(), m_data(wmData)
  {
  }

  WmDeviceData_from::WmDeviceData_from( const WmDeviceData_from &wmData )
    : DeviceData_from()
  {
    m_data = new CWiimoteData() ;
    *m_data = *wmData.m_data ;
  }

  WmDeviceData_from::~WmDeviceData_from()
  {
    if( m_data != NULL )
    {
      delete m_data ;
      m_data = NULL ;
    }
  }

  // To be sur to have 2 distinct objects.
  void WmDeviceData_from::operator=( const WmDeviceData_from& wmDataFrom )
  {
    if( this!=&wmDataFrom && m_data!=wmDataFrom.m_data )
    {
      if( m_data == NULL )
        m_data = new CWiimoteData() ;

      *m_data = *(wmDataFrom.m_data) ;
    }
  }

  WmDeviceData_to::WmDeviceData_to()
    : DeviceData_to(),
      m_setRumble(NULL), m_updateRumble(false),
      m_setLED(0), m_updateLED(false),
      m_setSleepThread(PLUGIN_WM_SLEEPTHREAD_ONOFF), m_updateSleepThread(false)
  {
    m_setRumble = new RumbleSettings() ;
  }

  WmDeviceData_to::WmDeviceData_to( RumbleSettings* rumble, int LED )
    : DeviceData_to(), 
      m_setRumble(rumble), m_updateRumble(true),
      m_setLED(LED), m_updateLED(false),
      m_setSleepThread(PLUGIN_WM_SLEEPTHREAD_ONOFF), m_updateSleepThread(false)
  {
    if( rumble == NULL )
      m_setRumble = new RumbleSettings() ;
  }

  WmDeviceData_to::WmDeviceData_to( const WmDeviceData_to& data )
    : DeviceData_to(), 
      m_updateRumble(true),
      m_updateLED(false),
      m_updateSleepThread(false)
  {
    m_setLED = data.m_setLED ;
    *m_setRumble = *(data.m_setRumble) ;
    m_setSleepThread = data.m_setSleepThread ;
  }

  WmDeviceData_to::~WmDeviceData_to()
  {
    if( m_setRumble != NULL )
    {
      delete m_setRumble ;
      m_setRumble = NULL ;
    }
  }

  bool WmDeviceData_to::getRumble( RumbleSettings &rumble_out )
  { 
    bool up=m_updateRumble ; 
    m_updateRumble = false ; 
    rumble_out = *m_setRumble ; 
    return up ; 
  }
  
  void WmDeviceData_to::setRumble( const RumbleSettings &rumble )
  { 
    m_updateRumble = true ; 
    *m_setRumble = rumble ; 
  }

  bool WmDeviceData_to::getLED( int &LED )
  { 
    bool up=m_updateLED ; 
    m_updateLED = false ; 
    LED = m_setLED ; 
    return up ; 
  }
   
  void WmDeviceData_to::setLED( int LED )
  { 
    m_updateLED = true ; 
    m_setLED = LED ; 
  }

  bool WmDeviceData_to::getHasSleepThread( bool &sleepThread )
  {
    bool up=m_updateSleepThread ;
    m_updateSleepThread = false ; 
    sleepThread = m_setSleepThread ; 
    return up ;
  }

  void WmDeviceData_to::setHasSleepThread( bool sleepThread )
  {
    m_updateSleepThread = true ;
    m_setSleepThread = sleepThread ;
  }

  void WmDeviceData_to::operator=( const WmDeviceData_to& wmDataTo )
  {
    if( this!=&wmDataTo && m_setRumble!=wmDataTo.m_setRumble )
    {
      if( m_setRumble == NULL )
        m_setRumble = new RumbleSettings() ;

      *m_setRumble = *(wmDataTo.m_setRumble) ;
      m_updateRumble = wmDataTo.m_updateRumble ;
      m_setLED = wmDataTo.m_setLED ;
      m_updateLED = wmDataTo.m_updateLED ;
      m_setSleepThread = wmDataTo.m_setSleepThread ;
      m_updateSleepThread = wmDataTo.m_updateSleepThread ;
    }
  }

  void WmDeviceData_to::initAttributsToZero()
  {
    if( m_setRumble != NULL )
    {
      delete m_setRumble ;
      m_setRumble = NULL ;
    }
    
    m_setRumble = new RumbleSettings() ;
    m_updateRumble = false ;
    m_setLED = 0 ;
    m_updateLED = false ;
    m_setSleepThread = false ;
    m_updateSleepThread = false ;
  } ;

  
  WmDevice::WmDevice()
    : Device(),
      m_cirBufferFrom(NULL), m_cirBufferTo(NULL),
      m_semThreadFinish(1),
      m_isRunning(false), m_hasSleepThread(PLUGIN_WM_SLEEPTHREAD_ONOFF),
      m_nbActionRealized(0),
      m_wii(NULL), m_wm(NULL), m_nc(NULL),
      m_rumble(NULL), 
      m_hasWm(false), m_hasNc(false), m_previousEvent(CWiimote::EVENT_NONE),
      m_t1(0), m_t2(0), m_t1Update(0), m_t2Update(0), m_t1Update2(0), m_t2Update2(0)
  {
    m_cirBufferFrom = new WIWO_sem<WmDeviceData_from*>( CIRBUFFER_DEFAULT_SIZE ) ;
    m_cirBufferTo = new WIWO_sem<WmDeviceData_to*>( CIRBUFFER_DEFAULT_SIZE ) ;
    m_threadFinished.fetchAndStoreRelaxed(1) ;
    
    m_time.start() ;
    m_nbUpdate = new WIWO<unsigned int>(20) ;
    m_nbUpdate2 = new WIWO<unsigned int>(20) ;
  }

  WmDevice::~WmDevice()
  {
    stopPoll() ;

    if( m_cirBufferFrom != NULL )
    {
      delete m_cirBufferFrom ;
      m_cirBufferFrom = NULL ;
    }

    if( m_cirBufferTo != NULL )
    {
      delete m_cirBufferTo ;
      m_cirBufferTo = NULL ;
    }

    if( m_rumble != NULL )
    {
      delete m_rumble ;
      m_rumble = NULL ;
    }

    if( m_nbUpdate != NULL )
    {
      delete m_nbUpdate ;
      m_nbUpdate = NULL ;
    }

    if( m_nbUpdate2 != NULL )
    {
      delete m_nbUpdate2 ;
      m_nbUpdate2 = NULL ;
    }

    deleteWii() ;
    
    #if __WMDEBUG_CHEMWRAPPER
    mytoolbox::dbgMsg( "WmDevice::~WmDevice(): End" ) ;
    #endif
  }

  void WmDevice::deleteWii()
  {
    if( m_wm!=NULL && m_wm->isConnected() )
    { 
      m_wm->Disconnect() ; // Close Bluetooth connection.
      m_wm->Disconnected() ; // Clean up the wiimote structure.
      delete m_wii ; m_wii = NULL ; 
    }

    m_wm = NULL ;
    m_nc = NULL ;
    m_hasWm = false ;
    m_hasNc = false ;
  }


  bool WmDevice::hasDeviceDataAvailable()
  {
    if( m_isRunning && m_cirBufferFrom!=NULL )
      return !(m_cirBufferFrom->isEmpty()) ;
    else
      return false ;
  }


  /**
    * Get the last state data of Wiimote (blocking call).
    */
  WmDeviceData_from* WmDevice::getDeviceDataFrom()
  { 
    if( m_isRunning && m_cirBufferFrom!=NULL )
    {
      WmDeviceData_from *data=NULL ;
      m_cirBufferFrom->popFront( data ) ;
      return data ;
    }

    return NULL ;
  }

  /**
    * Set the last wanted state for Wiimote (blocking call).
    */
  void WmDevice::setDeviceDataTo( const WmDeviceData_to& wmDataTo )
  {
    if( m_isRunning && m_cirBufferTo!=NULL )
    {
      WmDeviceData_to *data=new WmDeviceData_to() ;
      *data = wmDataTo ;
      m_cirBufferTo->pushBack(data) ;
    }
  }


  bool WmDevice::connectAndStart()
  {
    if( connectWm() > 0 )
    { // Running!

      bool isConnect=this->connect( &m_deviceThread, SIGNAL(started()), SLOT(runPoll()) ) ;
      if( !isConnect )
        mytoolbox::dbgMsg( "Problem connection signal : m_deviceThread.started() -> WmDevice.runPoll() !!" ) ;
      this->moveToThread( &m_deviceThread ) ;

      m_threadFinished.fetchAndStoreRelaxed(0) ;
      m_deviceThread.start() ;

      return true ;
    }
    else
      return false ;
  }


  /**
    * Realize the connection of the Wiimote :
    *  - search a Wiimote ;
    *  - connect to the Wiimote ;
    *  - initiate some Wiimote elements.
    */
  int WmDevice::connectWm()
  {
    #if __WMDEBUG_WMDEVICE
    QString msg=tr("WmDevice::connectWm()") ;
    mytoolbox::dbgMsg( msg ) ;
    #endif
    
    int nbFound=0, nbConnect=0 ;
    CWiimote *wm=NULL ;

    // Let to restart the connection properly.
    deleteWii() ;
    
    #if __WMDEBUG_WMDEVICE
    msg=tr("WmDevice::connectWm() : new CWii(1)") ;
    mytoolbox::dbgMsg( msg ) ;
    #endif
    
    m_wii = new CWii(1) ;
    
    #if __WMDEBUG_WMDEVICE
    msg= tr("WmDevice::connectWm() : new CWii(1) : Passed") ;
    mytoolbox::dbgMsg( msg ) ;
    #endif

    // Searching.
    mytoolbox::dbgMsg( "Searching for wiimotes... Turn them on !" ) ;
    mytoolbox::dbgMsg( "Press 1+2 !" ) ;
    nbFound = m_wii->Find( (int)WMAVO_CONNECTION_TIMEOUT ) ;
    //cout << "Found " << nbFound << " wiimotes" << endl ;

    // Under windows, even not found and not connected, 
    // if there is a wiimote in the device manager,
    // it connects ...
    if( nbFound > 0 )
    {
      mytoolbox::dbgMsg( "Connecting to wiimotes..." ) ;
      std::vector<CWiimote*>& wms=m_wii->Connect() ;
      nbConnect = wms.size() ;
      wm = wms.at(0) ;
    }

    if( nbConnect <= 0 )
    {
      mytoolbox::dbgMsg( "Wiimote NO connected !!!" ) ;
      deleteWii() ;
    }
    else
    {
      mytoolbox::dbgMsg( "Wiimote connected !" ) ;
      m_hasWm = true ;
      m_wm = wm ;

      m_wm->SetLEDs( CWiimote::LED_1 | (WMEX_ADJUST_HYDROGEN?CWiimote::LED_4:0x0)) ; // Light LED 1.
      m_wm->IR.SetMode( CIR::ON ) ; // Activate IR.
      m_wm->IR.SetSensitivity(WMAVO_IRSENSITIVITY) ;
      m_wm->SetMotionSensingMode(CWiimote::ON) ;
      //m_wm->Accelerometer.SetAccelThreshold(3); // 5 by default.

      // For continue comm. (already with IR enable)
      //m_wm->SetFlags( CWiimote::FLAG_CONTINUOUS, 0x0 ) ;
      // For smoothed angle. (not effective)
      //m_wm->SetFlags( CWiimote::FLAG_SMOOTHING, 0x0 ) ;        

      //Rumble for 0.2 seconds as a connection ack
      m_wm->SetRumbleMode(CWiimote::ON);
      #if defined WIN32 || defined _WIN32
      Sleep(200);
      #else
      usleep(200000);    
      #endif
      //wiimote->SetRumbleMode(CWiimote::OFF); !!! 
      // (wiimote) should come directly from wiimote_t !!!
      m_wm->SetRumbleMode(CWiimote::OFF);

      // Activate the advanced rumble feature.
      m_rumble = new WmRumble(this) ;
      m_rumble->setRumbleEnabled( PLUGIN_WM_VIBRATION_ONOFF ) ;

      // Connect (== Get) the nunchuk.
      connectNc() ;
    }

    return nbConnect ;
  }

  bool WmDevice::connectNc()
  {
    m_nc = NULL ;

    if( m_wm != NULL
        && m_wm->ExpansionDevice.GetType() == m_wm->ExpansionDevice.TYPE_NUNCHUK )
    {
      m_nc = &(m_wm->ExpansionDevice.Nunchuk) ;
    }

    return m_hasNc ;
  }
  
  void WmDevice::runPoll()
  {
    m_deviceThread.setPriority( QThread::LowPriority ) ;
    
    if( !m_hasWm )
    {
      m_threadFinished.fetchAndStoreRelaxed(1) ;
      m_isRunning = false ;
    }
    else
    {
      m_semThreadFinish.acquire(1) ;
      
      bool hasUpdateFrom=false ;
      bool hasUpdateTo=false ;
      bool run=true ; // = m_isRunning.
      m_isRunning = true ;

      // Update local data.
      while( run && m_hasWm )
      {
        // Count nb action by second (used with breakpoint).
        m_t2Update = m_time.elapsed() ;
        if( (m_t2Update-m_t1Update) > 1000 )
        {
          #if __WMDEBUG_WMDEVICE
          QString msg= tr("WmDevice::runPoll() : tread has ") 
                       + QString::number( (*m_nbUpdate)[0] )
                       + tr(" actions/second" ) ;
          mytoolbox::dbgMsg( msg ) ;
          #endif
          
          m_t1Update = m_t2Update ;
          m_nbUpdate->pushFront(0) ;
          (*m_nbUpdate)[0]++ ;
        }
        else
        {
          (*m_nbUpdate)[0]++ ;
        }
        
        #if __WMDEBUG_WMDEVICE || __WMDEBUG_CHEMWRAPPER
        QString msg ;
        
        if( m_cirBufferFrom->isFull() )
        {
          msg= tr("WmDevice::runPoll() : m_cirBufferFrom->isFull !!!" ) ;
          mytoolbox::dbgMsg( msg ) ;
        }
        
        if( m_cirBufferTo->isFull() )
        {
          msg = tr("WmDevice::runPoll() : m_cirBufferTo->isFull !!!" ) ;
          mytoolbox::dbgMsg( msg ) ;
        }
        #endif


        // Check if there are some works.
        hasUpdateFrom = updateDataFrom() ; // Poll directly the Wiimote, not blocking call.

        if( !m_cirBufferTo->isEmpty() )
          hasUpdateTo = true ;
        else
          hasUpdateTo = false ;
        
        if( hasUpdateFrom || hasUpdateTo )
        { // Working.
          if( hasUpdateTo )
            updateDataTo() ;
        }

        // Sleeping ...
        if( m_hasSleepThread )
        {
          m_nbActionRealized ++ ;

          if( m_nbActionRealized > PLUGIN_WM_SLEEPTHREAD_NBTIME_BEFORE_SLEEP )
          {
            m_nbActionRealized = 0 ;
            m_deviceThread.msleep(PLUGIN_WM_SLEEPTHREAD_TIME) ;
          }
          else
          {
            m_deviceThread.yieldCurrentThread() ;
          }
        }
        else
          m_deviceThread.yieldCurrentThread() ;
          // With m_deviceThread.setPriority( QThread::LowPriority ) ;
        
        m_mutexIsRunning.lockForRead() ;
        run = m_isRunning ;
        m_mutexIsRunning.unlock() ;
      }

      m_threadFinished.fetchAndStoreRelaxed(1) ;
      
      #if __WMDEBUG_CHEMWRAPPER || __WMDEBUG_WMDEVICE
      mytoolbox::dbgMsg( "WmDevice::runPoll() m_threadFinished 1" ) ;
      #endif

      m_semThreadFinish.release(1) ;
    }
  }

  void WmDevice::stopPoll()
  {
    if( m_isRunning )
    {
      // Stop the working thread.
      m_mutexIsRunning.lockForWrite() ;
      m_isRunning = false ;
      m_mutexIsRunning.unlock() ;
     
      m_semThreadFinish.acquire(1) ;
      while( m_threadFinished == 0 ) ; 
      // "while" and "QAtomic" becomes useless since the semaphore (m_semThreadFinish).
      m_semThreadFinish.release(1) ;
      
      WmDeviceData_from *dataFrom=NULL ;
      WmDeviceData_to *dataTo=NULL ;

      while( !m_cirBufferFrom->isEmpty() )
      {
        m_cirBufferFrom->popFront(dataFrom) ;
        if( dataFrom != NULL )
        {
          delete dataFrom ;
          dataFrom = NULL ;
        }
      }
      
      while( !m_cirBufferTo->isEmpty() )
      {
        m_cirBufferTo->popFront(dataTo) ;
        if( dataTo != NULL )
        {
          delete dataTo ;
          dataTo = NULL ;
        }
      }
      
    }

    // Stop the event loop thread (run() method).
    m_deviceThread.quit() ;
    m_deviceThread.wait() ;
  }

  bool WmDevice::updateDataFrom()
  {
    bool r=false ;

    if( m_hasWm )
    {
      bool isPoll=false ;
      bool updateButton, updateAcc, updateIR ;

      m_mutex.lock() ; // Protect for rumble feature.
      m_wii->Poll( updateButton, updateAcc, updateIR ) ;
      m_mutex.unlock() ;

      //Poll the wiimotes to get the status like pitch or roll
      if( updateButton )
      {
        switch( m_wm->GetEvent() )
        {
        case CWiimote::EVENT_EVENT :
          //mytoolbox::dbgMsg( "--- EVENT_EVENT :( WmDevice::updateDataFrom )" ) ;
          isPoll = true ;
          break ;

        case CWiimote::EVENT_DISCONNECT :
        case CWiimote::EVENT_UNEXPECTED_DISCONNECT :
          mytoolbox::dbgMsg( "--- Wiimote DISCONNECTED :( WmDevice::updateDataFrom )" ) ;
          m_hasWm = false ;
          //m_wm = NULL ;
          break ;

        case CWiimote::EVENT_NUNCHUK_INSERTED:
          //mytoolbox::dbgMsg( "--- NUNCHUCK CONNECTED :) WmDevice::updateDataFrom )" ) ;
          connectNc() ;
          break ;

        case CWiimote::EVENT_NUNCHUK_REMOVED:
          //mytoolbox::dbgMsg( "--- NUNCHUCK DISCONNECTED :( WmDevice::updateDataFrom )" ) ;
          m_nc = NULL ;
          break ;

        case CWiimote::EVENT_NONE :
        case CWiimote::EVENT_STATUS :
        case CWiimote::EVENT_CONNECT :
        case CWiimote::EVENT_READ_DATA :
        case CWiimote::EVENT_GUITAR_HERO_3_CTRL_REMOVED :
        case CWiimote::EVENT_GUITAR_HERO_3_CTRL_INSERTED :
        case CWiimote::EVENT_CLASSIC_CTRL_REMOVED :
        case CWiimote::EVENT_CLASSIC_CTRL_INSERTED :
        default:
          //mytoolbox::dbgMsg( "--- EVENT_NONE ... :( WmDevice::updateDataFrom )" ) ;
          break ;
        }
      }

      if( updateButton || updateAcc || updateIR )
      {
        // Reduce data quantities.
        // In the future, when the out of the input device is standard, move or
        // copy this method in the wrapper.
        if( reduceSentData(*m_wm) )
        {
          // Count nb action by second (used with breakpoint).
          m_t2Update2 = m_time.elapsed() ;
          if( (m_t2Update2-m_t1Update2) > 1000 )
          {
            #if __WMDEBUG_WMDEVICE
            QString msg= tr("WmDevice::runPoll() : outgoing data ") 
                         + QString::number( (*m_nbUpdate2)[0] )
                         + tr( " actions/second." ) ;
            mytoolbox::dbgMsg( msg ) ;
            #endif
            
            m_t1Update2 = m_t2Update2 ;
            m_nbUpdate2->pushFront(0) ;
            (*m_nbUpdate2)[0]++ ;
          }
          else
          {
            (*m_nbUpdate2)[0]++ ;
          }

          // 1st stategy : If no place, no push data.
          if( !m_cirBufferFrom->isFull() )
          {
            //cout << "updateButton:" << updateButton << ", updateAcc:" << updateAcc << ", updateIR:" << updateIR << endl ;
            CWiimoteData *wmData=m_wm->copyData() ;
            
            if( wmData != NULL )
            { 
              WmDeviceData_from *wm=new WmDeviceData_from(wmData) ;
              
              if( wm==NULL || wm->getDeviceData()==NULL
                  || wm->getDeviceData()->GetBatteryLevel() < 0  
                  || wm->getDeviceData()->IR.GetNumDots()<0 
                  || wm->getDeviceData()->IR.GetNumDots()>4 )
              {
                mytoolbox::dbgMsg("Error data : WmDeviceData_from *wm!=NULL && no allocated ...") ;
              }
              else
              {             
                /* 
                cout << "m_wm->isConnected():" << m_wm->isConnected() << ", wm->getDeviceData()->isConnected()" << wm->getDeviceData()->isConnected() << endl ;
                
                float x, y, z ;
                m_wm->Accelerometer.GetOrientation( x, y, z ) ;              
                cout << "m_wm->Accelerometer.GetOrientation: x:" << x << ", y:" << y << ", z:" << z << endl ;
                
                wmData->Accelerometer.GetOrientation( x, y, z ) ;
                cout << "wmData1->Accelerometer.GetOrientation: x:" << x << ", y:" << y << ", z:" << z << endl ;
                */
                
                
                m_cirBufferFrom->pushBack( wm ) ;
                r = true ;
              }
            }
          }
        }

      }
    }

    return r ;
  }

  bool WmDevice::updateDataTo()
  {
    bool r=false ;

    if( !m_cirBufferTo->isEmpty() )
    {
      WmDeviceData_to *data=NULL ;
      m_cirBufferTo->popFront( data ) ;

      if( data != NULL )
      {
        bool hasSleepThread=false ;
        bool hasUpdated=false ;
        RumbleSettings rumble ;
        int led=0 ;

        hasUpdated = data->getRumble(rumble) ;
        if( hasUpdated )
          m_rumble->setSettings(rumble) ;

        hasUpdated = data->getHasSleepThread( hasSleepThread ) ;
        if( hasUpdated )
          m_hasSleepThread = hasSleepThread ;

        hasUpdated = data->getLED( led ) ;
        if( hasUpdated )
        {
          int a=0 ;
          if( led & 0x1 ) a = CWiimote::LED_1 ;
          if( led & 0x2 ) a |= CWiimote::LED_2 ;
          if( led & 0x4 ) a |= CWiimote::LED_3 ;
          if( led & 0x8 ) a |= CWiimote::LED_4 ;
          
          m_wm->SetLEDs( a ) ;
        }

        delete data ;
        r = true ;
      }
    }

    return r ;
  }

  /**
    * Reduce the number of data to output. Inform if the data is interesting or not.
    * @return TRUE if data is newed ; FALSE if data is useless.
    */
  bool WmDevice::reduceSentData( CWiimote &wm )
  {
    bool hasChanged=false ;
    int timeOut=0 ;

    // If( current state != previous state )
      // Save & send.
    // else
      // Wait 10 ms before to send data.

    m_t2 = m_time.elapsed() ;
    timeOut = m_t2 - m_t1 ;

    // TimeOut ?
    if( timeOut > PLUGIN_WM_TIMEOUT_BEFORE_SENDDATA )
      hasChanged = true ;

    if( !hasChanged )
    { // Action Wiimote ?
      
      if( wm.GetEvent()!=CWiimote::EVENT_NONE 
          && (wm.GetEvent()!=m_previousEvent || wm.Buttons.isJustChanged()) )
        hasChanged = true ;
    }

    if( !hasChanged )
    { // Action Nunchuk ?

      int exType = wm.ExpansionDevice.GetType();
      if( exType == wm.ExpansionDevice.TYPE_NUNCHUK ) 
      {
        if( wm.ExpansionDevice.Nunchuk.Buttons.isJustChanged() )
          hasChanged = true ;
      }
    }

    // Has changed ?
    if( hasChanged )
    { // YES, update data.
    
      m_t1 = m_t2 ;
      m_previousEvent = wm.GetEvent() ;
      return true ;
    }
    else // NO, no update.
      return false ;
  }

}
