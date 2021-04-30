#!/usr/bin/python

#  HP Redeye Protocol implemented on a AVR Mega328 using
#     8-bit Timer/Counter 0 in FAST PWM-mode.
#  Copyright (C) 2011 Christian Vogel <vogelchr@vogel.cx>
# 
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
# 
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
# 
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

import serial
import sys
import time


class RedEye(object) :
	def __init__(self,portname) :
		self.ser = serial.Serial(portname,baudrate=115200,timeout=5.0)
		while 1 :
			c = self.ser.read(1)
			if c == '' :
				raise RuntimeError("Did not receive hello message.");
			if c == '$' :
				break
		print >>sys.stderr,'RedEye: ready, now doing reset.'

	def reset(self) :
		# reset does not work on my 82240A!
		self.ser.write('\033\377')
		self.ser.flush()
		time.sleep(5.0) # reset needs some time.

	def writeln(self,s) :
		self.ser.write(s+'\n')
		self.ser.flush()
		time.sleep(1.0)

	def wide(self,onoff) :
		if onoff :
			# 27 253 : start printing double wide
			self.ser.write('\033\375')
		else :
			# 27 252 : start printing normal wide
			self.ser.write('\033\374')
		self.ser.flush()

	def underline(self,onoff) :
		if onoff :
			# 27 251 : start underlining
			self.ser.write('\033\373')
		else :
			# 27 250 : stop underlining
			self.ser.write('\033\372')
		self.ser.flush()

R = RedEye('/dev/ttyUSB0')

R.writeln('**********************')
R.wide(True)
R.writeln('Hello world')
R.wide(False)
R.writeln('**********************')
R.underline(True)
R.writeln('This is important.')
R.underline(False)
time.sleep(1.0)


