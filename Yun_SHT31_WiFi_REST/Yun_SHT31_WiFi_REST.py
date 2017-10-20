#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
...
"""

import json
import urllib2

import time
import matplotlib.pyplot as plt


log_file = "Yun_SHT31_WiFi_REST.log"
fmt_print = "%s, %14.3f, %9.3f °C, %9.3f %%rf"
fmt_write = "%s, %14.3f, %9.3f, %9.3f"


def read_mon_values():
    ret = []
#    data = urllib2.urlopen("http://arduino.local/arduino/mon/U").read()
#    ret.append( data.split()[-1][:-1] )
#    data = urllib2.urlopen("http://arduino.local/arduino/mon/I").read()
#    ret.append( data.split()[-1][:-1] )
#    data = urllib2.urlopen("http://arduino.local/arduino/mon/C").read()
#    ret.append( data.split()[-1][:-3] )
#    data = urllib2.urlopen("http://arduino.local/arduino/mon/E").read()
#    ret.append( data.split()[-1][:-1] )
#    data = urllib2.urlopen("http://arduino.local/arduino/mon/t").read()
#    ret.append( data.split()[-1][:-1] )
#    data = urllib2.urlopen("http://arduino.local/arduino/mon/W").read()
#    ret.append( data.split()[-1][:-1] )

    data = json.load(urllib2.urlopen("http://192.168.11.12/"))
    #print data["variables"]["temperature"]
    #print data["variables"]["humidity"]
    ret.append( data["variables"]["temperature"] )
    ret.append( data["variables"]["humidity"] )
    return tuple(map(float, ret))

def blink():
    urllib2.urlopen("http://192.168.11.12/arduino/digital/13/1")
    time.sleep(.25)
    urllib2.urlopen("http://192.168.11.12/arduino/digital/13/0")


#plt.axis([0, 10, 0, 1])
plt.ylim([0., 100.])
plt.grid(True)
plt.ion()

print "Retrieving live data from Yun, starting ..."

parsed = json.load(urllib2.urlopen("http://192.168.11.12/"))
print json.dumps(parsed, indent=4, sort_keys=True)

blink()

start = time.time()
while True:
    ts = (time.asctime(), time.time())

    temperature, humidity = read_mon_values()

    output = fmt_print % (ts + (temperature, humidity))
    print output

    output = fmt_write % (ts + (temperature, humidity))
    with open(log_file, "a") as log:
        log.write(output + "\n")

    plt.scatter([time.time()-start]*2, [temperature, humidity], color=['r', 'b'])

    #time.sleep(10.)
    #plt.pause(2.)
    plt.pause(10.)
