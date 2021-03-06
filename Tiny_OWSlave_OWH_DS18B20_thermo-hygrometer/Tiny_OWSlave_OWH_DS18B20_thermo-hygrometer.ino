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
 *      - https://www.re-innovation.co.uk/docs/enable-brown-out-detect-on-attiny/
 *      - https://github.com/MCUdude/MightyCore#link-time-optimization--lto
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
 *      - Save EEPROM: "EEPROM retained"
 *      - Timer 1 Clock: "CPU"
 *      - B.O.D.: "B.O.D. Disabled"
 *        - try "B.O.D. Enabled (2.7v)" to increase stability (reset if voltage low)
 *      - LTO (1.6.11 + only): "Disabled"
 *        - "Enabled" to reduce compiled code size (does it still run stable?)
 *          (interferes with digispark board: remove board "Digispark AVR" and "attiny", then reinstall "attiny" again)
 *      - Chip: "ATtiny85"
 *      - Clock: "16 MHz (PLL)" - fewer errors, more dorps from bus
 *        - or "8 MHz (internal)" - more errors, fewer drops from bus
 * !      - try crystal in order to have a more stable timebase
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
 * (via 220ohm to 1<-TX)   3  PB3 |2   7| PB2  2   SCL Adafruit_Si7021
 *           1-wire DATA   4  PB4 |3   6| PB1  1   (via 4.7k to LED on GND)
 *                            GND |4   5| PB0  0   SDA Adafruit_Si7021
 *                                -------
 * @see https://github.com/SpenceKonde/ATTinyCore/blob/master/avr/extras/ATtiny_x5.md
 * NOTE! - It's very important to use pullups on the SDA & SCL lines!
 *         I2C pullups should be 1-100k, so 4.7k fits well.
 * optional: - P1 (HWPin 5) a LED can be added to check for regular operation.
 *           - P3 (HWPin 2) can be connected to an Arduino Uno/Nano for serial
 *             output for debugging. The Arduino needs a connection from
 *             RESET to GND to disable the atmega328. For 1 or 8MHz only!
 *           - To use it with (external) 16MHz oscillator remove LED or use attiny84.
 *             According to https://github.com/gioblu/PJON/issues/142 PJON supports 84.
 *             (Using https://github.com/datacute/Tiny4kOLED instead of LED would be
 *             great but memory footprint is too big. Consider using tiny84@16MHz-ext.)
 *
 *                      ATtinyX4 Pin Layout (ATtiny84)
 *                      for 16MHz external oscillator
 *                                ---_---
 *                                .     .
 *                    5V      VCC |1  14| GND
 *                 XTAL1  10  PB0 |2  13| PA0  0
 *                 XTAL2   9  PB1 |3  12| PA1  1   (via 4.7k to LED on GND)
 *                        RST PB3 |4  11| PA2  2
 *                         8  PB2 |5  10| PA3  3   (via 220ohm to 1<-TX ?)
 *                         7  PA7 |6   9| PA4  4   SCL Adafruit_Si7021
 *   Adafruit_Si7021 SDA   6  PA6 |7   8| PA5  5   1-wire DATA
 *                                -------
 * @see https://github.com/SpenceKonde/ATTinyCore/blob/master/avr/extras/ATtiny_x4.md
 * NOTE! - The Pin for 1wire needs to be moved to another Pin, e.g. Pin 5 as 4 is
 *         either occupied by SCL or in alternative mode by SDA.
 */

// In DEBUG mode 1wire interface is replaced by serial ouput
//#define ENABLE_DEBUG

//#define TEMP_OFFSET  -272.9  // offset of internal temp on chip in kelvin (deg celcius)
//#define TEMP_COEFF    1.075  // scaling of internal temp on chip
#define TEMP_OFFSET  -260  // 16 MHz (PLL)
//#define TEMP_OFFSET  -215  // 8 MHz (internal)
#define TEMP_COEFF    1.0

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
//constexpr uint8_t pin_onewire   { 5 };  // for tiny84 with external 16MHz osc.

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
    for (int i=0; i<8; i+=1) {
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
    if (blinking()) {
        // Set temp
//        static float temperature = 20.0;
//        temperature += 0.1;
//        if (temperature > 30.0) temperature = 20.0;
#ifndef ENABLE_DEBUG
//        ds18b20.setTemperature(temperature);
        ds18b20.setTemperature(sensor.readTemperature());
        ds18b21.setTemperature(sensor.readHumidity());
        ds18b22.setTemperature(getVcc());
        ds18b23.setTemperature(getTemp());  // not calibrated
#else
//        mySerial.println(temperature);
        mySerial.print("T: ");
        mySerial.print(sensor.readTemperature(), 2);
        mySerial.print("\tH: ");
        mySerial.print(sensor.readHumidity(), 2);
        mySerial.print("\tVs: ");
        mySerial.print(getVcc());
        mySerial.print("\tTi: ");
        mySerial.println(getTemp());
#endif
    }
}

bool blinking(void)
{
    const uint32_t interval     = 2000;         // interval at which to blink (milliseconds)
    static uint32_t nextMillis  = millis();     // will store next time LED will updated

    if (millis() > nextMillis) {
        nextMillis += interval;             // save the next time you blinked the LED
        static uint8_t ledState = LOW;      // ledState used to set the LED
        if (ledState == LOW)    ledState = HIGH;
        else                    ledState = LOW;
        digitalWrite(pin_led, ledState);
        return 1;
    }
    return 0;
}

// https://github.com/cano64/ArduinoSystemStatus/blob/master/SystemStatus.cpp
// http://21stdigitalhome.blogspot.ch/2014/10/trinket-attiny85-internal-temperature.html
// (https://www.mikrocontroller.net/topic/315667)
// (https://hackaday.com/2014/02/01/atmega-attiny-core-temperature-sensors/)
float getTemp(void)
{
//    ADCSRA &= ~(_BV(ADATE) |_BV(ADIE)); // Clear auto trigger and interrupt enable
//    ADCSRA |= _BV(ADEN);                // Enable AD and start conversion
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

float getVcc(void)
{
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
