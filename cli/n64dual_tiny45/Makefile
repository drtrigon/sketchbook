# Name: Makefile
# Project: hid-data example
# Author: Christian Starkjohann
# Creation Date: 2008-04-07
# Tabsize: 4
# Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
# License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)

DEVICE  = attiny45
F_CPU   = 16500000	# in Hz
FUSES   = -U lfuse:w:0xd1:m -U hfuse:w:0xd7:m -U efuse:w:0xff:m # (see fuse calculator)
# PROGRAMMER = avrispmkII -P usb
PROGRAMMER = usbasp

AVRDUDE = sudo avrdude -c $(PROGRAMMER) -p $(DEVICE)
CFLAGS  = -Iusbdrv -I. -DDEBUG_LEVEL=03
OBJECTS = usbdrv/usbdrv.o usbdrv/usbdrvasm.o usbdrv/oddebug.o usbdrv/osccal.o main.o n64.o

COMPILE = avr-gcc -Wall -Os -DF_CPU=$(F_CPU) $(CFLAGS) -mmcu=$(DEVICE) -o main.hex -o main.bin -Wl,-Map=main.map

##############################################################################
# Fuse values for particular devices
##############################################################################
# If your device is not listed here, go to
# http://palmavr.sourceforge.net/cgi-bin/fc.cgi
# and choose options for external crystal clock and no clock divider

# symbolic targets:
help:
	@echo "This Makefile has no default rule. Use one of the following:"
	@echo "make hex ....... to build main.hex"
	@echo "make program ... to flash fuses and firmware"
	@echo "make fuse ...... to flash the fuses"
	@echo "make flash ..... to flash the firmware (use this on metaboard)"
	@echo "make clean ..... to delete objects and hex file"
	@echo "make all ....... to compile and flash"

all: clean hex program
	
hex: clean main.hex

program: flash fuse

# rule for programming fuse bits:
fuse:
	$(AVRDUDE) $(FUSES)

# rule for uploading firmware:
flash: main.hex
	$(AVRDUDE) -U flash:w:main.hex:i

# rule for deleting dependent files (those which can be built by Make):
clean:
	rm -f *.hex *.lst *.obj *.cof *.list *.map *.eep.hex *.elf *.o usbdrv/*.o *.s usbdrv/*.s

# Generic rule for compiling C files:
.c.o:
	$(COMPILE) -c $< -o $@

# Generic rule for assembling Assembler source files:
.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

# Generic rule for compiling C to assembler, used for debugging only.
.c.s:
	$(COMPILE) -S $< -o $@

# file targets:

# Since we don't want to ship the driver multipe times, we copy it into this project:
main.elf: $(OBJECTS)
	$(COMPILE) -o main.bin $(OBJECTS)
	./checksize main.bin

main.hex: main.elf
	rm -f main.hex main.eep.hex
	avr-objcopy -j .text -j .data -O ihex main.bin main.hex
	avr-size main.hex

# debugging targets:

disasm:	main.elf
	avr-objdump -d main.elf

cpp:
	$(COMPILE) -E main.c
