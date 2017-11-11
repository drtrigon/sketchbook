#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
...
"""

import json
import urllib2

import time
import matplotlib.pyplot as plt

import sys


CONF = {
  'sensor': "192.168.11.7",

  'interval_update': 20.,
  'interval_timeout': 3.,

  'log_file': "Yun_SHT31_WiFi_REST.log",
  'fmt_print': "%s, %14.3f, %9.3f °C, %9.3f %%rf, %9.3f V",
  'fmt_write': "%s, %14.3f, %9.3f, %9.3f, %9.3f",
}
CONF['interval_timeout'] = max(CONF['interval_timeout'], CONF['interval_update']/10)


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

    while True:
        try:
            data = json.load(urllib2.urlopen("http://%s/" % CONF['sensor'], timeout = CONF['interval_timeout']))
            break
        except KeyboardInterrupt:
            print "Ctrl+C: quit."
            sys.exit()
        except:
            print sys.exc_info()[0], sys.exc_info()[1]
            #print sys.exc_info()[2]
    #print data["variables"]["temperature"]
    #print data["variables"]["humidity"]
    ret.append( data["variables"]["temperature"] )
    ret.append( data["variables"]["humidity"] )
    ret.append( data["variables"]["Vs"] )
    return tuple(map(float, ret))

def blink():
    urllib2.urlopen("http://%s/arduino/digital/13/1" % CONF['sensor'])
    time.sleep(.25)
    urllib2.urlopen("http://%s/arduino/digital/13/0" % CONF['sensor'])


#plt.axis([0, 10, 0, 1])
plt.ylim([0., 100.])
plt.grid(True)
ax = plt.gca()
#ax.set_xticks([1., 2., 3., 4., 5.])
ax.set_yticks(range(0, 110, 10))
plt.ion()

print "Run using this configuration:"

print json.dumps(CONF, indent=4, sort_keys=True)

print "Retrieving live data from Yun, starting ..."

#print urllib2.urlopen("http://%s/" % CONF['sensor']).read()
parsed = json.load(urllib2.urlopen("http://%s/" % CONF['sensor']))
print json.dumps(parsed, indent=4, sort_keys=True)

blink()

while True:
    temperature, humidity, Vs = read_mon_values()  # reading may take some time ...
    ts = (time.asctime(), time.time())             # ... thus get time afterwards

    output = CONF['fmt_print'] % (ts + (temperature, humidity, Vs))
    print output

    output = CONF['fmt_write'] % (ts + (temperature, humidity, Vs))
    with open(CONF['log_file'], "a") as log:
        log.write(output + "\n")

    plt.scatter([ts[1]]*3, [temperature, humidity, Vs], color=['r', 'b', 'g'])

    #time.sleep(CONF['interval_update'])
    plt.pause(CONF['interval_update'])
