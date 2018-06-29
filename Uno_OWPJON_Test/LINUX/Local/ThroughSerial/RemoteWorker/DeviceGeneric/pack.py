# $ python pack.py i 7 | python unpack.py i
# $ printf "*`printf "\xFF"``python pack.py f 7.1`*"
# $ printf "`printf "\xFF"``python pack.py f 7.1`" | sudo ./owpshell /dev/ttyACM0 9600 44 | python unpack.py f  # send "\xFF" and a float to id 44 and unpack result as "f"loat
# For more info and further examples see unittest.py.

import sys
import struct

print(struct.pack(sys.argv[1], float(sys.argv[2])))
