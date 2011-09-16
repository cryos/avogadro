#!/usr/bin/python

"""
synthdemo.py

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

Note: This python example uses the pywii (wiiusecpp) library available from
      http://missioncognition.net.  See http://www.wiiuse.net to get the wiiuse
      library which is required to build wiiusecpp.  See http://fluidsynth.org
      for information about the FluidSynth library (available in Ubuntu synaptic
      repository). Go to http://code.google.com/p/pyfluidsynth/ to get the
      python wrapper for FluidSynth.
      
"""

import time
import math

import fluidsynth
import pywii
from pywii import WiiApp

class WiiSynth(WiiApp):
    def Initialize(self):
        # Call the base WiiApp initializer
        WiiApp.Initialize(self)

        # Start up fluid synth
        self.fs = fluidsynth.Synth()
        self.fs.start()

        # Load a voice file and select it.
        sfid = self.fs.sfload("example.sf2")
        self.fs.program_select(0, sfid, 0, 0)
        
        # A dictionary to keep track of notes being played by wiimote ID.
        self.playingNote = {}

    def WiimoteInit(self, Wiimote, WiimoteNum):
        # Call the base class function
        WiiApp.WiimoteInit(self, Wiimote, WiimoteNum)
        
        # Turn on the accelerometer
        Wiimote.SetMotionSensingMode(Wiimote.ON)

    def Cleanup(self):
        # Turn off any notes still playing.
        while self.playingNote:
            id, note = self.playingNote.popitem()
            self.fs.noteoff(0, note)

        # Close the synth
        self.fs.delete()

    def HandleEvent(self, wm):
        id = wm.GetID()
        note = 0

        if wm.isUsingACC():
            wmAx, wmAy, wmAz = wm.Accelerometer.GetGravityVector()
            
            note = int(200.0 * math.atan2( math.pi-wmAx, -wmAz ) / math.pi) - 100

        else:
            wmPitch = wmRoll = wmYaw = 0.0
            wmAx = wmAy = wmAz = 0.0

        exType =  wm.ExpansionDevice.GetType()
        if exType == wm.ExpansionDevice.TYPE_NUNCHUK:
            nc = wm.ExpansionDevice.Nunchuk
            ncPitch, ncRoll, ncYaw = nc.Accelerometer.GetOrientation()
            ncAngle, ncMagnitude = nc.Joystick.GetPosition()
            ncAx, ncAy, ncAz = nc.Accelerometer.GetGravityVector()

            # Nunchuk joystick determines pitch
            if ncMagnitude > .2:
                note = int(ncAngle * 105.0 / 360.0) + 1
                
            # Nunchuk pitch determines pitch, negative pitch is pulling back so we invert
            note = int(-ncPitch)
        else:
            ncPitch = ncRoll = ncYaw = 0.0
            ncAngle = ncMagnitude = 0.0
            ncAx = ncAy = ncAz = 0.0

        # Bound note to the range 0 to 105 -- 0 is no note, 1 is lowest 105 is highest
        if note < 0:
            note = 0
        elif note > 105:
            note = 105
    
        start = False
        stop = False

        if id in self.playingNote:
            if wmAz < -0.5:
                stop = True
        else:
            if wmAz > 1.5:
                start = True

        if start and note:
            self.fs.noteon(0, note, 30)
            self.playingNote[id] = note
            print 'Note %d on' % note

        if stop and (id in self.playingNote):
            note = self.playingNote.pop(id)
            self.fs.noteoff(0, note)
            print 'Note %d off' % note

    def HandleDisconnect(self, wm):
        id = wm.GetID()
        if id in self.playingNote:
            note = self.playingNote.pop(id)
            self.fs.noteoff(0, note)
            print 'Note %d off' % note

if __name__ == '__main__':
    app = WiiSynth()
    app.Initialize()
    app.Loop(pywii.Wii.POLL_FOREVER)
    app.Cleanup()
