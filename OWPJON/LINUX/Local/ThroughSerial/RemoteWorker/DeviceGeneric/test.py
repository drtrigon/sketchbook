#!/usr/bin/python

# $ sudo python test.py
# $ sudo python test.py owp:dg:v1

# Works with LocalUDP variant also, as owpshell has same syntax (port and
# rate are ignored).

from __future__ import print_function
import subprocess
import unittest
import sys
import os

READ_INFO  = 0x01  # return generic sensor info
READ_VCC   = 0x11  # return supply voltage
READ_TEMP  = 0x12  # return chip temperature
READ       = 0x21  # return memory data
WRITE      = 0x22  # store memory data
WRITE_CAL  = 0x32  # store calibration value in memory

OWPSHELL = ["owpshell",
            "owpshell-ubuntu14.04",]

SERIAL = ["/dev/ttyACM0",
          "/dev/ttyUSB0",]


class TestDevice(object):  # do NOT run this as unittest

    SENSOR = None

    WRITE_DATA = "ABC"
    WRITE_SIZE = 36

    def setUp(self):
        if (sensor_type != self.SENSOR):
            self.skipTest("device not supported by this test")

    def _run(self, cmd, stdin=None):
        # r = subprocess.check_output([cmd], shell=True, input=stdin)
        p = subprocess.Popen([cmd], shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
        if stdin:
            p.stdin.write(stdin)
        p.stdin.close()
        p.wait()
        r = p.stdout.read()
        return r[:-1]  # omitt last char which is a newline '\n'

    def _test(self, cmd, stdin=None):
        r = self._run(cmd, stdin=stdin)
#        print(">" + r + "<")
#        print(">", end='')
#        for c in r:
#            print(hex(ord(c)), end='')
#            print(" ", end='')
#        print("<\n")
        return r

# --- test command line args (param) with READ_INFO and READ_VCC ---
    def test_param_READ_INFO(self):
        # print("param: test READ_INFO")
        # $ sudo ./owpshell /dev/ttyACM0 9600 44 "`printf "\x01"`"  # send "\x01" to id 44 and show result
        # print('$ sudo ./owpshell /dev/ttyACM0 9600 44 "`printf "\\x01"`"')
        # result = self._test('./owpshell /dev/ttyACM0 9600 44 "`printf "\x01"`"')
        result = self._test('./%s %s 9600 44 "`printf "%c"`"' % (OWPSHELL, SERIAL, READ_INFO))
        self.assertEqual(result, self.SENSOR)

    def test_param_READ_VCC(self):
        # print("param: test READ_VCC")
        # $ sudo ./owpshell /dev/ttyACM0 9600 44 "`printf "\x11"`" | python unpack.py f  # send "\x11" to id 44 and unpack result as "f"loat
        # print('$ sudo ./owpshell /dev/ttyACM0 9600 44 "`printf "\\x11"`" | python unpack.py f')
        result = self._test('./%s %s 9600 44 "`printf "%c"`" | python unpack.py f' % (OWPSHELL, SERIAL, READ_VCC))
        self.assertEqual(result, "3.700000047683716,")

# "WRITE = 0x22" will result in python in "/bin/sh: 1: Syntax error: Unterminated quoted string"
# thus use stdin version below - that will always work
# test('./owpshell /dev/ttyACM0 9600 44 "`printf "%cABC"`" | python unpack.py B' % WRITE,
#      "3,")

# --- test stdin (pipe) with all ---
    def test_pipe_READ_INFO(self):
        # print("pipe: test READ_INFO")
        # $ printf "\x01" | sudo ./owpshell /dev/ttyACM0 9600 44
        # print('$ printf "\\x01" | sudo ./owpshell /dev/ttyACM0 9600 44')
        result = self._test('./%s %s 9600 44' % (OWPSHELL, SERIAL),
                            stdin="%c" % READ_INFO)
        self.assertEqual(result, self.SENSOR)

    def test_pipe_READ_VCC(self):
        # print("pipe: test READ_VCC")
        # $ printf "\x11" | sudo ./owpshell /dev/ttyACM0 9600 44 | python unpack.py f
        # print('$ printf "\\x11" | sudo ./owpshell /dev/ttyACM0 9600 44 | python unpack.py f')
        result = self._test('./%s %s 9600 44 | python unpack.py f' % (OWPSHELL, SERIAL),
                            stdin="%c" % READ_VCC)
        self.assertEqual(result, "3.700000047683716,")

    def test_pipe_READ_TEMP(self):
        # print("pipe: test READ_TEMP")
        # $ printf "\x12" | sudo ./owpshell /dev/ttyACM0 9600 44 | python unpack.py f
        # print('$ printf "\\x12" | sudo ./owpshell /dev/ttyACM0 9600 44 | python unpack.py f')
        result = self._test('./%s %s 9600 44 | python unpack.py f' % (OWPSHELL, SERIAL),
                            stdin="%c" % READ_TEMP)
        self.assertEqual(result, "42.41999816894531,")

    @unittest.expectedFailure  # this test may fail if device was not resetted before
    def test_pipe_READ(self):
        # print("pipe: test READ (after reset)")
        # $ printf "\x21" | sudo ./owpshell /dev/ttyACM0 9600 44
        # print('$ printf "\\x21" | sudo ./owpshell /dev/ttyACM0 9600 44')
        result = self._test('./%s %s 9600 44' % (OWPSHELL, SERIAL),
                            stdin="%c" % READ)
        self.assertEqual(result, "\x00" * self.WRITE_SIZE)

    def test_pipe_WRITE(self):
        # print("pipe: test WRITE")
        # $ printf "\x22ABC" | sudo ./owpshell /dev/ttyACM0 9600 44 | python unpack.py B
        # print('$ printf "\\x22%s" | sudo ./owpshell /dev/ttyACM0 9600 44 | python unpack.py B' % self.WRITE_DATA)
        result = self._test('./%s %s 9600 44 | python unpack.py B' % (OWPSHELL, SERIAL),
                            stdin="%c%s" % (WRITE, self.WRITE_DATA))
        # in order to clear the display (" " * self.WRITE_SIZE) would have to be sent first
        resul_ = self._test('./%s %s 9600 44 | python unpack.py B' % (OWPSHELL, SERIAL),
                            stdin="%c%s" % (WRITE, ("\x00" * self.WRITE_SIZE)))  # reset memory for other tests
        self.assertEqual(result, "%i," % len(self.WRITE_DATA))

    # @unittest.skip("disable for debugging")
    def test_pipe_READafterWRITE(self):  # for expected output for read
        # print("pipe: test WRITE")
        # $ printf "\x22ABC" | sudo ./owpshell /dev/ttyACM0 9600 44 | python unpack.py B
        # print('$ printf "\\x22%s" | sudo ./owpshell /dev/ttyACM0 9600 44 | python unpack.py B' % self.WRITE_DATA)
        resul_ = self._test('./%s %s 9600 44 | python unpack.py B' % (OWPSHELL, SERIAL),
                            stdin="%c%s" % (WRITE, self.WRITE_DATA))
        # print("pipe: test READ (after WRITE)")
        # $ printf "\x21" | sudo ./owpshell /dev/ttyACM0 9600 44
        # print('$ printf "\\x21" | sudo ./owpshell /dev/ttyACM0 9600 44')
        result = self._test('./%s %s 9600 44' % (OWPSHELL, SERIAL),
                            stdin="%c" % READ)
        # print("pipe: test WRITE")
        # $ printf "\x22ABC" | sudo ./owpshell /dev/ttyACM0 9600 44 | python unpack.py B
        # print('$ printf "\\x22%s" | sudo ./owpshell /dev/ttyACM0 9600 44 | python unpack.py B' % ("\x00" * self.WRITE_SIZE))
        # in order to clear the display (" " * self.WRITE_SIZE) would have to be sent first
        resul_ = self._test('./%s %s 9600 44 | python unpack.py B' % (OWPSHELL, SERIAL),
                            stdin="%c%s" % (WRITE, ("\x00" * self.WRITE_SIZE)))  # reset memory for other tests
        self.assertEqual(result, self.WRITE_DATA + "\x00" * (self.WRITE_SIZE - len(self.WRITE_DATA)))

    def test_pipe_WRITE_CAL(self):
        # print("pipe: test WRITE_CAL")
        import struct
        # $ printf "\x32`python pack.py f 7.1`" | sudo ./owpshell /dev/ttyACM0 9600 44 | python unpack.py f
        # print('$ printf "\\x32`python pack.py f 7.1`" | sudo ./owpshell /dev/ttyACM0 9600 44 | python unpack.py f')
        result = self._test('./%s %s 9600 44 | python unpack.py f' % (OWPSHELL, SERIAL),
                            stdin="%c%s" % (WRITE_CAL, struct.pack("f", 7.1)))
        self.assertEqual(result, "8.199999809265137,")


class TestDeviceGeneric(TestDevice, unittest.TestCase):  # run this as unittest

    SENSOR = "owp:dg:v1"


class TestDeviceLCD(TestDevice, unittest.TestCase):  # run this as unittest

    SENSOR = "owp:dg:lcd:v1"

    # WRITE_DATA = "abcdefghijklmnopqrstuvwxyz"
    WRITE_DATA = "123456789012345678901234567890"
# TODO: !!! TEST FULL STRING LENGHT OF 8x21 = 168 bytes/chars !!!


class TestDeviceTiny(TestDevice, unittest.TestCase):  # run this as unittest

    SENSOR = "owp:dg:tiny:v1"

    WRITE_SIZE = 14  # more does not work
# TODO: !!! GET LONGER DATA STRINGS WORKING !!!


#class TestDeviceSensorStation(TestDevice, unittest.TestCase):  # run this as unittest
#
#    SENSOR = "owp:ss:v1"


def get_sensor_type():
    # $ sudo ./owpshell /dev/ttyACM0 9600 44 "`printf "\x01"`"  # send "\x01" to id 44 and show result
    # print('$ sudo ./owpshell /dev/ttyACM0 9600 44 "`printf "\\x01"`"')
    # run('./owpshell /dev/ttyACM0 9600 44 "`printf "\x01"`"',
    return TestDevice._run(TestDevice(), './%s %s 9600 44 "`printf "%c"`"' % (OWPSHELL, SERIAL, READ_INFO))

def find_ressource(ressources):
    for item in ressources:
        if os.path.exists(item):
            return item
    return None


if __name__ == '__main__':
    print("unittest for OWPJON devices")
    print("usage: %s [device-test]\n\n"
          "device under test (DUT) must have ID 44\n\n"
          "options for device-test:" % sys.argv[0])
    for k in locals().keys():
        if ("Test" in k) and (locals()[k].SENSOR):
            print("  %s" % locals()[k].SENSOR)
    OWPSHELL = find_ressource(OWPSHELL)
    if not OWPSHELL:
        raise BaseException("owpshell binary could not be found")
    SERIAL = find_ressource(SERIAL)
    if not SERIAL:
        raise BaseException("serial port could not be found")
    if (len(sys.argv) < 2):
        sensor_type = get_sensor_type()
    else:
        sensor_type = sys.argv[1]
    print("type: %s\n\n" % sensor_type)
    print("owpshell: %s\n" % OWPSHELL)
    print("serial: %s\n" % SERIAL)
    del sys.argv[1:]
    unittest.main(verbosity=2)
