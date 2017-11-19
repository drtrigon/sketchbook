/*
* Helvetiny85 / Digispark Test
* (ATtiny85 eqipped with Micronucleus USB bootloader - not serial)
* 
* Links:
*   https://playground.boxtec.ch/doku.php/arduino/attinyusb/helvetiny85
*   https://digistump.com/board/index.php?topic=106.0 (udev rule or sudo; sudo needs reinstall of libs and boards)
*   https://playground.boxtec.ch/doku.php/arduino/attinyusb/micronucleus
*   https://rayshobby.net/hid-class-usb-serial-communication-for-avrs-using-v-usb/
*   https://digistump.com/wiki/digispark/tutorials/debugging
* 
* Setup:
*   1.) File > Preferences > Additional Boards Manager URLs: http://digistump.com/package_digistump_index.json
*   2.) Tools > Board: ... > Boards Manager...: "digispark"
*   3.) Tools > Board: ... > Digispark (Default - 16.5mhz)
*   4.) VirtualBox USB Filter: "Unbekanntes Gerät 16D0:0753 [010B]"
*         Hersteller-ID: 16d0
*            Produkt-ID: 0753
*           Revision-Nr: 010b
*         (in order to get this data you need an fresh micronucleus or press RESET and run 'lsusb' before code execution starts - a few secs)
*   5.) Upload: Start Upload then push RESET button (during 60s timeout)
* 
*  Caution: micronucleus: library/micronucleus_lib.c:66: micronucleus_connect: Assertion `res >= 4' failed.
* Solution: Run Arduino IDE as root/sudo (need re-setup of everything) or use udev rules, see link above.
* 
*  Caution: Using e.g. DigisparkKeyboard > Keyboard "bricks" it as uploads are not possible anymore
*           (even though this would be a way to output debug info and replace serial - currently it needs another chip; File > Examples > DigiparkSoftSerial > Digi_1io_SerialDbg)
* Solution: Use an Arduino as ISP to reflash micronucleus 1.11 hex file
*           $ /home/osboxes/data/03Tools/arduino/arduino-1.8.3/hardware/tools/avr/bin/avrdude -C/home/osboxes/data/03Tools/arduino/arduino-1.8.3/hardware/tools/avr/etc/avrdude.conf -v -pattiny85 -cstk500v1 -P/dev/ttyACM0 -b19200 -Uflash:w:/home/osboxes/sketchbook/micronucleus-1.11.hex:i -U lfuse:w:0xe1:m -U hfuse:w:0xdd:m -U efuse:w:0xfe:m
* 
* ATtiny85 Pin
*            4    via resistor (? ohms) to LED that is connected to Pin 6
*            6    LED (connected via resistor to Pin 4)
* 
*/

/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO 
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino model, check
  the Technical Specs of your board  at https://www.arduino.cc/en/Main/Products
  
  This example code is in the public domain.

  modified 8 May 2014
  by Scott Fitzgerald
  
  modified 2 Sep 2016
  by Arturo Guadalupi
  
  modified 8 Sep 2016
  by Colby Newman
*/


#define LED_BUILTIN  1  // this LED is NOT builtin (see above on how to add)

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
}
