# N64 Controller HID Joystick based on ATTiny (N64TinyUSB)

@ref Uno_ppmUSBjoy shows how to implement a HID Joystick (no need for driver on most OSes) on AVR platform using VUSB.
That project uses an ATMega328 (Arduino Uno) to connect a PPM source (R/C Transmitter) to a PC.

This project should do the same using a N64 Controller on the input. As we want to improve we aim at using minimal hardware
and ressources and thus want to replace the Arduino Uno by an ATTiny ([Helvetiny](https://github.com/boxtec/helvetiny85)).

As it turns out somebody was faster:
* attiny45 n64-usb (new version): https://matthiaslinder.com/?view=attiny45+n64-usb
* n64, gc to usb (old version): https://matthiaslinder.com/?view=n64%2C+gc+to+usb

By the way it exists for (S)NES also:
* tinyntendo: https://github.com/kunaakos/tinyntendo

## attiny45 n64-usb

First change PROGRAMMER and may be others in Makefile. Use Arduino Uno as ISP for ATTiny.

flash fuses and firmware (from already existing hex file)
> make program

delete objects and hex file
> make clean
compile and flash
> make all

(lets try the guy; use hex/code from attiny45 n64-usb and eagle board/schematics from helvetiny85 ... jlcpcb)

## PCB (design based on Helvetiny 1.0)

@ref n64dual_schematic.png
