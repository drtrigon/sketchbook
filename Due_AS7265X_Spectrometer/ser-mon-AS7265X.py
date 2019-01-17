#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
@brief Retrieve and display pixel data from ADNS-3050 sensor.

@file Uno_ADNS_3050_Test/ser-mon-adns-3050.py

@see MultiWii_2_4/MultiWii

@author drtrigon
...

consider using miniterm or arduino ide serial monitor
"""

import serial
import matplotlib.pyplot as plt
#import numpy as np
#freq = [610, 680, 730, 760, 810, 860,
#        560, 585, 645, 705, 900, 940,
#        410, 435, 460, 485, 510, 535] # latest data sheet
ser = serial.Serial('/dev/ttyACM0', baudrate=115200)  # open first serial port
print ser.portstr       # check which port was really used
print ser.baudrate
#ser.write("hello")      # write a string
print "Waiting for RESET of the Arduino..."
while (len(ser.readline()) != 13):
    pass
for i in range(11):
    print ser.readline(),
print "--- " * 10
plt.ion()
data_dict = {}
while True:
    s = ser.readline()
    s = s.strip()
    #print len(s)
    print s
    data = s.split(",")
    if (len(data) == 2):
        data[1] = data[1].replace('ovf', 'nan')
        data_dict[int(data[0])] = float(data[1])
    else:
        if (len(data_dict) == 18):
            #print data_dict
            plt.gcf().clear()
            plt.bar(data_dict.keys(), data_dict.values(), width=10)
            #plt.xlim((min(freq)-10, max(freq)+10))
            plt.xlim((400, 950))
            plt.draw()
        elif (len(data_dict) > 0):
            print "Only %i of 18 data points fetched, data missing!" %\
	          len(data_dict)
        data_dict = {}
#    print ser.read(),
#    time.sleep(1.)
#    plt.pause(1.)
ser.close()             # close port
