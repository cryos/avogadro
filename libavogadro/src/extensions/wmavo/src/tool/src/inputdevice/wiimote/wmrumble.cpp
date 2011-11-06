
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


#include"wmrumble.h"

namespace InputDevice
{
  const int WmRumble::m_gapBetweenMinMaxTremor=WMRUMBLE_MAX_DURATION_TREMOR-WMRUMBLE_MIN_DURATION_TREMOR ;
  const int WmRumble::m_gapBetweenMinMaxPose=WMRUMBLE_MIN_DURATION_POSE-WMRUMBLE_MAX_DURATION_POSE ;
  const int WmRumble::m_gapBetweenStepTremor=m_gapBetweenMinMaxTremor/100 ;
  const int WmRumble::m_gapBetweenStepPose=m_gapBetweenMinMaxPose/100 ;


  RumbleSettings::RumbleSettings()
  {
    m_enable = true ; m_updateEnable = false ;
    m_start = false ;
    m_continu = false ;
    m_loop = false ;
    m_gradual = -1 ;
    m_step = -1 ;
    m_stepMin = 0 ;
    m_stepMax = 0 ;
  }

  RumbleSettings::RumbleSettings( bool enable )
  {
    m_enable = enable ; m_updateEnable = true ;
    m_start = false ;
    m_continu = false ;
    m_loop = false ;
    m_gradual = -1 ;
    m_step = -1 ;
    m_stepMin = 0 ;
    m_stepMax = 0 ;
  }

  RumbleSettings::RumbleSettings( bool continu, bool loop, int gradual )
  {
    m_enable = true ; m_updateEnable = false ;
    m_start = false ;
    m_continu = continu ;
    m_loop = loop ;
    m_gradual = gradual ;
    m_step = -1 ;
    m_stepMin = 0 ;
    m_stepMax = 0 ;
  }

  RumbleSettings::~RumbleSettings()
  {
  }

  void RumbleSettings::setEnable( bool e )
  {
    m_enable = e ;
    m_updateEnable = true ;
  }

  void RumbleSettings::setStart( bool s )
  {
    m_start = s ;
  }

  void RumbleSettings::setContinue( bool c )
  {
    m_continu = c ;
    if( c ) setLoop(false) ;
  }

  void RumbleSettings::setLoop( bool l )
  {
    m_loop = l ;
    if( l ) setContinue(false) ;
  }

  void RumbleSettings::setGradual( int g )
  {
    m_gradual = g ;
  }

  /**
    * Return if the rumble feature is enable or not.
    * @param isEnable_out Get the "enable" value.
    * @return TRUE if the value has changed, else FALSE.
    */
  bool RumbleSettings::getEnable( bool &isEnable_out )
  {
    bool b=m_updateEnable ;
    m_updateEnable = false ;
    isEnable_out = m_enable ;
    return b ;
  }

  /**
    * Return if the rumble has started.
    * @return TRUE if the rumble has started, else FALSE.
    */
  bool RumbleSettings::getStart() const
  {
    return m_start ;
  }

  /**
    * Return if the rumble is in "continue mode" => no sequence, juste rumble non stop.
    * @return TRUE if the rumble is continue, else FALSE.
    */
  bool RumbleSettings::getContinue() const
  {
    return m_continu ;
  }

  /**
    * Return if the rumble is in "loop mode" => sequence of rumble.
    * @return TRUE if the rumble is continue, else FALSE.
    */
  bool RumbleSettings::getLoop() const 
  {
    return m_loop ;
  }

  /**
    * Return the value of the rumble sequence in "loop mode".
    * @return An int value between 1 and 100.
    */
  int RumbleSettings::getGradual() const
  {
    return m_gradual ;
  }

  /**
    * Return the Angstrom value of the distance save.
    * @return the Angstrom value.
    */
  double RumbleSettings::getDistanceCurrent() const
  {
    return m_step ;
  }

  /**
    * Return the min Angstrom value of the distance save.
    * @return The min Angstrom value.
    */
  double RumbleSettings::getDistanceMin() const
  {
    return m_stepMin ;
  }

  /**
    * Return the max Angstrom value of the distance save.
    * @return The max Angstrom value.
    */
  double RumbleSettings::getDistanceMax() const
  {
    return m_stepMax ;
  }

  void RumbleSettings::setDistance( double current )
  {
    m_step = (current < 0.0 ? 0.0 : current ) ;
  }

  void RumbleSettings::setDistance( double min, double max )
  {
    m_stepMin = (min < 0.0 ? 0.0 : min ) ;
    m_stepMax = (max < 0.0 ? 0.0 : max ) ;

    if( m_stepMin > m_stepMax )
    {
      double a=m_stepMin ;
      m_stepMin = m_stepMax ;
      m_stepMax = a ;
    }
  }

  void RumbleSettings::setDistance( double current, double min, double max )
  {
    setDistance( current ) ;
    setDistance( min, max ) ;
  }



  WmRumble::WmRumble( WmDevice *parent,
                      unsigned long durationTremor, unsigned long betweenTremor,
                      unsigned int nbTremorBySequence )
      : m_parent(parent)
  {

    cout << "QAtomicInt::isFetchAndStoreNative():" << QAtomicInt::isFetchAndStoreNative() << endl ;
    cout << "QAtomicInt::isFetchAndStoreWaitFree():" << QAtomicInt::isFetchAndStoreWaitFree() << endl ;

    m_rumbleEnabled.fetchAndStoreRelaxed(0) ; // false
    m_quit.fetchAndStoreRelaxed(1) ; // true
    m_durationTremor.fetchAndStoreRelaxed((int)durationTremor) ;
    m_betweenTremor.fetchAndStoreRelaxed((int)betweenTremor) ;
    m_nbTremorBySequence.fetchAndStoreRelaxed((int)nbTremorBySequence) ;
    m_betweenSequence.fetchAndStoreRelaxed(0) ;
    m_nbSequence.fetchAndStoreRelaxed(1) ;
    m_continue.fetchAndStoreRelaxed(0) ; // false
    m_loop.fetchAndStoreRelaxed(1) ; // true
    m_gradual.fetchAndStoreRelaxed(0) ;
  }

  WmRumble::~WmRumble(){}

  bool WmRumble::getRumbleEnabled()
  {
    int a=m_rumbleEnabled ;
    return ( a==0 ? false : true ) ;
  }

  unsigned long WmRumble::getDurationTremor()
  {
    return (unsigned long)m_durationTremor ;
  }

  unsigned long WmRumble::getBetweenSequence()
  {
    return (unsigned long)m_betweenSequence ;
  }

  unsigned int WmRumble::getNbTremorBySequence()
  {
    return (unsigned int)m_nbTremorBySequence ;
  }

  unsigned long WmRumble::getBetweenTremor()
  {
    return (unsigned long)m_betweenTremor ;
  }

  unsigned int WmRumble::getNbSequence()
  {
    return (unsigned long)m_nbSequence ;
  }

  bool WmRumble::getContinue()
  {
    int a=m_continue ;
    return ( a==0 ? false : true ) ;
  }

  bool WmRumble::getLoop()
  {
    int a=m_loop ;
    return ( a==0 ? false : true ) ;
  }

  int WmRumble::getGradual()
  {
    return m_gradual ;
  }


  bool WmRumble::isQuit()
  {
    return m_quit==1 ;
  }


  bool WmRumble::isRunning()
  {
    return m_quit==0 ;
  }


  /**
    * Enable or Disable the rumble feature.
    * @param state Rumble : On/Off
    */
  void WmRumble::setRumbleEnabled( bool activeRumble )
  {
    if( (int)activeRumble != m_rumbleEnabled )
    {
      if( activeRumble )
        m_rumbleEnabled.fetchAndStoreRelaxed(1) ;
      else
        m_rumbleEnabled.fetchAndStoreRelaxed(0) ;

      if( !activeRumble )
      {
        if( !isQuit() || getContinue() )
          quit() ;
      }
    }
  }

  void WmRumble::setDurationTremor( unsigned long durationTremor )
  {
    int d=durationTremor ;
    if( d != m_durationTremor )
    {
      if( d>=WMRUMBLE_MIN_TIME && d<=WMRUMBLE_MAX_TIME )
        m_durationTremor.fetchAndStoreRelaxed(d) ;
    }
  }

  void WmRumble::setBetweenSequence( unsigned long betweenSequence )
  {
    int d=betweenSequence ;
    if( d != m_betweenSequence )
    {
      if( d>=0 && d<=WMRUMBLE_MAX_TIME )
        m_betweenSequence.fetchAndStoreRelaxed(d) ;
    }
  }

  void WmRumble::setNbTremorBySequence( unsigned int nbTremorBySequence )
  {
    int d=nbTremorBySequence ;
    if( d != m_nbTremorBySequence )
    {
      if( d>=0 && d<=WMRUMBLE_MAX_TIME ) // ~
        m_nbTremorBySequence.fetchAndStoreRelaxed(d) ;
    }
  }

  void WmRumble::setBetweenTremor( unsigned long betweenTremor )
  {
    int d=betweenTremor ;
    if( d != m_betweenTremor )
    {
      if( d>=0 && d<=WMRUMBLE_MAX_TIME )
        m_betweenTremor.fetchAndStoreRelaxed(d) ;
    }
  }

  void WmRumble::setNbSequence( unsigned int nbSequence )
  {
    int d=nbSequence ;
    if( d != m_nbSequence )
    {
      if( d>=0 && d<=WMRUMBLE_MAX_TIME ) // ~
        m_nbSequence.fetchAndStoreRelaxed(d) ;
    }
  }

  void WmRumble::setContinue( bool continu )
  {
    if( (int)continu != m_continue )
    {
      if( continu )
        m_continue.fetchAndStoreRelaxed(1) ;
      else
        m_continue.fetchAndStoreRelaxed(0) ;
    }
  }

  void WmRumble::setLoop( bool loop )
  {
    if( (int)loop != m_loop )
    {
      if( loop )
        m_loop.fetchAndStoreRelaxed(1) ;
      else
        m_loop.fetchAndStoreRelaxed(0) ;
    }
  }

  /**
    * Set the rumble of the Wiimote according to a sequence adjusted by a value.
    * @param gradual The level of the rumble sequence, between 0 (nothing) to 100 (continuous).
    */
  void WmRumble::setGradual( int gradual )
  {
    if( gradual != m_gradual )
    {
      if( gradual < 0 )
        m_gradual.fetchAndStoreRelaxed(-1) ;

      if( gradual>0 && gradual<100 )
      {
        int g=m_gradual ;

        if( g != gradual )
        {
          if( (g<=0 || g>=100) && !isRunning() )
          {
            //cout << "wmrumble: setGradual(): go()" << endl ;
            this->start() ;
          }

          m_gradual.fetchAndStoreRelaxed(gradual) ;
          m_continue.fetchAndStoreRelaxed(0) ;

          m_nbTremorBySequence.fetchAndStoreRelaxed(1) ;
          m_betweenTremor.fetchAndStoreRelaxed(0) ;
          m_nbSequence.fetchAndStoreRelaxed(1) ;

          // Method 1
          //m_durationTremor = WMRUMBLE_MIN_DURATION_TREMOR + m_gapBetweenStepTremor * gradual ;
          //m_betweenTremor = WMRUMBLE_MAX_DURATION_POSE + m_gapBetweenStepPose * gradual ;

          // Method 2
          if( gradual>0 && gradual<=30 )
            m_durationTremor.fetchAndStoreRelaxed(WMRUMBLE_MIN_TIME) ; // 150 // 80

          if( gradual>30 && gradual<=60 )
            m_durationTremor.fetchAndStoreRelaxed(WMRUMBLE_MIN_TIME) ; // 200 // 120

          if( gradual>60 && gradual<100 )
            m_durationTremor.fetchAndStoreRelaxed(WMRUMBLE_MIN_TIME) ; // 250 // 150

          m_betweenTremor.fetchAndStoreRelaxed( WMRUMBLE_MIN_TIME + (WMRUMBLE_MAX_DURATION_POSE + m_gapBetweenStepPose * int(float(gradual)/1.3))) ; // /1.3 : to reduce the rumble : TODO : code better that ...
        }
      }
      else
      { // Stop the thread.
        
        m_quit.fetchAndStoreRelaxed(1) ;

        m_gradual.fetchAndStoreRelaxed(0) ;
        m_continue.fetchAndStoreRelaxed(0) ;
        m_nbTremorBySequence.fetchAndStoreRelaxed(1) ;
        m_nbSequence.fetchAndStoreRelaxed(1) ;
        m_betweenTremor.fetchAndStoreRelaxed(0) ;

        m_durationTremor.fetchAndStoreRelaxed(0) ;
        m_betweenTremor.fetchAndStoreRelaxed(0) ;
      }
    }
  }


  /**
    * Set the rumble of the Wiimote according to the parameters.
    * @param go TRUE to begin rumble ; else FALSE
    * @param continu TRUE to have a continuous vibration ; else FALSE to realize a sequence of rumble.
    */
  void WmRumble::setStartInContinue( bool go )
  {
    bool needUpdate=false ;

    if( go!=isRunning() || !getContinue() )
      needUpdate = true ;

    if( needUpdate )
    {
      if( go )
      { // Remember: continu => no thread active, just rumble to ON.

        if( isRunning() && !getContinue() )
          quit() ; // To change mode.

        if( !getContinue() )
          setContinue( true ) ;

        if( !isRunning() )
          start() ;
      }
      else
      {
        //cout << "wmavo: Rumble FIN" << endl ;
        if( isRunning() ) // "getContinue()" redundant
          quit() ;
      }
    }
  }

  /**
    * Set the rumble of the Wiimote according to the parameters.
    * @param go TRUE to begin rumble ; else FALSE
    * @param gradual [0;100] => predetermined sequence from nothing to continuous rumble
    */
  void WmRumble::setStartInLoop( bool go, int gradual )
  {
    bool needUpdate=false ;

    if( go!=isRunning() || !getLoop() )
      needUpdate = true ;

    if( gradual != getGradual() )
      setGradual( gradual ) ;

    if( needUpdate )
    {
      if( go )
      {
        if( isRunning() && !getLoop() )
          quit() ; // To change mode.

        if( !getLoop() )
          setLoop( true ) ;

        if( !isRunning() )
          start() ;
      }
      else
      {
        //cout << "wmavo: Rumble FIN" << endl ;
        if( isRunning() ) // "getContinue()" redundant
          quit() ;
      }
    }
  }


  /**
    * Set the rumble of the Wiimote according to the parameters.
    * @param go TRUE to begin rumble ; else FALSE
    * @param gradual [0;100] => predetermined sequence from nothing to continuous rumble
    */
  void WmRumble::setStartOnce( bool go, int gradual )
  {
    bool needUpdate=true ;
      
    if( gradual != getGradual() )
      setGradual( gradual ) ;

    if( needUpdate )
    {
      if( go )
      {
         // To change mode.
        if( isRunning() ) quit() ;
        if( getLoop() ) setLoop( false ) ;
        if( getContinue() ) setContinue( false ) ;
        start() ;
      }
      else
      {
        //cout << "wmavo: Rumble FIN" << endl ;
        if( isRunning() ) quit() ;
      }
    }
  }

  void WmRumble::setSettings( RumbleSettings& settings ) 
  {
    bool enable=false ;
    bool hasEnable=settings.getEnable(enable) ;
    bool go=settings.getStart() ;
    bool continu=settings.getContinue() ;
    bool loop=settings.getLoop() ;
    int grad=settings.getGradual() ;
    double dist=settings.getDistanceCurrent() ;
    double distMax=settings.getDistanceMax() ;
    double distMin=settings.getDistanceMin() ;

    if( hasEnable==true && enable!=getRumbleEnabled() )
      setRumbleEnabled( enable ) ;

    if( enable )
    {
      if( continu ) 
        setStartInContinue( go ) ;
      else if( loop && dist>=0 )
        adjustRumble1( go, dist, distMin, distMax ) ;
      else if( loop ) 
        setStartInLoop( go, grad ) ;
      else if( !loop && !continu )
        setStartOnce( go, grad ) ;
      else
      {
        if( !go && isRunning() )
          quit() ;
      }
    }
  }

  /**
    * Calcul, adjust and inform the type of vibration to the wrapper.
    * @param active "Activate" the rumble
    * @param posAtom The position of the reference atom for the distance with others atoms
    * @param atomNotUse The atom exception which does not take in the calcul.
    */
  void WmRumble::adjustRumble1( bool active, double distCurrent, double distMin, double distMax )
  {
    if( !active || (distMin==0.0 && distMax==0.0) )
    {
      //mytoolbox::dbgMsg( "wmext: adjustRumble 3: setWmRumble(false)" ) ;
      quit() ;
    }
    else
    {
      //cout << "distMax:" << distMax << " distMin:" << distMin <<  endl ;
      //cout << "distCurrent:" << distCurrent << endl ;

      if( distCurrent>=0 && distCurrent<distMin )
      {
        //mytoolbox::dbgMsg( "wmext: adjustRumble: setWmRumble(true, true)" ) ;
        //setWmRumble(true, true) ;
        quit() ;
      }
      else if( distCurrent>=distMin && distCurrent<=distMax )
      {
        int g=100-int( ( 
                         (distCurrent-distMin>0? 
                            distCurrent-distMin : distCurrent)
                          *100.0 ) / (distMax-distMin) ) ;
        // distMax -> 100
        // distMin -> 0
        // x   -> y

        if( g>0 && g<100 )
        {
          //mytoolbox::dbgMsg( "wmext: adjustRumble: setWmRumble(true, false, true, g )" ) ;
          setStartInLoop( true, g ) ;
        }
        else // >=distMax+1
        {
          //mytoolbox::dbgMsg( "wmext: adjustRumble 1: setWmRumble(false)" ) ;
          quit() ;
        }
      }

      else
      {
        //mytoolbox::dbgMsg( "wmext: adjustRumble 2: setWmRumble(false)" ) ;
        quit() ;
      }
    }
  }



  void WmRumble::run()
  {
    //if( m_quit == false ) Do nothing ... The thread is running.

    unsigned int i=0, j=0 ;
    unsigned long k=0, l=0 ;

    //cout << "wmrumble: run(): debut, m_quit=false" << endl ;
    m_quit.fetchAndStoreRelaxed(0) ;


    // rumbleEnabled<-True par wmConnect, wmDisconnect
    if( m_parent!=NULL && m_rumbleEnabled && !m_quit)
    {
      if( m_continue )
      {
        // Pour éviter les problèmes de concurrence avec wmPoll()
        m_parent->m_mutex.lock() ;
        m_parent->m_wm->SetRumbleMode( CWiimote::ON ) ;
        m_parent->m_mutex.unlock() ;
        //cout << "wmrumble: run(): continue, rumble(ON)" << endl ;
      }
      else
      {
        do
        {
          do
          {
            do
            {
              // Activate Rumble.

              m_parent->m_mutex.lock() ;
              m_parent->m_wm->SetRumbleMode( CWiimote::ON ) ;
              m_parent->m_mutex.unlock() ;
              //cout << "wmrumble: run(): graduel, rumble(ON)" << endl ;

              // Operating time of rumble.

              k = 0 ;
              l = m_durationTremor ;

              while( m_rumbleEnabled && !m_quit && k<l )
              {
                k += WMRUMBLE_TIME_SLEEP ;
                this->msleep(WMRUMBLE_TIME_SLEEP) ; 
                // Be careful => the rumble time is a multiple of WMRUMBLE_TIME_SLEEP!
                l = m_durationTremor ;
              }

              // Desactivate rumble.
              m_parent->m_mutex.lock() ;
              m_parent->m_wm->SetRumbleMode( CWiimote::OFF ) ;
              m_parent->m_mutex.unlock() ;
              //cout << "wmrumble: run(): graduel, rumble(OFF)" << endl ;

              // Sleep time of rumble between tremors.
              k = 0 ;
              l = m_betweenTremor ;

              while( m_rumbleEnabled && !m_quit && m_loop && k<l )
              {
                k += WMRUMBLE_TIME_SLEEP ;
                this->msleep(WMRUMBLE_TIME_SLEEP) ;
                // Be careful => the rumble time is a multiple of WMRUMBLE_TIME_SLEEP!
                l = m_betweenTremor ;
              }

              l = m_nbTremorBySequence ;

            }while( m_rumbleEnabled && !m_quit && m_loop && (++j)<(unsigned int)l ) ;


            // Sleep time of rumble between sequences.
            j = 0 ;
            k = 0 ;
            l = m_betweenSequence ;

            while( m_rumbleEnabled && !m_quit && m_loop && k<l )
            {
              k += WMRUMBLE_TIME_SLEEP ;
              this->msleep(WMRUMBLE_TIME_SLEEP) ;
              l = m_betweenSequence ;
            }

            //cout << "m_durationTremor:" << m_durationTremor << " m_betweenTremor:" << m_betweenTremor << " m_nbTremorBySequence:" << m_nbTremorBySequence << endl ;
            //cout << "m_betweenSequence:" << m_betweenSequence << " m_nbSequence:" << m_nbSequence << " m_loop:" << m_loop << endl ;

            l = m_nbSequence ;

          }while( m_rumbleEnabled && !m_quit && m_loop && (++i)<(unsigned int)l ) ;

          i = 0 ;

        }while( m_rumbleEnabled && !m_quit && m_loop ) ;

        m_quit.fetchAndStoreRelaxed(1) ;
      }
    }
  }

  void WmRumble::quit()
  {
    if( m_continue && isRunning() )
    {
      m_parent->m_mutex.lock() ;
      m_parent->m_wm->SetRumbleMode( CWiimote::OFF ) ;
      m_parent->m_mutex.unlock() ;
      //cout << "wmrumble: quit(): continue, rumble(OFF)" << endl ;
    }

    m_quit.fetchAndStoreRelaxed(1) ;
    //cout << "wmrumble: quit():m_quit=true" << endl ;

    setContinue( false ) ;
    setLoop( false ) ;

    wait() ;
    QThread::quit() ;
  }
}
