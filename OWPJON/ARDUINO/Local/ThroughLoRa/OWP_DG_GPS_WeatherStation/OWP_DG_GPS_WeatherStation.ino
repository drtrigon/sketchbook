/**
 * needs doxygen docu - derived from ThroughLoRa/DeviceGeneric, Uno_Dragino_LoRa_GPS_Shield_TTN and https://github.com/Play-Zone/BME280_MAX44009/blob/master/example.ino
 * also derived from https://www.arduinolibraries.info/libraries/bme280 and https://www.arduinolibraries.info/libraries/i2-c-sensor-lib-i-lib
 * ...
 *
 * Arduino Uno (5V):
 * use it with dragino lora gps shield (set ENABLE_GPS) or with
 * lorabee only (do not set ENABLE_GPS)
 * weatherstation breakout is always needed
 * consider using hardware serial for gps (if gps used in OWPJON device)
 *
 * Arduino Mini 3.3V:
 * It is a minial atmega328 w. 8MHz oscil. so a procedure similar to
 * "Using on (minimal/raw) atmega328 w. 16MHz oscil." as mentioned
 * in @see OWPJON/ARDUINO/Local/SoftwareBitBang/OWP_DG_1w-adaptor/OWP_DG_1w-adaptor.ino
 * works here also (this uses 5V so board needs to be taken out of 3.3V circuit!):
 *   use Uno with ArduinoISP Example and "Arduino as ISP"
 *   connect (like attiny85) for ISP (SPI) programming (13-13, 12-12, 11-11,
 *     10-RESET, 5V-VCC, GND-GND - 5V also for 3.3V boards possible!) see e.g.
 *       https://www.arduino.cc/en/tutorial/arduinoISP
 *       https://www.arduino.cc/en/Hacking/MiniBootloader
 *       http://www.gammon.com.au/breadboard
 *       https://github.com/nickgammon/arduino_sketches
 *       (for debugging see "Chip not detected", "Alternate clock source")
 *   select "Arduino Pro or Pro Mini" board, select the correct "Procesor"
 *     (depending on voltage on clock) and Tools > Burn Bootloader
 *   then compile and upload sketch with (Upload button doesn't work)
 *     Sketch > Upload Using Programmer
 *   (selecting othe 328 board like Uno or Nano would work also but
 *     not allow to select 8MHz clock)
 *   info on (RobotDyn) Arduino (Pro) Mini (3.3V):
 *     https://learn.sparkfun.com/tutorials/using-the-arduino-pro-mini-33v/all
 *     https://robotdyn.com/catalog/promini-atmega328p.html
 *     pinout: https://robotdyn.com/pub/media/0G-00006155==ProMINI-ATmega328P(10PCS)-USBCH340-KIT/DOCS/PINOUT==0G-00004034==ProMINI-ATmega328P.pdf
 *     schematic: https://robotdyn.com/pub/media/0G-00006155==ProMINI-ATmega328P(10PCS)-USBCH340-KIT/DOCS/Schematic==0G-00004034==ProMINI-ATmega328P.pdf
 *
 * For a plain 3.3V operation we can use a Due:
 *   use Due with ArduinoISP Example and "Arduino as ISP"
 *     uncomment '#define USE_OLD_STYLE_WIRING' (line 81) in order to use same wiring
 *       as mentioned before (using the SPI/ICSP header does not work (yet))
 *     for programming the Due with ArduinoISP use the "Programming Port" (USB
 *       connector in middle of board)
 *     for programming the target (Arduino Mini) and use as "Arduino as ISP", use
 *       the "Native Port" (USB connector at the edge of the board)
 *         https://petervanhoyweghen.wordpress.com/2015/07/21/arduinoisp-reliability-and-portability-improvements/
 *     in case of issues reset the Due by pressing the button and re-select USB port
 *     in case Burn Bootloader fails check enable verbose output (Preferences) and
 *       check it, sometimes it tries to burn twice and only 2nd time fails (ignore it)
 *   select "Arduino Pro or Pro Mini" board etc. as mentioned before
 *
 * Upload using bootloader with Due RX/TX as mentioned in
 *   @see ESP8266_SHT31_WiFi_REST/ESP8266_SHT31_WiFi_REST.ino does not work yet
 *   need to investigate the signals, may be the bootloader (MiniCore?), etc.
 *
 * Direct Pinout (not using shields, e.g. for Arduino Mini 3.3V):
 *   Arduino Mini 3.3V            Peripherial
 *          VCC                  supply: 3.3V
 *          VCC                LoRa Bee:    1 (3.3V)
 *          VCC             Weather St.:  VCC
 *          GND                  supply:  GND
 *          GND                LoRa Bee:   10 (GND)
 *          GND             Weather St.:  GND
 *            9                LoRa Bee:    5 (RESET)   - SPI
 *           10                LoRa Bee:   17 (CS/NSS)
 *           11                LoRa Bee:   11 (DI/MOSI) - SPI
 *           12                LoRa Bee:    4 (DO/MISO) - SPI
 *           13                LoRa Bee:   18 (CLK/SCK) - SPI
 *           A4             Weather St.:  SDA           - I2C
 *           A5             Weather St.:  SCL           - I2C
 *   optional:
 *          RAW                  supply: 5V (+2.5V...+16V)
 *   (optionally replaces 3.3V supply e.g. by 5V or 3.7V LiPo,
 *   Lora Bee and Weather St. still need to be connected to VCC,
 *   VCC then gives max. 180 mA!)
 *   @see Uno_Dragino_LoRa_GPS_Shield_TTN/Lora GPS Shield v1.3.sch.pdf
 *   @url https://robotdyn.com/pub/media/0G-00006155==ProMINI-ATmega328P(10PCS)-USBCH340-KIT/DOCS/Schematic==0G-00004034==ProMINI-ATmega328P.pdf
 *   @url http://ww1.microchip.com/downloads/en/DeviceDoc/20005719A.pdf
 *   @url https://robotdyn.com/promini-atmega328p.html
 *   For programming of Arduino Mini as described above it has either
 *   to be removed from circuit (socket) or the Lora Bee SPI connections
 *   (pin 10-13) have to be removed and connected to programmer.
 */
/***********************************************************************
*
* Sample-Sketch for the BME280 / MAX44009 Breakout (WeatherStation)
*
* Connections:
* Arduino -> Breakout
* VCC        VCC
* GND        GND
* SDA        SDA
* SCL        SCL
*
* LoRa GPS Shield hardware configuration (connections, jumpers, etc):
* @ref Uno_Dragino_LoRa_GPS_Shield_TTN/Uno_Dragino_LoRa_GPS_Shield_TTN.ino
* @ref ../BlinkingSwitch_SWBB-TL
*
* Make sure there is th S1 jumpers installed on the board (to GND)
*
*************************************************************************/

//#define ENABLE_DEBUG
//#define ENABLE_UNITTEST
//#define ENABLE_GPS
//#define ENABLE_DEBUG_GPS

#define SENSOR     "owp:ws:v1"

#define READ_INFO  0x01  // return generic sensor info
#define READ_VCC   0x11  // return supply voltage
#define READ_TEMP  0x12  // return chip temperature
#define READ       0x21  // return memory data
#define WRITE      0x22  // store memory data
#define WRITE_CAL  0x32  // store calibration value in memory
#define READ_VALUE_XXX 0xFF
#define _READ_VALUE_TEMP  0xFE  // temporary value
#define _READ_VALUE_HUM   0xFD  // temporary value
#define _READ_VALUE_PRES  0xFC  // temporary value
#define _READ_VALUE_ILLU  0xFB  // temporary value
// more can be added as needed ...
// do not use 0x00 as this is the string terminator

uint8_t mem_buffer[256];

#ifdef ENABLE_GPS
#include <SoftwareSerial.h>
#include <TinyGPS.h>

TinyGPS gps;
SoftwareSerial nss(A2, A1); // Arduino RX, TX to conenct to GPS module.
#endif

#include <BME280I2C.h>
#include <Wire.h>             // Needed for legacy versions of Arduino.

/* Recommended Modes -
   Based on Bosch BME280I2C environmental sensor data sheet.

Weather Monitoring :
   forced mode, 1 sample/minute
   pressure ×1, temperature ×1, humidity ×1, filter off
   Current Consumption =  0.16 μA
   RMS Noise = 3.3 Pa/30 cm, 0.07 %RH
   Data Output Rate 1/60 Hz

Humidity Sensing :
   forced mode, 1 sample/second
   pressure ×0, temperature ×1, humidity ×1, filter off
   Current Consumption = 2.9 μA
   RMS Noise = 0.07 %RH
   Data Output Rate =  1 Hz

Indoor Navigation :
   normal mode, standby time = 0.5ms
   pressure ×16, temperature ×2, humidity ×1, filter = x16
   Current Consumption = 633 μA
   RMS Noise = 0.2 Pa/1.7 cm
   Data Output Rate = 25Hz
   Filter Bandwidth = 0.53 Hz
   Response Time (75%) = 0.9 s


Gaming :
   normal mode, standby time = 0.5ms
   pressure ×4, temperature ×1, humidity ×0, filter = x16
   Current Consumption = 581 μA
   RMS Noise = 0.3 Pa/2.5 cm
   Data Output Rate = 83 Hz
   Filter Bandwidth = 1.75 Hz
   Response Time (75%) = 0.3 s

*/
BME280I2C::Settings settings(
  BME280::OSR_X1,
  BME280::OSR_X1,
  BME280::OSR_X1,
  BME280::Mode_Forced,
  BME280::StandbyTime_1000ms,
  BME280::Filter_Off,
  BME280::SpiEnable_False,
  0x76 // I2C address. I2C specific.
);

BME280I2C bme(settings);   // I2C address: 0x76

//#include <Wire.h>
#include "i2c.h"
#include "i2c_MAX44009.h"
MAX44009 max44009;         // I2C address: 0x4A (hard-coded in i2c_MAX44009.h)

#define PJON_INCLUDE_TL
#include <PJON.h>

// <Strategy name> bus(selected device id)
PJON<ThroughLora> bus(43);

/**
 *  Arduino IDE: put your setup code here, to run once.
 */
void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); // Initialize LED 13 to be off
//#ifdef ENABLE_DEBUG
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
//#endif

//  bus.strategy.setPins(ss, reset, dio0);
  bus.strategy.setFrequency(869200000UL);  // 869.2 MHz
  // Optional
  bus.strategy.setSignalBandwidth(250E3);  // default is 125E3
  bus.strategy.setTxPower(10);             // default is 17
  //bus.strategy.setSpreadingFactor(7);      // default is 7
  //bus.strategy.setCodingRate4(5);          // default is 5
  bus.begin();
  bus.set_receiver(receiver_function);

#ifdef ENABLE_DEBUG
  Serial.begin(9600);
  Serial.println(F(__FILE__));
#endif

#ifdef ENABLE_GPS
  nss.begin(9600);
#endif

  Wire.begin();
  if ((!bme.begin()) || (bme.chipModel() != BME280::ChipModel_BME280)) {
#ifdef ENABLE_DEBUG
    Serial.println(F("BME280I2C sensor missing!"));
#endif
  }
  // Change some settings before using.
  settings.tempOSR = BME280::OSR_X4;
  bme.setSettings(settings);

  if (!max44009.initialize()) {
#ifdef ENABLE_DEBUG
    Serial.println(F("MAX44009 sensor missing"));
    //while (1) {};
#endif
  }
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
  case _READ_VALUE_TEMP: {
    float temp(NAN), hum(NAN), pres(NAN);
    bme.read(pres, temp, hum, BME280::TempUnit(BME280::TempUnit_Celsius), BME280::PresUnit(BME280::PresUnit_Pa));
    bus.reply((char*)(&temp), sizeof(float));
  }
  break;
  case _READ_VALUE_HUM: {
    float temp(NAN), hum(NAN), pres(NAN);
    bme.read(pres, temp, hum, BME280::TempUnit(BME280::TempUnit_Celsius), BME280::PresUnit(BME280::PresUnit_Pa));
    bus.reply((char*)(&hum), sizeof(float));
  }
  break;
  case _READ_VALUE_PRES: {
    float temp(NAN), hum(NAN), pres(NAN);
    bme.read(pres, temp, hum, BME280::TempUnit(BME280::TempUnit_Celsius), BME280::PresUnit(BME280::PresUnit_Pa));
    bus.reply((char*)(&pres), sizeof(float));
  }
  break;
  case _READ_VALUE_ILLU: {
    static unsigned long mLux_value;
    max44009.getMeasurement(mLux_value);
    float val = mLux_value;
    bus.reply((char*)(&val), sizeof(float));
  }
  break;
  case READ_VALUE_XXX: {
// FOR TESTING ONLY
    /*float flat, flon, falt;
    unsigned long fix_age;
    gps.f_get_position(&flat, &flon, &fix_age);
    falt=gps.f_altitude();  // +/- altitude in meters
    flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6;//save six decimal places
    flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6;
    falt == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : falt, 2;//save two decimal places
    float val = fix_age;
    float val = flon;
    float val = flat;
    float val = falt;*/
    float val = NAN;
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
#ifdef ENABLE_DEBUG
  for(int i=0; i < 2000; ++i) {  // loop for about 2s
#endif
    bus.update();
    bus.receive(1000);
    //bus.receive(10000);

#ifdef ENABLE_GPS
    while (nss.available()) {
      if (gps.encode(nss.read())) {
        // new gps data ready for processing, e.g.:
        //gps.f_get_position(&flat, &flon, &fix_age);
      }
    }
#endif
#ifdef ENABLE_DEBUG
  }

  printSensorData();
#endif
};

#ifdef ENABLE_DEBUG
void printSensorData()
{
#ifdef ENABLE_GPS
  /*Convert GPS data to format*/
  // applications which are not resource constrained, it may be more convenient
  // to use floating-point numbers by invoking any of the f_* methods. These
  // require the floating point libraries, which might add another 600+ bytes.

  float flat, flon, falt;
  unsigned long fix_age;
  // returns +/- latitude/longitude in degrees
  gps.f_get_position(&flat, &flon, &fix_age);
  //// time in hhmmsscc, date in ddmmyy
  //gps.get_datetime(&date, &time, &fix_age);
  falt=gps.f_altitude();  // +/- altitude in meters
  flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6;//save six decimal places
  flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6;
  falt == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : falt, 2;//save two decimal places
  //float fc = gps.f_course(); // course in degrees
  //float fmps = gps.f_speed_mps(); // speed in m/sec
  //float fkmph = gps.f_speed_kmph(); // speed in km/hr

  if(flon!=1000.000000) {
    Serial.print(F("Milliseconds since last fix: "));
    Serial.println(fix_age);
    if ((fix_age == 0) || (fix_age == TinyGPS::GPS_INVALID_AGE))
      Serial.println(F("Error: No fix detected!"));
    else if (fix_age > 5000)
      Serial.println(F("Warning: possible stale data!"));
    else
      Serial.println(F("Data is current."));
    Serial.print(F("Position (long, lat, altitude): "));
    Serial.print(flon);
    Serial.print(F(","));
    Serial.print(flat);
    Serial.print(F(","));
    Serial.println(falt);

#ifdef ENABLE_DEBUG_GPS
    unsigned long chars;
    unsigned short sentences, failed_checksum;
    // statistics
    gps.stats(&chars, &sentences, &failed_checksum);

    Serial.print(F("Bytes processed: "));
    Serial.println(chars);
    Serial.print(F("Sentences (valid, failed): "));
    Serial.print(sentences);
    Serial.print(F(","));
    Serial.println(failed_checksum);
#endif
  }
#endif

  float temp(NAN), hum(NAN), pres(NAN);
  bme.read(pres, temp, hum, BME280::TempUnit(BME280::TempUnit_Celsius), BME280::PresUnit(BME280::PresUnit_Pa));

  Serial.print(F("Temp: "));
  Serial.print(temp);
  Serial.print(F("\tHumidity: "));
  Serial.print(hum);
  Serial.print(F("% RH"));
  Serial.print(F("\tPressure: "));
  Serial.print(pres);
  Serial.print(F(" Pa"));

  static unsigned long mLux_value;
  max44009.getMeasurement(mLux_value);

  Serial.print(F("\tmLUX: "));
  Serial.println(mLux_value);
  Serial.println();
}
#endif

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
