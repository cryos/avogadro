/*
 *	example.cpp
 *
 *	Written By:
 *		James Thomas
 *		Email: jt@missioncognition.net
 *	Copyright 2009
 *
 *      Copyright (c) 2011 Mickael Gadroy
 *
 *	This file is part of wiiusecpp.
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
 *
 */

#include "wiimote.h"


Wiimote::Wiimote( WiimotedataShare *wmDataShare) :
    QObject(), m_wii(1), m_wiimotes(NULL),
    m_wmDataShare(wmDataShare), m_traveledDistance(0)
{}

Wiimote::~Wiimote()
{}

bool Wiimote::connectToDevice()
{
    //Find the wiimote & search for up to five seconds.
    printf("Searching for wiimotes... Turn them on!\n");
    int numFound = m_wii.Find(5);
    printf("Found %d wiimotes\n", numFound);

    // Under Windows, even if a Wiimote is not connected but it can appear
    // in "Devices and printers", so it connects it ...
    if( numFound <= 0 )
      return false ;

    // Connect to the wiimote
    printf( "Connecting to wiimotes...\n" ) ;
    m_wiimotes = &(m_wii.Connect()) ;
    printf( "Connected to %d wiimotes\n", (int)m_wiimotes->size() ) ;

    if( (int) m_wiimotes->size() <= 0 )
      return false ;
    else
    {
        // Setup the wiimotes
        int index=0 ;
        for( size_t i = 0; i < m_wiimotes->size() ; ++i )
        {
            // Use a reference to make working with the iterator handy.
            CWiimote *wiimote = (*m_wiimotes)[i] ;

            //Set Leds
            wiimote->SetLEDs(LED_MAP[++index]) ;

            // Set sensibility.
            wiimote->Accelerometer.SetAccelThreshold(3); // 5 by default.
            //int a=wiimote->Accelerometer.GetAccelThreshold() ;

            // Continuous information.
            //wiimote->SetFlags( CWiimote::FLAG_CONTINUOUS, 0x0 ) ; // Do not work ...

            //Rumble for 0.2 seconds as a connection ack
            wiimote->SetRumbleMode(CWiimote::ON);
            #ifndef WIN32
                usleep(200000);
            #else
                Sleep(200);
            #endif
            wiimote->SetRumbleMode(CWiimote::OFF);
        }

        return true ;
    }
}

int Wiimote::runDevice()
{
    CWiimote *wiimote=NULL ;

    while( m_wiimotes->size() ) // Go so long as there are wiimotes left to poll
    {
        // Not more (not 20) to receive the Wiimote information in real time.
        #ifndef WIN32
        usleep(120000);
        #else
        //Sleep(12);
        #endif

        bool updateButton, updateAcc, updateIR ;
        m_wii.Poll( updateButton, updateAcc, updateIR ) ;

        //Poll the wiimotes to get the status like pitch or roll
        if( updateButton )
        {
            for( size_t i = 0; i < m_wiimotes->size() ; ++i )
            {
                // Use a reference to make working with the iterator handy.
                wiimote = (*m_wiimotes)[i] ;

                switch(wiimote->GetEvent())
                {
                    case CWiimote::EVENT_EVENT:
                        HandleEvent(*wiimote); break;
                    case CWiimote::EVENT_STATUS:
                        HandleStatus(*wiimote) ; break;
                    case CWiimote::EVENT_DISCONNECT:
                    case CWiimote::EVENT_UNEXPECTED_DISCONNECT:
                        HandleDisconnect(*wiimote); break;
                    case CWiimote::EVENT_READ_DATA:
                        HandleReadData(*wiimote); break;
                    default:
                        break;
                }
            }
        }

        if( updateButton || updateAcc || updateIR )
        {
            // Copy all time for the acceleration values for test!
            for( size_t i = 0; i < m_wiimotes->size() ; ++i )
            {
                // Use a reference to make working with the iterator handy.
                wiimote = (*m_wiimotes)[i] ;
                m_wmDataShare->push_back( wiimote->copyData() );
            }
        }
    }

    return 0;
}

void Wiimote::HandleEvent(CWiimote &wm)
{
    if(wm.Buttons.isJustPressed(CButtons::BUTTON_MINUS)) wm.SetMotionSensingMode(CWiimote::OFF) ;
    if(wm.Buttons.isJustPressed(CButtons::BUTTON_PLUS))  wm.SetMotionSensingMode(CWiimote::ON) ;
    if(wm.Buttons.isJustPressed(CButtons::BUTTON_DOWN))  wm.IR.SetMode(CIR::OFF);
    if(wm.Buttons.isJustPressed(CButtons::BUTTON_UP))    wm.IR.SetMode(CIR::ON);
    if(wm.Buttons.isJustPressed(CButtons::BUTTON_B))     wm.ToggleRumble();
}

void Wiimote::HandleStatus(CWiimote &wm)
{
    printf("\n");
    printf("--- CONTROLLER STATUS [wiimote id %i] ---\n\n", wm.GetID());
    printf("\n");

    printf("attachment: %i\n", wm.ExpansionDevice.GetType());
    printf("speaker: %i\n", wm.isUsingSpeaker());
    printf("ir: %i\n", wm.isUsingIR());
    printf("leds: %i %i %i %i\n", wm.isLEDSet(1), wm.isLEDSet(2), wm.isLEDSet(3), wm.isLEDSet(4));
    printf("battery: %f %%\n", wm.GetBatteryLevel());
}

void Wiimote::HandleDisconnect(CWiimote &wm)
{
    printf("\n");
    printf("--- DISCONNECTED [wiimote id %i] ---\n", wm.GetID());
    printf("\n");
}

void Wiimote::HandleReadData(CWiimote &wm)
{
    printf("\n");
    printf("--- DATA READ [wiimote id %i] ---\n", wm.GetID());
    printf("\n");
}
