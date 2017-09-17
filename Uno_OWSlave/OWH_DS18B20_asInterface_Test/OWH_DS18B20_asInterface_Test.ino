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
 *    
 *    -> new "Generic", "SensorStation" interface
 */

constexpr bool enable_debug = 0;

#include "OneWireHub.h"
#include "DS18B20.h"

/*#include <Wire.h>
/*#include "i2c.h"

#include "i2c_TCS3772.h"
TCS3772 tcs3772;

#include "i2c_MPL3115A2.h"
MPL3115A2 mpl3115;

#include "i2c_SI7021.h"
SI7021 si7021;*/

constexpr uint8_t pin_led       { 13 };
constexpr uint8_t pin_onewire   { 8 };

auto hub    = OneWireHub(pin_onewire);

auto ds18b0 = DS18B20(0x28, 0x00, 0x55, 0x44, 0x33, 0x22, 0x11); // Light, RED, without unit, int16 from 0 to 2047
auto ds18b1 = DS18B20(0x28, 0x01, 0x55, 0x44, 0x33, 0x22, 0x11); // Light, GREEN, same as above
auto ds18b2 = DS18B20(0x28, 0x02, 0x55, 0x44, 0x33, 0x22, 0x11); // Light, BLUE, same as above
auto ds18b3 = DS18B20(0x28, 0x03, 0x55, 0x44, 0x33, 0x22, 0x11); // Light, CLEAR, same as above
auto ds18b4 = DS18B20(0x28, 0x04, 0x55, 0x44, 0x33, 0x22, 0x11); // Pressure, pascal - sealevel (101325), int16 from -2048 to +2047
auto ds18b5 = DS18B20(0x28, 0x05, 0x55, 0x44, 0x33, 0x22, 0x11); // humidity, percent * 16, int16 from 0 to 1600 (translates to 0 to 100%)
auto ds18b6 = DS18B20(0x28, 0x06, 0x55, 0x44, 0x33, 0x22, 0x11); // temp, °C*16, int16 from -2048 to +2047 (translates to -128 to 128°C)
auto ds18b7 = DS18B20(0x28, 0x07, 0x55, 0x44, 0x33, 0x22, 0x11); // unused

//#include <avr/wdt.h>    // watchdog

bool blinking(void);

void setup()
{
//    wdt_reset();
//    wdt_enable (WDTO_250MS);

    if (enable_debug)
    {
        Serial.begin(115200);
        Serial.println("OneWire-Hub DS18B20 Temperature-Sensor");
        Serial.flush();

        Serial.print("Probe TCS3772: ");
//        if (tcs3772.initialize()) Serial.println("Sensor found");
//        else Serial.println("Sensor missing");

        Serial.print("Probe MPL3115A2: ");
//        if (mpl3115.initialize()) Serial.println("Sensor found");
//        else Serial.println("Sensor missing");

        Serial.print("Probe SI7021: ");
//        if (si7021.initialize()) Serial.println("Sensor found!");
//        else Serial.println("Sensor missing");
    }
    else
    {
//        tcs3772.initialize();
//        mpl3115.initialize();
//        si7021.initialize();
    }

//    tcs3772.setAGain(4);
//    tcs3772.setATime(17);

//    mpl3115.setEnabled(0); // manual mode,onetime-measure:
//    mpl3115.setAltimeter(0);
//    mpl3115.triggerMeasurement();

//    si7021.triggerMeasurement();

    pinMode(pin_led, OUTPUT);

    // Setup OneWire
    hub.attach(ds18b0);
    hub.attach(ds18b1);
    hub.attach(ds18b2);
    hub.attach(ds18b3);
    hub.attach(ds18b4);
    hub.attach(ds18b5);
    hub.attach(ds18b6);
    //hub.attach(ds18b7);

//    wdt_reset();

    updateSensorTCS();
    updateSensorMPL();
    updateSensorSI7();
}

void updateSensorTCS(void) // 8560 559
{
    uint16_t value_crgb[4];

//    tcs3772.getMeasurement(value_crgb);

    if (enable_debug)
    {
        Serial.print("R: ");
        Serial.print(value_crgb[1]);
        Serial.print(" G: ");
        Serial.print(value_crgb[2]);
        Serial.print(" B: ");
        Serial.print(value_crgb[3]);
        Serial.print(" C: ");
        Serial.print(value_crgb[0]);
        Serial.println("");
    }

    for (uint8_t i=0; i<4; ++i)
    {
        value_crgb[i] = value_crgb[i]>>2;
        if (value_crgb[i] > 2047) value_crgb[i] = 2047;
    }

/*    ds18b0.setTemperatureRaw(value_crgb[1]);
    ds18b1.setTemperatureRaw(value_crgb[2]);
    ds18b2.setTemperatureRaw(value_crgb[3]);
    ds18b3.setTemperatureRaw(value_crgb[0]);*/

    long vcc = readVcc();
    double tmp = GetTemp();

    ds18b0.setTemperatureRaw(vcc);
    ds18b1.setTemperatureRaw(tmp);
    ds18b2.setTemperatureRaw(vcc);
    ds18b3.setTemperatureRaw(tmp);
}

void updateSensorMPL(void)
{
    constexpr uint32_t pascal_offset = 101325; // at sealevel

    uint32_t pascal;
//    mpl3115.getPressure(pascal); // gives pascal in 18bit resolution
    const int16_t pascal16 = pascal - pascal_offset;

//    mpl3115.triggerMeasurement();

    if (enable_debug)
    {
        Serial.print("Pascal: ");
        Serial.print(pascal16);
        Serial.println("");
    }

//    ds18b4.setTemperatureRaw(pascal16);

    uint32_t test = 4242;

    ds18b4.setTemperatureRaw(test);
}

void updateSensorSI7(void)
{
    float humi, temp;

//    si7021.getHumidity(humi);
//    si7021.getTemperature(temp);
//    si7021.triggerMeasurement();

    if (enable_debug)
    {
        Serial.print("TEMP: ");
        Serial.print(temp);
        Serial.print(" HUMI: ");
        Serial.print(humi);
        Serial.println("");
    }

    const int16_t temp16 = temp * 16; // natural ds18b20, should be safe from +127 to -128
    const int16_t humi16 = humi * 16; // natural ds18b20

//    ds18b5.setTemperatureRaw(humi16);
//    ds18b6.setTemperatureRaw(temp16);

    long vcc = readVcc();
    double tmp = GetTemp();

    ds18b5.setTemperatureRaw(tmp);
    ds18b6.setTemperatureRaw(vcc);
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
        if (process == 0) updateSensorTCS();
        if (process == 1) updateSensorMPL();
        if (process == 2) updateSensorSI7();
        if (++process>2)  process = 0;
    }
}

bool blinking(void)
{
    constexpr  uint32_t interval    = 2000;          // interval at which to blink (milliseconds)
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
  t = (wADC - 324.31 ) / 1.22;

  // The returned temperature is in degrees Celsius.
  return (t);
}
