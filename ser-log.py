#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
@brief Retrieve and log serial data.

@file ser-log.py

@author drtrigon
...
"""

import serial
import time
#import numpy as np
import matplotlib.pyplot as plt

ser = serial.Serial('/dev/ttyACM0')  # open first serial port
print ser.portstr       # check which port was really used

plt.axis([0, 20000, -0.1, 3])
plt.ion()
plt.show()

i = 0
x = list()
y = list()

#ser.write("hello")      # write a string
while True:
    try:
        ser.flushInput()
        temp_y = ser.readline()
        temp_y = temp_y.split()[0]
        print i, temp_y
#        x.append(i)
#        y.append(temp_y)
        plt.scatter(i, temp_y, s=0.1)
        i += 1
        plt.draw()
        time.sleep(1.)
    except KeyboardInterrupt:
        print "...",
        break
    except:
        pass

ser.close()             # close port
print "bye!"
