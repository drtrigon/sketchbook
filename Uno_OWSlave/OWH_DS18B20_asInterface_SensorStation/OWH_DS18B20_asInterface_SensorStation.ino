/*
 *    Example-Code that emulates a bunch of DS18B20 and puts real sensor-values in it:
 *    - light-Intensity: red, green, blue, white
 *    - air-pressure in pascal
 *    - relative humidity in percent
 *    - temperature in degC
 *
 *    Tested with:
 *    - DS9490R-Master, atmega328@16MHz as Slave
 *    - OW-SERVER ENET+owshell, Nano as Slave
 *    - LinkHubE + owfshttpd, Nano as Slave
 *      --> ONLY FIRST 8 SLAVES RECOGNIZED !!
 *      --> BMP183 broken?
 */
/********************************************************************************
    Program. . . . OWH_DS18B20_asInterface_SensorStation
    Author . . . . Ursin Solèr (according to design orgua)
    Written. . . . 24 Sep 2017.
    Description. . Act as swarm of One Wire Slaves (emulates multiple DS18B20)
                   Read all the analog inputs (A0-A5) and return values on demand
    Hardware . . . Arduino Uno/Nano (w/ ATmega328)
?    Hardware . . .
?            AT.... Pin Layout (Arduino UNO)
?                  ---_---
?                  .     .
?              RST |?   ?| PD13       LED
?              3V3 |?   ?| PD12
? VCC    B_5V   5V |?   ?| PD11 B_DS0 TCS3200D S0 (Output scaling/gain)
?              GND |?   ?| PD10 B_DS1 TCS3200D S1 (Output scaling/gain)
? Ground BGND  GND |?   ?| PD09 B_DS2 TCS3200D S2 (Photodiode type/color)
?              Vin |?   ?| PD08 B_DS3 TCS3200D S3 (Photodiode type/color)
? Sens 1 B_S01  A0 |?   ?| PD07 B_S11 Sens 11 (TCS3200D)
?                  .     .
? Sens 4 B_S04  A3 |?   ?| PD02       One Wire
? Sens 5 B_S05  A4 |?   ?| PD01
? Sens 6 B_S06  A5 |?   ?| PD00
?                  -------

********************************************************************************

0x 01 00 00 00   -- float --   read MQ-135 (A0) and return float / Air Quality
0x 04 00 00 00   -- float --   read MQ-2 (A3) and return float / Flamable/Combustible gas
0x 06 00 00 00   -- float --   read SEN02281P and return float / Loudness (Grove)
0x 0C 00 00 00   -- float --   read MIC "sensor" as log and return float
0x 0B 00 00 00   -- float --   read UV sensor and return float
0x 0D 00 00 00   -- float --   read TSL2561 and return float
0x 0E 00 00 00   -- float --   read TSL2561 (broadband) and return float
0x 0F 00 00 00   -- float --   read TSL2561 (IR) and return float
0x 1A 00 00 00   -- float --   read BMP183 and return float
0x 1B 00 00 00   -- float --   read BMP183 (Temp) and return float

0x FF 02 00 00   -- float --   get "RTC" counts: millis()/1000.
0x FF 03 00 00   -- float --   measure supply voltage Vcc
             ?   -- float --   measure chip temperature

********************************************************************************/

// In DEBUG mode 1wire interface is replaced by serial ouput
constexpr bool enable_debug = 0;
//constexpr bool enable_debug = 1;

//#define SoftwareVer 2.0

#include "OneWireHub.h"
#include "DS18B20.h"

#include <Adafruit_Sensor.h>

// based on Adafruit_TSL2561 sensor driver
#include <Wire.h>
#include <Adafruit_TSL2561_U.h>

// based on Adafruit_BMP183 sensor driver
#include <SPI.h>
#include <Adafruit_BMP183.h>

//  Pin Layouts
constexpr uint8_t pin_led       { 13 };
constexpr uint8_t pin_onewire   { 8 };

//  MeasPins, MUX, TCS, software SPI (define your own pins; use A2 instead of 13 as clk)
#define MQ135        A0    // MQ-135 analog pin
#define MQ2          A1    // MQ-2 analog pin
#define SEN02281P    A3    // SEN02281P analog pin
#define GUVAS12SD    A7    // GUVA-S12SD analog pin
#define MIC          A6    // MIC analog pin
//I2C (Wire) library occupies A4 (SDA), A5 (SCL)
//#define BMP183_CLK  13
#define BMP183_CLK   A2    // CLK (hardware SPI needs pin 13 here!)
//#define BMP183_CLK    9    // CLK (hardware SPI needs pin 13 here!)
#define BMP183_SDO   12    // AKA MISO
#define BMP183_SDI   11    // AKA MOSI
#define BMP183_CS    10    // chip-select pin (use any pin)

Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
//Adafruit_BMP183 bmp = Adafruit_BMP183(BMP183_CS);    // initialize with hardware SPI
Adafruit_BMP183 bmp = Adafruit_BMP183(BMP183_CLK, BMP183_SDO, BMP183_SDI, BMP183_CS);    // or initialize with software SPI and use any 4 pins

auto hub    = OneWireHub(pin_onewire);

auto ds18b00 = DS18B20(0x28, 0x00, 0x55, 0x44, 0x33, 0x22, 0x11); // 0x 01 00 00 00   -- float --   read MQ-135 (A0) and return float / Air Quality
// Light, RED, without unit, int16 from 0 to 2047
auto ds18b01 = DS18B20(0x28, 0x01, 0x55, 0x44, 0x33, 0x22, 0x11); // 0x 04 00 00 00   -- float --   read MQ-2 (A3) and return float / Flamable/Combustible gas
// Light, GREEN, same as above
auto ds18b02 = DS18B20(0x28, 0x02, 0x55, 0x44, 0x33, 0x22, 0x11); // 0x 06 00 00 00   -- float --   read SEN02281P and return float / Loudness (Grove)
// Light, BLUE, same as above
auto ds18b03 = DS18B20(0x28, 0x03, 0x55, 0x44, 0x33, 0x22, 0x11); // 0x 0C 00 00 00   -- float --   read MIC "sensor" as log and return float
// Light, CLEAR, same as above
auto ds18b04 = DS18B20(0x28, 0x04, 0x55, 0x44, 0x33, 0x22, 0x11); // 0x 0B 00 00 00   -- float --   read UV sensor and return float
// Pressure, pascal - sealevel (101325), int16 from -2048 to +2047
auto ds18b05 = DS18B20(0x28, 0x05, 0x55, 0x44, 0x33, 0x22, 0x11); // 0x 0D 00 00 00   -- float --   read TSL2561 and return float
// humidity, percent * 16, int16 from 0 to 1600 (translates to 0 to 100%)
auto ds18b06 = DS18B20(0x28, 0x06, 0x55, 0x44, 0x33, 0x22, 0x11); // 0x 0E 00 00 00   -- float --   read TSL2561 (broadband) and return float
// temp, °C*16, int16 from -2048 to +2047 (translates to -128 to 128°C)
auto ds18b07 = DS18B20(0x28, 0x07, 0x55, 0x44, 0x33, 0x22, 0x11); // 0x 0F 00 00 00   -- float --   read TSL2561 (IR) and return float
// unused
auto ds18b08 = DS18B20(0x28, 0x08, 0x55, 0x44, 0x33, 0x22, 0x11); // 0x 1A 00 00 00   -- float --   read BMP183 and return float
// Light, RED, without unit, int16 from 0 to 2047
auto ds18b09 = DS18B20(0x28, 0x09, 0x55, 0x44, 0x33, 0x22, 0x11); // 0x 1B 00 00 00   -- float --   read BMP183 (Temp) and return float
// Light, GREEN, same as above
auto ds18b10 = DS18B20(0x28, 0x0A, 0x55, 0x44, 0x33, 0x22, 0x11); // 0x FF 02 00 00   -- float --   get "RTC" counts: millis()/1000.
// Light, BLUE, same as above
auto ds18b11 = DS18B20(0x28, 0x0B, 0x55, 0x44, 0x33, 0x22, 0x11); // 0x FF 03 00 00   -- float --   measure supply voltage Vcc
// Light, CLEAR, same as above
auto ds18b12 = DS18B20(0x28, 0x0C, 0x55, 0x44, 0x33, 0x22, 0x11); //              ?   -- float --   measure chip temperature
// unused
//auto ds18b13 = DS18B20(0x28, 0x0D, 0x55, 0x44, 0x33, 0x22, 0x11); // Pressure, pascal - sealevel (101325), int16 from -2048 to +2047
//auto ds18b14 = DS18B20(0x28, 0x0E, 0x55, 0x44, 0x33, 0x22, 0x11); // humidity, percent * 16, int16 from 0 to 1600 (translates to 0 to 100%)
//auto ds18b15 = DS18B20(0x28, 0x0F, 0x55, 0x44, 0x33, 0x22, 0x11); // temp, °C*16, int16 from -2048 to +2047 (translates to -128 to 128°C)

//#include <avr/wdt.h>    // watchdog

bool blinking(void);

//  Commonly used variables
unsigned int mmin, mmax, val, mavg;
uint16_t broadband, infrared;
sensors_event_t event;

void setup()
{
//    wdt_reset();
//    wdt_enable (WDTO_250MS);

    if (enable_debug)
    {
        Serial.begin(115200);
        Serial.println("Nano_OWSlave/OWH_DS18B20_asInterface_SensorStation:");
        Serial.println("OneWire-Hub multiple DS18B20");
        String stringOne =  String(ds18b00.ID[0], HEX) + String(ds18b00.ID[1], HEX) + String(ds18b00.ID[2], HEX) + String(ds18b00.ID[3], HEX) + String(ds18b00.ID[4], HEX) + String(ds18b00.ID[5], HEX) + String(ds18b00.ID[6], HEX);
        Serial.println(stringOne);
        Serial.println("...");
        stringOne =  String(ds18b12.ID[0], HEX) + String(ds18b12.ID[1], HEX) + String(ds18b12.ID[2], HEX) + String(ds18b12.ID[3], HEX) + String(ds18b12.ID[4], HEX) + String(ds18b12.ID[5], HEX) + String(ds18b12.ID[6], HEX);
        Serial.println(stringOne);
        Serial.println("Version: 1");
        Serial.println("OWHub: 2.2.0");
        Serial.println(__DATE__);
        Serial.println(__TIME__);
        stringOne =  String(ARDUINO, DEC);
        Serial.print(stringOne);
        Serial.print("/");
        Serial.println(__VERSION__);
        Serial.println(__FILE__);
        Serial.print("Vs: ");
        Serial.println(readVcc()/1000.);
        Serial.print("Ti: ");
        Serial.println(GetTemp());

        Serial.print("Probe TSL2561: ");
        if (tsl.begin()) Serial.println("Sensor found");
        else Serial.println("Sensor missing (check your wiring or I2C ADDR)");

        Serial.print("Probe BMP183: ");
        if (bmp.begin()) Serial.println("Sensor found");
        else Serial.println("Sensor missing (check your wiring)");
    }
    else
    {
        tsl.begin();
        bmp.begin();
    }

    /* Initialise the TSL2561 sensor */
    /* Display some basic information on this sensor */
//    displaySensorDetailsTSL();
    /* Setup the sensor gain and integration time */
    /* You can also manually set the gain or enable auto-gain support */
    // tsl.setGain(TSL2561_GAIN_1X);      /* No gain ... use in bright light to avoid sensor saturation */
    // tsl.setGain(TSL2561_GAIN_16X);     /* 16x gain ... use in low light to boost sensitivity */
    tsl.enableAutoRange(true);            /* Auto-gain ... switches automatically between 1x and 16x */
    /* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
    tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
    // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
    // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */

    // Initialise the Pin usage
    pinMode(pin_led, OUTPUT);

    // set pins to defined state (not needed to enable analogRead())
    pinMode(MQ135, INPUT);
    pinMode(MQ2, INPUT);
    pinMode(SEN02281P, INPUT);
    pinMode(GUVAS12SD, INPUT);
    pinMode(MIC, INPUT);

    // Setup OneWire
    hub.attach(ds18b00);
    hub.attach(ds18b01);
    hub.attach(ds18b02);
    hub.attach(ds18b03);
    hub.attach(ds18b04);
    hub.attach(ds18b05);
    hub.attach(ds18b06);
    hub.attach(ds18b07);
    hub.attach(ds18b08);
    hub.attach(ds18b09);
    hub.attach(ds18b10);
    hub.attach(ds18b11);
    hub.attach(ds18b12);
    //hub.attach(ds18b13);
    //hub.attach(ds18b14);
    //hub.attach(ds18b15);

//    wdt_reset();

//    updateSensor();

    if (enable_debug)
        Serial.println("config done");
}

void loop()
{
//    wdt_reset();
    // following function must be called periodically
    hub.poll();

    // Blink triggers the state-change
    if (blinking())
    {
        static uint8_t process = 0;
        updateSensor(process);
        //if (++process>11)  process = 0;
        process = (process + 1) % 12;    // sensor update steps 0...11 (more than 1 sensor per step possible)
    }
}

bool blinking(void)
{
    const  uint32_t interval    = 2000;          // interval at which to blink (milliseconds)
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

/********************************************************************************
    Process ... and prepare sensor readings
********************************************************************************/
void updateSensor(uint8_t process){
//    process = 7;
    switch (process) {
        case 0:                                 // "A0" (MQ-135)
            pack(ds18b00, analogRead(MQ135)/1023.);
            // unit ppm, calibration needed
            break;
        case 1:                                 // "A3" (MQ-2)
            pack(ds18b01, analogRead(MQ2)/1023.);
            // unit ppm, calibration needed
            break;
        case 2:                                 // "A5" (SEN02281P; Loudness (Grove))
//            pack(ds18b02, analogRead(SEN02281P)/1023.);
//            // arbitrary unit
//            break;
            mmin = 1023;
            mmax = 0;
            //for(unsigned long i=0; i<10000; ++i) {
            for(unsigned long i=0; i<1000; ++i) {
                val = analogRead(SEN02281P);
                mmin = min(mmin, val);   // simple if ... < should be faster...
                mmax = max(mmax, val);   // "
                //delayMicroseconds(1);
            }
            pack(ds18b02, log((mmax-mmin)/1023.));
            // unit amplitude in rel. voltage, calibration needed
            break;
        case 3:                                 // MIC "sensor": Sparkfun ...
            mmin = 1023;
            mmax = 0;
            //for(unsigned long i=0; i<10000; ++i) {
            for(unsigned long i=0; i<1000; ++i) {
                val = analogRead(MIC);
                mmin = min(mmin, val);   // simple if ... < should be faster...
                mmax = max(mmax, val);   // "
                //delayMicroseconds(1);
            }
            pack(ds18b03, log((mmax-mmin)/1023.));
            // unit amplitude in rel. voltage, calibration needed
            break;
        case 4:                                 // UV sensor: Adafruit GUVA-S12SD
            /* Display the results [mW/cm^2] or [UV index] */
            pack( ds18b04, (analogRead(GUVAS12SD) * 0.01) );  // mW/cm^2
            //pack( (analogRead(MeasPin) * 0.049) ); // UV index
            break;
        case 5:                                 // Light sensor: Adafruit_TSL2561
            /* Get a new sensor event */
            tsl.getEvent(&event);
            /* Display the results [lux] */
            pack(ds18b05, event.light);  // 0 = "sensor overload"
            break;
        case 6:                                 // Light sensor: Adafruit_TSL2561
            /* Get a new sensor event */
            broadband = 0;
            infrared = 0;
            /* Populate broadband and infrared with the latest values */
            tsl.getLuminosity (&broadband, &infrared);
            /* Scale for gain (1x or 16x) */
            if (!tsl._tsl2561Gain) broadband *= 16;
            if (!tsl._tsl2561Gain) infrared *= 16;
            pack(ds18b06, broadband);
            pack(ds18b07, infrared);
            break;
        case 7:                                 // Barometric pressure sensor: Adafruit_BMP183
            ///* Display atmospheric pressue in Hecto-Pascals [mbar] */
            //pack(ds18b08, bmp.getPressure() / 100.);
            /* Display atmospheric pressue in [bar] */
            pack(ds18b08, bmp.getPressure() / 100000.);
            break;
        case 8:                                 // Barometric pressure sensor: Adafruit_BMP183
            /* First we get the current temperature from the BMP085 [*C] */
            pack(ds18b09, bmp.getTemperature());
            break;

/*        case 9:                                 // Internal "System" Commands (ISC)
            pack(SoftwareVer);
            break;*/

        case 9:                                 // Internal "System" Commands (ISC)
            pack(ds18b10, millis()/1000.);
            break;
        case 10:                                 // Internal "System" Commands (ISC)
            // Measure and calculate supply voltage on Arduino 168 or 328 [V]
            pack(ds18b11, readVcc()/1000.);
            break;
        case 11:                                 // Internal "System" Commands (ISC)
            // Measure and calculate supply voltage on Arduino 168 or 328 [V]
            pack(ds18b12, GetTemp());
            break;

        default: // nop/pass
            break;
    }
}

/********************************************************************************
    Pack float into ... (compatible with python, owfs, ???)
    Print debug info to serial output
********************************************************************************/
void pack(DS18B20& ds18b, float val) {
    if (enable_debug)
    {
        Serial.print(millis()/1000.);
        Serial.print(" : ");
        String stringOne =  String(ds18b.ID[0], HEX) + String(ds18b.ID[1], HEX) + String(ds18b.ID[2], HEX) + String(ds18b.ID[3], HEX) + String(ds18b.ID[4], HEX) + String(ds18b.ID[5], HEX) + String(ds18b.ID[6], HEX);
        Serial.print(stringOne);
        Serial.print(" : ");
        Serial.print(val);
        Serial.println("");
    }

//    ds18b.setTemperatureRaw(static_cast<int16_t>(val));
    ds18b.setTemperature(val);    // -55...125 allowed
}

/********************************************************************************
    Accessing the secret voltmeter on the Arduino 168 or 328
    http://code.google.com/p/tinkerit/wiki/SecretVoltmeter
********************************************************************************/
long readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return result;
}

/********************************************************************************
    Internal Temperature Sensor for ATmega328 types
    https://playground.arduino.cc/Main/InternalTemperatureSensor
********************************************************************************/
double GetTemp(void)
{
  unsigned int wADC;
  double t;

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
  t = (wADC - 330.81 ) / 1.22;

  // The returned temperature is in degrees Celsius.
  return (t);
}
