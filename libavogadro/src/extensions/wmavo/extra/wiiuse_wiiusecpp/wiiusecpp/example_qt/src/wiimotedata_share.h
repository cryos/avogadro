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


#ifndef _WIIMOTEDATA_DATA_H_
#define _WIIMOTEDATA_DATA_H_

#define SIZE_CIRCULARBUFFER 20

#include "wiwo.h"
#include "wiiusecpp.h"

#include <QObject>
#include <QSemaphore>

class WiimotedataShare : public QObject
{
public :
    WiimotedataShare() ;
    ~WiimotedataShare() ;

    /**
      * Push/Pop methods.
      * @{ */
    CWiimoteData* pop() ; //< Blocking call.
    void push_back( CWiimoteData* wmData ) ;
    CWiimoteData* getValues( int i ) ; //< Not thread safe ...
    // @}

    /**
      * Methods to compress data in the buffer.
      * @{ */
    // Compress data in the buffer (delete useles data).
    void compressData(){} ;
    void compressingData(){} ; //< Call compressData() in a thread.
    // @}



private :
    WIWO<CWiimoteData*> *m_cirBuffer ;
    QSemaphore m_freeData, m_usedData ;
};

#endif
