#!/usr/bin/python

# For more info and further examples see test.py and README.md.

import sys
import struct

print(str(struct.unpack(sys.argv[1], sys.stdin.readline()[:-1]))[1:-1])
