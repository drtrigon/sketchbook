
1. program Uno with example "ArduinoISP"

2. connect and wire UR4B to ISP shield (replace zero-force header) as oscillator will be needed
   confer ATmega328p-pinout-and-Specs-in-detail.png
   confer comments in "ArduinoISP"

3. select as programer "Arduino as ISP"

ISSUE: For the board "Arduino/Genuino Uno" the programer "Arduino as ISP" (-c avrisp) works for the bootloader ONLY! No sketch can be
uploaded that way!! You have to use MiniCore but be careful, as soon as you choose "Bootloader: Yes" it will also fall back (to -c arduino)!

4. select as board "Arduino/Genuino Uno" OR "MiniCore/ATmega328" and "Bootloader: Yes"

5. select "Burn Bootloader" and the IDE will burn the bootloader (optiboot) and set fuses

6. connect RESET to GND for the Arduino Board chip (ISP) or remove the chip
   connect RX (Pin2) and TX (Pin3) to the Arduino Board RX (0) and TX(1)
   if the chip was removed connect RESET (Pin1) to the Arduino Board RESET (enables automatic reset and upload)
   (disable the board chip and connect USB to UR4B)
!TODO: add schematic !!!

7. use Arduino IDE to compile and upload the code

8. if the chip was not removed: RESET the UR4B chip manually (continously retry until it works) while the IDE tries to upload
