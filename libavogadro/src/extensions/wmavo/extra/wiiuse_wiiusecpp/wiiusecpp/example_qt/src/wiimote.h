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

#ifndef WIIMOTE_H
#define WIIMOTE_H

#define _CRT_SECURE_NO_WARNINGS 1

#ifndef _WIN32 // WIN32
    #include <unistd.h>
    #include <wiiusecpp.h>
#else
    #include "wiiusecpp.h"
#endif

#include "wiimotedata_share.h"
#include <QObject>
#include <iostream>

using namespace std ;

const int LED_MAP[4] = {CWiimote::LED_1, CWiimote::LED_2, CWiimote::LED_3, CWiimote::LED_4} ;

class Wiimote : public QObject
{
    Q_OBJECT // For the thread.

// Public method.
public :
    Wiimote( WiimotedataShare *wmDataShare ) ;
    ~Wiimote() ;
    bool connectToDevice() ;


public Q_SLOTS :
    int runDevice() ;


// Private method.
private :
    void HandleEvent(CWiimote &wm) ;
    void HandleStatus(CWiimote &wm) ;
    void HandleDisconnect(CWiimote &wm) ;
    void HandleReadData(CWiimote &wm) ;


// Private attribut.
private :
    CWii m_wii ;
    std::vector<CWiimote*>* m_wiimotes ;
    WiimotedataShare *m_wmDataShare ;
    double m_traveledDistance ;
};

#endif // WIIMOTE_H
