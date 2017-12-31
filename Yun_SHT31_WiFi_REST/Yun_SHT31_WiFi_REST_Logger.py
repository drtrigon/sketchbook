#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
Logging script for Yun SHT31 (Arduino IDE) USB drive data.
"""

import json

import time
import matplotlib.pyplot as plt

import subprocess
import os
import csv

import numpy as np


CONF = {
  'sensor': "datalog.txt",
  'mount': ["lsblk | grep 'sdb1'"],

  'log_file': "Yun_ESP8266_SHT31_WiFi_REST.log",
  'fmt_print': "%s, %14.3f, %9.3f °C, %9.3f %%rf, %9.3f V",
  'fmt_write': "%s, %14.3f, %9.3f, %9.3f, %9.3f",

  'fmt_time': "%Y-%m-%d %H:%M:%S",
}


#plt.axis([0, 10, 0, 1])
plt.ylim([0., 100.])
plt.grid(True)
ax = plt.gca()
#ax.set_xticks([1., 2., 3., 4., 5.])
ax.set_yticks(range(0, 110, 10))
#plt.ion()

print "Run using this configuration:"

print json.dumps(CONF, indent=4, sort_keys=True)

print "Retrieving Yun data from USB drive, starting ..."

path = subprocess.check_output(CONF['mount'], shell=True).split()[-1]
print os.path.join(path, CONF['sensor'])
with open(os.path.join(path, CONF['sensor']), 'rb') as csvfile:
    spamreader = csv.reader(csvfile)#, delimiter=',', quotechar='|')
    data = [row for row in spamreader]

p = []
for i, line in enumerate(data):
    temperature, humidity, Vs = map(float, line[1:4])  # reading may take some time ...
    t = time.strptime(line[0], CONF["fmt_time"])   #
    ts = (time.asctime(t), time.mktime(t))         # ... thus get time afterwards
    p.append( (i, ts[1], temperature, humidity, Vs) )

    output = CONF['fmt_print'] % (ts + (temperature, humidity, Vs))
#    print output

#    output = CONF['fmt_write'] % (ts + (temperature, humidity, Vs))
#    with open(CONF['log_file'], "a") as log:
#        log.write(output + "\n")

    #plt.scatter([ts[1]]*3, [temperature, humidity, Vs], color=['r', 'b', 'g'])

    ##time.sleep(CONF['interval_update'])
    #plt.pause(CONF['interval_update'])

p = np.array(p)
t = p[:,1]
plt.scatter(t, p[:,2], color='r')
plt.scatter(t, p[:,3], color='b')
plt.scatter(t, p[:,4], color='g')
plt.show()
