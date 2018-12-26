/**
 * @brief Example-Code for a generic ATTiny85 Device (e.g. Sensor, Display)
 *
 * @file OWPJON/ATTINY/Local/SoftwareBitBang/DeviceGeneric/DeviceGeneric.ino
 *
 * @author drtrigon
 * @date 2018-08-08
 * @version 1.0
 *   @li first version derived from
 *       Tiny_OWSlave_OWH_DS18B20_thermo-hygrometer/Tiny_OWSlave_OWH_DS18B20_thermo-hygrometer.ino
 *       OWPJON/ARDUINO/Local/SoftwareBitBang/DeviceGeneric/DeviceGeneric.ino
 *       @see https://github.com/gioblu/PJON/wiki/ATtiny-interfacing
 *
 * @ref Tiny_OWSlave_OWH_DS18B20_thermo-hygrometer/Tiny_OWSlave_OWH_DS18B20_thermo-hygrometer.ino
 *
 * @verbatim
 * OneWire PJON Generic "OWPG" scheme:
 *   @ref OWPJON/LINUX/Local/LocalUDP/RemoteWorker/DeviceGeneric/DeviceGeneric.cpp
 *
 * Compatible with: atmega328 (Uno, Nano), atmega32u4 (Yun), attiny85
 *
 * Pinout:
 *                      ATtinyX5 Pin Layout (ATtiny85)
 *                                ---_---
 *                                .     .
 *                        RST PB5 |1   8| VCC      5V
 * (via 220ohm to 1<-TX)   3  PB3 |2   7| PB2  2   SCL Adafruit_Si7021
 *           OWPJON DATA   4  PB4 |3   6| PB1  1   (via 4.7k to LED on GND)
 *                            GND |4   5| PB0  0   SDA Adafruit_Si7021
 *                                -------
 *
 * Thanks to:
 * gioblu - PJON 11.0 and support
 *          @see https://www.pjon.org/
 *          @see https://github.com/gioblu/PJON
 * fredilarsen - support
 * @endverbatim
 */

//#define ENABLE_DEBUG  // ATmega328 (not ATtiny85)
#define ENABLE_UNITTEST

//#define TEMP_OFFSET  -260  // 16 MHz (PLL)
#define TEMP_OFFSET  -215  // 8 MHz (internal)
#define TEMP_COEFF    1.0

#define SENSOR     "owp:dg:tiny:v1"

#define READ_INFO  0x01  // return generic sensor info
#define READ_VCC   0x11  // return supply voltage
#define READ_TEMP  0x12  // return chip temperature
#define READ       0x21  // return memory data
#define WRITE      0x22  // store memory data
#define WRITE_CAL  0x32  // store calibration value in memory
#define READ_SI7021_TEMP  0x44  // see OWP_DG_1w-adaptor READ_DS18x20_TEMP
#define READ_SI7021_HUM   0x42  // see OWP_DG_1w-adaptor READ_DS2438_CHA
// more can be added as needed ...
// do not use 0x00 as this is the string terminator

#include "Adafruit_Si7021_Tiny.h"  // lib using adafruit/TinyWireM

constexpr uint8_t _LED_BUILTIN  { 1 };
constexpr uint8_t pin_onewire   { 4 };

Adafruit_Si7021_Tiny sensor = Adafruit_Si7021_Tiny();

uint8_t mem_buffer[16];

/*// https://github.com/gioblu/PJON/blob/master/documentation/configuration.md#extended-configuration
// https://github.com/gioblu/PJON/issues/222
#define PJON_INCLUDE_PACKET_ID true
//#define PJON_INCLUDE_ASYNC_ACK true
// Max number of old packet ids stored to avoid duplication
#define PJON_MAX_RECENT_PACKET_IDS 3  // by default 10*/

// https://github.com/gioblu/PJON/wiki/ATtiny-interfacing
#define PJON_INCLUDE_SWBB true // Include only SoftwareBitBang
// Max 10 characters packet (including overhead)
//#define PJON_PACKET_MAX_LENGTH 10
#define PJON_PACKET_MAX_LENGTH 30
// Avoid using packet buffer
#define PJON_MAX_PACKETS        0
#include <PJON.h>

// <Strategy name> bus(selected device id)
PJON<SoftwareBitBang> bus(99);

/**
 *  Arduino IDE: put your setup code here, to run once.
 */
void setup()
{
  bus.strategy.set_pin(pin_onewire);
  bus.begin();
  bus.set_receiver(receiver_function);

#ifdef ENABLE_DEBUG
  Serial.begin(9600);
#endif

  pinMode(_LED_BUILTIN, OUTPUT);

  // initialize I2C TinyWireM sensor lib
  sensor.begin();

  for (int i=0; i<8; i+=1) {
    analogWrite(_LED_BUILTIN, B00000001<<i);
    delay(500);
  }
};

void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info)
{
  /* Make use of the payload before sending something, the buffer where payload points to is
     overwritten when a new message is dispatched */
#ifdef ENABLE_DEBUG
  //Serial.println(payload);
  Serial.println(payload, HEX);
#endif
  digitalWrite(_LED_BUILTIN, HIGH);
  switch (payload[0]) {
    // example on how to binary serialize using casting
    // (for human readable format JSON can be used if needed)
  case READ_INFO: {
    // char arrays (string) can be passed directly ...
    //bus.reply(SENSOR, strlen(SENSOR));
    //bus.send_packet_blocking(45, SENSOR, strlen(SENSOR));
    bus.send_packet(45, SENSOR, strlen(SENSOR));
  }
  break;
  case READ_VCC: {
    // ... other types can be sent by casting its pointer to (char*) ...
    // ... and multiple variables can be sent at once using struct or union.
    // (for most of the values float should be suitable)
    float val = readVcc();
    bus.send_packet(45, (char*)(&val), sizeof(float));
  }
  break;
  case READ_TEMP: {
    float val = readTemp();
    bus.send_packet(45, (char*)(&val), sizeof(float));
  }
  break;
  case READ: {
//    bus.reply((char*)mem_buffer, sizeof(mem_buffer));
    bus.send_packet(45, (char*)mem_buffer, 14);
  }
  break;
  case WRITE: {
    uint8_t val = length-1;
    memcpy(&mem_buffer[0], &payload[1], val);
    bus.send_packet(45, (char*)(&val), sizeof(uint8_t));
  }
  break;
  case WRITE_CAL: {
    float val = *((float*)&payload[1]);  // takes the next 4 bytes
    val = val + 1.1;
    bus.send_packet(45, (char*)(&val), sizeof(float));
  }
  break;
  case READ_SI7021_TEMP: {
    float val = sensor.readTemperature();
    bus.reply((char*)(&val), sizeof(float));
  }
  break;
  case READ_SI7021_HUM: {
    float val = sensor.readHumidity();
    bus.reply((char*)(&val), sizeof(float));
  }
  break;
  default:
    // nop
    break;
  }
  //delay(30);
  digitalWrite(_LED_BUILTIN, LOW);
}

/**
 *  OneWirePJON device status update function.
 *
 * @return (void)
 */
void loop()
{
  //bus.update();
  bus.receive(1000);
};

/**
 *  Accessing the secret voltmeter on the ATtiny85 (not ATmega328).
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
  //reads internal 1V1 reference against VCC
  ADMUX = _BV(MUX3) | _BV(MUX2); // For ATtiny85/45
  //ADMUX = 0xF | _BV( REFS1 );         // ADC4 (Temp Sensor) and Ref voltage = 1.1V;
  delay(10); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA, ADSC));
  //uint8_t low = ADCL;
  //unsigned int val = (ADCH << 8) | low;
  unsigned int val = ADC;
  //discard previous result
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA, ADSC));
  //low = ADCL;
  //val = (ADCH << 8) | low;
  val = ADC;

  //return ((long)1024 * 1100) / val;
  return ((float)1024 * 1.1) / val;
}

/**
 *  Internal Temperature Sensor for the ATtiny85 (not ATmega328).
 *
 * @param void
 * @see receiver_function()
 * @see https://github.com/cano64/ArduinoSystemStatus/blob/master/SystemStatus.cpp
 * @see http://21stdigitalhome.blogspot.ch/2014/10/trinket-attiny85-internal-temperature.html
 * @return The chip temperature in [°C]
 */
float readTemp(void)
{
#ifdef ENABLE_UNITTEST
  return 42.42;
#endif
//  ADCSRA &= ~(_BV(ADATE) |_BV(ADIE)); // Clear auto trigger and interrupt enable
//  ADCSRA |= _BV(ADEN);                // Enable AD and start conversion
  //ADMUX = (1<<REFS0) | (1<<REFS1) | (1<<MUX3); //turn 1.1V reference and select ADC8
  ADMUX = 0xF | _BV( REFS1 );         // ADC4 (Temp Sensor) and Ref voltage = 1.1V;
  delay(10); //wait for internal reference to settle
  // start the conversion
  ADCSRA |= bit(ADSC);
  //sbi(ADCSRA, ADSC);
  // ADSC is cleared when the conversion finishes
  while (ADCSRA & bit(ADSC));
  //while (bit_is_set(ADCSRA, ADSC));
  //uint8_t low  = ADCL;
  //uint8_t high = ADCH;
  int a = ADC;
  //discard first reading
  ADCSRA |= bit(ADSC);
  while (ADCSRA & bit(ADSC));
  //low  = ADCL;
  //high = ADCH;
  //a = (high << 8) | low;
  a = ADC;
  //ADCSRA &= ~(_BV(ADEN));        // disable ADC
  // Temperature compensation using the chip voltage would go here
  //return a - 272; //return temperature in C
  return(((float)a + TEMP_OFFSET) / TEMP_COEFF);
}
