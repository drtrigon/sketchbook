#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
...
"""

import time, struct, os

offset = int(time.mktime(time.strptime("Thu Jan  1 00:00:00 1970", "%a %b %d %H:%M:%S %Y")))

def init():
        os.system("ls /mnt/1wire/")
        os.system("ls /mnt/1wire/uncached/")    # (check for sensor !)
        os.system("ls /mnt/1wire/uncached/24.E20000000002/")

        f = open("/mnt/1wire/uncached/24.E20000000002/running", "w")
        f.write("1\n")
        f.close()
        #time.sleep(.5)
        f = open("/mnt/1wire/uncached/24.E20000000002/running", "w")
        f.write("1\n")
        f.close()
        #time.sleep(.5)
        f = open("/mnt/1wire/uncached/24.E20000000002/running", "w")
        f.write("1\n")
        f.close()

def write(val):
        #f = open("/mnt/1wire/uncached/24.E20000000002/udate", "w")
        #f.write("7\n")
        f = open("/mnt/1wire/uncached/24.E20000000002/date", "w")
        f.write(val + "\n")
        f.close()
        #time.sleep(.5)
        f = open("/mnt/1wire/uncached/24.E20000000002/running", "w")
        f.write("1\n")
        f.close()

def read():
        # read continously until valid value available
        i = 0
        while True:
                i += 1
                f = open("/mnt/1wire/uncached/24.E20000000002/date", "r")
                val = f.read()
                f.close()

                print i, val

                # check whether the value is valid
                b = unpack_val(val)[1]
                if not (-1 == struct.unpack('i', b)[0]) or (i > 10):
                        break
        return (val, i)

def pack_val(val):
        b = struct.pack('i', val)
        s = time.strftime("%a %b %d %H:%M:%S %Y", time.localtime(val + offset))
        return (b, s)

def unpack_val(val):
    i = int(time.mktime(time.strptime(val, "%a %b %d %H:%M:%S %Y")))
    v = i - offset
    b = struct.pack('i', v)
#       u = struct.unpack('i', b[...])
    return (v, b)
#    return (v, b, u)

def readall():
        for i in range(11):
                s = pack_val(i)[1]
#               s = "Mon May 22 12:39:30 1961"
                print s
                write(s)
                write(s)
                write(s)

#               time.sleep(.1)

                s = read()[0]
                print s
                b = unpack_val(s)[1]

                # http://stackoverflow.com/questions/12214801/print-a-string-as-hex-bytes
                print " ".join("{:02x}".format(ord(c)) for c in b)

                v = struct.unpack('i', (b[::-1][:-1]+'\x00'))[0]
                print v, (v/1023.*5.)

                time.sleep(1.)

def readall2():
        for i in range(11):
                s = pack_val(i)[1]
#               s = "Mon May 22 12:39:30 1961"
                print s
                while True:
                        write(s)
                        write(s)
                        write(s)
                        time.sleep(.1)
                        (s2, tries) = read()
                        if not (unpack_val(s2)[0] == -1):
                                break
                        if (tries == 11):
                                time.sleep(1.)
                                init()  # reset 'error' and restart reading (owfs)
                b = unpack_val(s2)[1]

                # http://stackoverflow.com/questions/12214801/print-a-string-as-hex-bytes
                print " ".join("{:02x}".format(ord(c)) for c in b)

                v = struct.unpack('i', (b[::-1][:-1]+'\x00'))[0]
                print v, (v/1023.*5.)

                time.sleep(1.)


print 1, pack_val(1)
print 2, pack_val(2)
print 3, pack_val(3)
print struct.unpack('i', '\x01\x23\xCD\xEF'), pack_val(struct.unpack('i', '\x01\x23\xCD\xEF')[0])

(v, b) = unpack_val("Mon May 22 12:39:29 1961")
print (v, b)
# http://stackoverflow.com/questions/12214801/print-a-string-as-hex-bytes
print " ".join("{:02x}".format(ord(c)) for c in b)

init()
time.sleep(1.)

#write("Thu Jan  1 00:00:02 1970")
#time.sleep(3.)
#write("Thu Jan  1 00:00:03 1970")
#time.sleep(3.)
#write("Mon May 22 12:39:29 1961")

#time.sleep(1.)
#readall()
readall2()
