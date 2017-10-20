/*
  This a simple example of the aREST Library working with
  the Arduino Yun. See the README file for more details.

  (Written in 2014 by Marco Schwartz under a GPL license.)

// SHT31
// This code is designed to work with the SHT31_I2CS I2C Module available from Adafruit.
// Work-a-round for ESP8266_SHT31_WiFi_REST as it has issues with I2C comm.
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
 *       5V   VIN Adafruit_SHT31
 *      GND   GND Adafruit_SHT31
 *      SCL   SCL Adafruit_SHT31
 *      SDA   SDA Adafruit_SHT31
 *      (USB power by Power/BatteryPack)
 ****************************************************/

// In DEBUG mode additional serial and REST ouput are enabled
//#define DEBUG

// Import required libraries
#include <Bridge.h>
#include <YunServer.h>
#include <YunClient.h>
#include <aREST.h>

#include <Wire.h>
#include "Adafruit_SHT31.h"

// Create aREST instance
aREST rest = aREST();

// on how to use the Yun as client in your existing
// local wifi network: see "Yun WebInterface" above
// (compare also to ESP8266_SHT31_WiFi_REST)
/*// WiFi parameters
//const char* ssid = "your_wifi_network_name";
//const char* password = "your_wifi_network_password";*/

// Yun Server
YunServer server(80);

Adafruit_SHT31 sht31 = Adafruit_SHT31();

// Variables to be exposed to the API
float temperature;
float humidity;

String sDESC    = String("SHT31 Sensor I2C Mini Module");
String sDATE    = String(__DATE__);
String sTIME    = String(__TIME__);
String sIDE     = String(ARDUINO, DEC);
String sVERSION = String(__VERSION__);
String sFILE    = String(__FILE__);
#ifdef DEBUG
// DOES NOT WORK: causes strange issues and then firefox tries to download a bin-file!
//String sDEBUG   = String();
#endif

//// Declare functions to be exposed to the API
//int ledControl(String command);

void setup(void)
{
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

  // Init variables and expose them to REST API
  //temperature = 24;
  //humidity = 40;
  temperature = sht31.readTemperature();
  humidity = sht31.readHumidity();
  rest.variable("temperature",&temperature);
  rest.variable("humidity",&humidity);
#ifdef DEBUG
  Serial.print("Temp *C = "); Serial.println(temperature);
  Serial.print("Hum. % = "); Serial.println(humidity);
#endif

  rest.variable("DESC",&sDESC);
  rest.variable("DATE",&sDATE);
  rest.variable("TIME",&sTIME);
  rest.variable("IDE",&sIDE);
  rest.variable("VERSION",&sVERSION);
  rest.variable("FILE",&sFILE);
#ifdef DEBUG
  //rest.variable("DEBUG",&sDEBUG);
#endif

   // DOES NOT WORK PROPERLY: .../led/o, .../led/1, .../led/0, .../led?1, etc.
  /*// Function to be exposed
  rest.function("led",ledControl);*/

  // Give name & ID to the device (ID should be 6 characters long)
  rest.set_id("008");
  rest.set_name("mighty_cat");

  // Bridge startup
  Bridge.begin();

  // Listen for incoming connection only from localhost
  server.begin();

/*  // Print the IP address
  Serial.println(WiFi.localIP());*/

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

#ifdef DEBUG
  Serial.println("config done");
  Serial.flush();
  //sDEBUG = String("config done");
#endif
}

void loop() {

  // Handle REST calls
  YunClient client = server.accept();
  rest.handle(client);

  // Update Sensor Measurements
  temperature = sht31.readTemperature();
  humidity = sht31.readHumidity();
#ifdef DEBUG
  Serial.print("Temp *C = "); Serial.println(temperature);
  Serial.print("Hum. % = "); Serial.println(humidity);
#endif

}

/*// Custom function accessible by the API
int ledControl(String command) {

  // Get state from command
  int state = command.toInt();
#ifdef DEBUG
  Serial.println(command);
  //sDEBUG = String(command);
#endif

  digitalWrite(LED_BUILTIN,state);
  return 1;
}*/
