/*
 *      Copyright (c) 2011 Mickael Gadroy
 *
 *	This file is part of example_qt of the wiiusecpp library.
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


#include "wiimotedata_share.h"

WiimotedataShare::WiimotedataShare() :
    m_cirBuffer(NULL),
    m_freeData(SIZE_CIRCULARBUFFER), m_usedData(0)
{
    m_cirBuffer = new WIWO<CWiimoteData*>(SIZE_CIRCULARBUFFER) ;
}

WiimotedataShare::~WiimotedataShare()
{
    if( m_cirBuffer )
    {
        delete m_cirBuffer ;
        m_cirBuffer = NULL ;
    }
}

/**
  * Pop data. The wanted data is in parameter for semaphore/reference reasons.
  *  - The reference copies the data for security reason after out the mutex.
  *  - The semaphore can not able to return a reference in the mutex section.
  * CAUTION: Do not forget to delete the objet after use.
  * @param wmDataOut
  */
CWiimoteData* WiimotedataShare::pop()
{
    CWiimoteData* wmDataOut ;

    m_usedData.acquire() ;
    wmDataOut = m_cirBuffer->getTete() ;
    m_cirBuffer->pull() ;
    m_freeData.release() ;

    return wmDataOut ;
}

void WiimotedataShare::push_back( CWiimoteData *wmData )
{
    m_freeData.acquire() ;
    m_cirBuffer->pushAtEnd( wmData ) ;
    m_usedData.release() ;
}

CWiimoteData* WiimotedataShare::getValues(int i)
{
    if( i>0 && i<m_cirBuffer->getNbCaseUse() )
        return (*m_cirBuffer)[i] ;
    else
        return NULL ;
}
