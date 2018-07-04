#!/usr/bin/python

# For more info and further examples see test.py and README.md.
# Output is per default thought to be list of floats.

import sys
import struct

try:
    v = struct.unpack(sys.argv[1], sys.stdin.readline()[:-1])
except:
    v = [float('nan')]  # 'nan' is a valid float signing an error
print(str(v)[1:-1])