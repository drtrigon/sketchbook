#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
...
Uno_OWSlave / OWGeneric
Uno_OWSlave / OWGeneric_SensorStation
"""

import time, struct, os

offset = int(time.mktime(time.strptime("Thu Jan  1 00:00:00 1970", "%a %b %d %H:%M:%S %Y")))

debug = False

def debug(switch):
        global debug
        debug = switch

def init():
        if debug:
                dbg = ""
        else:
                dbg = " >/dev/null"
        os.system("ls /mnt/1wire/" + dbg)
        os.system("ls /mnt/1wire/uncached/" + dbg)    # (check for sensor !)
        os.system("ls /mnt/1wire/uncached/24.E20000000002/" + dbg)

        for i in range(3):
                f = open("/mnt/1wire/uncached/24.E20000000002/running", "w")
                f.write("1\n")
                f.close()

def write(val):
        f = open("/mnt/1wire/uncached/24.E20000000002/date", "w")
        f.write(val + "\n")
        f.close()
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

                if debug:
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
#       return (v, b, u)


def receive_data():
        data = []
        for i in range(11):
                s = pack_val(i)[1]
                if debug:
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

                if debug:
                        # http://stackoverflow.com/questions/12214801/print-a-string-as-hex-bytes
                        print " ".join("{:02x}".format(ord(c)) for c in b)

                v = struct.unpack('i', (b[::-1][:-1]+'\x00'))[0]
                if debug:
                        print v, (v/1023.*5.)

                data.append( (v, b, s2) )

        return data


if __name__ == '__main__':
        debug(False)

        tic = time.time()
        data = receive_data()
        toc = time.time()

        print data
        print toc-tic
