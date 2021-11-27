
UR4B


Can be used in several different ways - soft- and hardware settings.

Oscillator can be omitted any by using MiniCore the internal 8 MHz (inacurate w/o calibration) can be used -
this frees 2 I/O pins.
!TODO: break those 2 pins out as well and allow to solder pins in order to connect them !

In the mentioned case but also when using the external osc. the board can be used and ISP programed like a bare atmega328 (ArduinoISP, MiniCore, nobootloader).

It can be equipped with USB by using Micronuleus bootloader and adding 3 resistors and 2 zehner diodes and use VUSB (micronucleus).

For serial VUSB CDC could the be used (Helvetiny_Blink_CDC). Even more if the USB port is already there when using micronucleus.

Also possible is to set it up like an ordinary Uno (using default bootloader optiboot or MiniCore) and use an Uno boards USB chip
for communication and programing/upload.


Summary:

1. no bootloader (MiniCore)
    pros: minimal hardware
    cons: needs ISP/ICSP for programing (always)
    hardware: ISP/ICSP (e.g. Uno with ArduinoISP sketch)
    pins: 4 (ohne VCC, GND)
        A. internal osc. (no need for soldering the oscillator)
        B. external osc.
2. bootloader: Micronucleus
    pros: no need for USB chip or ISP/ICSP for programing
    cons: needs programer binary (does not work from VM)
    hardware: none, (USB port passives; 3 res, 2 zener - but all USB chip/connections need them, see e.g. Uno Refernce Design)
    pins: 2
        C. external osc.
3. bootloader: MiniCore or default (optiboot)
    pros: minimal hardware, very simple usage (2-3 wires to connect only, can be used from Arduino IDE like an Uno)
    cons: needs USB chip (from Uno, Nano, ...) for programing (always)
    hardware: USB chip (e.g. Uno, Nano, ...)
    pins: 2-3
        D. internal osc. (MiniCore)
        E. external osc. (MiniCore, optiboot)
(4. bootloader: fastboot (1-wire interface, e.g. like ESCs) - NOT TESTED)
  https://www.mikrocontroller.net/articles/AVR_Bootloader_FastBoot_von_Peter_Dannegger#Arduino_und_FastBoot
  https://github.com/jdeitmerg/fastboot
    pros: minimal hardware, very very simple usage (1 wire to connect only)
    cons: needs USB chip (from Uno, Nano, ...) for programing (always), needs programer binary
    hardware: USB chip (e.g. Uno, Nano, ...)
    pins: 1
(4. bootloader: One-Way-Loader / OWL (Ein-Weg/unidirektional e.g. für einfache optsche Verbindung Fototransistor/Fotodiode) - NOT TESTED)
  http://jtxp.org/tech/onewayloader.htm
  https://www.rc-network.de/threads/arduino-bootloader-welcher-ist-f%C3%BCr-uns-optimal.743237/#post-7291836
    pros, cons, hardware, pins wie oben

(ALL need ISP/ICSP for first time setup; uploade/programing/burning of bootloader)

(USB chip = serial USB TTL interface = e.g. Uno with main chip on RESET or removed)



UR4B alternative (optiboot - but Ardino Mini Pro can als be used for all other variants mentioned before)


Arduino Uno = Arduino Micro (HoodLoader2 or USB-Serial) + Arduino Mini Pro (Micro = 32u4 and is better than Uno's USB chip, Nano has FTDI -> not prog.able)

Price: CHF ~30 > CHF ~12 + CHF ~10 (true? else use UR4B instead of Mini Pro - recently found Micros and Micro Pro for CHF 30?!..)

Usually only 1 of both has to be buyed!

(like Uno that allows to remove USB chip while not used)

Micro/32u4:
- use HoodLoader2 without sketch (not to timeout): https://github.com/NicoHood/HoodLoader2/blob/master/avr/bootloaders/HexFiles/HoodLoader2_0_5_Micro_atmega32u4.hex
- OR (even simpler) use USB-Serial (from HoodLoader2): https://github.com/NicoHood/HoodLoader2/blob/master/avr/examples/USB-Serial/USB-Serial.ino (recommended/preferred)

(Keep in mind that HoodLoader2 in bootloader mode supports faster USART speeds (2M full duplex) and that this us just an example how to 
reproduce a simple USB-Serial converter.)

When/If using HoodLoader2 (not USB-Serial) you have the advantage to be able to run an additional sketch also on the Micro/32u4, in that case
confer also:
- https://github.com/NicoHood/HoodLoader2/wiki/How-to-use-reset
- https://github.com/NicoHood/HoodLoader2/wiki/HoodLoader2-32u4-Usage

Connect Micro TX (Pin0), RX (Pin1) and Pin4 to Mini Pro/UR4B RX (0/Pin2), TX(1/Pin3) and 100nF cap. connected to RESET (Pin1).
Also add the pull-up 10k res. between Mini Pro/UR4B RESET and VCC (5V) - otherwise the upload will fail quite often due to bad reset.

(Another alternative is to use Gscheiduino's avr-isp-mkII-clone: https://github.com/ehajo/mkII that can be switched between 
USB nach Seriell-Wandler (Jumper offen) and AVR ISP mkII Clone (Jumper geschlossen), see https://www.ehajo.de/dokuwiki/bausaetze:gscheiduino-firmware)

(Again another alternative is LUFA USBtoSerial: https://github.com/abcminiuser/lufa/tree/master/Projects/USBtoSerial)


(AVR-CDC on 328 as serial USB TTL interface is to slow ~38400: http://www.recursion.jp/prose/avrcdc/cdc-232.html#schematic)

(UPDI Programmer: https://github.com/permaBox/picoTinyCore/blob/master/MakeUPDIProgrammer.md)

(UR4B Board: https://www.tindie.com/products/DrTrigon/ur4b-v1-pcb-only/)
