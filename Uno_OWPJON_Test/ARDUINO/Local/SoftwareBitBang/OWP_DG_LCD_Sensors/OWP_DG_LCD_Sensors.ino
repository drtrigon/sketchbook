/*
 *    Example-Code for a generic Device (e.g. Sensor, Display)
 *    (OneWire PJON Generic "OWPG")
 *
 *    Use eg. together:
 *      Uno_OWPJON_Test/LINUX/Local/ThroughSerial/RemoteWorker/DeviceGeneric
 *      Uno_OWPJON_Test/ARDUINO/Local/ThroughSerial/SoftwareBitBangSurrogate/Surrogate
 *      Uno_OWPJON_Test/ARDUINO/Local/SoftwareBitBang/DeviceGeneric (this sketch)
 *
 *    Compatible with: atmega328 (Uno, Nano), atmega32u4 (Yun)
 */

//#define ENABLE_DEBUG
#define ENABLE_UNITTEST

#define SENSOR     "owp:dg:lcd:v1"

#define READ_INFO  0x01  // return generic sensor info
#define READ_VCC   0x11  // return supply voltage
#define READ_TEMP  0x12  // return chip temperature
#define READ       0x21  // return memory data
#define WRITE      0x22  // store memory data
#define WRITE_CAL  0x32  // store calibration value in memory
// more can be added as needed ...
// do not use 0x00 as this is the string terminator

//uint8_t mem_buffer[255];

//#define FREQ_BLINK_OK   2000  // fast blink 0.5 Hz all OK
//#define FREQ_BLINK_ERR  4000  // slow blink 0.25 Hz no update for >3min
//// hint: bigger interval makes the 1wire bus more responsive

#define pin_onewire   12
#define pin_led       13

uint8_t mem_read[22];
uint8_t mem_buffer[256];
uint8_t mem_buffer_[256];

unsigned long RAM_lRebootCount __attribute__ ((section (".noinit")));   // these variables retain uncanged in memory during reboot
unsigned long RAM_lRebootCount_ __attribute__ ((section (".noinit")));  // copy for checksum (power-up detection)
unsigned long EEPROM_lPowerCount;                                       // these variables retain uncanged in eeprom during power-cycle
unsigned long EEPROM_lPowerCount_;                                      // copy for checksum (first use detection)*/

unsigned long time_last_update;

// https://bigdanzblog.wordpress.com/2014/10/24/arduino-watchdog-timer-wdt-example-code/
#include <avr/wdt.h>    // watchdog
#include <EEPROM.h>

#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

SSD1306AsciiWire oled;

#include <PJON.h>

// <Strategy name> bus(selected device id)
PJON<SoftwareBitBang> bus(44);  // Unique-Device-ID (UD-ID)

/**
 *  Arduino IDE: put your setup code here, to run once.
 */
void setup()
{
  // - immediately disable watchdog timer so set will not get interrupted
  // - any 'slow' activity needs to be completed before enabling the watchdog timer.
  // - the following forces a pause before enabling WDT. This gives the IDE a chance to
  //   call the bootloader in case something dumb happens during development and the WDT
  //   resets the MCU too quickly. Once the code is solid, remove this.
  wdt_disable();

  Wire.begin();
  oled.begin(&Adafruit128x64, 0x3C);
  oled.setFont(System5x7);  // 8 lines, 21.1 rows on the 128x64
  oled.clear();
  //oled.set2X();
  oled.println("OWPJON DG LCD+Sens");
  oled.println(SENSOR);
  oled.println("Ver: 1.0 / PJON: 11.0");
  oled.print(__DATE__);
  oled.print(" ");
  oled.println(__TIME__);
  String stringOne = String(ARDUINO, DEC);
  oled.print(stringOne.c_str());
  oled.print(" / ");
  oled.println(__VERSION__);
  oled.print("Vs: ");
  oled.print(readVcc());
  oled.print(" / Ti: ");
  oled.println(readTemp());

  // https://www.arduino.cc/en/Reference/EEPROM
  EEPROM.get(sizeof(unsigned long)*0, EEPROM_lPowerCount);
  EEPROM.get(sizeof(unsigned long)*1, EEPROM_lPowerCount_);
  if (EEPROM_lPowerCount != EEPROM_lPowerCount_) {  // setup-up reset
    EEPROM_lPowerCount  = 0;
    EEPROM_lPowerCount_ = 0;
  }
  // https://forum.arduino.cc/index.php?topic=232362.0
  if (RAM_lRebootCount != RAM_lRebootCount_) {      // power-up reset
    RAM_lRebootCount  = 0;
    RAM_lRebootCount_ = 0;
    ++EEPROM_lPowerCount;
    //++EEPROM_lPowerCount_;
// writing to eeprom limited to ~100'000 times thus disabled
//    EEPROM.put(sizeof(unsigned long)*0, EEPROM_lPowerCount);
//    EEPROM.put(sizeof(unsigned long)*1, EEPROM_lPowerCount_);
// writing to eeprom disabled
  }
  oled.print("P: ");
  oled.print(EEPROM_lPowerCount);
  oled.print(" / R: ");
  oled.println(RAM_lRebootCount);
  ++RAM_lRebootCount;
  ++RAM_lRebootCount_;

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); // Initialize LED 13 to be off

  mem_read[22] = 0;

  bus.strategy.set_pin(pin_onewire);
  bus.begin();
  bus.set_receiver(receiver_function);

  oled.println("config done");

  delay(2000);

  oled.clear();

  time_last_update = millis();

  wdt_reset();
  wdt_enable(WDTO_8S);

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
  digitalWrite(LED_BUILTIN, HIGH);
  switch (payload[0]) {
  // example on how to binary serialize using casting
  // (for human readable format JSON can be used if needed)
  case READ_INFO: {
    // char arrays (string) can be passed directly ...
    bus.reply(SENSOR, strlen(SENSOR));
    } break;
  case READ_VCC: {
    // ... other types can be sent by casting its pointer to (char*) ...
    // ... and multiple variables can be sent at once using struct or union.
    // (for most of the values float should be suitable)
    float val = readVcc();
    bus.reply((char*)(&val), sizeof(float));
    } break;
  case READ_TEMP: {
    float val = readTemp();
    bus.reply((char*)(&val), sizeof(float));
    } break;
  case READ: {
//    bus.reply((char*)mem_buffer, sizeof(mem_buffer));
// TODO: ??? related to PJON_PACKET_MAX_LENGTH ? default:50 ?
    bus.reply((char*)mem_buffer, 36);  // related to PJON_PACKET_MAX_LENGTH ?
    } break;
  case WRITE: {
    uint8_t val = length-1;
    memcpy(&mem_buffer[0], &payload[1], val);
    //bus.reply((char*)(&val), sizeof(uint8_t));

    // read in blocks of 21 byte due to the LCD (8x21 =168bytes)
    for(char i=0; i < 8 ; i++) {
      //oled.setCursor(0,5);
      //oled.print(i);
      if(memcmp(&mem_buffer[i*21], &mem_buffer_[i*21], 21) != 0) {
        wdt_reset();

        time_last_update = millis();

        memcpy(&mem_buffer_[i*21], &mem_buffer[i*21], 21);
        memcpy(mem_read, &mem_buffer[i*21], 21);

        oled.setCursor(0,i);           //Set the cursor to (i)th line, 0th Column
        //mem_buffer_[(i+1)*21] = 0;
        //oled.print(reinterpret_cast<const char *>(&mem_buffer_[i*21]));
        oled.print(reinterpret_cast<const char *>(mem_read));
      }
    }

    bus.reply((char*)(&val), sizeof(uint8_t));
    } break;
  case WRITE_CAL: {
    float val = *((float*)&payload[1]);  // takes the next 4 bytes
    val = val + 1.1;
    bus.reply((char*)(&val), sizeof(float));
    } break;
// TODO: add functions to get reboot and power-on count, add function to control "backlight"
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
  wdt_reset();
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
//    t = (wADC - 324.31 ) / 1.22;
    t = (wADC - 330.81 ) / 1.22;    // Arduino Uno R3 (mega328)

    // The returned temperature is in degrees Celsius.
    return (t);
}
