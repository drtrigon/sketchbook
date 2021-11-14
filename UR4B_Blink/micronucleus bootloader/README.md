
1. program Uno with example "ArduinoISP"

2. connect and wire UR4B to ISP shield (replace zero-force header) as oscillator will be needed
   confer ATmega328p-pinout-and-Specs-in-detail.png
   confer comments in "ArduinoISP"

3. $ sudo apt-get install avrdude

4. $ sudo avrdude -c avrisp -b 19200 -P /dev/ttyACM0 -p atmega328p -U flash:w:m328p_extclock.hex:i
   confer https://github.com/micronucleus/micronucleus/blob/master/firmware/releases/m328p_extclock.hex

5. $ sudo avrdude -c avrisp -b 19200 -P /dev/ttyACM0 -p atmega328p -U lfuse:w:0xff:m -U hfuse:w:0xd9:m -U efuse:w:0xff:m
   to check fuses use: $ sudo avrdude -c avrisp -b 19200 -P /dev/ttyACM0 -p atmega328p -v

ISSUE: efuse set to 7 (first 3 bits 111 instead of 101)

ISSUE: micronucleus wants 101 but says 0xff ... ???!!!

6. connect PC1 (Pin24) to a 68ohm resistor to D-, a 3.6V zehner to GND and a 2.2k pullup to VCC
   Connect PC2 (Pin25) to a 68ohm resistor to D+ and a 3.6V zehner to GND
   confer schemes in ppmUSBjoy and cli/n64dual_tiny45
!TODO: add schematic !!!

7. connect to USB and check presence by looking for "ID 16d0:0753 MCS Digistump DigiSpark": $ lsusb

8. use Arduino IDE to compile the code:
   - in Preferences enable verbose mode
   - choose default (Arduino/Genuino) board
   - Verify the code (not Upload)
   - from last output line before the summary (3rd last line) get hex file name, e.g. /tmp/arduino_build_339368/Blink.ino.hex

9. use micronucleus command line to upload: $ sudo ./micronucleus /tmp/arduino_build_339368/Blink.ino.hex
   confer https://github.com/micronucleus/micronucleus/releases

NOTE: This works well in VM because as long as there is no sketch on the atmega328 the bootloader
does not timeout and run the sketch. After the first upload there is a sketch that will be run instead
and the USB device vanishes. The VM cannot bind the device fast enough. Only way to upload then is
to do it from host (mxlinux) directly - needs the micronucleus command line binary and the hex files.

In order to make it work within the VM, compile it using either:
- increase the timeout (discouraged): FAST_EXIT_NO_USB_MS and AUTO_EXIT_MS in micronucleus/firmware/configuration/m328p_extclock/bootloaderconfig.h 
  see https://github.com/micronucleus/micronucleus/blob/master/firmware/configuration/m328p_extclock/bootloaderconfig.h#L169
- or enable pin/button to enter (recommended): ENTRYMODE in micronucleus/firmware/configuration/m328p_extclock/bootloaderconfig.h
  see https://github.com/micronucleus/micronucleus/blob/master/firmware/configuration/m328p_extclock/bootloaderconfig.h#L101

(Instead of un- and re-plugging a reset can be triggered.)
