
PJON 1-wire bus/network (OWPJON or 1wPJON)
------------------------------------------

Meant to replace all 1wire stuff (Dallas/Maxim commercial) some day.
Meantwhile both busses/systems can be used together.
The SoftwareBitBang (SWBB) part is the OWPJON or 1wPJON bus.

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

Now a program needs to be written tha allows e.g. shell/command line
access to the bus (similar to owshell read and write funcs). That can
also be used from collectd e.g.

There are already solutions around like PJON-gRPC or ModuleInterface:
https://github.com/fredilarsen/ModuleInterface/tree/master/examples/WebPage
Currently both are hard to compile and understand, but that will change
(PJON-gRPC is easier to compile in ubuntu 16.10, docu of ModuleInterface
is up to be improved).



Further possible schemes (productive system):

-----------------           ---------------           ---------------
| linuxpc/raspi |           | Arduino+ETH |           | Arduino/AVR |
| LocalUDP      |   <--->   | LocalUDP    |           |             |
|               |           | SWBB        |   <--->   | SWBB        |
| Server        |           | Master      |           | Slaves      |
-----------------           ---------------           ---------------

Notes: - SoftwareBitBangSurrogate might/should be replaced by Router solutions (router_extender)
         https://github.com/gioblu/PJON/tree/master/examples/ARDUINO/Local/SoftwareBitBang/Tunneler  (e.g. BlinkingRGBSwitch)
       - Arduino+ETH can be replaced by ESP8266 (3.3<->5.0v shifting might be needed) but needs GlobalUDP
       - Arduino+ETH could be replaced by Arduino Yun, if PJON would support Yun
       - Server and Master might be joined (linuxpc/raspi + Arduino+ETH) into a Raspi(Zero) with wifi (3.3<->5.0v shifting?)
       - LocalUDP could also be replaced by EthernetTCP but like GlobalUDP not so convenient
