
1. program Uno with example "ArduinoISP"

2. connect and wire UR3B to ISP shield (replace zero-force header) as oscillator will be needed
   confer ATmega328p-pinout-and-Specs-in-detail.png
   confer comments in "ArduinoISP"

3. select as programer "Arduino as ISP"

ISSUE: For the board "Arduino/Genuino Uno" the programer "Arduino as ISP" (-c avrisp) works for the bootloader ONLY! No sketch can be
uploaded that way!! You have to use MiniCore but be careful, as soon as you choose "Bootloader: Yes" it will also fall back (to -c arduino)!

4. select as board "MiniCore/ATmega328" and "Bootloader: No" (this will invoke -c avrisp)

5. select "Burn Bootloader" and MiniCore will burn an empty bootloader and set fuses

6. use Arduino IDE to compile and upload the code


(Basically the same technique as described in micronucleus/README.md using the Arduino IDE in verbose mode for compilation only and doing manual uploads using avrdude -c avrisp ... could be used - but this is very cumbersone as ArduinoISP sketch can be overwriten easily and thus
needs lots of refreshing.)
