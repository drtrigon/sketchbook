// https://www.instructables.com/id/Interfacing-With-a-Mouse-Sensor-ADNS-3050/
// https://github.com/Tom101222/Adns-3050-Optical-Sensor
/*
 * Connections
 *
 *           ADNS-3050                 ||   Arduino Uno/Nano
 *
 *               5 - GND          - (braun)  - GND
 *               1 - SCLK         - (rot)    -  13
 * -----------   3 - MISO         - (blau)   -  12
 * | 1 2 3 4 |   2 - MOSI         - (gelb)   -  11
 * | 5 6 7 8 |   8 - Chip Select  - (violet) -   3 (any pin you want can be changed later, e.g. 10, default is 3)
 * -----------   4 - PWR(5V)      - (grau)   -  5v
 *               6 - 3V PWR INPUT - (orange) -  ?
 *               7 - MOTION       - (grün)   -  ?
 *
 * schärfste Distanz derzeit ca. g = 12cm = 120mm, also
 * 1/f = 1/b + 1/g -> 1/b = 1/f - 1/g = (g-f)/(f*g) -> b = (f*g)/(g-f)
 * wir wollen b = f also beträgt die nötige Korrektur (Verkürzung des Tubus zw. Linse und chip)
 * b-f = (f^2)/(g-f)
 * mit g = 120mm und f = 4.6mm ist b-f = 0.183mm
 * ---
 * bilder von handy wie angeschlossen in verzeichnis kopieren
*/

// http://www.multiwii.com/forum/viewtopic.php?f=7&t=1413
// http://flug.synerflight.com/file/view/290/multiwii-24-with-optical-flow-sonar-and-lidar-support

#include "ADNS3050.h"
#include <SPI.h>

#define PIX_GRAB 0x0b

int x = 0;
int y = 0;
byte pix;
byte inByte = 0;         // incoming serial byte

void setup()
{
  startup();

  // Setting the Resolution
  // Resolution is determined by the value of bits 4-2 in the
  // MOUSE_CNTRL register (0x0d). By default this is set to 1000 cpi.
  // To change the resolution just write to the register with the
  // corresponding value . For example, to set the resolution to
  // 2000 cpi just use:
  //Write(0x0d,B11000);
  // This sets bits 4-2 to 110 and bits 1 and 0 to 0 (needed for
  // operation). 110 corresponds with the value found on page 29 of
  // the datasheet for a resolution of 2000

  Serial.begin(9600);  // if not used defaults to 115200

  byte id = 0;
  id= Read(PROD_ID);
  if (id == 9) {
    Serial.println("Communication SuccesSful");
  }
  Serial.println(id);
  delay(1000);
  Serial.println("What mode would you like to run [m=movements,g=pixel grabber]?");
}

void loop()
{

  while (Serial.available() == 0) {
    delay(1);
  }

  // get incoming byte:
  inByte = Serial.read();

  if(inByte == 'g') {
//    Serial.println("PIX_GRAB");
    // PIX_GRAB (pixel grabber)
    Write(PIX_GRAB, 0x00);  // reset grabber to origin
    for(int i=0; i < 361; ) {
      pix = Read(PIX_GRAB);
      if(pix & 0b10000000) {
//        Serial.print(pix & 0b01111111, HEX);
//        Serial.println(Read(PIX_GRAB), BIN);
        Serial.write(pix & 0b01111111);
//        Serial.write((pix & 0b01111111) << 1);
        ++i;
      }
      delayMicroseconds(1000);
    }
    Serial.println("");
//  } else if() {  // ...
  } else {  // e.g. 'm'
//    Serial.println("get movements");
    while (Serial.available() == 0) {
      // get movements
      x = getX();
      y = 0-getY();
      Serial.print(x);
      Serial.print(" ");
      Serial.println(y);
      delay(1000);
    }
  }

}
