
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
```

Server: LINUX/Local/ThroughSerial/RemoteWorker/Transmitter/
        (kubuntu 14.04, TS)
Master: ARDUINO/Local/ThroughSerial/SoftwareBitBangSurrogate/Surrogate/
        (Uno, TS, SWBB pin 7)
Slaves: ARDUINO/Local/SoftwareBitBang/BlinkWithResponse/Receiver/
        (Yun, SWBB pin12)

Notes: - putting `bus.set_synchronous_acknowledge(false);` before `bus.begin();`
         in both, the Receiver and the Transmitter makes TS run stable

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
```

Server: LINUX/Local/ThroughSerial/RemoteWorker/DeviceGeneric/
        (kubuntu 14.04, TS)
Master: ARDUINO/Local/ThroughSerial/SoftwareBitBangSurrogate/Surrogate/
        (Uno, TS, SWBB pin 7)
Slaves: ARDUINO/Local/SoftwareBitBang/DeviceGeneric/
        ARDUINO/Local/SoftwareBitBang/OWP_DG_LCD_Sensors/
        ATTINY/Local/SoftwareBitBang/DeviceGeneric/
        (Yun, SWBB pin12)

Notes: - the attiny code uses 5554 bytes (67%) compiled (may be less with LTO), see
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
```

Server: LINUX/Local/LocalUDP/RemoteWorker/DeviceGeneric/
        (kubuntu 14.04, LUDP)
Master: ARDUINO/Local/SoftwareBitBang/Tunneler/BlinkingSwitch/
        (Uno+EthernetShield W5100 based, LUDP, SWBB pin 7)
Slaves: (all as mentioned before)

In this case the `owpshell` program works exactly the same except for the fact that it
does not need any serial port info (and thus no sudo either):

    $ printf "\x01" | ./owpshell - - 44


Further possible schemes (productive system):

```
-----------------           ---------------           ---------------
| linuxpc/raspi |           | Arduino+ETH |           | Arduino/AVR |
| LocalUDP      |   <--->   | LocalUDP    |           |             |
|               |           | SWBB        |   <--->   | SWBB        |
| Server        |           | Master      |           | Slaves      |
-----------------           ---------------           ---------------
```

Notes: - Setup, test and enjoy https://github.com/fredilarsen/ModuleInterface/tree/master/examples/WebPage (may be on a Raspi2)
       - Arduino+ETH can be replaced by ESP8266 (3.3<->5.0v shifting might be needed, may be not) but needs GlobalUDP (LUDP would be nice)
       - Arduino+ETH could be replaced by Arduino Yun, if PJON LUDP would support Yun
       - Server and Master might be joined (linuxpc/raspi + Arduino+ETH) into a Raspi(Zero) with wifi (3.3<->5.0v shifting?)
       - LocalUDP could also be replaced by EthernetTCP but like GlobalUDP not so convenient
