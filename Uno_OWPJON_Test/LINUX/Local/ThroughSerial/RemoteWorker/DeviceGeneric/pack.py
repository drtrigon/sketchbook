#!/usr/bin/python

# For more info and further examples see test.py and README.md.

import sys
import struct

print(struct.pack(sys.argv[1], float(sys.argv[2])))
