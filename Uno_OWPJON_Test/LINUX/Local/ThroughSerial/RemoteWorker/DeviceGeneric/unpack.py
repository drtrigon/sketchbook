# $ sudo ./owpshell 44 "`echo -e "\x01"`"  # send "\x01" to id 44 and show result
# $ sudo ./owpshell 44 "`echo -e "\x02"`" | python unpack.py f  # send "\x02" to id 44 and unpack result as "f"loat
# For more info and further examples see unittest.py.

import sys
import struct

#print(struct.unpack(sys.argv[1], sys.stdin.readline()[:-1]))
print(str(struct.unpack(sys.argv[1], sys.stdin.readline()[:-1]))[1:-1])
