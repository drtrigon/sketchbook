/**
 * @brief Example-Code for Adaptor Device to connect Dallas/Maxim DS2438 and DS18x20 1wire Slaves
 *
 * @file OWPJON/ARDUINO/Local/SoftwareBitBang/OWP_DG_1w-adaptor/OWP_DG_1w-adaptor.ino
 *
 * @author drtrigon
 * @date 2018-08-02
 * @version 1.0
 *   @li first version derived from
 *       OWPJON/ARDUINO/Local/SoftwareBitBang/DeviceGeneric/DeviceGeneric.ino
 *       using Arduino OneWire docu and DS2438 Arduino OneWire Library
 *
 * @ref OWPJON/ARDUINO/Local/SoftwareBitBang/DeviceGeneric/DeviceGeneric.ino
 * @see https://playground.arduino.cc/Learning/OneWire
 * @see https://github.com/jbechter/arduino-onewire-DS2438/blob/master/examples/DS2438TemperatureAndVoltage/DS2438TemperatureAndVoltage.ino
 * @see https://cdn.shopify.com/s/files/1/0164/3524/files/MultiSensor_Manual_v1.5.pdf?15845834050373852166
 *
 * @verbatim
 * OneWire PJON Generic "OWPG" scheme:
 *   @ref OWPJON/LINUX/Local/LocalUDP/RemoteWorker/DeviceGeneric/DeviceGeneric.cpp
 *
 * Compatible with: atmega328 (Uno, Nano), atmega32u4 (Yun)
 *   ID 42: Nano Test Device on outdoor SWBB bus (testing)
 *
 * Pinout:
 *   1wire PJON data bus (OWPJON SWBB):
 *        1WIRE DATA    -> Arduino Pin D12
 *        GND black     -> Arduino GND
 *   1wire Dallas/Maxim data bus (MicroLAN):
 *        1WIRE DATA    -> Arduino Pin D2
 *   A 4.7k pull-up resistor must be connected to the 1-wire bus (Pin D2).
 *   Needs a weak pull-up (that the uP cannot provide).
 *
 * Example sending READ_DS... commands to device id 42:
 * $ printf "\x4A" | ./owpshell - - 42 | ../../../ThroughSerial/RemoteWorker/DeviceGeneric/unpack.py BBBBBBBB
 * 38, 0, 0, 215, 1, 0, 0, 77
 * $ printf "\x4B" | ./owpshell - - 42 | ../../../ThroughSerial/RemoteWorker/DeviceGeneric/unpack.py BBBBBBBB
 * 40, 66, 243, 63, 5, 0, 0, 10
 * $ printf "\x41" | ./owpshell - - 42 | ../../../ThroughSerial/RemoteWorker/DeviceGeneric/unpack.py f
 * 23.03125,
 * $ printf "\x42" | ./owpshell - - 42 | ../../../ThroughSerial/RemoteWorker/DeviceGeneric/unpack.py f
 * 1.690000057220459,
 * $ printf "\x43" | ./owpshell - - 42 | ../../../ThroughSerial/RemoteWorker/DeviceGeneric/unpack.py f
 * 4.590000152587891,
 * $ printf "\x44" | ./owpshell - - 42 | ../../../ThroughSerial/RemoteWorker/DeviceGeneric/unpack.py f
 * 27.125,
 * More info can be found in @ref OWPJON/README.md. Confer also the docu of
 * following files for info about tests on Ubuntu or Raspberry Pi Server (owpshell):
 *   - @ref OWPJON/LINUX/Local/LocalUDP/RemoteWorker/DeviceGeneric/DeviceGeneric.cpp
 *   - @ref OWPJON/LINUX/Local/ThroughSerial/RemoteWorker/DeviceGeneric/DeviceGeneric.cpp
 *
 * Thanks to:
 * gioblu - PJON 11.0 and support
 *          @see https://www.pjon.org/
 *          @see https://github.com/gioblu/PJON
 * fredilarsen - support
 * @endverbatim
 */

//#define ENABLE_DEBUG
//#define ENABLE_UNITTEST

#define SENSOR     "owp:1w:v1"

#define READ_INFO  0x01  // return generic sensor info
#define READ_VCC   0x11  // return supply voltage
#define READ_TEMP  0x12  // return chip temperature
#define READ       0x21  // return memory data
#define WRITE      0x22  // store memory data
#define WRITE_CAL  0x32  // store calibration value in memory
#define READ_DS2438_TEMP   0x41
#define READ_DS2438_CHA    0x42
#define READ_DS2438_CHB    0x43
#define READ_DS18x20_TEMP  0x44
#define READ_DS2438_ROM    0x4A
#define READ_DS18x20_ROM   0x4B
// more can be added as needed ...
// do not use 0x00 as this is the string terminator

// define the Arduino digital I/O pin to be used for the 1-Wire network here
#define ONE_WIRE_PIN  2

#include <OneWire.h>
#include <DS2438.h>

uint8_t mem_buffer[256];

byte addr[8];

bool DS2438_ENABLE  = false;
byte DS2438_addr[8];
bool DS18x20_ENABLE = false;
byte DS18x20_addr[8];

OneWire ow(ONE_WIRE_PIN);
DS2438 ds2438(&ow, {});

#include <PJON.h>

// <Strategy name> bus(selected device id)
PJON<SoftwareBitBang> bus(42);

bool discoverOneWireDevices(const OneWire &ds, const uint8_t family = NULL);

/**
 *  Arduino IDE: put your setup code here, to run once.
 */
void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); // Initialize LED 13 to be off
#ifdef ENABLE_DEBUG
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
#endif

#ifdef ENABLE_DEBUG
  Serial.begin(9600);

  Serial.println("Looking for 1-Wire devices...");
  discoverOneWireDevices(ow);

  Serial.println("Used 1-Wire devices...");
#endif

  DS2438_ENABLE = discoverOneWireDevices(ow, 0x26);
  if(DS2438_ENABLE) {
    memcpy(DS2438_addr, addr, 8);
    ds2438 = DS2438(&ow, DS2438_addr);
    ds2438.begin();
  }

  DS18x20_ENABLE = discoverOneWireDevices(ow, 0x28);  // DS18B20
  //DS18x20_ENABLE = discoverOneWireDevices(ow, 0x10);  // DS18S20 or old DS1820
  //DS18x20_ENABLE = discoverOneWireDevices(ow, 0x22);  // DS1822
  if(DS18x20_ENABLE) {
    memcpy(DS18x20_addr, addr, 8);
  }

  bus.strategy.set_pin(12);
  bus.begin();
  bus.set_receiver(receiver_function);
};

void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info)
{
  /* Make use of the payload before sending something, the buffer where payload points to is
     overwritten when a new message is dispatched */
#ifdef ENABLE_DEBUG
  //Serial.println(payload);
  Serial.println(payload[0], HEX);
#endif
  digitalWrite(LED_BUILTIN, HIGH);
  switch (payload[0]) {
    // example on how to binary serialize using casting
    // (for human readable format JSON can be used if needed)
  case READ_INFO: {
    // char arrays (string) can be passed directly ...
    bus.reply(SENSOR, strlen(SENSOR));
  }
  break;
  case READ_VCC: {
    // ... other types can be sent by casting its pointer to (char*) ...
    // ... and multiple variables can be sent at once using struct or union.
    // (for most of the values float should be suitable)
    float val = readVcc();
    bus.reply((char*)(&val), sizeof(float));
  }
  break;
  case READ_TEMP: {
    float val = readTemp();
    bus.reply((char*)(&val), sizeof(float));
  }
  break;
  case READ: {
//    bus.reply((char*)mem_buffer, sizeof(mem_buffer));
// TODO: ??? related to PJON_PACKET_MAX_LENGTH ?
    bus.reply((char*)mem_buffer, 36);  // related to PJON_PACKET_MAX_LENGTH ?
  }
  break;
  case WRITE: {
    uint8_t val = length-1;
    memcpy(&mem_buffer[0], &payload[1], val);
    bus.reply((char*)(&val), sizeof(uint8_t));
  }
  break;
  case WRITE_CAL: {
    float val = *((float*)&payload[1]);  // takes the next 4 bytes
    val = val + 1.1;
    bus.reply((char*)(&val), sizeof(float));
  }
  break;
  case READ_DS2438_TEMP: {
    float val = NAN;
#ifndef ENABLE_UNITTEST
    if(DS2438_ENABLE) {
      ds2438.update();
      if (!ds2438.isError())
        val = ds2438.getTemperature();
    }
#else
    val = 11.;
#endif
    bus.reply((char*)(&val), sizeof(float));
  }
  break;
  case READ_DS2438_CHA: {
    float val = NAN;
#ifndef ENABLE_UNITTEST
    if(DS2438_ENABLE) {
      ds2438.update();
      if (!ds2438.isError())
        val = ds2438.getVoltage(DS2438_CHA);
    }
#else
    val = 12.;
#endif
    bus.reply((char*)(&val), sizeof(float));
  }
  break;
  case READ_DS2438_CHB: {
    float val = NAN;
#ifndef ENABLE_UNITTEST
    if(DS2438_ENABLE) {
      ds2438.update();
      if (!ds2438.isError())
        val = ds2438.getVoltage(DS2438_CHB);
    }
#else
    val = 13.;
#endif
    bus.reply((char*)(&val), sizeof(float));
  }
  break;
  case READ_DS18x20_TEMP: {
    float val = NAN;
#ifndef ENABLE_UNITTEST
    if(DS18x20_ENABLE)
      val = readDS18x20(ow, DS18x20_addr);
#else
    val = 21.;
#endif
    bus.reply((char*)(&val), sizeof(float));
  }
  break;
  case READ_DS2438_ROM: {
    bus.reply((char*)(&DS2438_addr), 8);
  }
  break;
  case READ_DS18x20_ROM: {
    bus.reply((char*)(&DS18x20_addr), 8);
  }
  break;
  default:
    // nop
    break;
  }
  //delay(30);
  digitalWrite(LED_BUILTIN, LOW);
}

/**
 *  OneWirePJON device status update function.
 *
 * @return (void)
 */
void loop()
{
  bus.update();
  bus.receive(1000);
};

/**
 *  Accessing the secret voltmeter on the Arduino 168 or 328.
 *
 * @see receiver_function()
 * @see http://code.google.com/p/tinkerit/wiki/SecretVoltmeter
 * @return The supply voltage in [V]
 */
float readVcc()
{
#ifdef ENABLE_UNITTEST
  return 3.7;
#endif
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return result / 1000.;
}

/**
 *  Internal Temperature Sensor for ATmega328 types.
 *
 * @param void
 * @see receiver_function()
 * @see https://playground.arduino.cc/Main/InternalTemperatureSensor
 * @return The chip temperature in [°C]
 */
float readTemp(void)
{
#ifdef ENABLE_UNITTEST
  return 42.42;
#endif
  unsigned int wADC;
  float t;

  // The internal temperature has to be used
  // with the internal reference of 1.1V.
  // Channel 8 can not be selected with
  // the analogRead function yet.

  // Set the internal reference and mux.
  ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
  ADCSRA |= _BV(ADEN);  // enable the ADC

  delay(20);            // wait for voltages to become stable.

  ADCSRA |= _BV(ADSC);  // Start the ADC

  // Detect end-of-conversion
  while (bit_is_set(ADCSRA,ADSC));

  // Reading register "ADCW" takes care of how to read ADCL and ADCH.
  wADC = ADCW;

  // The offset of 324.31 could be wrong. It is just an indication.
//  t = (wADC - 324.31 ) / 1.22;
  t = (wADC - 330.81 ) / 1.22;    // Arduino Uno R3 (mega328)

  // The returned temperature is in degrees Celsius.
  return (t);
}

// https://www.hacktronics.com/Tutorials/arduino-1-wire-address-finder.html
bool discoverOneWireDevices(const OneWire &ds, const uint8_t family = NULL)
{
  byte i;
  byte present = 0;
  byte data[12];
  //byte addr[8];

  ds.reset_search();
  while(ds.search(addr)) {
    //Serial.println("Found \'1-Wire\' device with address:");
#ifdef ENABLE_DEBUG
    Serial.print("ROM = ");
    for( i = 0; i < 8; i++) {
      Serial.print("0x");
      if (addr[i] < 16) {
        Serial.print('0');
      }
      Serial.print(addr[i], HEX);
      if (i < 7) {
        Serial.print(", ");
      }
    }
    Serial.println("");
#endif
    if ( OneWire::crc8( addr, 7) != addr[7]) {
#ifdef ENABLE_DEBUG
      Serial.println("CRC is not valid!");
#endif
      ds.reset_search();
      return false;
    }
    if(addr[0] == family) {
      ds.reset_search();
      return true;
    }
  }
#ifdef ENABLE_DEBUG
  Serial.println("That's it.");
#endif
  ds.reset_search();
  return false;
}

float readDS18x20(const OneWire ds, const byte addr[])
{
  byte i;
  byte present = 0;
  byte type_s = 0;
  byte data[12];
  //byte addr[8];

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end

  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.

  present = ds.reset();
  ds.select(addr);
  ds.write(0xBE);         // Read Scratchpad

//  Serial.print("  Data = ");
//  Serial.print(present, HEX);
//  Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
//    Serial.print(data[i], HEX);
//    Serial.print(" ");
  }
//  Serial.print(" CRC=");
//  Serial.print(OneWire::crc8(data, 8), HEX);
//  Serial.println();

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  return (float)raw / 16.0;
}
