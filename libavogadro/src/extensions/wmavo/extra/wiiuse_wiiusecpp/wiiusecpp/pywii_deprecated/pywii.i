#
# pywii.i
#
# Written By:
#	James Thomas
#	Email: jt@missioncognition.net
#	Web: http://missioncognition.net/
#
# Copyright 2009
#
# This file is part of wiiusecpp / pywii.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

%module pywii
%include "std_vector.i"

# Used for CWiimote::ReadData
%array_class(unsigned char, byteArray);

%{
    /* Includes the header in the wrapper code */
    #include <wiiusecpp.h>
%}

# Allows list access to the CWiimote class instances
%template(wmVector) std::vector<CWiimote>;

# Allows list access to the IRDot class instances
%template(dotVector) std::vector<CIRDot>;

# Allows passing a string to CWiimote::WriteData
%typemap(in) (unsigned char *Data, unsigned int Length)
{
    $1 = (unsigned char *) PyString_AsString($input);
    $2 = (unsigned int) PyString_Size($input);
}

# Convert pass by reference to list return values for function paramters used in the library.
# The comments in typemaps.i states that this should be returning tuples but it is not.
%apply int * OUTPUT { byte &X, byte &Y, int &X, int &Y }
%apply float * OUTPUT { float &X, float &Y, float &Z, float &Pitch, float &Roll, float &Yaw, float &Angle, float &Magnitude }

# Rename EpansionDevicePtr to ExpansionDevice since python does not use pointers.
#%rename(ExpansionDevice) ExpansionDevicePtr;

# Rename the classes to ones more suitable for Python.  CWii becomes Wii.
%rename(Wii) CWii;

# All other classes are renamed adding a single underscore in front
# to indicate that they are not normally for use by the user.
%rename(_ClassButtonBase) CButtonBase;
%rename(_ClassButtons) CButtons;
%rename(_ClassNunchukButtons) CNunchukButtons;
%rename(_ClassClassicButtons) CClassicButtons;
%rename(_ClassGH3Buttons) CGH3Buttons;
%rename(_ClassJoystick) CJoystick;
%rename(_ClassAccelerometer) CAccelerometer;
%rename(_ClassIRDot) CIRDot;
%rename(_ClassIR) CIR;
%rename(_ClassExpansionDevice) CExpansionDevice;
%rename(_ClassNunchuk) CNunchuk;
%rename(_ClassClassic) CClassic;
%rename(_ClassGuitarHero3) CGuitarHero3;
%rename(_ClassWiimote) CWiimote;

# Parse the header file to generate wrappers
%include "wiiusecpp.h"

# Extensions for CWiimote
%extend CWiimote
{
    %pythoncode 
    %{
        # Make ReadData return a string
        def ReadData(self, Offset, Length):
            buffer = byteArray(Length)
            _pywii.CWiimote_ReadData(self, buffer, Offset, Length)
            return cdata(buffer, Length)

        # Make GetEventBuffer return a string
        def GetEventBuffer(self):
            return cdata(_pywii.CWiimote_GetEventBuffer(self), _pywii.CWiimote_EVENT_BUFFER_LENGTH)
    %}
};

%extend CWii
{
    %pythoncode
    %{
        # Add a handy function that is not in the C++ class
        def QuickConnect(self):
            print 'Searching for wiimotes... Turn them on!'
            #Find the wiimote
            numFound = self.Find(5) # Search for up to five seconds

            print 'Found %d wiimotes' % numFound

            print 'Connecting to wiimotes...'
            # Connect to the wiimote
            wiimotes = self.Connect()

            print 'Connected to %d wiimotes' % len(wiimotes)

            return wiimotes
        
        # This keeps track to the events that require our list of wiimotes to be reloaded.
        _EVENTS_NEEDING_REFRESH = ( _pywii._ClassWiimote_EVENT_DISCONNECT, 
                                    _pywii._ClassWiimote_EVENT_UNEXPECTED_DISCONNECT )

        POLL_NON_BLOCKING = 0
        POLL_BLOCKING = 1
        POLL_FOREVER =2

        # An enhanced pythonic version of Poll
        def PollMapped(self, HandlerMap, Mode=POLL_NON_BLOCKING):
            once = True
            eventOccured = False
            # Go Forever if there are wiimotes left to poll
            # Wait for an event to occur if we are in blocking mode
            # Or just check one time for an event.
            while (self.GetNumConnectedWiimotes() and (Mode == self.POLL_FOREVER)) or \
                  ((not eventOccured) and (Mode == self.POLL_BLOCKING)) or once:
                once = False

                #Poll the wiimotes to get the status for pitch, roll, buttons, etc.
                eventOccured = self.Poll()
                
                if(eventOccured):
                    refreshWiimotes = False

                    # At least one event occurred somewhere -- lets find them.
                    for wiimote in self.GetWiimotes(False):
                        event = wiimote.GetEvent()
                        
                        # Check if the event does somthing that require the 
                        # Wiimotes array to be refreshed after processing events.
                        if event in self._EVENTS_NEEDING_REFRESH:
                            refreshWiimotes = True

                        # Find the handler function for this event
                        handlerFunction = HandlerMap.get(event)
                        
                        # Call the handler
                        if handlerFunction:
                            handlerFunction(wiimote)

                    # Refresh the Wiimotes array if needed
                    # This keeps the Wiimote objects consistent with the
                    # underlying C structure states.
                    if refreshWiimotes:
                        self.RefreshWiimotes()
                        
            return eventOccured
    %}
};

%pythoncode
%{

# Make an application base class to simplify making our own pywii applications.
class WiiApp:
    LED_MAP = [ _pywii._ClassWiimote_LED_1, _pywii._ClassWiimote_LED_2,
                _pywii._ClassWiimote_LED_3, _pywii._ClassWiimote_LED_4]

    def Initialize(self):
        self.wii = Wii() # Defaults to 4 remotes

        #Find the wiimote
        numFound = self.wii.Find(5) # Search for up to five seconds

        # Connect to the wiimote
        wiimotes = self.wii.Connect()

        # Setup the wiimotes
        for index, wiimote in enumerate(wiimotes):
            self.WiimoteInit(wiimote, index)

        self.handlerMap = { _pywii._ClassWiimote_EVENT_EVENT: self.HandleEvent,
                            _pywii._ClassWiimote_EVENT_STATUS: self.HandleStatus,
                            _pywii._ClassWiimote_EVENT_DISCONNECT: self.HandleDisconnect,
                            _pywii._ClassWiimote_EVENT_UNEXPECTED_DISCONNECT: self.HandleDisconnect,
                            _pywii._ClassWiimote_EVENT_READ_DATA: self.HandleReadData,
                            _pywii._ClassWiimote_EVENT_NUNCHUK_INSERTED: self.HandleNunchukInserted,
                            _pywii._ClassWiimote_EVENT_CLASSIC_CTRL_INSERTED: self.HandleClassicInserted,
                            _pywii._ClassWiimote_EVENT_GUITAR_HERO_3_CTRL_INSERTED: self.HandleGH3Inserted,
                            _pywii._ClassWiimote_EVENT_NUNCHUK_REMOVED: self.HandleStatus,
                            _pywii._ClassWiimote_EVENT_CLASSIC_CTRL_REMOVED: self.HandleStatus,
                            _pywii._ClassWiimote_EVENT_GUITAR_HERO_3_CTRL_REMOVED: self.HandleStatus }

        return len(wiimotes)

    def Cleanup(self):
        pass

    def WiimoteInit(self, Wiimote, WiimoteNum):
        import time

        # Set Leds
        Wiimote.SetLEDs(self.LED_MAP[WiimoteNum])

        # Rumble for 0.2 seconds as a connection ack
        Wiimote.SetRumbleMode(Wiimote.ON)
        time.sleep(0.2)
        Wiimote.SetRumbleMode(Wiimote.OFF)

    def Loop(self, PollType=Wii.POLL_NON_BLOCKING):
        return self.wii.PollMapped(self.handlerMap, PollType)

    def HandleEvent(self, wm):
        pass

    def HandleStatus(self, wm):
        pass

    def HandleDisconnect(self, wm):
        pass

    def HandleReadData(self, wm):
        pass

    def HandleNunchukInserted(self, wm):
        pass

    def HandleClassicInserted(self, wm):
        pass

    def HandleGH3Inserted(self, wm):
        pass
%}
