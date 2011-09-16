
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

#include "chemicalwrapper.h"

namespace WrapperInputToDomain
{

  ChemicalWrapData_from::ChemicalWrapData_from()
  {
  }

  ChemicalWrapData_from::~ChemicalWrapData_from()
  {
  }

  ChemicalWrapData_to::ChemicalWrapData_to()
    : m_operatingMode(0), m_updateOpMode(false),
      m_menuMode(false), m_updateMenuMode(false),
      m_irSensitive(0), m_updateIRSensitive(false), 
      m_hasSleepThread(PLUGIN_WM_SLEEPTHREAD_ONOFF), m_updateSleepThread(false)
  {
  }

  ChemicalWrapData_to::~ChemicalWrapData_to()
  {
  }

  ChemicalWrap::ChemicalWrap( InputDevice::Device *dev ) 
    : m_dev(dev), m_wmToChem(NULL),
      m_cirBufferFrom(NULL), m_cirBufferTo(NULL),
      m_actionsGlobalPrevious(0), m_actionsWrapperPrevious(0),
      m_forceUpdateWmTool(false), m_forceUpdateOrNot(false),
      m_isRunning(false), 
      m_hasSleepThread(PLUGIN_WM_SLEEPTHREAD_ONOFF), 
      m_nbActionRealized(0),
      m_t1(0), m_t2(0), m_t1Update(0), m_t2Update(0), m_t1Update2(0), m_t2Update2(0),
      m_nbWmToolNotFinished(0), m_nbDataInWmBuffer(0), m_nbActionsApplied(0),
      m_bufferFromIsFull(false), m_bufferToIsFull(false)
  {
    m_cirBufferFrom = new WIWO_sem<ChemicalWrapData_from*>( CIRBUFFER_DEFAULT_SIZE ) ;
    m_cirBufferTo = new WIWO_sem<ChemicalWrapData_to*>( CIRBUFFER_DEFAULT_SIZE ) ;
    m_wmToChem = new WmToChem(WMAVO_OPERATINGMODE3) ;
    m_actionsAreApplied.fetchAndStoreRelaxed(0) ;
    m_threadFinished.fetchAndStoreRelaxed(1) ;
      
    m_time.start() ;
    m_nbUpdate = new WIWO<unsigned int>(20) ;
    m_nbUpdate2 = new WIWO<unsigned int>(20) ;
  }

  ChemicalWrap::~ChemicalWrap()
  {
    stopPoll() ;

    if( m_cirBufferFrom != NULL )
    {
      delete( m_cirBufferFrom ) ;
      m_cirBufferFrom = NULL ;
    }

    if( m_cirBufferTo != NULL )
    {
      delete( m_cirBufferTo ) ;
      m_cirBufferTo = NULL ;
    }

    if( m_wmToChem != NULL )
    {
      delete( m_wmToChem ) ;
      m_wmToChem = NULL ;
    }
  }

  ChemicalWrapData_from* ChemicalWrap::getWrapperDataFrom()
  { 
    if( m_isRunning && m_cirBufferFrom!=NULL )
    {
      ChemicalWrapData_from *data=NULL ;
      m_cirBufferFrom->popFront( data ) ;
      return data ;
    }
    else
      return NULL ;
  }

  void ChemicalWrap::setWrapperDataTo( const ChemicalWrapData_to& dataTo )
  { 
    if( m_isRunning && m_cirBufferTo!=NULL )
    {
      ChemicalWrapData_to *data=new ChemicalWrapData_to() ;
      *data = dataTo ;
      m_cirBufferTo->pushBack(data) ;
    }
  }

  bool ChemicalWrap::connectAndStart()
  {
    bool isConnect=this->connect( &m_wrapperThread, SIGNAL(started()), SLOT(runPoll()) ) ;
    if( !isConnect )
        mytoolbox::dbgMsg( "Problem connection signal : m_wrapperThread.started() -> ChemicalWrap.runPoll() !!" ) ;

    this->moveToThread( &m_wrapperThread ) ;

    m_threadFinished.fetchAndStoreRelaxed(0) ;
    m_wrapperThread.start() ;

    return true ;
  }

  void ChemicalWrap::setOnActionsApplied()
  {
    #if __WMDEBUG_CHEMWRAPPER
    //mytoolbox::dbgMsg( "Slot setOnActionsApplied == 1" ) ;
    #endif
    // Under Linux, sometime, this slot is not called ... As if the processEvent() is not called.
    m_actionsAreApplied.fetchAndStoreRelaxed(1) ;
    m_nbActionsApplied ++ ;
  }
  
  
  void ChemicalWrap::runPoll()
  {
    m_wrapperThread.setPriority( QThread::LowPriority ) ;
    
    if( m_dev != NULL )
    {
      bool needUpdateDataTo=false ;
      bool needUpdateDataFrom=false ;
      bool displayIsFinished=false ;
      ChemicalWrapData_from *chemData=NULL ;

      m_isRunning = true ;

      while( m_isRunning )
      {
        // Count nb action by second (used with breakpoint).
        m_t2Update = m_time.elapsed() ;
        if( (m_t2Update-m_t1Update) > 1000 )
        {
          #if __WMDEBUG_CHEMWRAPPER
          QString msg= tr("ChemicalWrap::runPoll() : tread has ") 
                       + QString::number( (*m_nbUpdate)[0] )
                       + tr(" actions/second, Nb data in WmBuffer :")
                       + QString::number( m_nbDataInWmBuffer ) ;
          mytoolbox::dbgMsg( msg ) ;
          
          msg = tr( "ChemicalWrap::runPoll() : Actions not applied: " ) + QString::number( m_nbWmToolNotFinished ) 
                + tr( ", vs applied:") + QString::number( m_nbActionsApplied ) ;
          if( m_bufferFromIsFull )
            msg += ", buffer From Full !" ;
          if( m_bufferToIsFull )
            msg += ", buffer To Full !" ;
            
          mytoolbox::dbgMsg( msg ) ;
          #endif
          
          m_t1Update = m_t2Update ;
          m_nbUpdate->pushFront(0) ;
          (*m_nbUpdate)[0]++ ;
          m_nbWmToolNotFinished = 0 ;
          m_nbDataInWmBuffer = 0 ;
          m_nbActionsApplied = 0 ;
          m_bufferFromIsFull = false ;
          m_bufferToIsFull = false ;
          
          if( m_forceUpdateWmTool )
            emit wmWorksBad() ;
          else
            emit wmWorksGood() ;
        }
        else
        {
          (*m_nbUpdate)[0]++ ;
        }

        // Something need update.
        m_forceUpdateWmTool = false ;  
        displayIsFinished = (m_actionsAreApplied==0?false:true) ;

        if( m_dev->hasDeviceDataAvailable() )
        {
          #if __WMDEBUG_CHEMWRAPPER
          m_nbDataInWmBuffer ++ ;
          #endif
          
          chemData = updateDataFrom() ;
        }
        else
          chemData = NULL ;

        if( !m_cirBufferTo->isEmpty() )
          needUpdateDataTo = updateDataTo() ;
        else
          needUpdateDataTo = false ;

        if( !m_cirBufferFrom->isEmpty() )
          needUpdateDataFrom = true ;
        else
          needUpdateDataFrom = false ;

        #if __WMDEBUG_CHEMWRAPPER
        QString msg ;      
        if( m_cirBufferFrom->isFull() ) m_bufferFromIsFull = true ;
        if( m_cirBufferTo->isFull() ) m_bufferToIsFull = true ;
        #endif

        if( chemData!=NULL || needUpdateDataFrom || needUpdateDataTo )
        { // Working ...

          if( chemData != NULL )
          {              
            //#if defined WIN32 || defined _WIN32
            if( !reduceSentData(displayIsFinished, chemData) )
            {
              delete chemData ;
            }
            else
            //#endif 
            {
              // 1st stategy : If no place, no push data.
              if( !m_cirBufferFrom->isFull() )
              {
                m_cirBufferFrom->pushBack( chemData ) ;
                
                // Count nb action by second (used with breakpoint).
                m_t2Update2 = m_time.elapsed() ;
                if( (m_t2Update2-m_t1Update2) > 1000 )
                {
                  #if __WMDEBUG_CHEMWRAPPER
                  QString msg= tr("ChemicalWrap::runPoll() : outgoing data ")
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
              }
              else
              {
                delete chemData ;
                
                #if __WMDEBUG_CHEMWRAPPER
                QString msg= tr("ChemicalWrap::runPoll() : m_cirBufferFrom->isFull !") ;
                mytoolbox::dbgMsg( msg ) ;
                #endif
              }
            }
          }

          if( (needUpdateDataFrom && displayIsFinished) || m_forceUpdateWmTool )
          {
            m_actionsAreApplied.fetchAndStoreRelaxed(0) ;
            emit newActions() ;
          }

          //if( needUpdateDataTo )
            // Nothing to do.
            
        }
        
        // Sleeping ...
        if( m_hasSleepThread )
        {
          m_nbActionRealized ++ ;

          if( m_nbActionRealized > PLUGIN_WM_SLEEPTHREAD_NBTIME_BEFORE_SLEEP )
          {
            m_nbActionRealized = 0 ;
            m_wrapperThread.msleep(PLUGIN_WM_SLEEPTHREAD_TIME) ;
          }
          else
          {
            m_wrapperThread.yieldCurrentThread() ;
          }
        }
        else
          m_wrapperThread.yieldCurrentThread() ;
          // With m_wrapperThread.setPriority( QThread::LowPriority ) ;
          
      
        // Call event loop (to get "incoming signals").
        QCoreApplication::processEvents() ; 
        // By default : QEventLoop::AllEvent "& !QEventLoop::WaitForMoreEvents"
        // http://doc.qt.nokia.com/latest/qeventloop.html#ProcessEventsFlag-enum
      }

      // Must be disconnect, else there is a crash when this object is deleted.
      // Once QCoreApplication::processEvents() called, this object is "attached" at QCoreApplication.
      // So with this static method, this object must force to disconnect every signal after use,
      // mainly with QCoreApplication object.
      this->disconnect() ;

      m_threadFinished.fetchAndStoreRelaxed(1) ;
      
      #if __WMDEBUG_CHEMWRAPPER
      mytoolbox::dbgMsg( "ChemicalWrap::runPoll() m_threadFinished=1" ) ;
      #endif
    }
    else
    {
      m_threadFinished.fetchAndStoreRelaxed(1) ;
      m_isRunning = false ;
    }
  }
  

  void ChemicalWrap::stopPoll()
  {
    if( m_isRunning )
    {
      // Stop the working thread.
      m_isRunning = false ;
      while( m_threadFinished == 0 ) ;
      
      #if __WMDEBUG_CHEMWRAPPER
      mytoolbox::dbgMsg( "ChemicalWrap::stopPoll() passed" ) ;
      #endif
      
      ChemicalWrapData_from *dataFrom=NULL ;
      ChemicalWrapData_to *dataTo=NULL ;

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
    m_wrapperThread.quit() ;
  }

  ChemicalWrapData_from* ChemicalWrap::updateDataFrom()
  {
    ChemicalWrapData_from *r=NULL ;
    CWiimoteData *wmData=NULL ;
    bool isConnect=false ;

    if( m_dev != NULL )
    {
      InputDevice::WmDevice* dev=dynamic_cast<InputDevice::WmDevice*>(m_dev) ;
      InputDevice::WmDeviceData_from *wmDataFrom=NULL ;

      if( dev != NULL )
      {
        wmDataFrom = dev->getDeviceDataFrom() ;

        if( wmDataFrom != NULL )
          wmData = wmDataFrom->getDeviceData() ;
        else
          mytoolbox::dbgMsg("Error data : in ChemicalWrapper.updateDataFrom wmDataFrom == NULL ...\n") ;
          
        if( wmData!=NULL && wmData->isConnected() )
          isConnect = true ;
        /*
        else
        {
          float x, y, z ;
          wmData->Accelerometer.GetOrientation( x, y, z ) ;              
          cout << "wmData2->Accelerometer.GetOrientation: x:" << x << ", y:" << y << ", z:" << z << " + " << wmData->isConnected() << endl ;
        }
        */

        if( wmData==NULL
            || !isConnect
            || wmData->GetBatteryLevel()<0 
            || wmData->IR.GetNumDots()<0 || wmData->IR.GetNumDots()>4 )
        {
          mytoolbox::dbgMsg("Error data : in ChemicalWrapper.updateDataFrom *wm!=NULL && no allocated ...") ;
          
          if( wmData == NULL )
            mytoolbox::dbgMsg("  wmData == NULL") ;
          else
          {
            if( wmData->isConnected() ) mytoolbox::dbgMsg("  wmData->isConnected()") ;
            else mytoolbox::dbgMsg("  wmData->is NOT Connected()") ;            
            if( wmData->GetBatteryLevel() < 0 ) mytoolbox::dbgMsg("  No Battery") ;
            if( wmData->IR.GetNumDots()<0 ) mytoolbox::dbgMsg("  Number of LED <0") ;
            if( wmData->IR.GetNumDots()>4 ) mytoolbox::dbgMsg("  Number of LED >4") ;
          }
        }
      }
      else
          mytoolbox::dbgMsg("Error data : in ChemicalWrapper.updateDataFrom : dynamic_cast<InputDevice::WmDevice*>(m_dev) ...\n") ;
      

      if( wmData!=NULL && isConnect /*wmData->isConnected()*/
          && m_wmToChem->convert(wmData) )
      {
        // Initiate data.
        ChemicalWrapData_from *chemData=new ChemicalWrapData_from() ;
        WrapperData_from::wrapperActions_t waData ;
        WrapperData_from::positionCamera_t camData ;
        WrapperData_from::positionPointed_t posData ;

        waData.actionsGlobal = 0 ;
        waData.actionsWrapper = m_wmToChem->getActions() ;
        waData.wrapperType = WRAPPER_ID_CHEMICAL ;
        chemData->setWrapperAction( waData ) ;

        camData.angleRotateDegree[0] = m_wmToChem->getAngleCamRotateXDeg() ;
        camData.angleRotateDegree[1] = m_wmToChem->getAngleCamRotateYDeg() ;
        camData.angleRotateDegree[2] = 0 ;
        camData.distanceTranslate[0] = m_wmToChem->getDistCamTranslateX() ;
        camData.distanceTranslate[1] = m_wmToChem->getDistCamTranslateY() ;
        camData.distanceTranslate[2] = m_wmToChem->getDistCamZoom() ;
        chemData->setPositionCamera( camData ) ;

        posData.posCursor = m_wmToChem->getPosCursor() ;
        posData.pos3dCur = m_wmToChem->getPos3dCurrent() ;
        posData.pos3dLast = m_wmToChem->getPos3dLast() ;
        chemData->setPositionPointed( posData ) ;

        /*
        chemData->nbDotsDetected = m_wmD->IR.GetNumDots() ;
        chemData->nbSourcesDetected = m_wmavo->getWiimote()->IR.GetNumSources() ;
        chemData->distBetweenSources = (int)m_wmavo->getWiimote()->IR.GetDistance() ;
        */

        r = chemData ;
      }

      if( wmData != NULL )
        delete wmData ;
    }

    return r ;
  }

  bool ChemicalWrap::updateDataTo()
  {
    bool r=false ; 

    // Let a minimum before a set
    //this->msleep( WMAVOTH_SLEEPBEFORE_NEXTROUND ) ;
    
    //if( isConnected = wmavo->wmIsConnected() )
    {
      ChemicalWrapData_to *data=NULL ; 
      //if( !m_cirBufferTo->isEmpty() )
      {
        m_cirBufferTo->popFront(data) ;

        if( data != NULL )
        {
          int opMode ;
          bool menuMode, hasSleepThread ;
          int irSensitive ;
          bool hasUpdated=false ;

          hasUpdated = data->getOperatingMode(opMode) ;
          if( hasUpdated )
          {
            m_wmToChem->setOperatingMode(opMode) ;
            
            #if __WMDEBUG_CHEMWRAPPER
            QString msg= tr("ChemicalWrap::updateDataTo() : m_wmToChem->setOperatingMode(opMode)") ;
            mytoolbox::dbgMsg( msg ) ;
            #endif
          }

          hasUpdated = data->getMenuMode(menuMode) ;
          if( hasUpdated )
          {
            m_wmToChem->setMenuMode(menuMode) ;
            
            #if __WMDEBUG_CHEMWRAPPER
            QString msg= tr("ChemicalWrap::updateDataTo() : m_wmToChem->setMenuMode(menuMode)") ;
            mytoolbox::dbgMsg( msg ) ;
            #endif
          }

          hasUpdated = data->getIRSensitive(irSensitive) ;
          if( hasUpdated )
          {
            m_wmToChem->setIrSensitive(irSensitive) ;
            
            #if __WMDEBUG_CHEMWRAPPER
            QString msg= tr("ChemicalWrap::updateDataTo() : m_wmToChem->setIrSensitive(irSensitive)") ;
            mytoolbox::dbgMsg( msg ) ;
            #endif
          }

          hasUpdated = data->getHasSleepThread(hasSleepThread) ;
          if( hasUpdated )
          {
            m_hasSleepThread = hasSleepThread ;
            
            #if __WMDEBUG_CHEMWRAPPER
            QString msg= tr("ChemicalWrap::updateDataTo() : m_hasSleepThread = hasSleepThread") ;
            mytoolbox::dbgMsg( msg ) ;
            #endif
          }

          delete data ;
          
          r = true ;
        }
      }
    }

    return r ;
  }

  /**
    * Reduce the number of data to output. Inform if the data is interesting or not.
    * @return TRUE if data is newed ; FALSE if data is useless.
    */
  bool ChemicalWrap::reduceSentData( bool displayIsFinished, WrapperData_from *chemData )
  {
    bool dataIsInteresting=false ;

    if( chemData != NULL )
    {
      if( displayIsFinished )
      { // If display not working, add some works if available.

        dataIsInteresting = true ;
      }
      else
      { // If display working, do not add works if possible.
      
        m_nbWmToolNotFinished ++ ;

        WrapperData_from::wrapperActions_t wa=chemData->getWrapperAction() ;

        if( m_actionsGlobalPrevious!=wa.actionsGlobal 
            || m_actionsWrapperPrevious!=wa.actionsWrapper )
        {
          dataIsInteresting = true ;
          m_actionsGlobalPrevious = wa.actionsGlobal ;
          m_actionsWrapperPrevious = wa.actionsWrapper ;
          m_forceUpdateWmTool = true ;
        }
        else
        {
          // Study if we must force to send data to WmTool.
          if( m_nbWmToolNotFinished > PLUGIN_WM_NBLOSTDATA_BEFORE_COUNTERSTRIKE 
              && m_nbWmToolNotFinished > m_nbActionsApplied
              )
          {
            
            if( m_forceUpdateOrNot )
            {
              m_forceUpdateWmTool = true ;
              
              #if __WMDEBUG_CHEMWRAPPER
              mytoolbox::dbgMsg( "Force update" ) ;
              #endif
            }
            
            m_forceUpdateOrNot = !m_forceUpdateOrNot ;
          }
        }
      }
    }

    return dataIsInteresting ;
  }

}
