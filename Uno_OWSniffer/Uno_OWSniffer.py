#!/usr/bin/python
# -*- coding: utf-8 -*-

# http://blog.oscarliang.net/use-arduino-as-usb-serial-adapter-converter/
# https://www.arduino.cc/en/uploads/Main/arduino-uno-schematic.pdf
# Use Method 1: Jumper RESET pin to GND
#               then Connect 1wire to pin 1 (TX) and GND
# see Fritzing Schemes: Uno_OWSniffer.fzz
# use Sniffer Shield PCB in order to setup the Arduino (RESET jumper)
#   and connect it to 1wire bus (RJ45 header)
#
# TUTORIAL 214: Using a UART to Implement a 1-Wire Bus Master
# http://dangerousprototypes.com/2012/10/28/app-note-using-uart-to-implement-a-1-wire-bus-master/
# http://www.maximintegrated.com/en/app-notes/index.mvp/id/214
# RESET and PRESENCE: 9600 Baud, 8 bits, no parity, 1 stop
# READ and WRITE: 115200 Baud, 8 bits, no parity, 1 stop
# (fast switching of baud rates is possible, but we cannot
#  control the possition in the input buffer, thus it's lossy)
#
# $ miniterm.py -D -D -D /dev/ttyACM0 9600
# $ miniterm.py -D -D -D /dev/ttyACM0 115200
#
# TODO:
# * pulse shape/timing analysis - look at bit sequence
# * read/write 0 detection (write 0 0x80 ? vs. read 0 0xFE and lower ? saw a
#   lot of 0xfc and 0x80 ...)
# * buffering of data on all levels for debug access/view
# * detect RESET w/o PRESENCE (0x00 all other commands give at least a
#   0x80 - with Arduino Uno on OWServer ENET)
# * make it work with adaptive LinkHubE properly (currently working with
#   OW_SERVER_ENET only)

import serial, time


RESET       = ['\xF0', '\xE0', '\x90', ]
RESET115200 = ['\x00\x00', ]

# http://owfs.sourceforge.net/simple_commands.html
commands = {
    0x55: ("MATCH_ROM", 7),    # (command, # of data bytes)
    0xF0: ("SEARCH_ROM", 0),
}


def sniff_TUT214():
    while True:
        # RESET & PRESENCE DETECTION
        # http://stackoverflow.com/questions/9271777/change-baudrate-in-pyserial-while-connected-to-device
        ser.baudrate = 9600
        ser.flushInput()

        waiting = 0
        while (not waiting):
            waiting = ser.inWaiting()
        bytes = ser.read(waiting)

        # SNIFFER: READ & WRITE DETECTION
        # do this first - since need to be fast - before new byte comes
        # into buffer
        ser.baudrate = 115200
        ser.flushInput()

        if (waiting == 1):
            # F0 - 1111 0000 - Master RESET w/o Slave PRESENCE
            # E0 - 1110 0000 - Master RESET w Slave PRESENCE
            #                  (minimum internal timing)
            # 90 - 1001 0000 - Master RESET w Slave PRESENCE
            #                  (maximum internal timing)
            if (bytes[0] in RESET):
                print 'RESET-0x' + bytes[0].encode('hex').upper()
            else:
                print 'ERROR-RESET-0x' + bytes[0].encode('hex').upper()
        elif (waiting > 1):
            print "Missed parts of the byte sequence. LEN =", (waiting - 1)
            print 'MISS: 0x' + bytes.encode('hex').upper()
            continue

        waiting = 0
        while (not waiting):
            waiting = ser.inWaiting()
        bytes = ser.read(waiting)

        # WORK/PROCESS: Do some other work on the data
        print "SNIFF:"
        a, c = [], []
        for b in bytes:
            a.append(hex(ord(b)))
            c.append(int(ord(b) == 0xff))
        print a
        print c

#        time.sleep(300) # sleep 5 minutes

        # Loop restarts once the sleep is finished


def sniff_115200():
    bytes   = ""
    bits    = 0x00
    sc      = 0
    onewire = []
    t       = 0
    while True:
        ser.baudrate = 115200
        ser.flushInput()

        # Receive and buffer the data
        waiting = 0
        while (not waiting):
            waiting = ser.inWaiting()
        bytes += ser.read(waiting)

        # Do some other work on the data
        while bytes:
            # RESET & PRESENCE DETECTION
            # @115200: 00 00
            if (bytes[:2] in RESET115200):
                # print "\nRESET",
                if onewire and (onewire[1] in commands):
                    cmd = commands[onewire[1]]
                    end = (2+cmd[1])
                    print ("\n%.3f   " % t) + cmd[0] +\
                          "(" + hex(onewire[1]) + ") " +\
                          "".join(map(chr,
                                      onewire[2:end])).encode('hex').upper(),
                if (sc % 8):
                    print "\n%.3f   ERROR: %i bits not used! bit/bus-error?" \
                          % (t, sc),
                # print '0x' + bytes[:2].encode('hex').upper()
                t = time.time()
                print "\n%.3f RP" % t,
                onewire = ['RP']
                sc = 0
                bytes = bytes[2:]
            # SNIFFER: READ & WRITE DETECTION
            else:
                # print '0x' + bytes[0].encode('hex').upper(),
                # print int(ord(bytes[0]) == 0xff),
                bits = bits>>1
                # READ/WRITE 1 (otherwise it's READ/WRITE 0)
                if (ord(bytes[0]) == 0xff):
                    bits |= 0b10000000
                sc = (sc + 1) % 8
                if (sc == 0):
                    # print hex(bits),
                    print chr(bits).encode('hex').upper(),
                    onewire.append(bits)
                    # print bin(bits),
                bytes = bytes[1:]

    # Loop restarts once the sleep is finished


ser = serial.Serial('/dev/ttyACM0')

# sniffer according to official Standards/Specs given in
# TUTORIAL 214 (RESET @9600)
#sniff_TUT214()
# sniffer always @115200 (possible to implement)
sniff_115200()

ser.close()  # Only executes once the loop exits
