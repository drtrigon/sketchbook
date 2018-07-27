#!/bin/bash

printf "\x01" | ./owpshell-ubuntu14.04 - - 44
#owp:dg:v1
printf "\x11" | ./owpshell-ubuntu14.04 - - 44 | ./unpack-test.py f
#sleep 1
printf "\x12" | ./owpshell-ubuntu14.04 - - 44 | ./unpack-test.py f

printf "\x01" | ./owpshell-ubuntu14.04 - - 43
printf "\x11" | ./owpshell-ubuntu14.04 - - 43 | ./unpack-test.py f
printf "\x12" | ./owpshell-ubuntu14.04 - - 43 | ./unpack-test.py f
printf "\xFF" | ./owpshell-ubuntu14.04 - - 43 | ./unpack-test.py f

printf "\x01" | ./owpshell-ubuntu14.04 - - 42
#owp:dg:1w:v1
#printf "\x11" | ./owpshell-ubuntu14.04 - - 42 | ./unpack-test.py f
##sleep 1
#printf "\x12" | ./owpshell-ubuntu14.04 - - 42 | ./unpack-test.py f
printf "\x4A" | ./owpshell-ubuntu14.04 - - 42 | ./unpack-test.py BBBBBBBB
#38, 0, 0, 215, 1, 0, 0, 77
printf "\x4B" | ./owpshell-ubuntu14.04 - - 42 | ./unpack-test.py BBBBBBBB
#0, 0, 0, 0, 0, 0, 0, 0
printf "\x41" | ./owpshell-ubuntu14.04 - - 42 | ./unpack-test.py f
#28.09375,
printf "\x42" | ./owpshell-ubuntu14.04 - - 42 | ./unpack-test.py f
#10.229999542236328,
printf "\x43" | ./owpshell-ubuntu14.04 - - 42 | ./unpack-test.py f
#4.510000228881836,
printf "\x44" | ./owpshell-ubuntu14.04 - - 42 | ./unpack-test.py f
#nan,
