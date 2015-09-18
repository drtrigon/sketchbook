#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
...
Uno_OWSlave / OWGeneric
Uno_OWSlave / OWGeneric_SensorStation

prepare:
pi@raspberrypi ~ $ sudo mkdir /mnt/1wire-generic
pi@raspberrypi ~ $ sudo chmod o+w /mnt/1wire-generic

http://serverfault.com/questions/82857/prevent-duplicate-cron-jobs-running

crontab -e
#* * * * * /usr/bin/flock -n /tmp/fcj.lockfile /usr/local/bin/frequent_cron_job --minutely
* * * * * /usr/bin/flock -n /tmp/fcj.lockfile python /home/pi/owgeneric_arduino.py
#*/2 * * * * /usr/bin/flock -n /tmp/fcj.lockfile python /home/pi/owgeneric_arduino.py
"""

ow_root        = "/mnt/1wire/"
owgeneric_root = "/mnt/1wire-generic/"


import time, struct
import os, shutil

#import signal
#from sys import exit
#import atexit

offset = int(time.mktime(time.strptime("Thu Jan  1 00:00:00 1970", "%a %b %d %H:%M:%S %Y")))

__debug = False

def debug(switch):
        global __debug
        __debug = switch

#def cleanup():
#       shutil.rmtree(owgeneric_root)
#       print "exit"

def init(slave):
        if __debug:
                dbg = ""
        else:
                dbg = " >/dev/null"
        os.system("ls %s%s" % (os.path.join(ow_root, ""), dbg))
        os.system("ls %s%s" % (os.path.join(ow_root, "uncached", ""), dbg))   # (check for sensor !)
        os.system("ls %s%s" % (os.path.join(ow_root, "uncached", slave, ""), dbg))

        for i in range(3):
                f = open(os.path.join(ow_root, "uncached", slave, "running"), "w")
                f.write("1\n")
                f.close()

def write(slave, val):
        f = open(os.path.join(ow_root, "uncached", slave, "date"), "w")
        f.write(val + "\n")
        f.close()
        f = open(os.path.join(ow_root, "uncached", slave, "running"), "w")
        f.write("1\n")
        f.close()

def read(slave):
        # read continously until valid value available
        dev = os.path.join(ow_root, "uncached", slave, "date")
        i = 0
        while True:
                i += 1
                f = open(dev, "r")
                val = f.read()
                f.close()

                if __debug:
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

def receive_data(address):
#       s = pack_val(address)[1]
        (b, s) = pack_val(address)
        if __debug:
                print address
#               print " ".join("{:02x}".format(ord(c)) for c in b),
                print s
#       process_delay = 0.1
        process_delay = 0.2
        #while True:
        for t in range(3):
# TODO: why is the first read needed here; optimize/debug arduino code
                read(slave)    # needed; otherwise the write/read sequence fails resp. lacks behind (?!) - but 1 write is enough now (3x faster!)
                write(slave, s)
                time.sleep(process_delay)
                (s2, tries) = read(slave)
                if not (unpack_val(s2)[0] == -1):
                        break
                if (tries == 11):
#                       if (t > 0):
#                               process_delay = 3.0
                        time.sleep(1.)
                        init(slave)  # reset 'error' and restart reading (owfs)
        if (unpack_val(s2)[0] == -1):   # not successfull after 3 tries - stop the script to stop occupying the bus
                return None
        b = unpack_val(s2)[1]

        if __debug:
                # http://stackoverflow.com/questions/12214801/print-a-string-as-hex-bytes
                print " ".join("{:02x}".format(ord(c)) for c in b)

#       v = struct.unpack('i', (b[::-1][:-1]+'\x00'))[0]
        v = struct.unpack('f', b)[0]
        if __debug:
                print v

        return (v, b, s2)

def read_slave_sensors(slave):
        sensor_count = receive_data(struct.unpack('i', "\xFF\x01\x00\x00")[0])[0]
        ver = receive_data(struct.unpack('i', "\xFF\x02\x00\x00")[0])[0]
        rtc = receive_data(struct.unpack('i', "\xFF\x03\x00\x00")[0])[0]
        vcc = receive_data(struct.unpack('i', "\xFF\x04\x00\x00")[0])[0]
        if __debug:
                print "Sensor Count:", sensor_count
                print "Software Ver:", ver
                print "     Up-Time:", rtc
                print "         Vcc:", vcc
        meta = [sensor_count, ver, rtc, vcc]

        data = []
        for i in range(1, int(sensor_count)+1):
                data.append( receive_data(i) )

        return (data, meta)

def get_slaves():
        if __debug:
                dbg = ""
        else:
                dbg = " >/dev/null"
        for i in range(2):
                os.system("ls %s%s" % (os.path.join(ow_root, ""), dbg))
                os.system("ls %s%s" % (os.path.join(ow_root, "uncached", ""), dbg))   # (check for sensor !)
        slaves = []
        if os.path.exists(os.path.join(ow_root, "uncached")):
                contents = os.listdir(os.path.join(ow_root, "uncached"))
                for c in contents:
                        if "24.E2" in c:
                                # ... and may be check os.path.join(ow_root, "uncached", c, "running") for '0' as well
                                # OWGeneric_... Arduino Uno device
                                if __debug:
                                        print "OWGeneric Arduino Uno device"
                                slaves.append(c)
        return slaves


if __name__ == '__main__':
#       # http://stackoverflow.com/questions/18499497/how-to-process-sigterm-signal-gracefully
#       # http://code.activestate.com/recipes/533117-cleaning-up-when-killed/
#       atexit.register(cleanup)
#       # Normal exit when killed (redir SIGKILL to exit)
#       signal.signal(signal.SIGTERM, lambda signum, stack_frame: exit(1))
#       signal.signal(signal.SIGINT, lambda signum, stack_frame: exit(1))

        debug(False)
#       debug(True)

#       os.makedirs(owgeneric_root)

        slaves = []
        while True:
                #slaves_old = slaves
                slaves_old = os.listdir(owgeneric_root)
                slaves = get_slaves()

                if __debug:
                        print slaves

                for slave in slaves_old:
                        if slave not in slaves:
                                shutil.rmtree(os.path.join(owgeneric_root, slave))

                # when re-entering the script; how to know which sensor to read next?
                # find the one with oldest 'all' file to know what to update
                min_mtime = time.time()
                process_slaves = []
                for slave in slaves:
                        dev = os.path.join(owgeneric_root, slave)
                        if not os.path.exists(dev):
                                os.mkdir(dev)

                                fn = os.path.join(dev, "all")
                                f = open(fn, "w")
                                f.write("")
                                f.close()

                                fn = os.path.join(dev, "error")
                                f = open(fn, "w")
                                f.write("0")
                                f.close()

                        (mode, ino, dev, nlink, uid, gid, size, atime, mtime, ctime) = os.stat(os.path.join(dev, "all"))
                        if mtime < min_mtime:
                                min_mtime = mtime
                                process_slaves.append(slave)

                if __debug:
                        print process_slaves

                #for slave in slaves:
                for slave in process_slaves:
                        dev = os.path.join(owgeneric_root, slave)

                        tic = time.time()
                        (data, meta) = read_slave_sensors(slave)
                        toc = time.time()

                        if __debug:
                                print data
                                print toc-tic

                        if (data == None):
                                fn = os.path.join(dev, "error")
                                f = open(fn, "r")
                                err_count = int(f.read())
                                f.close()
                                data = [ (err_count+1,) ]
                        else:
                                fn = os.path.join(dev, "all")

                        fn_temp = fn + "-" + str(time.time())
                        f = open(fn_temp, "w")
                        f.write(" ".join([str(d[0]) for d in data]))
                        f.close()
                        os.rename(fn_temp, fn)
                        # Rename the file or directory src to dst. If dst is a directory,
                        # OSError will be raised. On Unix, if dst exists and is a file,
                        # it will be replaced silently if the user has permission. The
                        # operation may fail on some Unix flavors if src and dst are on
                        # different filesystems. If successful, the renaming will be an
                        # atomic operation (this is a POSIX requirement). On Windows,
                        # if dst already exists, OSError will be raised even if it is a
                        # file; there may be no way to implement an atomic rename when
                        # dst names an existing file.

#                       time.sleep(max((60. - (toc-tic)), 0.)
                break
