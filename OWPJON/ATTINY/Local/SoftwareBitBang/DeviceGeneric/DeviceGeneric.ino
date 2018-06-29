/*
 *    Example-Code for a generic Device (e.g. Sensor, Display)
 *    (OneWire PJON Generic "OWPG")
 *
 *    Use eg. together:
 *      Uno_OWPJON_Test/LINUX/Local/ThroughSerial/RemoteWorker/DeviceGeneric
 *      Uno_OWPJON_Test/ARDUINO/Local/ThroughSerial/SoftwareBitBangSurrogate/Surrogate
 *      Uno_OWPJON_Test/ARDUINO/Local/SoftwareBitBang/DeviceGeneric (this sketch)
 *    or
 *      Uno_OWPJON_Test/LINUX/Local/LocalUDP/RemoteWorker/DeviceGeneric/
 *      Uno_OWPJON_Test/ARDUINO/Local/SoftwareBitBang/Tunneler/BlinkingSwitch/
 *      Uno_OWPJON_Test/ARDUINO/Local/SoftwareBitBang/DeviceGeneric (this sketch)
 *
 *    Compatible with: atmega328 (Uno, Nano), atmega32u4 (Yun), attiny85
 *    @see https://github.com/gioblu/PJON/wiki/ATtiny-interfacing
 */

//#define ENABLE_DEBUG  // ATmega328 (not ATtiny85)
#define ENABLE_UNITTEST

#define TEMP_OFFSET  -272.9  // offset of internal temp on chip in kelvin (deg celcius)
#define TEMP_COEFF    1.075  // scaling of internal temp on chip

#define SENSOR     "owp:dg:tiny:v1"

#define READ_INFO  0x01  // return generic sensor info
#define READ_VCC   0x11  // return supply voltage
#define READ_TEMP  0x12  // return chip temperature
#define READ       0x21  // return memory data
#define WRITE      0x22  // store memory data
#define WRITE_CAL  0x32  // store calibration value in memory
// more can be added as needed ...
// do not use 0x00 as this is the string terminator

uint8_t mem_buffer[16];

// https://github.com/gioblu/PJON/wiki/ATtiny-interfacing
#define PJON_INCLUDE_SWBB true // Include only SoftwareBitBang
// Max 10 characters packet (including overhead)
//#define PJON_PACKET_MAX_LENGTH 10
#define PJON_PACKET_MAX_LENGTH 30
// Avoid using packet buffer
#define PJON_MAX_PACKETS        0
#include <PJON.h>

// <Strategy name> bus(selected device id)
PJON<SoftwareBitBang> bus(44);

/**
 *  Arduino IDE: put your setup code here, to run once.
 */
void setup()
{
  bus.strategy.set_pin(2);
  bus.begin();
  bus.set_receiver(receiver_function);

#ifdef ENABLE_DEBUG
  Serial.begin(9600);
#endif
};

void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info)
{
  /* Make use of the payload before sending something, the buffer where payload points to is
     overwritten when a new message is dispatched */
#ifdef ENABLE_DEBUG
  //Serial.println(payload);
  Serial.println(payload, HEX);
#endif
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
  default:
    // nop
    break;
  }
  //delay(30);
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
