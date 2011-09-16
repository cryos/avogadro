/*
 *      Copyright (c) 2011 Mickael Gadroy
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef _WIWO_SEM_H_
#define _WIWO_SEM_H_

#include "warning_disable_begin.h"
#include "wmavo_const.h"
#include "wiwo.h"
#include <QObject>
#include <QSemaphore>
#include <QMutex>
#include "warning_disable_end.h"

/**
  * As WIWO class with Qt semaphore.
  */
template<class T> class WIWO_sem : protected WIWO<T>, public QObject
{
public :
  inline WIWO_sem( int sizeStock ) 
    : WIWO<T>(sizeStock), m_freeData(sizeStock), m_usedData(0) {};
  inline ~WIWO_sem(){} ;

  /** Test @{ */
  inline bool isEmpty(){ return (m_usedData.available()<=0) ; } ;
  inline bool isFull(){ return (m_freeData.available()<=0) ; } ;
  //@}

  /**
    * Push/Pop methods, Blocking call.
    * CAUTION: If you manipulate pointers, do not forget to delete it after use.
    * @{ */
  inline void pushBack( T obj )
  {
    m_pushMutex.lock() ;
    m_freeData.acquire() ;

    WIWO<T>::pushBack( obj ) ;

    m_usedData.release() ;
    m_pushMutex.unlock() ;
  } ;

  inline void popFront( T& obj_out )
  {
    m_usedData.acquire() ;
    obj_out = WIWO<T>::getFront() ;
    (*this)[0] = 0 ; // !! To test. Sometime, a the final plugin return a wrong value ...
    WIWO<T>::popFront() ;
    m_freeData.release() ;
  } ;
  // @}
  
  /**
    * Methods to compress data in the buffer.
    * @{ */
  // Compress data in the buffer (delete useless data).
  inline void compressData(){} ;
  inline void compressingData(){} ; //< Call compressData() in a thread.
  // @}


// Private attributs.
private :
  QSemaphore m_freeData, m_usedData ;
  QMutex m_pushMutex ;
};

#endif
