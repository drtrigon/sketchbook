/********************************************************************************
 TODO: ...
 
    Program. . . . Uno_Write_LCD
    Author . . . . Ursin Solèr (according to design by Ian Evans)
    Written. . . . 10 Oct 2015.
    Description. . LiquidCrystal Library
                   Demonstrates the use a 16x2 LCD display.  The LiquidCrystal
                     library works with all LCD displays that are compatible with the
                     Hitachi HD44780 driver. There are many of them out there, and you
                     can usually tell them by the 16-pin interface.
    Hardware . . . Arduino Uno/Nano (w/ ATmega328)

 The shield:
 LCD     UNO
  1  VSS GND  / A5
  2  VDD 5V   / A4
  3  Vo  POT  / A3
  4  RS  PIN2 / A2
  5  R/W R?   / A1 (resitor to GND, value unknown?!)
  6  E   PIN3 / A0
  7  DB0 NC
  8  DB1 NC
  9  DB2 NC
 10  DB3 NC
 11  DB4 PIN4
 12  DB5 PIN5
 13  DB6 PIN6
 14  DB7 PIN7
 15  A   NC
 16  K   NC

 The circuit:
 * LCD RS pin to digital pin (12) 2
 * LCD Enable pin to digital pin (11) 3
 * LCD D4 pin to digital pin (5) 4
 * LCD D5 pin to digital pin (4) 5
 * LCD D6 pin to digital pin (3) 6
 * LCD D7 pin to digital pin (2) 7
 * LCD R/W pin to ground
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)

 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/LiquidCrystalDisplay

 http://pdf1.alldatasheet.com/datasheet-pdf/view/609067/LUMEX/LCM-S01602DSF.html
 https://www.arduino.cc/en/Tutorial/HelloWorld?from=Tutorial.LiquidCrystal
********************************************************************************/

// include the library code:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
//LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("hello, world!");
}

void loop() {
/*  // Turn off the display:
  lcd.noDisplay();
  delay(500);
  // Turn on the display:
  lcd.display();
  delay(500);*/

  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print(millis() / 1000);

}
