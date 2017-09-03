#!/bin/bash

# http://reverseengineering.stackexchange.com/questions/125/how-do-i-figure-out-what-is-burned-on-an-arduino-rom
# http://www.avrfreaks.net/forum/how-can-i-disassemble-hex-file
# http://www.avrfreaks.net/forum/hex-elf-converter


# 1. Extracting the hex code from the Uno R3 (ATmega328):

#avrdude -F -v -pm328p -cstk500v1 -P/dev/ttyUSB0 -b19200 -D -Uflash:r:program.bin:r

#avrdude -pm328p -cstk500v1 -Uflash:r:$1.flash.hex:r -Ulfuse:r:$1.lfuse.hex:r -Uhfuse:r:$1.hfuse.hex:r -Ueeprom:r:$1.eeprom.hex:r


# 2. Converting Hex code to original source:

#avr-objdump -s -m atmega8 -D $1

avr-objdump -s -m avr -D $1 > $1.dump

#avr-objdump -s -m avr -D $1.flash.hex > $1.flash.dump


# 3. Generate other source file formats (elf, bin)

avr-objcopy -I ihex -O binary $1 $1.bin
avr-objcopy -I binary -O elf32-avr $1.bin $1.elf
avr-objdump -s -m avr -D $1.elf > $1.elf.dump
