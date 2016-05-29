#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
...
"""

import urllib2
import time

import numpy as np
import matplotlib.pyplot as plt

calibration = {
    'V': lambda V: 9.3 * V            * 1.E-3,  #  ~0.1V/V [returns V]
    'I': lambda I: (I - 2502.) / 225. * 1.E+3,  # ~200mV/A, offset ~2500mV [returns mA]
    }

def read_adc_value(adc):
    data = urllib2.urlopen("http://arduino.local/arduino/adc/%s" % adc).read()
    return ( int(data.split()[-2]), float(data.split()[-1][1:-3]) )

print "Please make sure to connect to 'ArduinoYun-XXXXXXXXXXXX' wifi/wlan first!"

#plt.axis([0, 10, 0, 1])
plt.ylim([0., 15.])
plt.ion()

i = 0
while True:
    ts = (time.asctime(), time.time())

    adc0 = read_adc_value(0)
    voltage = calibration['V'](adc0[1])

    adc1 = read_adc_value(1)
    current = calibration['I'](adc1[1])

    resistance = voltage / (current*1.E-3)
    power = voltage * (current*1.E-3)

    #output = "%s, %014.3f, %09.3f V, %09.3f mA, %9.3f Ohm, %9.3f W" % (ts + (voltage, current, resistance, power))
    output = "%s, %14.3f, %9.3f V, %9.3f mA, %9.3f Ohm, %9.3f W" % (ts + (voltage, current, resistance, power))
    print output

    output = "%s, %14.3f, %09.3f, %09.3f, %09.3f, %09.3f" % (ts + (voltage, current, resistance, power))
    log = open("Yun_Log_BatteryDisCharging.log", "a")
    log.write(output + "\n")
    log.close()

    #plt.scatter(i, voltage)
    plt.scatter([i]*2, [voltage, current*1.E-3], color=['b', 'r'])

    #time.sleep(10.)
    plt.pause(10.)

    i += 1
