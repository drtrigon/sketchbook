/**************************************************************************/
/*!
    @file     Adafruit_Si7021_Tiny.cpp
    @author   Limor Fried (Adafruit Industries)
    @license  BSD (see license.txt)

    This is a library for the Adafruit Si7021 breakout board
    ----> https://www.adafruit.com/products/3251

    Adafruit invests time and resources providing this open source code,
    please support Adafruit and open-source hardware by purchasing
    products from Adafruit!

    @section  HISTORY

    v1.0  - First release
*/
/**************************************************************************/

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#include <TinyWireM.h>
#include <Adafruit_Si7021_Tiny.h>


/**************************************************************************/

Adafruit_Si7021_Tiny::Adafruit_Si7021_Tiny(void) {
  _i2caddr = SI7021_DEFAULT_ADDRESS;
  sernum_a = sernum_b = 0;
}

bool Adafruit_Si7021_Tiny::begin(void) {
  TinyWireM.begin();

  reset();
  if (readRegister8(SI7021_READRHT_REG_CMD) != 0x3A) return false;

  readSerialNumber();

  //Serial.println(sernum_a, HEX);
  //Serial.println(sernum_b, HEX);

  return true;
}

float Adafruit_Si7021_Tiny::readHumidity(void) {
  TinyWireM.beginTransmission(_i2caddr);
  TinyWireM.write((uint8_t)SI7021_MEASRH_NOHOLD_CMD);
  TinyWireM.endTransmission(false);
  delay(25);

  TinyWireM.requestFrom(_i2caddr, 3);
  uint16_t hum = TinyWireM.read();
  hum <<= 8;
  hum |= TinyWireM.read();
  uint8_t chxsum = TinyWireM.read();

  float humidity = hum;
  humidity *= 125;
  humidity /= 65536;
  humidity -= 6;

  return humidity;
}

float Adafruit_Si7021_Tiny::readTemperature(void) {
  TinyWireM.beginTransmission(_i2caddr);
  TinyWireM.write((uint8_t)SI7021_MEASTEMP_NOHOLD_CMD);
  TinyWireM.endTransmission(false);
  delay(25);

  TinyWireM.requestFrom(_i2caddr, 3);
  uint16_t temp = TinyWireM.read();
  temp <<= 8;
  temp |= TinyWireM.read();
  uint8_t chxsum = TinyWireM.read();

  float temperature = temp;
  temperature *= 175.72;
  temperature /= 65536;
  temperature -= 46.85;

  return temperature;
}

void Adafruit_Si7021_Tiny::reset(void) {
  TinyWireM.beginTransmission(_i2caddr);
  TinyWireM.write((uint8_t)SI7021_RESET_CMD);
  TinyWireM.endTransmission();
  delay(50);
}

void Adafruit_Si7021_Tiny::readSerialNumber(void) {
  TinyWireM.beginTransmission(_i2caddr);
  TinyWireM.write((uint8_t)SI7021_ID1_CMD>>8);
  TinyWireM.write((uint8_t)SI7021_ID1_CMD&0xFF);
  TinyWireM.endTransmission();

  TinyWireM.requestFrom(_i2caddr, 8);
  sernum_a = TinyWireM.read();
  TinyWireM.read();
  sernum_a <<= 8;
  sernum_a |= TinyWireM.read();
  TinyWireM.read();
  sernum_a <<= 8;
  sernum_a |= TinyWireM.read();
  TinyWireM.read();
  sernum_a <<= 8;
  sernum_a |= TinyWireM.read();
  TinyWireM.read();

  TinyWireM.beginTransmission(_i2caddr);
  TinyWireM.write((uint8_t)SI7021_ID2_CMD>>8);
  TinyWireM.write((uint8_t)SI7021_ID2_CMD&0xFF);
  TinyWireM.endTransmission();

  TinyWireM.requestFrom(_i2caddr, 8);
  sernum_b = TinyWireM.read();
  TinyWireM.read();
  sernum_b <<= 8;
  sernum_b |= TinyWireM.read();
  TinyWireM.read();
  sernum_b <<= 8;
  sernum_b |= TinyWireM.read();
  TinyWireM.read();
  sernum_b <<= 8;
  sernum_b |= TinyWireM.read();
  TinyWireM.read();
}

/*******************************************************************/

void Adafruit_Si7021_Tiny::writeRegister8(uint8_t reg, uint8_t value) {
  TinyWireM.beginTransmission(_i2caddr);
  TinyWireM.write((uint8_t)reg);
  TinyWireM.write((uint8_t)value);
  TinyWireM.endTransmission();

  //Serial.print("Wrote $"); Serial.print(reg, HEX); Serial.print(": 0x"); Serial.println(value, HEX);
}

uint8_t Adafruit_Si7021_Tiny::readRegister8(uint8_t reg) {
  uint8_t value;
  TinyWireM.beginTransmission(_i2caddr);
  TinyWireM.write((uint8_t)reg);
  TinyWireM.endTransmission(false);

  TinyWireM.requestFrom(_i2caddr, 1);
  value = TinyWireM.read();

  //Serial.print("Read $"); Serial.print(reg, HEX); Serial.print(": 0x"); Serial.println(value, HEX);
  return value;
}

uint16_t Adafruit_Si7021_Tiny::readRegister16(uint8_t reg) {
  uint16_t value;
  TinyWireM.beginTransmission(_i2caddr);
  TinyWireM.write((uint8_t)reg);
  TinyWireM.endTransmission();

  TinyWireM.requestFrom(_i2caddr, 2);
  value = TinyWireM.read();
  value <<= 8;
  value |= TinyWireM.read();

  //Serial.print("Read $"); Serial.print(reg, HEX); Serial.print(": 0x"); Serial.println(value, HEX);
  return value;
}
