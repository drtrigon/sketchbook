
PJON 1-wire bus/network (OWPJON or 1wPJON)
------------------------------------------

Meant to replace all 1wire stuff (Dallas/Maxim commercial) some day.
Meantwhile both busses/systems can be used together.
The SoftwareBitBang (SWBB) part is the OWPJON or 1wPJON bus.

```
-----------------           -----------------           ---------------
| kubuntu 14.04 |           | Arduino Uno   |           | Arduino Yun |
| ThroughSerial |   <--->   | ThroughSerial |           |             |
|               |           | SWBB pin 7    |   <--->   | SWBB pin 12 |
| Server        |           | Master        |           | Slaves      |
-----------------           -----------------           ---------------

Server: LINUX/Local/ThroughSerial/RemoteWorker/Transmitter/
        (kubuntu 14.04, TS)
Master: ARDUINO/Local/ThroughSerial/SoftwareBitBangSurrogate/Surrogate/
        (Uno, TS, SWBB pin 7)
Slaves: ARDUINO/Local/SoftwareBitBang/BlinkWithResponse/Receiver/
        (Yun, SWBB pin12)
```

Notes:
- putting `bus.set_synchronous_acknowledge(false);` before `bus.begin();`
  in both, the Receiver and the Transmitter makes TS run stable
- the terms "Master" and "Slave" might be missleading, as basically
  communication between all devices in any direction is possible

Arduino Uno and Yun can basically be any kind of AVR/Arduino (as supported
by PJON), no special needs except for a serial-usb port on one of them.

Linux parts (server) are derived from Raspi (RPI) and therefore need
wiringPi in order to compile succesfully:

    $ git clone git://git.drogon.net/wiringPi
    $ cd wiringPi
    $ ./build

    $ make
    $ sudo ./Transmitter

This setup can be used for developping, testing and also as a first
productive system.

There are already solutions around like PJON-gRPC or ModuleInterface:
https://github.com/fredilarsen/ModuleInterface/tree/master/examples/WebPage
Currently both are hard to compile and understand, but that will change
(PJON-gRPC is easier to compile in ubuntu 16.10, docu of ModuleInterface
is up to be improved).


```
-----------------           -----------------           ---------------
| kubuntu 14.04 |           | Arduino Uno   |           | Arduino Yun |
| ThroughSerial |   <--->   | ThroughSerial |           |             |
|               |           | SWBB pin 7    |   <--->   | SWBB pin 12 |
| Server        |           | Master        |           | Slaves      |
-----------------           -----------------           ---------------

Server: LINUX/Local/ThroughSerial/RemoteWorker/DeviceGeneric/
        (kubuntu 14.04, TS)
Master: ARDUINO/Local/ThroughSerial/SoftwareBitBangSurrogate/Surrogate/
        (Uno, TS, SWBB pin 7)
Slaves: ARDUINO/Local/SoftwareBitBang/DeviceGeneric/
        ARDUINO/Local/SoftwareBitBang/OWP_DG_LCD_Sensors/
        ATTINY/Local/SoftwareBitBang/DeviceGeneric/
        (Yun, SWBB pin12)
```

Notes:
- the attiny code uses 5554 bytes (67%) compiled (may be less with LTO), see
  https://github.com/drtrigon/sketchbook/blob/result/docu/OWPJON/ATTINY/Local/SoftwareBitBang/DeviceGeneric/DeviceGeneric.ino.compile#L55

The program `owpshell` (DeviceGeneric) allows e.g. shell/command line
access to the bus (similar to owshell read and write funcs). That can
also be used from collectd e.g. It supports passing of data as command
line parameter and via stdin (pipe). Later supports all posible chars.

Read device info:

    $ printf "\x01" | sudo ./owpshell /dev/ttyACM0 9600 44

Write to device memory and read for verification:

    $ printf "\x22ABC" | sudo ./owpshell /dev/ttyACM0 9600 44 | python unpack.py B
    $ printf "\x21" | sudo ./owpshell /dev/ttyACM0 9600 44

Write a float e.g. for calibration/configuration of device:

    $ printf "\x32`python pack.py f 7.1`" | sudo ./owpshell /dev/ttyACM0 9600 44 | python unpack.py f

As can be seen the un-/packing of values (casting from/to binary representation)
is done by two python script `unpack.py` and `pack.py`. Furthermore there is also
a unittest python script called `test.py`.

Automatically detect the device type and test it:

    $ sudo python test.py

Force to test against a specific device type:

    $ sudo python test.py owp:dg:v1

Now the ThroughSerial (TS) part can be replaced by LocalUDP (LUDP may be over wifi?)
according to:

The setup can also be modified by replacing the ThroughSerial (TS) part by
LocalUDP (LUDP may be over wifi?). This needs an EthernetShield for the
Arduino Uno used as Master.

```
-----------------           -----------------           ---------------
| kubuntu 14.04 |           | Arduino Uno   |           | Arduino Yun |
| LocalUDP      |   <--->   | LocalUDP      |           |             |
|               |           | SWBB pin 7    |   <--->   | SWBB pin 12 |
| Server        |           | Master        |           | Slaves      |
-----------------           -----------------           ---------------

Server: LINUX/Local/LocalUDP/RemoteWorker/DeviceGeneric/
        (kubuntu 14.04, LUDP)
Master: ARDUINO/Local/SoftwareBitBang/Tunneler/BlinkingSwitch/
        (Uno+EthernetShield W5100 based, LUDP, SWBB pin 7)
Slaves: (all as mentioned before)
```

In this case the `owpshell` program works exactly the same except for the fact that it
does not need any serial port info (and thus no sudo either):

    $ printf "\x01" | ./owpshell - - 44



RASPBERRY PI (1) SETUP

We now want to setup this schemes (productive system):

```
-----------------           ---------------           ---------------
| linuxpc/raspi |           | Arduino+ETH |           | Arduino/AVR |
| LocalUDP      |   <--->   | LocalUDP    |           |             |
|               |           | SWBB        |   <--->   | SWBB        |
| Server        |           | Master      |           | Slaves      |
-----------------           ---------------           ---------------
```

Installation of g++ 4.8 is needed if missing due to '-std=c++11' on Wheezy
follow these steps:

```
pi@raspberrypi ~ $ cat /etc/os-release
```
gives "wheezy", see output:
```
PRETTY_NAME="Raspbian GNU/Linux 7 (wheezy)"
...
VERSION="7 (wheezy)"
```
so we can install g++ 4.8 in order to be able to compile PJON 11.0 (adds ~30-50MB)
```
pi@raspberrypi ~ $ sudo apt-get install g++-4.8
pi@raspberrypi ~ $ g++
```
auto tab completition gives: g++ g++-4.6 g++-4.8

Next step is to get and compile (OW)PJON 11:

```
pi@raspberrypi ~ $ mkdir OWPJON
pi@raspberrypi ~ $ cd OWPJON/
pi@raspberrypi ~/OWPJON $ wget https://github.com/gioblu/PJON/archive/11.0.tar.gz
pi@raspberrypi ~/OWPJON $ tar -xvzf 11.0.tar.gz
pi@raspberrypi ~/OWPJON $ cd PJON-11.0/examples/LINUX/Local/LocalUDP/
pi@raspberrypi ~/OWPJON/PJON-11.0/examples/LINUX/Local/LocalUDP $ mkdir -p RemoteWorker/DeviceGeneric/
pi@raspberrypi ~/OWPJON/PJON-11.0/examples/LINUX/Local/LocalUDP $ cd RemoteWorker/DeviceGeneric/
```
copy files, e.g. by copy-paste from kate to nano window:
```
pi@raspberrypi ~/OWPJON/PJON-11.0/examples/LINUX/Local/LocalUDP/RemoteWorker/DeviceGeneric $ nano Makefile
pi@raspberrypi ~/OWPJON/PJON-11.0/examples/LINUX/Local/LocalUDP/RemoteWorker/DeviceGeneric $ nano DeviceGeneric.cpp
pi@raspberrypi ~/OWPJON/PJON-11.0/examples/LINUX/Local/LocalUDP/RemoteWorker/DeviceGeneric $ nano pack.py
pi@raspberrypi ~/OWPJON/PJON-11.0/examples/LINUX/Local/LocalUDP/RemoteWorker/DeviceGeneric $ nano unpack.py
pi@raspberrypi ~/OWPJON/PJON-11.0/examples/LINUX/Local/LocalUDP/RemoteWorker/DeviceGeneric $ chmod +x pack.py unpack.py
pi@raspberrypi ~/OWPJON/PJON-11.0/examples/LINUX/Local/LocalUDP/RemoteWorker/DeviceGeneric $ ls -la
total 72
drwxr-xr-x 2 pi pi  4096 Jul  7 15:06 .
drwxr-xr-x 3 pi pi  4096 Jul  7 09:53 ..
-rw-r--r-- 1 pi pi  4422 Jul  7 15:01 DeviceGeneric.cpp
-rw-r--r-- 1 pi pi   305 Jul  7 15:03 Makefile
-rwxr-xr-x 1 pi pi 41259 Jul  7 15:04 owpshell
-rwxr-xr-x 1 pi pi   162 Jul  7 15:06 pack.py
-rwxr-xr-x 1 pi pi   325 Jul  7 15:06 unpack.py
pi@raspberrypi ~/OWPJON/PJON-11.0/examples/LINUX/Local/LocalUDP/RemoteWorker/DeviceGeneric $ make raspi
pi@raspberrypi ~/OWPJON/PJON-11.0/examples/LINUX/Local/LocalUDP/RemoteWorker/DeviceGeneric $ printf "\x01" | ./owpshell - - 44
owp:dg:v1
pi@raspberrypi ~/OWPJON/PJON-11.0/examples/LINUX/Local/LocalUDP/RemoteWorker/DeviceGeneric $ printf "\x11" | ./owpshell - - 44 | ./unpack.py f
4.751999855041504,
pi@raspberrypi ~/OWPJON/PJON-11.0/examples/LINUX/Local/LocalUDP/RemoteWorker/DeviceGeneric $ printf "\x12" | ./owpshell - - 44 | ./unpack.py f
28.02459144592285,
```
and finally add the sensors/devices to your logging system e.g. collectd:
```
$ python owpjon_sensor.py
$ collectd -C /etc/collectd/collectd.conf -T
$ cat /var/log/syslog
$ sudo /etc/init.d/collectd restart
```




Possible wireless solutions going through wall e.g. (galvanic isolated):
(would allow to turn wifi - used for 1wire - off during night again)

```
-----------------           -----------------           ---------------
| DraginoLG01-S |           | Arduino+LoRa  |           | Arduino/AVR |
| LoRa          |   <--->   | LoRa          |           |             |
| SWBB          |           | SWBB          |   <--->   | SWBB        |
| Slaves/Tunnel |           | Master/Tunnel |           | Slaves      |
-----------------           -----------------           ---------------
SWBB bus indoors            SWBB bus outdoors
```

Dragino is 3v3 and needs a level shifter - I used this circuit (somehow
inspired by V-USB but is not the same):
```
3v3 IO  ---o---\/\/\/\---  5v IO
           |   resistor (68 better 150...4k7)
          ---
          | |  3v6 Zener (protect 3v3)
          ---
           |
          GND
```
see also http://www.partsim.com/simulator/#148247.

It works but has at least these disadvantages:
  - knee of zener known not to be very sharp, so 3v6 are not precise
  - using 150 ohms as minimum is recommended in order to be protected against shorts
    - a device on the 5v bus needs to be able to drive (5v - 3.6v) / R ~ 0.4..20mA for a high
    - currently 4k7 is used
    - current through zener might peak if 3.3V supply is higher (simulation
      below shows about 3.6V@20mA - so using 3.6V zener might be the safe choice)
  - (cut-off frequency might be low due to big capacity and big resistor)

Further improvements might include; setup a "3 way" tunneler (LUDP, SWBB, SWBB)
that gives 2 SWBB pins (on same bus; a 5v and a 3v3 bus).

An alternative proposal using a schottky diode is shown in
http://www.partsim.com/simulator/#148244.



- LoRa-SWBB: Dargino/Yun for LoRa indoors, Uno+LoRa outdoors - outdoor low-power possible
- AnalogSampling-SWBB: Single LED or Laser Diode bidirectional through window - low-power posible - needs testing and safety measure, e.g. cats remove sender and look into laser diode ...
- OverSampling-SWBB: 315/433MHz or HC-12 radio - low-power possible? where to order from?
- LUDP-SWBB (LUDP via wifi); Yun, ESP8266 (GUPD, 3.3v), raspi zero (3.3v using wifi stick or zero w), uno+eth+lan2wifi (like now), maybe use yun as "router"/lan2wifi (https://hackingmajenkoblog.wordpress.com/2017/06/02/configuring-yun-wifi/) - wifi thus high-power
- For testing and specifing network stability use e.g.:
  - https://github.com/gioblu/PJON/tree/master/examples/ARDUINO/Local/AnalogSampling/NetworkAnalysis - then build a tunneler or surrogate
- OWP_DG_1w-adaptor for mega328 could may be modified to fit in an attiny (mega88); OWPJON-1wire (master) converter for refurbishing of old 1wire devices
    - https://github.com/mikaelpatel/Arduino-OWI, https://github.com/mikaelpatel/Arduino-OWI/tree/master/examples/ATtiny





Notes:
- Setup, test and enjoy https://github.com/fredilarsen/ModuleInterface/tree/master/examples/WebPage (may be on a Raspi2)
- Arduino+ETH can be replaced by ESP8266 (3.3<->5.0v shifting might be needed, may be not) but needs GlobalUDP (LUDP would be nice)
- Arduino+ETH could be replaced by Arduino Yun or even Dragino, if PJON LUDP would support Yun (needs Yun to support UDP)
- Server and Master might be joined (linuxpc/raspi + Arduino+ETH) into a Raspi(Zero) with wifi (3.3<->5.0v shifting?)
- LocalUDP could also be replaced by EthernetTCP but like GlobalUDP not so convenient
