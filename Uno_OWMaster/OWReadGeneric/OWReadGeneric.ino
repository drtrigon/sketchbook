#include <OneWire.h>

// Parasite power mode
// http://playground.arduino.cc/Learning/OneWire
// When operating in parasite power mode, only two wires are
// required (eg. iButtons): one data wire, and ground. In this
// mode, at the master, a 4.7k pull-up (power line) resistor
// must be connected to the 1-wire bus. When the line is in a
// "high" state, the device pulls current to charge an internal
// capacitor. 
//
// RTC Chip / iButton
// http://datasheets.maximintegrated.com/en/ds/DS1904.pdf
//
// Family codes:
// http://owfs.sourceforge.net/commands.html
//OneWire ds(13);  // on pin 13
OneWire ds(12);  // on pin 12 (enables sketch upload during 1wrie bus connection)

byte inByte = 0;

void setup(void) {
  // initialize inputs/outputs
  // start serial port
  Serial.begin(9600);
}

void loop(void) {
  byte i, j;
  byte present = 0;
  byte data[12];
  byte addr[8];

  if (Serial.available() > 0) {
    // get incoming byte:
    inByte = Serial.read();
  }
  
  if (inByte != 'r')
    return;

  ds.reset_search();
  if ( !ds.search(addr)) {
      Serial.print("No more addresses.\n");
      ds.reset_search();
      return;
  }

  Serial.print("R=");
  for( i = 0; i < 8; i++) {
    Serial.print(addr[i], HEX);
    Serial.print(" ");
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.print("CRC is not valid!\n");
      return;
  }

  if ( addr[0] == 0x24) {
      Serial.print("Device is a DS2415 family device.\n");
  }
  else {
      Serial.print("Device family is not recognized: 0x");
      Serial.println(addr[0],HEX);
      return;
  }

  for ( j = 0; j < 11; j++) {           // we need 7 channels
    ds.reset();
    ds.select(addr);
    ds.write(0x99,1);         // select sensor and start conversion, with parasite power on at the end (WRITE CLOCK)
    ds.write(0x00,1);         // control byte
    ds.write(0x00,1);         // 4-byte sensor address
    ds.write(0x00,1);         // 4-byte sensor address
    ds.write(0x00,1);         // 4-byte sensor address
    ds.write(j,1);         // 4-byte sensor address

    delay(100);     // maybe 750ms is enough, maybe not
    // we might do a ds.depower() here, but the reset will take care of it.

    present = ds.reset();
    ds.select(addr);    
    ds.write(0x66);           // read sensor data (READ CLOCK)

    Serial.print("P=");
    Serial.print(present,HEX);
    Serial.print(" ");
    for ( i = 0; i < 5; i++) {           // we need 5 bytes
      data[i] = ds.read();
      Serial.print(data[i], HEX);
      Serial.print(" ");
    }
    Serial.print(" CRC=");
    Serial.print( OneWire::crc8( data, 8), HEX);
    Serial.println();
  }

  delay(1000);     // maybe 750ms is enough, maybe not
}
