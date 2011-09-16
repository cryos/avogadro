#!/usr/bin/python

"""
example.py

Written By:
	James Thomas
	Email: jt@missioncognition.net
	Web: http://missioncognition.net/

Copyright 2009

This file is part of wiiusecpp / pywii.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Note: This python example follows the example.c program distributed with
      the wiiuse library.  See http://www.wiiuse.net to get the wiiuse
      library which is required to build this package.
"""

import pywii
from time import sleep

LED_MAP = [pywii._ClassWiimote.LED_1, pywii._ClassWiimote.LED_2, pywii._ClassWiimote.LED_3, pywii._ClassWiimote.LED_4]

def HandleEvent(wm):
    if wm.Buttons.isJustPressed(wm.Buttons.BUTTON_MINUS):
        wm.SetMotionSensingMode(wm.OFF)
    
    if wm.Buttons.isJustPressed(wm.Buttons.BUTTON_PLUS):
        wm.SetMotionSensingMode(wm.ON)

    if wm.Buttons.isJustPressed(wm.Buttons.BUTTON_DOWN):
        wm.IR.SetMode(wiimote.IR.OFF)
    
    if wm.Buttons.isJustPressed(wm.Buttons.BUTTON_UP):
        wm.IR.SetMode(wiimote.IR.ON)

    if wm.Buttons.isJustPressed(wm.Buttons.BUTTON_B):
        wm.ToggleRumble()
    
    prefixString = 'Controller [%i]:  ' % wm.GetID()

    if wm.Buttons.isPressed(wm.Buttons.BUTTON_A): print prefixString + 'A pressed'
    if wm.Buttons.isPressed(wm.Buttons.BUTTON_B): print prefixString + 'B pressed'
    if wm.Buttons.isPressed(wm.Buttons.BUTTON_UP): print prefixString + 'Up pressed'
    if wm.Buttons.isPressed(wm.Buttons.BUTTON_DOWN): print prefixString + 'Down pressed'
    if wm.Buttons.isPressed(wm.Buttons.BUTTON_LEFT): print prefixString + 'Left pressed'
    if wm.Buttons.isPressed(wm.Buttons.BUTTON_RIGHT): print prefixString + 'Right pressed'
    if wm.Buttons.isPressed(wm.Buttons.BUTTON_MINUS): print prefixString + 'Minus pressed'
    if wm.Buttons.isPressed(wm.Buttons.BUTTON_PLUS): print prefixString + 'Plus pressed'
    if wm.Buttons.isPressed(wm.Buttons.BUTTON_ONE): print prefixString + 'One pressed'
    if wm.Buttons.isPressed(wm.Buttons.BUTTON_TWO): print prefixString + 'Two pressed'
    if wm.Buttons.isPressed(wm.Buttons.BUTTON_HOME): print prefixString + 'Home pressed'
    
    # if the accelerometer is turned on then print angles
    if wm.isUsingACC():
        pitch, roll, yaw = wm.Accelerometer.GetOrientation()
        a_pitch, a_roll = wm.Accelerometer.GetRawOrientation()
        print prefixString + 'wiimote roll  = %f [%f]' % (roll, a_roll)
        print prefixString + 'wiimote pitch = %f [%f]' % (pitch, a_pitch)
        print prefixString + 'wiimote yaw   = %f' % yaw

    # if IR tracking is on then print the coordinates
    if wm.isUsingIR():
        print prefixString + 'Num IR Dots: ', wm.IR.GetNumDots()
        print prefixString + 'IR State: ', wm.IR.GetState()
        for i, dot in enumerate(wm.IR.GetDots()):
            if dot.isVisible():
                x, y = dot.GetCoordinate()
                print prefixString + 'IR source %i: (%u, %u)' % (i, x, y)
        
        x, y = wm.IR.GetCursorPosition()
        print prefixString + 'IR cursor: (%u, %u)' % (x, y)
        print prefixString + 'IR z distance: %f' % wm.IR.GetDistance()

    exType =  wm.ExpansionDevice.GetType()
    if exType == wm.ExpansionDevice.TYPE_NUNCHUK:
        nc = wm.ExpansionDevice.Nunchuk
        prefixString = 'Nunchuk [%i]:  ' % wm.GetID()
        if nc.Buttons.isPressed(nc.Buttons.BUTTON_C): print prefixString + 'C pressed'
        if nc.Buttons.isPressed(nc.Buttons.BUTTON_Z): print prefixString + 'Z pressed'

        pitch, roll, yaw = nc.Accelerometer.GetOrientation()
        a_pitch, a_roll = nc.Accelerometer.GetRawOrientation()
        print prefixString + 'roll  = %f [%f]' % (roll, a_roll)
        print prefixString + 'pitch = %f [%f]' % (pitch, a_pitch)
        print prefixString + 'yaw   = %f' % yaw
        
        angle, magnitude = nc.Joystick.GetPosition()
        print prefixString + 'joystick angle       = %f' % angle
        print prefixString + 'joystick magnitude   = %f' % magnitude

    elif exType == wm.ExpansionDevice.TYPE_CLASSIC:
        cc = wm.ExpansionDevice.Classic
        prefixString = 'Classic [%i]:  ' % wm.GetID()
        if cc.Buttons.isPressed(cc.Buttons.BUTTON_A): print prefixString + 'A pressed'
        if cc.Buttons.isPressed(cc.Buttons.BUTTON_B): print prefixString + 'B pressed'
        if cc.Buttons.isPressed(cc.Buttons.BUTTON_X): print prefixString + 'X pressed'
        if cc.Buttons.isPressed(cc.Buttons.BUTTON_Y): print prefixString + 'Y pressed'
        if cc.Buttons.isPressed(cc.Buttons.BUTTON_LEFT): print prefixString + 'Left pressed'
        if cc.Buttons.isPressed(cc.Buttons.BUTTON_UP): print prefixString + 'Up pressed'
        if cc.Buttons.isPressed(cc.Buttons.BUTTON_RIGHT): print prefixString + 'Right pressed'
        if cc.Buttons.isPressed(cc.Buttons.BUTTON_DOWN): print prefixString + 'Down pressed'
        if cc.Buttons.isPressed(cc.Buttons.BUTTON_PLUS): print prefixString + 'Plus pressed'
        if cc.Buttons.isPressed(cc.Buttons.BUTTON_MINUS): print prefixString + 'Minus pressed'
        if cc.Buttons.isPressed(cc.Buttons.BUTTON_HOME): print prefixString + 'Home pressed'
        if cc.Buttons.isPressed(cc.Buttons.BUTTON_ZL): print prefixString + 'ZL pressed'
        if cc.Buttons.isPressed(cc.Buttons.BUTTON_FULL_L): print prefixString + 'ZR pressed'
        if cc.Buttons.isPressed(cc.Buttons.BUTTON_FULL_R): print prefixString + 'ZR pressed'

        print prefixString + 'L button pressed         =  %f' % cc.GetLShoulderButton()
        print prefixString + 'R button pressed         = %f' % cc.GetRShoulderButton()
        
        angle, magnitude = cc.LeftJoystick.GetPosition()
        print prefixString + 'left joystick angle      = %f' % angle
        print prefixString + 'left joystick magnitude  = %f' % magnitude
        
        angle, magnitude = cc.RightJoystick.GetPosition()
        print prefixString + 'right joystick angle     = %f' % angle
        print prefixString + 'right joystick magnitude = %f' % magnitude

    elif exType == wm.ExpansionDevice.TYPE_GUITAR_HERO_3:
        gh = wm.ExpansionDevice.GuitarHero3
        prefixString = 'Guitar [%i]:  ' % wm.GetID()
        if gh.Buttons.isPressed(gh.Buttons.BUTTON_STRUM_UP): print prefixString + 'Strum Up pressed'
        if gh.Buttons.isPressed(gh.Buttons.BUTTON_STRUM_DOWN): print prefixString + 'Strum Down pressed'
        if gh.Buttons.isPressed(gh.Buttons.BUTTON_YELLOW): print prefixString + 'Yellow pressed'
        if gh.Buttons.isPressed(gh.Buttons.BUTTON_GREEN): print prefixString + 'Green pressed'
        if gh.Buttons.isPressed(gh.Buttons.BUTTON_BLUE): print prefixString + 'Blue pressed'
        if gh.Buttons.isPressed(gh.Buttons.BUTTON_RED): print prefixString + 'Red pressed'
        if gh.Buttons.isPressed(gh.Buttons.BUTTON_ORANGE): print prefixString + 'Orange pressed'
        if gh.Buttons.isPressed(gh.Buttons.BUTTON_PLUS): print prefixString + 'Plus pressed'
        if gh.Buttons.isPressed(gh.Buttons.BUTTON_MINUS): print prefixString + 'Minus pressed'
 
        print prefixString + 'whammy bar         =  %f' % gh.GetWhammyBar()

        angle, magnitude = gh.Joystick.GetPosition()
        print prefixString + 'joystick angle      = %f' % angle
        print prefixString + 'joystick magnitude  = %f' % magnitude
   
def HandleStatus(wm):
    print
    print '--- CONTROLLER STATUS [wiimote id %i] ---\n' % wm.GetID()
    print
    
    print 'attachment:      %i' % wm.ExpansionDevice.GetType()
    print 'speaker:         %i' % wm.isUsingSpeaker()
    print 'ir:              %i' % wm.isUsingIR()
    print 'leds:            %i %i %i %i' % (wm.isLEDSet(1), wm.isLEDSet(2), wm.isLEDSet(3), wm.isLEDSet(4))
    print 'battery:         %f %%' % wm.GetBatteryLevel()

def HandleDisconnect(wm):
    print
    print '--- DISCONNECTED [wiimote id %i] ---' % wm.GetID()
    print

def HandleReadData(wm):
    print
    print '--- DATA READ [wiimote id %i] ---' % wm.GetID()
    print

def HandleNunchukInserted(wm):
    print 'Nunchuk inserted on controller %i.' % wm.GetID()

def HandleClassicInserted(wm):
    print 'Classic controler inserted on controller %i.' % wm.GetID()

def HandleGH3Inserted(wm):
    print 'GH3 inserted on controller %i.' % wm.GetID()


# Start of example program

wii = pywii.Wii() # Defaults to 4 remotes

print 'Searching for wiimotes... Turn them on!'
# Find the wiimote
numFound = wii.Find(5) # Search for up to five seconds

print 'Found %d wiimotes' % numFound

print 'Connecting to wiimotes...'
# Connect to the wiimote
wiimotes = wii.Connect()

print 'Connected to %d wiimotes' % len(wiimotes)

# Setup the wiimotes
for index, wiimote in enumerate(wiimotes):
    #Set Leds
    wiimote.SetLEDs(LED_MAP[index])

    #Rumble for 0.2 seconds as a connection ack
    wiimote.SetRumbleMode(wiimote.ON)
    sleep(0.2)
    wiimote.SetRumbleMode(wiimote.OFF)

# Choose between the two methods of polling.  I've included both here so that
# people will know how to use either method.  I greatly prefer the mapped approach.
usePollMapped = True
if usePollMapped:
    handlerMap = {  pywii._ClassWiimote.EVENT_EVENT: HandleEvent,
                    pywii._ClassWiimote.EVENT_STATUS: HandleStatus,
                    pywii._ClassWiimote.EVENT_DISCONNECT: HandleDisconnect,
                    pywii._ClassWiimote.EVENT_UNEXPECTED_DISCONNECT: HandleDisconnect,
                    pywii._ClassWiimote.EVENT_READ_DATA: HandleReadData,
                    pywii._ClassWiimote.EVENT_NUNCHUK_INSERTED: HandleNunchukInserted,
                    pywii._ClassWiimote.EVENT_CLASSIC_CTRL_INSERTED: HandleClassicInserted,
                    pywii._ClassWiimote.EVENT_GUITAR_HERO_3_CTRL_INSERTED: HandleGH3Inserted,
                    pywii._ClassWiimote.EVENT_NUNCHUK_REMOVED: HandleStatus,
                    pywii._ClassWiimote.EVENT_CLASSIC_CTRL_REMOVED: HandleStatus,
                    pywii._ClassWiimote.EVENT_GUITAR_HERO_3_CTRL_REMOVED: HandleStatus }

    # If other processing needs to be done try wii.POLL_NON_BLOCKING, wii.POLL_BLOCKING
    # as appropriate depending on what you need to do.
    wii.PollMapped(handlerMap, wii.POLL_FOREVER)

else:  # The standard way.
    # The events will determine when the list of wiimote objects must be reloaded.
    reloadWiimotes = False

    while wiimotes: # Go so long as there are wiimotes left to poll
        if reloadWiimotes:
            # Regenerate the list of wiimotes
            wiimotes = wii.GetWiimotes()
            reloadWiimotes = False;

        # Poll the wiimotes to get the status like pitch or roll
        if(wii.Poll()):
            for wiimote in wiimotes:
                event = wiimote.GetEvent()
                
                # It is times like this that I wish Guido would add a case statement to python
                # Although I guess a dictionary could be used as a sort of dispatcher
                if event == wiimote.EVENT_EVENT:
                    HandleEvent(wiimote)
                    
                elif event == wiimote.EVENT_STATUS:
                    HandleStatus(wiimote)

                elif event == wiimote.EVENT_DISCONNECT or \
                     event == wiimote.EVENT_UNEXPECTED_DISCONNECT:
                    HandleDisconnect(wiimote)
                    reloadWiimotes = True
                    
                elif event == wiimote.EVENT_READ_DATA:
                    HandleReadData(wiimote)
                    
                elif event == wiimote.EVENT_NUNCHUK_INSERTED:
                    HandleNunchukInserted(wiimote)
                    reloadWiimotes = True
                    
                elif event == wiimote.EVENT_CLASSIC_CTRL_INSERTED:
                    HandleClassicInserted(wiimote)
                    reloadWiimotes = True
                    
                elif event == wiimote.EVENT_GUITAR_HERO_3_CTRL_INSERTED:
                    HandleGH3Inserted(wiimote)
                    reloadWiimotes = True
                    
                elif event == wiimote.EVENT_NUNCHUK_REMOVED or \
                     event == wiimote.EVENT_CLASSIC_CTRL_REMOVED or \
                     event == wiimote.EVENT_GUITAR_HERO_3_CTRL_REMOVED:
                    print 'An expansion was removed.'
                    HandleStatus(wiimote)
                    reloadWiimotes = True
                
