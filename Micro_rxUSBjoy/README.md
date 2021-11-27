https://gitlab.com/armasuisse_fhgr/ugv-boarai/micro_receiver_usb_gamepad

--- --- --- --- --- --- --- --- --- --- --- --- --- --- ---

Derived from ppmUSBjoy and allows to:

- connect Transmitter Trainer ports using PPM signal
- connect Receiver using PPM signal
- connect Receiver using SBUS(2) signal


EV GAR KEIN NEUES PROJEKT SONDERN NUR IN PPMUSBJOY integrieren:

- delay befor umschalten des Serial Ports - um programmierung zu ermöglichen
- ev. led als button für weitere funktionen
- led muss anzeigen ob ppm oder sbus




https://github.com/bolderflight/sbus-arduino

https://github.com/BrushlessPower/SBUS2-Telemetry

(https://github.com/fdivitto/sbus)


https://www.arduino.cc/reference/en/libraries/s.port-sensor-library-for-frsky/


http://vusb.wikidot.com/project:ppm-rc-to-usb-interface
oder
https://www.obdev.at/products/vusb/projects.html

--- --- --- --- --- --- --- --- --- --- --- --- --- --- ---

https://www.arduino.cc/reference/en/libraries/sbus2/
 
https://www.arduino.cc/reference/en/libraries/s.port-sensor-library-for-frsky/

--- --- --- --- --- --- --- --- --- --- --- --- --- --- ---

S.35: https://usb.org/sites/default/files/hut1_2.pdf

-> 1 Gamepad hat nur 10 (oder 11) analoge Axen - für mehr Kanäle z.B. 16 müssen also 2 Devices erzeugt werden!

--- --- --- --- --- --- --- --- --- --- --- --- --- --- ---

https://www.premium-modellbau.de/tarot-zyx-s-futaba-sbus-verbindungskabel-sbus-inverter

besser (Inverter, Level-Shifter, Bidirektional): https://github.com/BrushlessPower/SBUS2-Telemetry#inverter-schematic-for-atmega328p-arduino-pro-mini
