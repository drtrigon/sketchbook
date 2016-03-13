#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Use RFID tags/cards to control a machine, e.g. raspi server.

Register e.g. 1 card for shutdown and 1 card for reboot.
"""

# Registered RFID tags/card and assigned actions
actions = {
    "7C00568674D8": ('shutdown', "echo -shutdown-"),
    #"7C00568674D8": ('shutdown', "sudo shutdown -h now"),
    "7C00564A7212": ('restart', "echo -restart-"),
    #"7C00564A7212": ('restart', "sudo shutdown -r now"),
}

import time, os
import serial

# configure the serial connections (the parameters differs on the device you are connecting to)
ser = serial.Serial(
    port='/dev/ttyUSB0',
    baudrate=9600,
    parity=serial.PARITY_ODD,
    stopbits=serial.STOPBITS_TWO,
    bytesize=serial.SEVENBITS
)

ser.isOpen()

print 'Enter Ctrl+C to leave the application.'

while 1:
	out = ''
	# let's wait one second before reading output (let's give device time to answer)
	time.sleep(1)
	while (out[-1:] != '\x03') and (len(out) < 16):
		out += ser.read(1)

	if out != '':
		tag = out[1:-3]
		print len(out), tag, actions[tag][0]
		os.system(actions[tag][1])
