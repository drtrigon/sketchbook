#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
@brief Example-Code for a generic Server (e.g. Sensor, Display)

@file OWPJON/PJON-cython/DeviceGeneric.py

@see OWPJON/LINUX/Local/LocalUDP/RemoteWorker/DeviceGeneric/DeviceGeneric.cpp

@author drtrigon
...
"""

# clone and build manually:
# osboxes@osboxes:~$ git clone https://github.com/xlfe/PJON-cython
# osboxes@osboxes:~$ cd PJON-cython/
# osboxes@osboxes:~/PJON-cython$ git pull && git submodule init && git submodule update && git submodule status
# osboxes@osboxes:~/PJON-cython$ python setup.py build_ext --inplace
#
# may be you also need wiringPi for serial support:
# osboxes@osboxes:~$ git clone git://git.drogon.net/wiringPi
# osboxes@osboxes:~$ cd wiringPi/
# osboxes@osboxes:~/wiringPi$ ./build
# osboxes@osboxes:~/wiringPi$ cd ..
#
# may be you want to install it via pip instead:
# osboxes@osboxes:~$ sudo pip install pjon-cython
#
# run tests:
# osboxes@osboxes:~/PJON-cython$ python setup.py nosetests --with-doctest --doctest-extension=md
#
# osboxes@osboxes:~/sketchbook/OWPJON/PJON-cython$ ./test-OWPJON.sh
# owp:dg:v1
# owp:1w:v1

from __future__ import print_function

import pjon_cython as PJON
import sys
import time

# use different ID for different machines (linux, raspi, ...)
ID = 45  # default ID for linux (raspi uses 46)


class LocalUDP(PJON.LocalUDP):
#class ThroughSerial(PJON.ThroughSerial):

    """..."""

    def receive(self, data, length, packet_info):
        """..."""
        #print ("Recv ({}): {}".format(length, data))
        #print (packet_info)
        print(data)
        sys.exit(0)

#    def error(self, code, data, custom_pointer):
#        print("ERROR", file=sys.stderr)

#void error_handler(uint8_t code, uint16_t data, void *custom_pointer)
#{
#  if(code == PJON_CONNECTION_LOST) {
#    fprintf(stderr, "Connection with device ID %i is lost.\n", bus.packets[data].content[0]);
#  }
#  if(code == PJON_PACKETS_BUFFER_FULL) {
#    fprintf(stderr, "Packet buffer is full, has now a length of %i\n", data);
#    fprintf(stderr, "Possible wrong bus configuration!\n");
#    fprintf(stderr, "higher PJON_MAX_PACKETS in PJONDefines.h if necessary.\n");
#  }
#  if(code == PJON_CONTENT_TOO_LONG) {
#    fprintf(stderr, "Content is too long, length: %i\n", data);
#  }
#}


bus = LocalUDP(ID)
#bus = ThroughSerial(ID)

if ((len(sys.argv) == 2) and (sys.argv[1] == "--id")):
    import os
    import datetime
    print(datetime.datetime.fromtimestamp(os.path.getmtime(__file__)))
    print("ID: %i" % ID)
    print("BC: %i" % PJON.PJON_BROADCAST)
    sys.exit(0)

# Welcome to RemoteWorker 1 (Transmitter)

# Opening bus...

# Attempting to send a packet...
#bus.send(44, b'\x01')
if (len(sys.argv) < 5):  # get data to send from stdin or command line (stdin allows \0)
    buf = sys.stdin.read()
else:
    buf = sys.argv[4]
#l = len(buf)
#for i in range(3):  # try 3 times (using timeout), then exit
# clean bus from old (hanging) messages
time.sleep(.1)
#bus.update()
#bus.receive(1000)
#bus.set_receiver(receiver_function)

bus.send(int(sys.argv[3]), buf)

timer0 = time.time()  # get current time
timer1 = time.time()

ret = -1
while ((timer1 - timer0) < 3.):  # 3s timeout
    time.sleep(.01)              # multi-threading - give os and socket time as we have hardware buffer for UDP
    try:
        #bus.update()
        #ret = bus.receive(1000)      # 1ms timeout
        packets_to_send, ret = bus.loop()
        if (ret == PJON.PJON_ACK):
            sys.exit(0)              # Success!
        elif (ret == PJON.PJON_NAK):
            print("NAK: %i" % ret, file=sys.stderr)
        elif (ret == PJON.PJON_BUSY):     # wait 1s and restart timeout - allow bus to cool down
            print("BUSY: %i" % ret, file=sys.stderr)
            time.sleep(1.0)
#            j = 0
# TODO: restric maximum number of timeout resets
            timer0 = time.time()     # reset timeout
        elif (ret == PJON.PJON_FAIL):
            pass
        else:
            pass
    except:  # e.g. PJON_Connection_Lost
        print("Unexpected error:", sys.exc_info()[0])
        raise
    timer1 = time.time()

print("FAILED: %i" % ret, file=sys.stderr)
sys.exit(1)  # failed
