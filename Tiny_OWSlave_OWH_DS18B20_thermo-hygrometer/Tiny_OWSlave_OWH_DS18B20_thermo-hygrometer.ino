/*
 *    Example-Code that emulates a DS18B20 on ATtiny85
 *
 *    Tested with:
 *    - https://github.com/PaulStoffregen/OneWire --> DS18x20-Example, atmega328@16MHz as Slave
 *    - DS9490R-Master, atmega328@16MHz and teensy3.2@96MHz as Slave
 *    - LinkHubE + owfshttpd, ATtiny85 as Slave
 *    - OW-SERVER ENET + owshell, ATtiny85 as Slave, Port 3 seems to be broken
 *    - low-power mode ... ? currently uses < 10 mA not connected to 1wire
 *      http://www.rocketscream.com/blog/2011/07/04/lightweight-low-power-arduino-library/
 *      https://www.seeed.cc/ATTINY%20and%20ATMEGA's%20internal%20temperature%20sensor-p-161.html
 *    - tune internal OSC for 1wire timing
 *      http://becomingmaker.com/tuning-attiny-oscillator/
 *
 *    OneWireHub_config.h:
 *    - need to increase ONEWIRE_TIME_MSG_HIGH_TIMEOUT to 150000_us (x10):
 *      //constexpr timeOW_t ONEWIRE_TIME_MSG_HIGH_TIMEOUT     = { 15000_us };        // there can be these inactive / high timeperiods after reset / presence, this value defines the timeout for these
 *      constexpr timeOW_t ONEWIRE_TIME_MSG_HIGH_TIMEOUT     = { 150000_us };        // there can be these inactive / high timeperiods after reset / presence, this value defines the timeout for these
 *    - need to keep HUB_SLAVE_LIMIT on 8 (saves memory):
 *      #define HUB_SLAVE_LIMIT     8 // set the limit of the hub HERE, max is 32 devices
 *      //#define HUB_SLAVE_LIMIT    16 // set the limit of the hub HERE, max is 32 devices
 *    
 *    Programm ATtiny85 using Arduino Uno as ISP:
 *    - Links
 *      - https://playground.boxtec.ch/doku.php/arduino/attiny
 *      - https://sebastian.expert/could-not-find-usbtiny-device-0x23410x49/
 *      - https://learn.sparkfun.com/tutorials/tiny-avr-programmer-hookup-guide/attiny85-use-hints
 *      - https://forum.arduino.cc/index.php?topic=425532.0
 *      - http://forum.arduino.cc/index.php?topic=128963.0
 *      - http://www.ernstc.dk/arduino/tinycom.html
 *      - http://forum.arduino.cc/index.php?topic=91125.0
 *      - https://www.mikrocontroller.net/topic/88806
 *      - https://playground.arduino.cc/Code/USIi2c
 *      - https://learn.adafruit.com/adafruit-si7021-temperature-plus-humidity-sensor/wiring-and-test
 *    - use Arduino IDE >=1.8.3
 *    - install ATtiny option to IDE
 *      - File > Preferences > Additional Boards Manager URLs: http://drazzy.com/package_drazzy.com_index.json
 *      - Tools > Board: ??? > Boards Manager...
 *        search for: "tiny" and install (ATTinyCore 1.1.4)
 *    - upload ISP sketch to an Uno
 *      - File > Examples > ArduinoISP > ArduinoISP
 *    - insert ATtiny85 to programming (zero-force) socket and wire it:
 *        Arduino Pin   ATtiny Pin
 *          MOSI  D11   5
 *          MISO  D12   6
 *          SCK   D13   7
 *        Reset   D10   1
 *                GND   4
 *                VCC   8
 *        (>=10uF cap on Arduino Uno RESET pin not needed)
 *    - select ATtiny85:
 *      - Board: "ATtiny25/45/85"
 *      - Timer 1 Clock: "CPU"
 *      - B.O.D.: "B.O.D. Disabled"
 *      - LTO (1.6.11 + only): "Disabled"
 *      - Chip: "ATtiny85"
 *      - Clock: "8 MHz (internal)"
 *      - ( Clock: "16 MHz (PLL)" )
 *      - Port: ???
 *    - select Programmer: 
 *      - Tools > Programmer: "Arduino as ISP"
 *        (NOT "ArduinoISP" !)
 *    - set fuse settings (clock, etc.): 
 *      - Tools > Burn Bootloader
 *    - upload this code (programmer LED needs to be off)
 *    
 *                      ATtinyX5 Pin Layout (ATtiny85)
 *                                ---_---
 *                                .     .
 *                        RST PB5 |1   8| VCC      5V
 * (via 220ohm to 1<-TX)   A3 PB3 |2   7| PB2 A1   SCL Adafruit_Si7021
 *           1-wire DATA   A2 PB4 |3   6| PB1 PWM  (via 4.7k to LED on GND)
 *                            GND |4   5| PB0 PWM  SDA Adafruit_Si7021
 *                                -------
 * NOTE! - It's very important to use pullups on the SDA & SCL lines!
 *         I2C pullups should be 1-100k, so 4.7k fits well.
 * optional: - P0 (Pin 5) a LED can be added to check for regular operation.
 *           - P3 (Pin 2) can be connected to an Arduino Uno/Nano for serial
 *             output for debugging. The Arduino needs a connection from
 *             RESET to GND to disable the atmega328. For 1 or 8MHz only!
 */

// In DEBUG mode 1wire interface is replaced by serial ouput
//#define ENABLE_DEBUG

#include "OneWireHub.h"
#include "DS18B20.h"     // Digital Thermometer, 12bit
#include "Adafruit_Si7021_Tiny.h"  // lib using adafruit/TinyWireM
#ifdef ENABLE_DEBUG
#include <TinyDebugSerial.h>  // small mem footprint lib
#endif

// https://bigdanzblog.wordpress.com/2014/10/24/arduino-watchdog-timer-wdt-example-code/
// https://bigdanzblog.wordpress.com/2015/07/20/resetting-rebooting-attiny85-with-watchdog-timer-wdt/#comment-1297
// http://www.avrfreaks.net/comment/515785#comment-515785
#include <avr/wdt.h>    // watchdog

constexpr uint8_t pin_led       { 1 };
constexpr uint8_t pin_onewire   { 4 };

auto hub     = OneWireHub(pin_onewire);

auto ds18b20 = DS18B20(DS18B20::family_code, 0x00, 0x00, 0xB2, 0x18, 0xDA, 0x00); // DS18B20: 9-12bit, -55 -  +85 degC
#ifndef ENABLE_DEBUG
auto ds18b21 = DS18B20(DS18B20::family_code, 0x01, 0x00, 0xB2, 0x18, 0xDA, 0x00); // DS18B20: 9-12bit, -55 -  +85 degC
auto ds18b22 = DS18B20(DS18B20::family_code, 0x02, 0x00, 0xB2, 0x18, 0xDA, 0x00); // DS18B20: 9-12bit, -55 -  +85 degC
auto ds18b23 = DS18B20(DS18B20::family_code, 0x03, 0x00, 0xB2, 0x18, 0xDA, 0x00); // DS18B20: 9-12bit, -55 -  +85 degC
#endif

Adafruit_Si7021_Tiny sensor = Adafruit_Si7021_Tiny();

#ifdef ENABLE_DEBUG
TinyDebugSerial mySerial = TinyDebugSerial();
#endif

bool blinking(void);

void setup()
{
    // - immediately disable watchdog timer so set will not get interrupted
    // - any 'slow' activity needs to be completed before enabling the watchdog timer.
    // - the following forces a pause before enabling WDT. This gives the IDE a chance to
    //   call the bootloader in case something dumb happens during development and the WDT
    //   resets the MCU too quickly. Once the code is solid, remove this.
    MCUSR &= ~(1<<WDRF); // reset status flag (needed for tiny)
    wdt_disable();

#ifdef ENABLE_DEBUG
    // initialize TinyDebugSerial lib (needs Arduino Board)
    mySerial.begin(115200);
    mySerial.println("\nOneWire-Hub DS18B20 (Tiny)");
    mySerial.flush();
#endif

    pinMode(pin_led, OUTPUT);

    // initialize I2C TinyWireM sensor lib
    sensor.begin();

    // Setup OneWire
    hub.attach(ds18b20);
#ifndef ENABLE_DEBUG
    hub.attach(ds18b21);
    hub.attach(ds18b22);
    hub.attach(ds18b23);
#endif

#ifdef ENABLE_DEBUG
    // Test-Cases: the following code is just to show basic functions, can be removed any time
    // Test - set Temperatures to -56 degC (out of range)
    mySerial.print("Test - set -56 degC (underflow): ");
    ds18b20.setTemperature(int8_t(-56));
    mySerial.println(ds18b20.getTemperature());


    mySerial.println("conf done");
#endif

//    for (int i=0; i<=255; i+=5)
    for (int i=0; i<8; i+=1)
    {
        analogWrite(pin_led, B00000001<<i);
        delay(500);
    }

    wdt_reset();
    wdt_enable(WDTO_8S);
}

void loop()
{
    wdt_reset();
    // following function must be called periodically
#ifndef ENABLE_DEBUG
    hub.poll();
#endif
#ifdef ENABLE_DEBUG
    // this part is just for debugging (USE_SERIAL_DEBUG in OneWire.h must be enabled for output)
    if (hub.hasError()) hub.printError();
#endif

    // Blink triggers the state-change
    if (blinking())
    {
        // Set temp
//        static float temperature = 20.0;
//        temperature += 0.1;
//        if (temperature > 30.0) temperature = 20.0;
#ifndef ENABLE_DEBUG
//        ds18b20.setTemperature(temperature);
        ds18b20.setTemperature(sensor.readTemperature());
        ds18b21.setTemperature(sensor.readHumidity());
        ds18b22.setTemperature(getVcc());
        ds18b23.setTemperature(getTemp());  // 10deg low, works not well, may be better with 16MHz?
#else
//        mySerial.println(temperature);
        mySerial.print("T: "); mySerial.print(sensor.readTemperature(), 2);
        mySerial.print("\tH: "); mySerial.print(sensor.readHumidity(), 2);
        mySerial.print("\tVs: "); mySerial.print(getVcc());
        mySerial.print("\tTi: "); mySerial.println(getTemp());
#endif
    }
}

bool blinking(void)
{
    const uint32_t interval     = 2000;         // interval at which to blink (milliseconds)
    static uint32_t nextMillis  = millis();     // will store next time LED will updated

    if (millis() > nextMillis)
    {
        nextMillis += interval;             // save the next time you blinked the LED
        static uint8_t ledState = LOW;      // ledState used to set the LED
        if (ledState == LOW)    ledState = HIGH;
        else                    ledState = LOW;
        digitalWrite(pin_led, ledState);
        return 1;
    }
    return 0;
}

/* 
 * Trinket / ATtiny85 Temperature and Voltage Measurement
 * http://21stdigitalhome.blogspot.ch/2014/10/trinket-attiny85-internal-temperature.html
 * 
 * Adapted from
 * http://forum.arduino.cc/index.php/topic,26299.0.html,
 * http://www.mikrocontroller.net/topic/315667, and
 * http://goetzes.gmxhome.de/FOSDEM-85.pdf
*/
float getTemp(void)
{
  // Setup the Analog to Digital Converter -  one ADC conversion
  //   is read and discarded
  ADCSRA &= ~(_BV(ADATE) |_BV(ADIE)); // Clear auto trigger and interrupt enable
  ADCSRA |= _BV(ADEN);                // Enable AD and start conversion
  ADMUX = 0xF | _BV( REFS1 );         // ADC4 (Temp Sensor) and Ref voltage = 1.1V;
  delay(10);                         // Settling time min 1 ms, take 100 ms
  getADC();

  int i;
//  int t_celsius; 
  float t_celsius; 
  uint8_t vccIndex;
  float rawTemp, rawVcc;
  
  // Measure temperature
  ADCSRA |= _BV(ADEN);           // Enable AD and start conversion
  ADMUX = 0xF | _BV( REFS1 );    // ADC4 (Temp Sensor) and Ref voltage = 1.1V;
  delay(10);                    // Settling time min 1 ms, wait 100 ms

  rawTemp = (float)getADC();     // use next sample as initial average
//  for (int i=2; i<2000; i++) {   // calculate running average for 2000 measurements
//    rawTemp += ((float)getADC() - rawTemp) / float(i); 
//  }  
  ADCSRA &= ~(_BV(ADEN));        // disable ADC  
  
  rawVcc = getVcc();

  //index 0..13 for vcc 1.7 ... 3.0
  vccIndex = min(max(17,(uint8_t)(rawVcc * 10)),30) - 17;   

  // Temperature compensation using the chip voltage 
  // with 3.0 V VCC is 1 lower than measured with 1.7 V VCC 
//  t_celsius = (int)(chipTemp(rawTemp) + (float)vccIndex / 13);  
  t_celsius = (chipTemp(rawTemp) + (float)vccIndex / 13);  

  return t_celsius;
}

float getVcc(void)
{
  // Setup the Analog to Digital Converter -  one ADC conversion
  //   is read and discarded
  ADCSRA &= ~(_BV(ADATE) |_BV(ADIE)); // Clear auto trigger and interrupt enable
  ADCSRA |= _BV(ADEN);                // Enable AD and start conversion
  ADMUX = 0xF | _BV( REFS1 );         // ADC4 (Temp Sensor) and Ref voltage = 1.1V;
  delay(10);                         // Settling time min 1 ms, take 100 ms
  getADC();

  int i;
  float rawVcc;
  
  // Measure chip voltage (Vcc)
  ADCSRA |= _BV(ADEN);   // Enable ADC
  ADMUX  = 0x0c | _BV(REFS2);    // Use Vcc as voltage reference,
                                 //    bandgap reference as ADC input
  delay(10);                    // Settling time min 1 ms, there is
                                 //    time so wait 100 ms
  rawVcc = (float)getADC();      // use next sample as initial average
//  for (int i=2; i<2000; i++) {   // calculate running average for 2000 measurements
//    rawVcc += ((float)getADC() - rawVcc) / float(i);
//  }
  ADCSRA &= ~(_BV(ADEN));        // disable ADC
  
  rawVcc = 1024 * 1.1f / rawVcc;

  return rawVcc;
}

// Calibration of the temperature sensor has to be changed for your own ATtiny85
// per tech note: http://www.atmel.com/Images/doc8108.pdf
float chipTemp(float raw) {
//  const float chipTempOffset = 272.9;           // Your value here, it may vary 
  const float chipTempOffset = 242.9;           // Your value here, it may vary
  const float chipTempCoeff = 1.075;            // Your value here, it may vary
  return((raw - chipTempOffset) / chipTempCoeff);
}
 
// Common code for both sources of an ADC conversion
int getADC() {
  ADCSRA  |=_BV(ADSC);          // Start conversion
  while((ADCSRA & _BV(ADSC)));    // Wait until conversion is finished
  return ADC;
}

