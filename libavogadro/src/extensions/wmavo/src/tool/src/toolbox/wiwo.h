/*
 *      Copyright (c) 2010, 2011 Mickael Gadroy
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


#pragma once
#ifndef __WIWO_H__
#define __WIWO_H__

#include "warning_disable_begin.h"
#include <iostream>
#include "warning_disable_end.h"

/**
 * wiwo for : What's In, What's Out.
 * The class lets to use the array either like a FIFO either like a LIFO either both.
 * This is a circular buffer with template.
 */
template<class T> class WIWO
{

private :
  int head, nbUsedCase ;
  int sizeWIWO ;

protected:
  T *buffer ;

public :

  /// Constructor by default, and destructor.

  inline WIWO( int sizeStock ) : head(0), nbUsedCase(0), sizeWIWO(sizeStock)
  {
    buffer = new T[sizeWIWO] ;
  } ;

  inline virtual ~WIWO(){ delete[] buffer ; } ;


  /// Getter/Setter.

  inline int getSizeCycle(){ return sizeWIWO ; } ;
  inline T getFront(){ return buffer[head] ; } ;
  inline T getBack(){ return (nbUsedCase<=0?buffer[head]:buffer[(head+nbUsedCase-1)%sizeWIWO]) ; } ;
  inline int getNbUsedCase(){ return (nbUsedCase<=0?0:nbUsedCase) ; }
  inline void RAZ(){ nbUsedCase = 0 ; } ;


  /// Public methods..

  /**
   * Y'a 'core d'la place ?
   */ 
  inline bool isEmpty(){ return (nbUsedCase==0) ; } ;

  /**
   * Test if the buffer is full.
   */
  inline bool isFull(){ return (nbUsedCase>=sizeWIWO) ; } ;

  /**
   * Copy the data at the back of the circular buffer.
   * <br/>Working "as a stack", if there is no place => not add in the buffer.
   * <br/>With pushBack() method, to work as a LIFO, use getBack()/popBack(). To work as a FIFO, use getFront()/popFront().
   * @param obj Object of T type to store.
   */
  inline bool pushBack( T obj )
  {
    if( nbUsedCase < sizeWIWO )
    {
      nbUsedCase++ ;
      buffer[(head+nbUsedCase-1) % sizeWIWO] = obj ;
      return true ;
    }
    else
      return false ;
  } ;


  /**
   * Copy the data at the front of the circular buffer.
   * <br/>Working "as a queue", if there is no place => overwrite the first incomming element.
   * <br/>With pushFront() method, to work as a LIFO, use getFront()/popFront(). To work as a FIFO, use getBack()/popBack().
   * @param obj Object of T type to store.
   */
  inline void pushFront( T obj )
  {
    if( (--head) < 0 )
      head = sizeWIWO-1 ;

    if( nbUsedCase < sizeWIWO )
      nbUsedCase++ ;

    buffer[head] = obj ;
  } ;

  /**
   * Pop the front data of the buffer (no other action is realized like deleted, tortured ...).
   */
  inline void popBack()
  {
    if( nbUsedCase > 0 )
      nbUsedCase -- ;
  } ;

  /**
   * Pop the back data of the buffer (no deletion is realized).
   */
  inline void/*T*/ popFront()
  {
    if( nbUsedCase > 0 )
    {
      head = (head+1) % sizeWIWO ;
      nbUsedCase -- ;
    }

    //return buffer[((head-1)<0?head-1:sizeWIWO-1)] ;
  } ;

  /**
   * Get the wanted object. 
   * </br>Be careful, the returned value is at the (n%sizeOfCircularBuffer) index.
   */
  inline T& operator [](unsigned int n)
  {
    return buffer[(head+n)%sizeWIWO] ;
  };

  /*
  template<class TT> friend ostream& operator << ( ostream &o, const WIWO<TT> &f )
  {
    for( int i=0 ; i<f.nbUsedCase ; i++ )
      o << f.buffer[(f.head+i)%f.sizeWIWO] << " " ;
    o<<endl ;

    return o ;
  } ;
  */
};

#endif

