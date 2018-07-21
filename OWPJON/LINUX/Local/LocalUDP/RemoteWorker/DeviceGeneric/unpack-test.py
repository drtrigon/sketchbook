#!/usr/bin/python

# For more info and further examples see test.py and README.md.
# Output is per default thought to be list of floats.

import sys
import struct

data = sys.stdin.read()
try:
    v = struct.unpack(sys.argv[1], data[:-1])
except:
    v = [float('nan')]  # 'nan' is a valid float signing an error
#print(str(v)[1:-1])
size = len(data[:-1])
byte = struct.unpack("B" * size, data[:-1])
print("%-30s %s" % (str(v)[1:-1], "HEX: " + ("0x%02X " * size) % byte))