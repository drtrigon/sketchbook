#!/bin/bash

#make; mv owpshell owpshell-ubuntu14.04

#printf "\x01" | python DeviceGeneric.py - - 99

printf "\x01" | python DeviceGeneric.py - - 44
#owp:dg:v1
#printf "\x11" | python DeviceGeneric.py - - 44 | ./unpack-test.py f
##sleep 1
#printf "\x12" | python DeviceGeneric.py - - 44 | ./unpack-test.py f

if [ "$1" != "" ]; then
    printf "\x01" | python DeviceGeneric.py - - 43
#    #printf "\x11" | python DeviceGeneric.py - - 43 | ./unpack-test.py f
#    #printf "\x12" | python DeviceGeneric.py - - 43 | ./unpack-test.py f
#    #define _READ_VALUE_TEMP  0xFE  // temporary value
#    printf "\xFE" | python DeviceGeneric.py - - 43 | ./unpack-test.py f
#    #define _READ_VALUE_HUM   0xFD  // temporary value
#    printf "\xFD" | python DeviceGeneric.py - - 43 | ./unpack-test.py f
#    #define _READ_VALUE_PRES  0xFC  // temporary value
#    printf "\xFC" | python DeviceGeneric.py - - 43 | ./unpack-test.py f
#    #define _READ_VALUE_ILLU  0xFB  // temporary value
#    printf "\xFB" | python DeviceGeneric.py - - 43 | ./unpack-test.py f
#    #printf "\xFF" | python DeviceGeneric.py - - 43 | ./unpack-test.py f
fi

printf "\x01" | python DeviceGeneric.py - - 42
#owp:dg:1w:v1
##printf "\x11" | python DeviceGeneric.py - - 42 | ./unpack-test.py f
###sleep 1
##printf "\x12" | python DeviceGeneric.py - - 42 | ./unpack-test.py f
#printf "\x4A" | python DeviceGeneric.py - - 42 | ./unpack-test.py BBBBBBBB
##38, 0, 0, 215, 1, 0, 0, 77
#printf "\x4B" | python DeviceGeneric.py - - 42 | ./unpack-test.py BBBBBBBB
##0, 0, 0, 0, 0, 0, 0, 0
#printf "\x41" | python DeviceGeneric.py - - 42 | ./unpack-test.py f
##28.09375,
#printf "\x42" | python DeviceGeneric.py - - 42 | ./unpack-test.py f
##10.229999542236328,
#printf "\x43" | python DeviceGeneric.py - - 42 | ./unpack-test.py f
##4.510000228881836,
#printf "\x44" | python DeviceGeneric.py - - 42 | ./unpack-test.py f
##nan,
