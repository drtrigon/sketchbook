// Wire Master Reader
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Reads data from an I2C/TWI slave device
// Refer to the "Wire Slave Sender" example for use with this

// Created 29 March 2006

// This example code is in the public domain.

// PSoC 4 Pioneer Kit: "2-Ch DAC (8/7-bit) with I2C interface"

// WRITE: 2 byte
// [DAC_CH1] [DAC_CH2]

// READ: 16 byte
// [DAC_CH1] [DAC_CH2] [ADC_CH1] [ADC_CH1] ...
// [ADC_CH2] [ADC_CH2] [ADC_CH3] [ADC_CH3] ...
// [ADC_CH4] [ADC_CH4] [ADC_TEMP] [ADC_TEMP] ...
// [RTC] [RTC] [RTC] [RTC]
// CH3 = OP1, CH4 = OP2

// ADC CH1: from 0 4D ... 0 4F -> 28.6mV (DMM)
//          to   7 BE ... 7 C0 -> 3.236V (DMM)
// ADC CH2: from 0 59 ... 0 5B -> 29.5mV (DMM)
//          to   7 A9 ... 7 AC -> 3.202V (DMM)


#include <Wire.h>

void setup() {
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output

  Serial.println("START");

  Wire.beginTransmission(0x08);
  Wire.write(0x17); // respond with message of 2 bytes
  Wire.write(0x42); // respond with message of 2 bytes
//  Wire.write(0x00); // respond with message of 2 bytes
//  Wire.write(0x00); // respond with message of 2 bytes
  Wire.endTransmission();
}

void loop() {
//  Wire.requestFrom(8, 6);    // request 6 bytes from slave device #8
  Wire.requestFrom(0x08, 16);    // request 16 bytes from slave device #8

/*  while (Wire.available()) { // slave may send less than requested
    char c = Wire.read(); // receive a byte as character
//    Serial.print(c);         // print the character
    Serial.print(c, HEX);         // print the character
  }*/
  for (int i=0; i<16; ++i) {
    Serial.print(Wire.read(), HEX);         // print the character
    Serial.print(" ");         // print the character
  }
  Serial.print("\n");         // print the character

/*  Wire.beginTransmission(0x08);
  Wire.write("\xff\xff"); // respond with message of 2 bytes
  Wire.endTransmission();*/

  delay(500);
}
