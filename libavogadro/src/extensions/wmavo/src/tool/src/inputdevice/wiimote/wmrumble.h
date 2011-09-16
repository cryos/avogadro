
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


#pragma once
#ifndef __WMAVO_RUMBLE_H__
#define __WMAVO_RUMBLE_H__

#include "warning_disable_begin.h"
#include "wmavo_const.h"
#include "wmdevice.h"

#if defined WIN32 || defined _WIN32
  #include "wiiusecpp.h"
#else
  #include <wiiusecpp.h>
#endif


#include <QThread>
#include <qmutex.h>
#include <QAtomicInt>

#include <iostream>
#include <Eigen/Core>
#include "warning_disable_end.h"

// N.B. :
// Generating src/moc_wmavo.cxx !
// Error: Meta object features not supported for nested classes
// So, the WmRumble class can not neste in WmAvo class, and it become friend with the WmAvo class.

// Pas de concurrence gérée pour les vars. booléennes (m_rumbleEnabled, m_quit, m_loop ...).

namespace InputDevice
{ 
  /**
    * @class SettingsRumble
    * @brief The class manages the settings of the Wiimote rumble.
    */
  class RumbleSettings
  {
  public :
    RumbleSettings( bool enable=true, bool start=false, bool continu=false, bool loop=false, int gradual=-1 ) ;
    ~RumbleSettings() ;
    void setEnable( bool e ) ; void setStart( bool s ) ;
    void setContinue( bool c ) ; void setLoop( bool l ) ;
    void setGradual( int g ) ;
    void setDistance( double current ) ;
    void setDistance( double min, double max ) ;
    bool getEnable() const ; bool getStart() const ;
    bool getContinue() const ; bool getLoop() const ;
    int getGradual() const ;
    double getDistanceCurrent() const ;
    double getDistanceMin() const ;
    double getDistanceMax() const ;
    
  private:
    bool m_enable ;
    bool m_start, m_continu, m_loop ;
    int m_gradual ;
    double m_step, m_stepMin, m_stepMax ;
  } ;

  /**
    * @class WmRumble
    * @brief The class manages the rumble feature of the Wiimote.
    */
  class WmRumble : public QThread
  {
    Q_OBJECT

    public :

      static const int m_gapBetweenMinMaxTremor, m_gapBetweenMinMaxPose ;
      static const int m_gapBetweenStepTremor, m_gapBetweenStepPose ;

    public :

      WmRumble( WmDevice *parent,
                unsigned long durationTremor=1000, unsigned long betweenTremor=1,
                unsigned int nbTremorBySequence=0 ) ;
      ~WmRumble() ;

      /**
        * Be careful, all method uses int value. (unsigned long) and (unsigned int) are here for ease the use with timer method for example.
        */
      bool getRumbleEnabled() ;
      unsigned long getDurationTremor() ;
      unsigned long getBetweenSequence() ;
      unsigned int getNbTremorBySequence() ;
      unsigned long getBetweenTremor() ;
      unsigned int getNbSequence() ;
      bool getContinue() ;
      bool getLoop() ;
      int getGradual() ;

      bool isRunning() ;
      bool isQuit() ;

      void setRumbleEnabled( bool activeRumble ) ;
      void setDurationTremor( unsigned long durationTremor ) ;
      void setBetweenSequence( unsigned long betweenSequence ) ;
      void setNbTremorBySequence( unsigned int nbTremorBySequence ) ;
      void setBetweenTremor( unsigned long betweenTremor ) ;
      void setNbSequence( unsigned int nbSequence ) ;
      void setContinue( bool continu ) ;
      void setLoop( bool loop ) ;
      void setGradual( int gradual ) ;
      void setSettings( const RumbleSettings& settings ) ;
      void setStartInContinue( bool start ) ;
      void setStartInLoop( bool start, int gradual=-1 ) ;
      void setStartOnce( bool start, int gradual=-1 ) ;

      void adjustRumble1( bool active, double distCurrent, double distMin, double distMax ) ;
          ///< Adjust rumble according to the distance

      void run() ;
      void quit() ;

    private :

      WmDevice *m_parent ; // (shortcut)
      QMutex m_mutexSet ;

      // Avoid concurent problem with wmPoll().
      QAtomicInt m_rumbleEnabled, m_quit ;
      QAtomicInt m_durationTremor, m_betweenTremor ; // ms
      QAtomicInt m_nbTremorBySequence ;
      QAtomicInt m_betweenSequence ; // ms
      QAtomicInt m_nbSequence ;
      QAtomicInt m_continue, m_loop ;
      QAtomicInt m_gradual ;

      /*
      bool m_rumbleEnabled, m_quit ;

      // A rumble is a sequence composed of tremors.

      // Can be configure :
      //    ... ... ...     : 3 sequences of 3 tremors
      // OR .   .   .   .   : 4 sequences of 1 tremor
      unsigned long m_durationTremor, m_betweenTremor ; // ms
      unsigned int m_nbTremorBySequence ;

      // Can be configure :
      // .. .. ..    .. .. ..    .. .. ..   : 3 sequences of 2 tremors
      // ....  ....          ....  ....     : 2 sequences of 4 tremors
      unsigned long m_betweenSequence ; // ms
      unsigned int m_nbSequence ;

      bool m_continue, m_loop ;
      int m_gradual ;
      */
  };

}
#endif
