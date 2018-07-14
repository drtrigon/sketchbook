#!/bin/bash

printf "\x01" | ./owpshell-ubuntu14.04 - - 44
#owp:dg:v1
printf "\x01" | ./owpshell-ubuntu14.04 - - 42
#owp:dg:1w:v1
printf "\x4A" | ./owpshell-ubuntu14.04 - - 42 | ../../../ThroughSerial/RemoteWorker/DeviceGeneric/unpack.py BBBBBBBB
#38, 0, 0, 215, 1, 0, 0, 77
printf "\x4B" | ./owpshell-ubuntu14.04 - - 42 | ../../../ThroughSerial/RemoteWorker/DeviceGeneric/unpack.py BBBBBBBB
#0, 0, 0, 0, 0, 0, 0, 0
printf "\x41" | ./owpshell-ubuntu14.04 - - 42 | ../../../ThroughSerial/RemoteWorker/DeviceGeneric/unpack.py f
#28.09375,
printf "\x42" | ./owpshell-ubuntu14.04 - - 42 | ../../../ThroughSerial/RemoteWorker/DeviceGeneric/unpack.py f
#10.229999542236328,
printf "\x43" | ./owpshell-ubuntu14.04 - - 42 | ../../../ThroughSerial/RemoteWorker/DeviceGeneric/unpack.py f
#4.510000228881836,
printf "\x44" | ./owpshell-ubuntu14.04 - - 42 | ../../../ThroughSerial/RemoteWorker/DeviceGeneric/unpack.py f
#nan,
