#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
...
"""

import urllib2
import time
import csv, cStringIO

import numpy as np
import matplotlib.pyplot as plt


log_file = "Yun_Log_BatteryDisCharging.log"
fmt_print = "%s, %14.3f, %9.3f s, %9.3f s, %9.3f V, %9.3f A, %9.3f Ohm, %9.3f W, %9.3f mAh, %9.3f J"
fmt_write = "%s, %14.3f, %9.3f, %9.3f, %9.3f, %9.3f, %9.3f, %9.3f, %9.3f, %9.3f"

#calibration = {
#    'V': lambda V: 9.3 * V            * 1.E-3,  #  ~0.1V/V [returns V]
#    'I': lambda I: (I - 2502.) / 225. * 1.E+3,  # ~200mV/A, offset ~2500mV [returns mA]
#    }


def read_adc_value(adc):
    data = urllib2.urlopen("http://arduino.local/arduino/adc/%s" % adc).read()
    return ( int(data.split()[-2]), float(data.split()[-1][1:-3]) )

def read_mon_values():
    ret = []
    data = urllib2.urlopen("http://arduino.local/arduino/mon/U").read()
    ret.append( data.split()[-1][:-1] )
    data = urllib2.urlopen("http://arduino.local/arduino/mon/I").read()
    ret.append( data.split()[-1][:-1] )
    data = urllib2.urlopen("http://arduino.local/arduino/mon/C").read()
    ret.append( data.split()[-1][:-3] )
    data = urllib2.urlopen("http://arduino.local/arduino/mon/E").read()
    ret.append( data.split()[-1][:-1] )
    data = urllib2.urlopen("http://arduino.local/arduino/mon/t").read()
    ret.append( data.split()[-1][:-1] )
    data = urllib2.urlopen("http://arduino.local/arduino/mon/W").read()
    ret.append( data.split()[-1][:-1] )
    return tuple(map(float, ret))

def read_mon_log():
    data = urllib2.urlopen("http://arduino.local/arduino/mon/log").read()
    spamreader = csv.reader(cStringIO.StringIO(data), quotechar='|')
    return [map(float, row) for row in spamreader if row[0]]


print "Please make sure to connect to 'ArduinoYun-XXXXXXXXXXXX' wifi/wlan first!"

print "Checking log file for timestamp of last entry ..."
with open(log_file, 'rb') as csvfile:
    spamreader = csv.reader(csvfile, quotechar='|')
    for row in spamreader:
        continue
delay = time.time() - time.mktime(time.strptime(row[0]))
getlog = (delay > 300.)

print "Last timestamp was:", row[0]
print "Time delay since then:", delay, "( >5mins?:", getlog, ")"

#plt.axis([0, 10, 0, 1])
plt.ylim([-1., 14.])
plt.grid(True)
plt.ion()

if getlog:
    print "Retrieving log data from Yun ..."

    W = read_mon_values()[-1]
    if (delay < 2*3600) and (W >= float(row[2])):  # there could still be data in the log we have already
        lW = float(row[2])
        print "Last entry watch value was:", lW
    else:
        lW = -1

    ts = (time.asctime(), time.time())

    i = 0
    for item in read_mon_log():
        if (lW >= float(item[0])):
            continue
        lW = float(item[0])
        output = fmt_print % (ts + tuple(item))
        print output

        output = fmt_write % (ts + tuple(item))
        with open(log_file, "a") as log:
            log.write(output + "\n")

        plt.scatter([item[0]]*2, item[2:4], color=['b', 'r'])

        i += 1

    print "Number of entries retrieved:", i

print "Retrieving live data from Yun, starting ..."

while True:
    ts = (time.asctime(), time.time())

#    adc0 = read_adc_value(0)
#    voltage = calibration['V'](adc0[1])
#
#    adc1 = read_adc_value(1)
#    current = calibration['I'](adc1[1])

    voltage, current, C, E, t, W = read_mon_values()

    resistance = np.float64(voltage) / current  # division with nan/inf (http://stackoverflow.com/questions/10011707/how-to-get-nan-when-i-divide-by-zero)
    power = voltage * current

    #output = "%s, %014.3f, %09.3f V, %09.3f mA, %9.3f Ohm, %9.3f W" % (ts + (voltage, current, resistance, power))
    output = fmt_print % (ts + (W, t, voltage, current, resistance, power, C, E))
    print output

    output = fmt_write % (ts + (W, t, voltage, current, resistance, power, C, E))
    with open(log_file, "a") as log:
        log.write(output + "\n")

    plt.scatter([W]*2, [voltage, current], color=['b', 'r'])

    #time.sleep(10.)
    plt.pause(60.)
