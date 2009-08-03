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

#include <avogadro/global.h>
#include <boost/python.hpp>
#include <avogadro/pythonwhenidle.h>

#include <QAbstractEventDispatcher>

namespace Avogadro
{
    
  PythonWhenIdle *PythonWhenIdle::instance = NULL;
  
  class PythonWhenIdlePrivate
  {
    public:
      PyThreadState *threadState;
      PyGILState_STATE gilState;
  };

  void PythonWhenIdle::initialize()
  {
    if (instance == NULL)
    {
      instance = new PythonWhenIdle();
    }
  }
  
  PythonWhenIdle::PythonWhenIdle() : d(new PythonWhenIdlePrivate)
  {
    d->threadState = NULL;
    
    connect(QAbstractEventDispatcher::instance(), SIGNAL(aboutToBlock()),
      this, SLOT(mainLoopAboutToBlock()));
    connect(QAbstractEventDispatcher::instance(), SIGNAL(awake()),
      this, SLOT(mainLoopAwake()));
  }
  
  PythonWhenIdle::~PythonWhenIdle()
  {
    delete d;
  }

  void PythonWhenIdle::mainLoopAboutToBlock()
  {
    // ensure that the GIL is locked before saving the thread state
    d->gilState = PyGILState_Ensure();
    // save the thread state and unlock the GIL
    d->threadState = PyEval_SaveThread();
  }
  
  void PythonWhenIdle::mainLoopAwake()
  {
    if (d->threadState == NULL) // if aboutToBlock() was not emitted earlier
    {
      return;
    }
    PyEval_RestoreThread(d->threadState);
    d->threadState = NULL;
    PyGILState_Release(d->gilState);
  }
      
} // namespace

#include "pythonwhenidle.moc"
