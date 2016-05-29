#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
...
"""

import urllib2
import time

calibration = {
    'V': lambda V: 9.3 * V            * 1.E-3,  #  ~0.1V/V [returns V]
    'I': lambda I: (I - 2502.) / 225. * 1.E+3,  # ~200mV/A, offset ~2500mV [returns mA]
    }

def read_adc_value(adc):
    data = urllib2.urlopen("http://arduino.local/arduino/adc/%s" % adc).read()
    return ( int(data.split()[-2]), float(data.split()[-1][1:-3]) )

print "Please make sure to connect to 'ArduinoYun-XXXXXXXXXXXX' wifi/wlan first!"

while True:
    ts = time.asctime()

    adc0 = read_adc_value(0)
    voltage = calibration['V'](adc0[1])

    adc1 = read_adc_value(1)
    current = calibration['I'](adc1[1])

    #output = "%s, %09.3f V, %09.3f mA" % (ts, voltage, current)
    output = "%s, %9.3f V, %9.3f mA" % (ts, voltage, current)
    print output

    output = "%s, %09.3f, %09.3f" % (ts, voltage, current)
    log = open("Yun_Log_BatteryDisCharging.log", "a")
    log.write(output + "\n")
    log.close()

    time.sleep(10.)
