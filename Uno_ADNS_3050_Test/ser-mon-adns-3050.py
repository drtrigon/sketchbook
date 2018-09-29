#!/usr/bin/python
# -*- coding: utf-8 -*-

# consider using miniterm or arduino ide serial monitor

import serial, time
import matplotlib.pyplot as plt
import numpy as np
ser = serial.Serial('/dev/ttyACM0')  # open first serial port
print ser.portstr       # check which port was really used
print ser.baudrate
#ser.write("hello")      # write a string
print "Please press the RESET button on the Arduino..."
print ser.readline(),
print ser.readline(),
print ser.readline(),
ser.write("g")  # select the pixel grabber
plt.ion()
while True:
    s = ser.readline()
    print len(s)
    print s
    for b in s:
        print hex(ord(b)),
#        print b,
    print
    if len(s) == 363:
        ba = np.array(bytearray(s[:-2]), dtype='uint8')
        ba = ba.reshape((19,19))
        ba *= 2
        ba = np.rot90(ba, 3)  # rotate origin to top-right
        print ba
#        plt.imshow(ba, interpolation='nearest')
        plt.imshow(ba, cmap='gray', vmin=0, vmax=255, interpolation='none')
        plt.draw()
#    print ser.read(),
#    time.sleep(1.)
#    plt.pause(1.)
ser.close()             # close port
