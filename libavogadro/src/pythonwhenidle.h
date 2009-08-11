/**********************************************************************
  PythonWhenIdle - Release python's GIL when the main loop is waiting for new events

  Copyright (C) 2009 by Sergey Smirnov
 
  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  Avogadro is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Avogadro is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 ***********************************************************************/  

#ifndef PYTHONWHENIDLE_H
#define PYTHONWHENIDLE_H

#include <QObject>

namespace Avogadro {

  class PythonWhenIdlePrivate;
  
  /**
   * @brief Singleton object that handles QAbstractEventDispatcher signals to
     enable asynchronous python callbacks.
     
     For example, there can be a middleware that has a thread running C code
     that listens for incoming requests from clients. When there is a pending
     request from a client the thread has to evoke a python callback.
     The python callback can't be evoked without locking the GIL so if it's
     already locked the C thread will suspend and wait for the GIL to unlock.
     When an application doesn't regularly release the GIL when it's not needed
     (e.g. waiting for new events) a very long unexpected lag may occur during
     external callback processing. This prevents correct operation of python
     plug-ins that use callbacks invoked by external threads.
     
     This singleton object corrects aforementioned drawback by installing 
     handlers that release the GIL regularly when a Qt application is about to
     start waiting for new events in the main loop. This handlers reclaim
     the original GIL state when the application awakes.
     
     This class should be initialized before entering the main loop of
     a Qt application.
   * 
   *
   */
  class A_EXPORT PythonWhenIdle : public QObject
  {
    Q_OBJECT

    public:
      /** Initialize a singleton object and connect aboutToBlock() and
       *  awake() signals of QAbstractEventDispatcher singleton to own slots.
       */
      static void initialize();
      
    private:
      PythonWhenIdlePrivate * const d;
    
      PythonWhenIdle();
      ~PythonWhenIdle();
      
      static PythonWhenIdle *instance;
      
    private Q_SLOTS:
      void mainLoopAboutToBlock();
      void mainLoopAwake();
  };

} // namespace

#endif

