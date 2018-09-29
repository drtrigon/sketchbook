#!/usr/bin/python
# -*- coding: utf-8 -*-

# consider using miniterm or arduino ide serial monitor

import serial, time
ser = serial.Serial('/dev/ttyACM0')  # open first serial port
print ser.portstr       # check which port was really used
print ser.baudrate
#ser.write("hello")      # write a string
while True:
#    print ser.readline()
    s = ser.readline()
    print len(s)
    print s
    for b in s:
        print hex(ord(b)),
#        print b,
    print
#    print ser.read(),
#    time.sleep(1.)
ser.close()             # close port
