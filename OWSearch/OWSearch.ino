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
// DS18S20 Temperature chip i/o
// Family codes:
// http://owfs.sourceforge.net/commands.html
OneWire ds(13);  // on pin 13

byte inByte = 's';   // r=run, s=stop

void setup(void) {
  // initialize inputs/outputs
  // start serial port
  Serial.begin(9600);

  //Serial.print("OWSearch v1.0\n");
}

void loop(void) {
  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];

  if (Serial.available() > 0) {
    // get incoming byte:
    inByte = Serial.read();
  }
  
  if (inByte == 's')
    return;

  delay(1000);

  ds.reset_search();

  // http://www.hacktronics.com/Tutorials/arduino-1-wire-address-finder.html
  while(ds.search(addr)) {
    Serial.print("\nFound \'1-Wire\' device with address:\n");

    Serial.print("R=");
    for( i = 0; i < 8; i++) {
      Serial.print(addr[i], HEX);
      Serial.print(" ");
    }

    if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.print("CRC is not valid!\n");
      return;
    }

    checkFamily(addr);
  }

  Serial.print("No more addresses.\n");
  ds.reset_search();
//  ds.depower();
  return;
}

void checkFamily(byte* addr) {
  if ( addr[0] == 0x10) {
      Serial.print("Device is a DS18S20 family device.\n");
  }
  else if ( addr[0] == 0x28) {
      Serial.print("Device is a DS18B20 family device.\n");
  }
  else if ( addr[0] == 0x81) {
      Serial.print("Device is a iButton 1420 family device.\n");
  }
  else if ( addr[0] == 0x02) {
      Serial.print("Device is a DS1425 family device.\n");
  }
  else if ( addr[0] == 0x22) {
      Serial.print("Device is a DS1822 family device.\n");
  }
  else if ( addr[0] == 0x21) {
      Serial.print("Device is a iButton 1921 family device.\n");
  }
  else if ( addr[0] == 0x1A) {
      Serial.print("Device is a iButton 1963L family device.\n");
  }
  else if ( addr[0] == 0x37) {
      Serial.print("Device is a iButton 1977 family device.\n");
  }
  else if ( addr[0] == 0x08) {
      Serial.print("Device is a iButton 1992 family device.\n");
  }
  else if ( addr[0] == 0x06) {
      Serial.print("Device is a iButton 1993 family device.\n");
  }
  else if ( addr[0] == 0x0A) {
      Serial.print("Device is a iButton 1995 family device.\n");
  }
  else if ( addr[0] == 0x0C) {
      Serial.print("Device is a iButton 1996 family device.\n");
  }
  else if ( addr[0] == 0x01) {
      Serial.print("Device is a DS2401/11 family device.\n");
  }
  else if ( addr[0] == 0x04) {
      Serial.print("Device is a DS2404 family device.\n");
  }
  else if ( addr[0] == 0x84) {
      Serial.print("Device is a DS2404S family device.\n");
  }
  else if ( addr[0] == 0x05) {
      Serial.print("Device is a DS2405 family device.\n");
  }
  else if ( addr[0] == 0x12) {
      Serial.print("Device is a DS2406/7 family device.\n");
  }
  else if ( addr[0] == 0x29) {
      Serial.print("Device is a DS2408 family device.\n");
  }
  else if ( addr[0] == 0x1F) {
      Serial.print("Device is a DS2409 family device.\n");
  }
  else if ( addr[0] == 0x3A) {
      Serial.print("Device is a DS2413 family device.\n");
  }
  else if ( addr[0] == 0x24) {
      Serial.print("Device is a DS2415 family device.\n");
  }
  else if ( addr[0] == 0x27) {
      Serial.print("Device is a DS2417 family device.\n");
  }
  else if ( addr[0] == 0x41) {
      Serial.print("Device is a DS2422 family device.\n");
  }
  else if ( addr[0] == 0x1D) {
      Serial.print("Device is a DS2423 family device.\n");
  }
  else if ( addr[0] == 0x14) {
      Serial.print("Device is a DS2430A family device.\n");
  }
  else if ( addr[0] == 0x2D) {
      Serial.print("Device is a DS2431 family device.\n");
  }
  else if ( addr[0] == 0x33) {
      Serial.print("Device is a DS2432 family device.\n");
  }
  else if ( addr[0] == 0x23) {
      Serial.print("Device is a DS2433/DS28EC20 family device.\n");
  }
  else if ( addr[0] == 0x1B) {
      Serial.print("Device is a DS2436 family device.\n");
  }
  else if ( addr[0] == 0x26) {
      Serial.print("Device is a DS2438 family device.\n");
  }
  else if ( addr[0] == 0x20) {
      Serial.print("Device is a DS2450 family device.\n");
  }
  else if ( addr[0] == 0x09) {
      Serial.print("Device is a DS2502 family device.\n");
  }
  else if ( addr[0] == 0x89) {
      Serial.print("Device is a 2502-E48/UNW family device.\n");
  }
  else if ( addr[0] == 0x0B) {
      Serial.print("Device is a DS2505 family device.\n");
  }
  else if ( addr[0] == 0x8B) {
      Serial.print("Device is a DS2505-UWN family device.\n");
  }
  else if ( addr[0] == 0x0F) {
      Serial.print("Device is a DS2506 family device.\n");
  }
  else if ( addr[0] == 0x8F) {
      Serial.print("Device is a DS2506-UWN family device.\n");
  }
  else if ( addr[0] == 0x31) {
      Serial.print("Device is a DS2720 family device.\n");
  }
  else if ( addr[0] == 0x36) {
      Serial.print("Device is a DS2740 family device.\n");
  }
  else if ( addr[0] == 0x51) {
      Serial.print("Device is a DS2751 family device.\n");
  }
  else if ( addr[0] == 0x30) {
      Serial.print("Device is a DS2760/1/2 family device.\n");
  }
  else if ( addr[0] == 0x2E) {
      Serial.print("Device is a DS2770 family device.\n");
  }
  else if ( addr[0] == 0x2C) {
      Serial.print("Device is a DS2890 family device.\n");
  }
  else if ( addr[0] == 0x1C) {
      Serial.print("Device is a DS28E04-100 family device.\n");
  }
  else if ( addr[0] == 0xFF) {
      Serial.print("Device is a LCD family device.\n");
  }
  else {
      Serial.print("Device family is not recognized: 0x");
      Serial.println(addr[0],HEX);
      return;
  }
}
