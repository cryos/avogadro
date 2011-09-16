/*
 *	example.cpp
 *
 *	Written By:
 *		James Thomas
 *		Email: jt@missioncognition.net
 *	Copyright 2009
 *
 *  Copyright (c) 2011 Mickael Gadroy
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

/*
 * Note:  This C++ example follows the example.c program distributed with
 * 	      the wiiuse library.
 *        See http://www.wiiuse.net to get the wiiuse library which is required
 *        to build this package.  A SWIG based Python wrapper for this C++ library
 *        is available from http://www.missioncognition.net.
 */
#define _CRT_SECURE_NO_WARNINGS 1

#include <iostream>

#ifndef _WIN32 // WIN32
#include <unistd.h>
#include <wiiusecpp.h>
#else
#include "wiiusecpp.h"
#endif

using namespace std ;


const int LED_MAP[4] = {CWiimote::LED_1, CWiimote::LED_2, CWiimote::LED_3, CWiimote::LED_4};
double traveledDistance ;

void HandleEvent(CWiimote &wm)
{
    char prefixString[64];

    if(wm.Buttons.isJustPressed(CButtons::BUTTON_MINUS)) wm.SetMotionSensingMode(CWiimote::OFF) ;
    if(wm.Buttons.isJustPressed(CButtons::BUTTON_PLUS))  wm.SetMotionSensingMode(CWiimote::ON) ;
    if(wm.Buttons.isJustPressed(CButtons::BUTTON_DOWN))  wm.IR.SetMode(CIR::OFF);
    if(wm.Buttons.isJustPressed(CButtons::BUTTON_UP))    wm.IR.SetMode(CIR::ON);
    if(wm.Buttons.isJustPressed(CButtons::BUTTON_B))     wm.ToggleRumble();

    sprintf(prefixString, "Controller [%i]: ", wm.GetID());

    if(wm.Buttons.isJustChanged()) printf("%s State of Wiimote buttons changed\n", prefixString);

    if(wm.Buttons.isPressed(CButtons::BUTTON_A)) printf("%s A pressed\n", prefixString);
    if(wm.Buttons.isPressed(CButtons::BUTTON_B)) printf("%s B pressed\n", prefixString);
    if(wm.Buttons.isPressed(CButtons::BUTTON_UP))printf("%s Up pressed\n", prefixString);
    if(wm.Buttons.isPressed(CButtons::BUTTON_DOWN))printf("%s Down pressed\n", prefixString);
    if(wm.Buttons.isPressed(CButtons::BUTTON_LEFT))printf("%s Left pressed\n", prefixString);
    if(wm.Buttons.isPressed(CButtons::BUTTON_RIGHT))printf("%s Right pressed\n", prefixString);
    if(wm.Buttons.isPressed(CButtons::BUTTON_MINUS))printf("%s Minus pressed\n", prefixString);
    if(wm.Buttons.isPressed(CButtons::BUTTON_PLUS))printf("%s Plus pressed\n", prefixString);
    if(wm.Buttons.isPressed(CButtons::BUTTON_ONE))printf("%s One pressed\n", prefixString);
    if(wm.Buttons.isPressed(CButtons::BUTTON_TWO))printf("%s Two pressed\n", prefixString);
    if(wm.Buttons.isPressed(CButtons::BUTTON_HOME))printf("%s Home pressed\n", prefixString);

    // if(the accelerometer is turned on then print angles
    if(wm.isUsingACC())
    {
        float pitch, roll, yaw, a_pitch, a_roll;
        wm.Accelerometer.GetOrientation(pitch, roll, yaw);
        wm.Accelerometer.GetRawOrientation(a_pitch, a_roll);
        //printf("%s wiimote roll = %f [%f]\n", prefixString, roll, a_roll);
        //printf("%s wiimote pitch = %f [%f]\n", prefixString, pitch, a_pitch);
        //printf("%s wiimote yaw = %f\n", prefixString, yaw);
        
        double x, y, z, a ; 
        wm.Accelerometer.GetGForceInG( x, y, z ) ;
        printf("\n%s wiimote gForce = %2.5f, %2.5f:%2.5f:%2.5f\n", prefixString, wm.Accelerometer.GetGForceInG(), x, y, z );
        wm.Accelerometer.GetJerkInMS3( x, y, z ) ;
        printf("%s wiimote jerk = %2.5f, %2.5f:%2.5f:%2.5f\n", prefixString, wm.Accelerometer.GetJerkInMS3(), x, y, z );
        wm.Accelerometer.GetVelocity( x, y, z ) ;
        printf("%s wiimote velocity = %2.5f, %2.5f:%2.5f:%2.5f\n", prefixString, wm.Accelerometer.GetVelocity(), x, y, z );
        wm.Accelerometer.GetDistance( x, y, z ) ;
        a = wm.Accelerometer.GetDistance() ;
        traveledDistance += a ;
        printf("%s wiimote distance:%2.5f, %2.5f:%2.5f:%2.5f ;\n", prefixString, a, x, y, z );
        printf("%s wiimote traveled distance :%2.5f ;\n", prefixString, traveledDistance );
    }

    // if(IR tracking is on then print the coordinates
    if(wm.isUsingIR())
    {
        int x, y;

        printf("%s Num IR Dots: %i\n", prefixString, wm.IR.GetNumDots());
        printf("%s IR State: %u\n", prefixString, wm.IR.GetState());

        std::vector<CIRDot*>& dots = wm.IR.GetDots();

        int index=0 ;
        for( std::vector<CIRDot*>::iterator i=dots.begin() ; i != dots.end() ; ++i )
        {
            if( (*i)->isVisible() )
            {
                (*i)->GetCoordinate(x, y);
                printf("%s IR source %i: (%i, %i)\n", prefixString, ++index, x, y);

                wm.IR.GetCursorPosition(x, y);
                printf("%s IR cursor: (%i, %i)\n", prefixString, x, y);
                printf("%s IR z distance: %f\n", prefixString, wm.IR.GetDistance());
            }
        }
    }

    int exType = wm.ExpansionDevice.GetType() ;
    if(exType == wm.ExpansionDevice.TYPE_NUNCHUK)
    {
        float pitch, roll, yaw, a_pitch, a_roll;
        float angle, magnitude;

        CNunchuk &nc = wm.ExpansionDevice.Nunchuk;

        sprintf(prefixString, "Nunchuk [%i]: ", wm.GetID());

        if(nc.Buttons.isJustChanged()) printf("%s State of Wiimote buttons changed\n", prefixString);
        if(nc.Buttons.isPressed(CNunchukButtons::BUTTON_C))printf("%s C pressed\n", prefixString);
        if(nc.Buttons.isPressed(CNunchukButtons::BUTTON_Z))printf("%s Z pressed\n", prefixString);

        nc.Accelerometer.GetOrientation(pitch, roll, yaw);
        nc.Accelerometer.GetRawOrientation(a_pitch, a_roll);
        printf("%s roll = %f [%f]\n", prefixString, roll, a_roll);
        printf("%s pitch = %f [%f]\n", prefixString, pitch, a_pitch);
        printf("%s yaw = %f\n", prefixString, yaw);

        nc.Joystick.GetPosition(angle, magnitude);
        printf("%s joystick angle = %f\n", prefixString, angle);
        printf("%s joystick magnitude = %f\n", prefixString, magnitude);
    }
    else if(exType == wm.ExpansionDevice.TYPE_CLASSIC)
    {
        float angle, magnitude;
        CClassic &cc = wm.ExpansionDevice.Classic;

        sprintf(prefixString, "Classic [%i]: ", wm.GetID());

        if(cc.Buttons.isPressed(CClassicButtons::BUTTON_A)) printf("%s A pressed\n", prefixString);
        if(cc.Buttons.isPressed(CClassicButtons::BUTTON_B)) printf("%s B pressed\n", prefixString);
        if(cc.Buttons.isPressed(CClassicButtons::BUTTON_X)) printf("%s X pressed\n", prefixString);
        if(cc.Buttons.isPressed(CClassicButtons::BUTTON_Y)) printf("%s Y pressed\n", prefixString);
        if(cc.Buttons.isPressed(CClassicButtons::BUTTON_LEFT))printf("%s Left pressed\n", prefixString);
        if(cc.Buttons.isPressed(CClassicButtons::BUTTON_UP))printf("%s Up pressed\n", prefixString);
        if(cc.Buttons.isPressed(CClassicButtons::BUTTON_RIGHT))printf("%s Right pressed\n", prefixString);
        if(cc.Buttons.isPressed(CClassicButtons::BUTTON_DOWN))printf("%s Down pressed\n", prefixString);
        if(cc.Buttons.isPressed(CClassicButtons::BUTTON_PLUS))printf("%s Plus pressed\n", prefixString);
        if(cc.Buttons.isPressed(CClassicButtons::BUTTON_MINUS))printf("%s Minus pressed\n", prefixString);
        if(cc.Buttons.isPressed(CClassicButtons::BUTTON_HOME))printf("%s Home pressed\n", prefixString);
        if(cc.Buttons.isPressed(CClassicButtons::BUTTON_ZL))printf("%s ZL pressed\n", prefixString);
        if(cc.Buttons.isPressed(CClassicButtons::BUTTON_FULL_L))printf("%s ZR pressed\n", prefixString);
        if(cc.Buttons.isPressed(CClassicButtons::BUTTON_FULL_R))printf("%s ZR pressed\n", prefixString);

        printf("%s L button pressed = %f\n", prefixString, cc.GetLShoulderButton());
        printf("%s R button pressed = %f\n", prefixString, cc.GetRShoulderButton());

        cc.LeftJoystick.GetPosition(angle, magnitude);
        printf("%s left joystick angle = %f\n", prefixString, angle);
        printf("%s left joystick magnitude = %f\n", prefixString, magnitude);

        cc.RightJoystick.GetPosition(angle, magnitude);
        printf("%s right joystick angle = %f\n", prefixString, angle);
        printf("%s right joystick magnitude = %f\n", prefixString, magnitude);
    }
    else if(exType == wm.ExpansionDevice.TYPE_GUITAR_HERO_3)
    {
        float angle, magnitude;
        CGuitarHero3 &gh = wm.ExpansionDevice.GuitarHero3;

        sprintf(prefixString, "Guitar [%i]: ", wm.GetID());

        if(gh.Buttons.isPressed(CGH3Buttons::BUTTON_STRUM_UP)) printf("%s Strum Up pressed\n", prefixString);
        if(gh.Buttons.isPressed(CGH3Buttons::BUTTON_STRUM_DOWN))printf("%s Strum Down pressed\n", prefixString);
        if(gh.Buttons.isPressed(CGH3Buttons::BUTTON_YELLOW)) printf("%s Yellow pressed\n", prefixString);
        if(gh.Buttons.isPressed(CGH3Buttons::BUTTON_GREEN))  printf("%s Green pressed\n", prefixString);
        if(gh.Buttons.isPressed(CGH3Buttons::BUTTON_BLUE))   printf("%s Blue pressed\n", prefixString);
        if(gh.Buttons.isPressed(CGH3Buttons::BUTTON_RED))    printf("%s Red pressed\n", prefixString);
        if(gh.Buttons.isPressed(CGH3Buttons::BUTTON_ORANGE)) printf("%s Orange pressed\n", prefixString);
        if(gh.Buttons.isPressed(CGH3Buttons::BUTTON_PLUS))   printf("%s Plus pressed\n", prefixString);
        if(gh.Buttons.isPressed(CGH3Buttons::BUTTON_MINUS))  printf("%s Minus pressed\n", prefixString);

        printf("%s whammy bar = %f\n", prefixString, gh.GetWhammyBar());

        gh.Joystick.GetPosition(angle, magnitude);
        printf("%s joystick angle = %f\n", prefixString, angle);
        printf("%s joystick magnitude = %f\n", prefixString, magnitude);
    }
}

void HandleStatus(CWiimote &wm)
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

void HandleDisconnect(CWiimote &wm)
{
    printf("\n");
    printf("--- DISCONNECTED [wiimote id %i] ---\n", wm.GetID());
    printf("\n");
}

void HandleReadData(CWiimote &wm)
{
    printf("\n");
    printf("--- DATA READ [wiimote id %i] ---\n", wm.GetID());
    printf("\n");
}

void HandleNunchukInserted(CWiimote &wm)
{
    printf("Nunchuk inserted on controller %i.\n", wm.GetID());
}

void HandleClassicInserted(CWiimote &wm)
{
    printf("Classic controler inserted on controller %i.\n", wm.GetID());
}

void HandleGH3Inserted(CWiimote &wm)
{
    printf("GH3 inserted on controller %i.\n", wm.GetID());
}

int main(int argc, char** argv)
{

  //std::vector<CIRDot*> a ;

  //a.clear() ;

    CWii wii; // Defaults to 4 remotes
    
    int reloadWiimotes=0 ;
    int numFound=0 ;
    
    traveledDistance = 0 ;

    //Find the wiimote & search for up to five seconds.
    printf("Searching for wiimotes... Turn them on!\n");
    numFound = wii.Find(5);
    printf("Found %d wiimotes\n", numFound);

    // Under Windows, even if a Wiimote is not connected but it can appear
    // in "Devices and printers", so it connects it ...
    if( numFound <= 0 )
      return 0 ;


    // Connect to the wiimote
    printf( "Connecting to wiimotes...\n" ) ;
    std::vector<CWiimote*>& wiimotes = wii.Connect() ;
    printf( "Connected to %d wiimotes\n", (int)wiimotes.size() ) ;

    if( (int) wiimotes.size() <= 0 )
      return 0 ;


    // Setup the wiimotes
    int index=0 ;
    for( size_t i = 0; i < wiimotes.size() ; ++i )
    {
        // Use a reference to make working with the iterator handy.
        CWiimote *wiimote = wiimotes[i];

        //Set Leds
        wiimote->SetLEDs(LED_MAP[++index]);

        // Continuous information.
        //wiimote->SetFlags( CWiimote::FLAG_CONTINUOUS, 0x0 ) ;

        //Rumble for 0.2 seconds as a connection ack
        wiimote->SetRumbleMode(CWiimote::ON);

        #ifndef WIN32
        usleep(200000);
        #else
        Sleep(200);
        #endif

        wiimote->SetRumbleMode(CWiimote::OFF);
    }

    while( wiimotes.size() ) // Go so long as there are wiimotes left to poll
    {
        if( reloadWiimotes )
        {
            // Regenerate the list of wiimotes
            wiimotes = wii.GetWiimotes();
            reloadWiimotes = 0;
        }

        
        // (My) Bad idea.
        // The data are received in continuous when the accelerometer values are on.
        // So the received data is late ... Worse, latency appears.
        #ifndef WIN32
        //usleep(200000);
        #else
        //Sleep(20);
        #endif
        
        //Poll the wiimotes to get the status like pitch or roll
        if( wii.Poll() )
        {
            for( size_t i = 0; i < wiimotes.size() ; ++i )
            {
                // Use a reference to make working with the iterator handy.
				        CWiimote *wiimote = wiimotes[i];
                CWiimoteData *wm=wiimote->copyData() ;
                switch(wiimote->GetEvent())
                {
                    case CWiimote::EVENT_EVENT:
                        HandleEvent(*wiimote);
                        break;

                    case CWiimote::EVENT_STATUS:
                        HandleStatus(*wiimote);
                        break;
                    case CWiimote::EVENT_DISCONNECT:
                    case CWiimote::EVENT_UNEXPECTED_DISCONNECT:
                        HandleDisconnect(*wiimote);
                        reloadWiimotes = 1;
                        break;

                    case CWiimote::EVENT_READ_DATA:
                        HandleReadData(*wiimote);
                        break;
                    case CWiimote::EVENT_NUNCHUK_INSERTED:
                        HandleNunchukInserted(*wiimote);
                        reloadWiimotes = 1;
                        break;
                    case CWiimote::EVENT_CLASSIC_CTRL_INSERTED:
                        HandleClassicInserted(*wiimote);
                        reloadWiimotes = 1;
                        break;
                    case CWiimote::EVENT_GUITAR_HERO_3_CTRL_INSERTED:
                        HandleGH3Inserted(*wiimote);
                        reloadWiimotes = 1;
                        break;
                    case CWiimote::EVENT_NUNCHUK_REMOVED:
                    case CWiimote::EVENT_CLASSIC_CTRL_REMOVED:
                    case CWiimote::EVENT_GUITAR_HERO_3_CTRL_REMOVED:
                        printf("An expansion was removed.\n");
                        HandleStatus(*wiimote);
                        reloadWiimotes = 1;
                        break;
                    default:
                        break;
                }
            }
        }
    }

    return 0;
}
