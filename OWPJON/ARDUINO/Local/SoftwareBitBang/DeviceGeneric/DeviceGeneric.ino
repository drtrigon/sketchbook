/**
 * @brief Example-Code for a generic Device (e.g. Sensor, Display)
 *
 * @file OWPJON/ARDUINO/Local/SoftwareBitBang/DeviceGeneric/DeviceGeneric.ino
 *
 * @author drtrigon
 * @date 2018-07-06
 * @version 1.0
 *   @li first version derived from PJON 11.0 examples
 *       examples/ARDUINO/Local/SoftwareBitBang/BlinkWithResponse/Receiver/Receiver.ino
 *
 * @ref OWPJON/ARDUINO/Local/SoftwareBitBang/BlinkWithResponse/Receiver/Receiver.ino
 * @see https://github.com/gioblu/PJON/blob/master/examples/ARDUINO/Local/SoftwareBitBang/BlinkWithResponse/Receiver/Receiver.ino
 *
 * @verbatim
 * OneWire PJON Generic "OWPG" scheme:
 *   Server e.g. linux machine or raspi
 *      OWPJON/LINUX/Local/LocalUDP/RemoteWorker/DeviceGeneric/
 *      OWPJON/LINUX/Local/ThroughSerial/RemoteWorker/DeviceGeneric/
 *   Tunnel(er) similar to 1wire master (similar cause we are on a multi-master bus) e.g. AVR
 *      OWPJON/ARDUINO/Local/SoftwareBitBang/Tunneler/BlinkingSwitch/
 *      OWPJON/ARDUINO/Local/SoftwareBitBang/Tunneler/BlinkingSwitch_SWBB-TS/
 *      OWPJON/ARDUINO/Local/ThroughSerial/SoftwareBitBangSurrogate/Surrogate/ (obsolete)
 *   Devices e.g. AVR
 *      OWPJON/ARDUINO/Local/SoftwareBitBang/DeviceGeneric/ (this sketch)
 *      OWPJON/ARDUINO/Local/SoftwareBitBang/OWP_DG_LCD_Sensors/
 *      ...
 *
 * Compatible with: atmega328 (Uno, Nano), atmega32u4 (Yun)
 *   ID 42: Nano Test Device on outdoor SWBB bus (testing)
 *   ID 44: Nano Test Device on indoor SWBB bus (productive; collectd)
 *
 * Pinout:
 *   1wire PJON data bus (OWPJON SWBB):
 *        1WIRE DATA    -> Arduino Pin D12
 *        GND black     -> Arduino GND
 *
 * Test on Ubuntu or Raspberry Pi Server (owpshell) confer the docu of
 * following files:
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

#define TEMP_OFFSET  -309.00
#define TEMP_COEFF      1.22

#define SENSOR     "owp:dg:v1"
#define OWPJONID   44
#define OWPJONPIN  12

#define READ_INFO  0x01  // return generic sensor info
#define READ_VCC   0x11  // return supply voltage
#define READ_TEMP  0x12  // return chip temperature
#define READ       0x21  // return memory data
#define WRITE      0x22  // store memory data
#define WRITE_CAL  0x32  // store calibration value in memory
// more can be added as needed ...
// do not use 0x00 as this is the string terminator

uint8_t mem_buffer[256];

#include <PJON.h>

// <Strategy name> bus(selected device id)
PJON<SoftwareBitBang> bus(OWPJONID);

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

  bus.strategy.set_pin(OWPJONPIN);
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
  t = (wADC + TEMP_OFFSET ) / TEMP_COEFF;

  // The returned temperature is in degrees Celsius.
  return (t);
}
