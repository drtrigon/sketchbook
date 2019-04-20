/**
 * @brief Example-Code for a Nano Sensor Station
 *        (SEN02281P, MIC, GUVA-S12SD, TSL2561, BME280)
 *
 * @file OWPJON/ARDUINO/Local/SoftwareBitBang/DeviceGeneric/DeviceGeneric.ino
 *
 * @author drtrigon
 * @date 2018-07-06
 * @version 1.0
 *   @li first version derived from
 *       OWPJON/ARDUINO/Local/SoftwareBitBang/DeviceGeneric/DeviceGeneric.ino
 *       Uno_OWSlave/OWH_DS18B20_asInterface_SensorStation/OWH_DS18B20_asInterface_SensorStation.ino
 *
 * @ref OWPJON/ARDUINO/Local/SoftwareBitBang/DeviceGeneric/DeviceGeneric.ino
 * @ref Uno_OWSlave/OWH_DS18B20_asInterface_SensorStation/OWH_DS18B20_asInterface_SensorStation.ino
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
 *        1WIRE DATA    -> Arduino Pin D8
 *        GND black     -> Arduino GND
 *
 * Test on Ubuntu or Raspberry Pi Server (owpshell) confer the docu of
 * following files:
 *   - @ref OWPJON/LINUX/Local/LocalUDP/RemoteWorker/DeviceGeneric/DeviceGeneric.cpp
 *   - @ref OWPJON/LINUX/Local/ThroughSerial/RemoteWorker/DeviceGeneric/DeviceGeneric.cpp
 *   - use ThroughSerial testing like:
 *     $ printf "\x01" | ./owpshell-ubuntu14.04 /dev/ttyUSB0 9600 40
 *     owp:ss:v1
 *     $ printf "\x53" | ./owpshell-ubuntu14.04 /dev/ttyUSB0 9600 40 | python unpack.py f
 *     970.3897094726562,
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

#define SENSOR     "owp:ss:v1"
#define OWPJONID   40
#define OWPJONPIN   8

#include "OWPJON-sensor-commands.h"

#define SEN02281P    A3    // SEN02281P analog pin
#define GUVAS12SD    A7    // GUVA-S12SD analog pin
#define MIC          A6    // MIC analog pin

#include <Wire.h>
#include <Adafruit_Sensor.h>

// based on Adafruit_TSL2561 sensor driver
#include <Adafruit_TSL2561_U.h>

// based on Adafruit_BME280 sensor driver
#include <Adafruit_BME280.h>

uint8_t mem_buffer[256];

//  Commonly used variables
unsigned int mmin, mmax, val, mavg;
uint16_t broadband, infrared;
sensors_event_t event;

Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
Adafruit_BME280 bme; // I2C

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

    /* Initialise the TSL2561 sensor */
    if(!tsl.begin())
    {
#ifdef ENABLE_DEBUG
        /* There was a problem detecting the ADXL345 ... check your connections */
        Serial.print("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!");
        while(1);
#endif
    }
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

    /* Initialise the BMP183 sensor */
    if(!bme.begin())
    {
#ifdef ENABLE_DEBUG
        /* There was a problem detecting the BMP183 ... check your connections */
        Serial.print("Ooops, no BME280 detected ... Check your wiring!");
        while(1);
#endif
    }

#ifdef ENABLE_DEBUG
  Serial.println(SENSOR);
  Serial.println(readVcc());
  Serial.println(readTemp());
  Serial.println(val);
//  Serial.println(mem_buffer);
  Serial.println(analogRead(SEN02281P));
  Serial.println(analogRead(MIC));
  Serial.println(analogRead(GUVAS12SD) * 0.01);  // mW/cm^2
  tsl.getEvent(&event);
  Serial.println(event.light);  // 0 = "sensor overload"
  broadband = 0;
  infrared = 0;
  tsl.getLuminosity (&broadband, &infrared);
  /* Scale for gain (1x or 16x) */
  if (!tsl._tsl2561Gain) broadband *= 16;
  if (!tsl._tsl2561Gain) infrared *= 16;
  Serial.println(broadband);
  Serial.println(infrared);
  Serial.println(bme.readTemperature());
  Serial.println(bme.readPressure() / 100.0F);
  Serial.println(bme.readHumidity());
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
  case READ_AUDIO_CHA: {
    // "A5" (SEN02281P; Loudness (Grove))
//    float val = analogRead(SEN02281P)/1023.;  // arbitrary unit
    mmin = 1023;
    mmax = 0;
    for(unsigned long i=0; i<1000; ++i) {
        val = analogRead(SEN02281P);
        mmin = min(mmin, val);   // simple if ... < should be faster...
        mmax = max(mmax, val);   // "
        //delayMicroseconds(1);
    }
    // unit amplitude in rel. voltage, calibration needed
    float val = log((mmax-mmin)/1023.);
    bus.reply((char*)(&val), sizeof(float));
  }
  break;
  case READ_AUDIO_CHB: {
    // MIC "sensor": Sparkfun ...
    mmin = 1023;
    mmax = 0;
    for(unsigned long i=0; i<1000; ++i) {
        val = analogRead(MIC);
        mmin = min(mmin, val);   // simple if ... < should be faster...
        mmax = max(mmax, val);   // "
        //delayMicroseconds(1);
    }
    // unit amplitude in rel. voltage, calibration needed
    float val = log((mmax-mmin)/1023.);
    bus.reply((char*)(&val), sizeof(float));
  }
  break;
  case READ_LIGHT_UV: {
    // UV sensor: Adafruit GUVA-S12SD
    /* Display the results [mW/cm^2] or [UV index] */
    float val = (analogRead(GUVAS12SD) * 0.01);  // mW/cm^2
    //float val = (analogRead(GUVAS12SD) * 0.049); // UV index
    bus.reply((char*)(&val), sizeof(float));
  }
  break;
  case READ_LIGHT_VIS: {
    // Light sensor: Adafruit_TSL2561
    /* Get a new sensor event */
    tsl.getEvent(&event);
    /* Display the results [lux] */
    float val = event.light;  // 0 = "sensor overload"
    bus.reply((char*)(&val), sizeof(float));
  }
  break;
  case READ_LIGHT_BB: {
    // Light sensor: Adafruit_TSL2561
    /* Get a new sensor event */
    broadband = 0;
    infrared = 0;
    /* Populate broadband and infrared with the latest values */
    tsl.getLuminosity (&broadband, &infrared);
    /* Scale for gain (1x or 16x) */
    if (!tsl._tsl2561Gain) broadband *= 16;
    if (!tsl._tsl2561Gain) infrared *= 16;
//    float val[2] = {broadband, infrared};
//    bus.reply((char*)(&val), sizeof(float)*2);
    float val = broadband;
    bus.reply((char*)(&val), sizeof(float));
  }
  break;
  case READ_LIGHT_IR: {
    // Light sensor: Adafruit_TSL2561
    /* Get a new sensor event */
    broadband = 0;
    infrared = 0;
    /* Populate broadband and infrared with the latest values */
    tsl.getLuminosity (&broadband, &infrared);
    /* Scale for gain (1x or 16x) */
    if (!tsl._tsl2561Gain) broadband *= 16;
    if (!tsl._tsl2561Gain) infrared *= 16;
    float val = infrared;
    bus.reply((char*)(&val), sizeof(float));
  }
  break;
  case READ_SENS_TEMP: {
    // Barometric pressure sensor: Adafruit_BME280 (Temp)
    /* First we get the current temperature from the BMP085 [*C] */
    float val = bme.readTemperature();
    bus.reply((char*)(&val), sizeof(float));
  }
  break;
  case READ_SENS_PRES: {
    // Barometric pressure sensor: Adafruit_BME280 (Press)
    /* Display atmospheric pressue in Hecto-Pascals [mbar] */
    float val = bme.readPressure() / 100.0F;
    /* Display atmospheric pressue in [bar] */
    //float val = bme.readPressure() / 100000.0F;
    bus.reply((char*)(&val), sizeof(float));
  }
  break;
  case READ_SENS_HUM: {
    // Barometric pressure sensor: Adafruit_BME280 (Humid)
    /* Display humidity in [%] */
    float val = bme.readHumidity();
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
