/*
  This a simple example of the aREST Library working with
  the Arduino Yun. See the README file for more details.

  (Written in 2014 by Marco Schwartz under a GPL license.)

// SHT31
// This code is designed to work with the SHT31_I2CS I2C Module available from Adafruit.
// Work-a-round for ESP8266_SHT31_WiFi_REST as it has issues with I2C comm.

/*********************************************************************
This is an example for our Monochrome OLEDs based on SSD1306 drivers

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/category/63_98

This example is for a 128x64 size display using I2C to communicate
3 pins are required to interface (2 I2C and one reset)

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.  
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************
*/
/*************************************************** 
 *    Usage:
 *      - switch it on (my be check with browser)
 *      - run python script:
 *        ~/sketchbook/Yun_SHT31_WiFi_REST$ ./Yun_SHT31_WiFi_REST.py
 *
 *    Links:
 *      - https://www.open-electronics.org/using-rest-with-arduino-yun/
 *      - https://stackoverflow.com/questions/7750557/how-do-i-get-json-data-from-restful-service-using-python
 *
 *    Yun upload in VirtualBox needs 2 USB filter in order to swap USB drivers fast enough for upload of sketches:
 *      - Name: Arduino LLC Arduino Yun [0100]
 *        Hersteller-ID: 2341
 *        Produkt-ID: 8041
 *        (lsusb in host system)
 *      - Name: Arduino LLC Arduino Yun [0001]
 *        Hersteller-ID: 2341
 *        Produkt-ID: 0041
 *        ("Upload" Button drücken, und während Programmierversuch (schnell!)
 *         ins host system wechseln für lsusb)
 *
 *    Yun upload via IP, password: "arduino"
 *
 *    Yun WebInterface:
 *      - connect to Yun SSID, if not possible reset Yun WLAN by pressing the WLAN reset button for >= 30s
 *        - http://192.168.240.1/
 *        - password: arduino
 *      - configure interface: https://www.arduino.cc/en/Guide/ArduinoYun
 *        - Press the Configuration button to proceed.
 *        - By default, the REST API access is password protected. It is possible to change this to access the services without a password. To change this setting, enter the Yún configuration panel. At the bottom of the page, you will see toggles for changing the access. 
 *          - set REST to open
 *        - Enter the name of the wiFi network you wish to connect to. 
 *          - connect to your local ssid using your password
 *
 *    REST interface:
 *      - connect to Yun IP (e.g. http://192.168.11.12/) to see REST output
 *      - test e.g. LED:
 *          http://[Yun-IP]/arduino/mode/13/output
 *          http://[Yun-IP]/arduino/digital/13/r
 *          http://[Yun-IP]/arduino/digital/13/1
 *          http://[Yun-IP]/arduino/digital/13/r
 *          http://[Yun-IP]/arduino/digital/13/0
 *          http://[Yun-IP]/arduino/digital/13/r
 *          (see Yun_Log_BatteryDisCharging for more REST tricks)
 *
 *    Arduino Yun Pin Layout
 *       5V   VIN Adafruit_SHT31, SSD1306 OLED
 *      GND   GND Adafruit_SHT31, SSD1306 OLED
 *      SCL   SCL Adafruit_SHT31, SSD1306 OLED
 *      SDA   SDA Adafruit_SHT31, SSD1306 OLED
 *      (USB power by Power/BatteryPack)
 ****************************************************/

// In DEBUG mode additional serial and REST ouput are enabled
//#define DEBUG

#define HORIZONTAL
//#define VERTICAL

#ifdef VERTICAL
#define SCALING   1.
#define OFFSET    0.
//#define MAX_TIME  128
#define MAX_TIME  60
#else  // HORIZONTAL
#define SCALING   -0.5
#define OFFSET    50.
//#define MAX_TIME  64
#define MAX_TIME  100
#endif

//#include <SPI.h>
#include <Wire.h>
#include "Adafruit_SHT31.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SHT31 sht31 = Adafruit_SHT31();

Adafruit_SSD1306 display(4);  // OLED_RESET = 4

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// Variables to be exposed to the API
float temperature;
float humidity;
float Vs, Ti;
int t = 0;
float last_humidity;

const String sDESC    = String("SHT31 I2C WiFi Sensor");
const String sDATE    = String(__DATE__);
const String sTIME    = String(__TIME__);
const String sIDE     = String(ARDUINO, DEC);
const String sVERSION = String(__VERSION__);
const String sFILE    = String(__FILE__);

void setup()   {                
#ifdef DEBUG
  // Start Serial
  Serial.begin(115200);

  while (!Serial)
    delay(10);     // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("SHT31 test");
  if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    //while (1) delay(1);
  }
  Serial.println(sht31.readStatus());
  //void reset(void);
  //void heater(boolean);
#else
  sht31.begin(0x44);   // Set to 0x45 for alternate i2c addr
#endif

  temperature = sht31.readTemperature();
  humidity = sht31.readHumidity();
  Vs = getVcc();
  Ti = getTemp();
#ifdef DEBUG
  Serial.print("Temp *C = "); Serial.println(temperature);
  Serial.print("Hum. % = "); Serial.println(humidity);
  Serial.print("Vs V = "); Serial.println(Vs);
  Serial.print("Ti *C = "); Serial.println(Ti);
#endif

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
//  display.begin(SSD1306_SWITCHCAPVCC, 0x3D);  // initialize with the I2C addr 0x3D (for the 128x64)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  // Clear the buffer.
  display.clearDisplay();

  // text display tests
  display.setTextSize(1);
  display.setTextColor(WHITE);
  //display.setTextColor(BLACK, WHITE); // 'inverted' text
  display.setCursor(0,0);
  display.println(sDESC);
  display.println(F("Version: 1"));
  display.println(sDATE);
  display.println(sTIME);
  display.print(sIDE);
  display.print("/");
  display.println(sVERSION);
  //display.println(sFILE);
//  display.display();

  display.println(F("config done"));
  display.display();
#ifdef DEBUG
  Serial.println(F("config done"));
  Serial.flush();
  //sDEBUG = String("config done");
#endif

  delay(2000.);
  display.clearDisplay();

  for(int x=0; x<=MAX_TIME; x+=10) {
    for(int y=0; y<=100; y+=10) {
#ifdef VERTICAL
      display.drawPixel((y*SCALING+OFFSET), x, WHITE);
#else
      display.drawPixel(x, (y*SCALING+OFFSET), WHITE);
#endif
    }
  }
  display.display();

  // Flash LED 2x to signal "boot ok"
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED on
  delay(500);                       // Wait
  digitalWrite(LED_BUILTIN, LOW);   // Turn the LED off
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(LED_BUILTIN, INPUT);      // reset to default (does not work - freezes)
}

void loop() {
  
  // Blink triggers the state-change
  if (blinking())
  {
    // Update Sensor Measurements
    last_humidity = humidity;
    temperature = sht31.readTemperature();
    humidity = sht31.readHumidity();
    Vs = getVcc();
    Ti = getTemp();
#ifdef VERTICAL
//    display.drawPixel(int(humidity*SCALING + OFFSET), t, WHITE);
    display.drawLine(int(last_humidity*SCALING + OFFSET), t-1, int(humidity*SCALING + OFFSET), t, WHITE);
#else
//    display.drawPixel(t, int(humidity*SCALING + OFFSET), WHITE);
    display.drawLine(t-1, int(last_humidity*SCALING + OFFSET), t, int(humidity*SCALING + OFFSET), WHITE);

    display.fillRect(0, 55, 128, 10, BLACK);
    display.setCursor(0,55);
    display.print(humidity);
    display.setCursor(50,55);
    display.print(temperature);
    display.setCursor(100,55);
    display.print(Vs);
#endif
    display.display();
    t = (t + 1) % MAX_TIME;
#ifdef DEBUG
    Serial.print("Temp *C = "); Serial.println(temperature);
    Serial.print("Hum. % = "); Serial.println(humidity);
    Serial.print("Vs V = "); Serial.println(Vs);
    Serial.print("Ti *C = "); Serial.println(Ti);
#endif

  /*for (int16_t i=0; i<display.height()/2; i+=2) {
    display.drawRect(i, i, display.width()-2*i, display.height()-2*i, WHITE);
    display.display();
    delay(1);
  }*/
  }

}

bool blinking(void)
{
    const  uint32_t interval    = 10000;          // interval at which to blink (milliseconds)
    static uint32_t nextMillis  = millis();     // will store next time LED will updated

    if (millis() > nextMillis)
    {
        nextMillis += interval;             // save the next time you blinked the LED
        static uint8_t ledState = LOW;      // ledState used to set the LED
        if (ledState == LOW)    ledState = HIGH;
        else                    ledState = LOW;
        digitalWrite(LED_BUILTIN, ledState);
        return 1;
    }
    return 0;
}

/********************************************************************************
    Accessing the secret voltmeter on the Arduino 168 or 328
    http://code.google.com/p/tinkerit/wiki/SecretVoltmeter
    settings for 32u4 (Leonardo, Yun)
    https://provideyourown.com/2012/secret-arduino-voltmeter-measure-battery-voltage/
********************************************************************************/
double getVcc() {
  long result;
  // Read 1.1V reference against AVcc
//  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);  // 328
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);  // 32u4
  ADCSRB =  (0 << MUX5);  // MUX 5 bit part of ADCSRB (needs to be reset due to GetTemp()!!! 32u4)
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return float(result)/1000.;
}

/********************************************************************************
    Internal Temperature Sensor for ATmega328 types
    https://playground.arduino.cc/Main/InternalTemperatureSensor
    settings for 32u4 (Leonardo, Yun)
    http://www.avrfreaks.net/forum/teensy-atmega32u4-chip-temperature-sensor
********************************************************************************/
double getTemp(void)
{
  unsigned int wADC;
  double t;

  // The internal temperature has to be used
  // with the internal reference of 1.1V.
  // Channel 8 can not be selected with
  // the analogRead function yet.

  // Set the internal reference and mux.
//  ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));  // 328
  ADMUX = (1 << MUX0) | (1 << MUX1) | (1 << MUX2);  // 32u4
  ADCSRB =  (1 << MUX5);  // MUX 5 bit part of ADCSRB (32u4)
  //ADCSRB |=  (1 << ADHSM);  // High speed mode (32u4)
  // Set Voltage Reference to internal 2.56V reference with external capacitor on AREF pin (32u4)
  ADMUX |= (1 << REFS1) | (1 << REFS0);  // 32u4
  ADCSRA |= _BV(ADEN);  // enable the ADC

  delay(20);            // wait for voltages to become stable.

  ADCSRA |= _BV(ADSC);  // Start the ADC

  // Detect end-of-conversion
  while (bit_is_set(ADCSRA,ADSC));

  // Reading register "ADCW" takes care of how to read ADCL and ADCH.
  wADC = ADCW;

  // The offset of 324.31 could be wrong. It is just an indication.
//  t = (wADC - 324.31 ) / 1.22;
//  t = (wADC - 330.81 ) / 1.22;    // Arduino Uno R3 (mega328)
  t = wADC / 10.;  // 32u4

  // The returned temperature is in degrees Celsius.
  return (t);
}
