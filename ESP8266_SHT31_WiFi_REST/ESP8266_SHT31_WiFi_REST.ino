/*
  This a simple example of the aREST Library for the ESP8266 WiFi chip.
  See the README file for more details.

  (Written in 2015 by Marco Schwartz under a GPL license.)

// SHT31
// This code is designed to work with the SHT31_I2CS I2C Mini Module available from ControlEverything.com.
*/
/*************************************************** 
 *    TODO:
 *      - sht31: not working...
 *               
 *      - small python script to retrieve and plot values
 *        https://stackoverflow.com/questions/7750557/how-do-i-get-json-data-from-restful-service-using-python
 *      - test LED output setting ".../led/o"
 *      - LED switch OFF ".../led/1" does not work! (ON works)
 *        command string does not contain the value, try e.g. ".../led?1"
 *        same with ".../digital/1/w/0",  ".../digital/1/w/1" works, test  ".../digital/1/r"
 *      - still need "set fuse settings (clock, etc.)" ? (Tools > Burn Bootloader)
 *    
 *    Programm ESP8266 using Arduino Due Board as 3.3V USB2serial converter:
 *    - Links
 *      - Hardware: https://www.itead.cc/wiki/ESP8266_Serial_WIFI_Module#Hardware
 *      - Arduino IDE mit dem ESP8266: https://arduino-hannover.de/2015/04/08/arduino-ide-mit-dem-esp8266/
 *      - ESP8266 with SHT31(-D) WiFi WebServer: https://github.com/ControlEverythingCommunity/SHT31/blob/master/ESP8266/SHT31.ino
 *                                               https://www.controleverything.com/content/Humidity?sku=SHT31_I2CS#tabs-0-product_tabset-2
 *      - ESP8266 I2C example: http://pdacontrolen.com/esp8266-display-oled-i2c-client-irc/ (SCL-GPIO0, SDA-GPIO2)
 *      - https://learn.adafruit.com/esp8266-temperature-slash-humidity-webserver/uploading-to-the-esp8266
 *    - use Arduino IDE >=1.8.3
 *    - install ESP8266 option to IDE
 *      - File > Preferences > Additional Boards Manager URLs: http://arduino.esp8266.com/stable/package_esp8266com_index.json
 *      - Tools > Board: ??? > Boards Manager...
 *        search for: "esp8266" and install (esp8266 2.3.0)
 *    - connect ESP8266 to an Arduino Due by wiring:
 *        Arduino Due Pin   ESP8266 Pin
 *                    GND   1 GND
 *                 TX0->1   2 UTXD          --------------------
 *                          3 GPIO2         |2 1        x--=== |
 *               3.3V VCC   4 CH_PD         |4 3           === |
 *                    GND   5 GPIO0         |6 5  ESP8266  === |
 *                          6 RST           |8 7  board      | |
 *                 RX0<-0   7 URXD          --------------------
 *               3.3V VCC   8 VCC
 *        Arduino Due RESET pin needs to be connected to GND (in order to disable the chip/sketch)
 *        (ESP8266 RST kann offen bleiben oder PullUp)
 *    - select ESP8266:
 *      - Board: "Generic ESP8266 Module"
 *      - Flash Mode: "DIO"
 *      - Flash Frequency: "40MHz"
 *      - CPU Frequency: "80MHz"
 *      - Flash Size: "512K (64K SPIFFS)"
 *      - Debug port: "Disabled"
 *      - Debug Level: "None"
 *      - Reset Method: "ck"
 *      - Upload Speed: "115200"
 *      - Port: ???
 *    - select Programmer:
 *      - Tools > Programmer: "AVRISP mkII"
 *        (this is the default for Arduinos)
 *    - upload this code
 *        (if it does not work dis- and re-connect the Due/ESP8266 or try the RST pin)
 *    
 *                      ESP8266 Pin Layout
 *                    GND   1 GND
 *                          2 UTXD          --------------------
 *     Adafruit_SHT31 SDA   3 GPIO2         |2 1        .--=== |
 *               3.3V VCC   4 CH_PD         |4 3           === |
 *     Adafruit_SHT31 SCL   5 GPIO0         |6 5  ESP8266  === |
 *                          6 RST           |8 7  board      | |
 *                          7 URXD          --------------------
 *               3.3V VCC   8 VCC
 * ( NOTE! - It's very important to use pullups on the SDA & SCL lines! )
 * (         I2C pullups should be 1-100k, so 4.7k fits well.           )
 * 
 *    - use serial monitor
 *      - Tools > Serial Monitor
 *      - Both NL & CR
 *      - 115200 baud
 ****************************************************/

// Import required libraries
#include <ESP8266WiFi.h>
#include <aREST.h>

#include <Wire.h>
#include "Adafruit_SHT31.h"
//// SHT31 I2C address is 0x44(68)
//#define Addr 0x44

// Create aREST instance
aREST rest = aREST();

// WiFi parameters
//const char* ssid = "your_wifi_network_name";
//const char* password = "your_wifi_network_password";
const char* ssid = "Buffalo-G-E69A";
const char* password = "14661655";

// The port to listen for incoming TCP connections
#define LISTEN_PORT           80

// Create an instance of the server
WiFiServer server(LISTEN_PORT);

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
//String sDEBUG   = String();

// Declare functions to be exposed to the API
int ledControl(String command);

void setup(void)
{
  // Start Serial
  Serial.begin(115200);

  Serial.println("SHT31 test");
  if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
//    while (1) delay(1);
  }
  Serial.println(sht31.readStatus());
  //void reset(void);
  //void heater(boolean);

  // Init variables and expose them to REST API
  temperature = 24;
  humidity = 40;
//  rest.variable("temperature",&temperature);
//  rest.variable("humidity",&humidity);
//  temperature = sht31.readTemperature();
//  humidity = sht31.readHumidity();
  rest.variable("temperature",&temperature);
  rest.variable("humidity",&humidity);

  rest.variable("DESC",&sDESC);
  rest.variable("DATE",&sDATE);
  rest.variable("TIME",&sTIME);
  rest.variable("IDE",&sIDE);
  rest.variable("VERSION",&sVERSION);
  rest.variable("FILE",&sFILE);
//  rest.variable("DEBUG",&sDEBUG);
 
  // Function to be exposed
  rest.function("led",ledControl);

  // Give name & ID to the device (ID should be 6 characters long)
  rest.set_id("1");
  rest.set_name("esp8266");

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());

  Serial.flush();

  // Flash LED 2x to signal "boot ok"
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
                                    // but actually the LED is on; this is because 
                                    // it is acive low on the ESP-01)
  delay(500);                       // Wait
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
//  pinMode(LED_BUILTIN, INPUT);      // reset to default (does not work - freezes)

//  sDEBUG = String("boot ok");
}

void loop() {

  // Handle REST calls
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  while(!client.available()){
    delay(1);
  }
  rest.handle(client);

  // Update Sensor Measurements
  temperature = sht31.readTemperature();
  humidity = sht31.readHumidity();
//  rest.variable("temperature",&temperature);
//  rest.variable("humidity",&humidity);
//  handleroot();

}

// Custom function accessible by the API
int ledControl(String command) {

  // Update Sensor Measurements
  temperature = sht31.readTemperature();
  humidity = sht31.readHumidity();
//  rest.variable("temperature",&temperature);
//  rest.variable("humidity",&humidity);
//  handleroot();

  // Get state from command
  int state = command.toInt();
//  sDEBUG = String(command);

  digitalWrite(LED_BUILTIN,state);
  return 1;
}

// from SH31.ino
/*void handleroot()
{
  unsigned int data[6];

  // Start I2C Transmission
  Wire.beginTransmission(Addr);
  // Send 16-bit command byte
  Wire.write(0x2C);
  Wire.write(0x06);
  // Stop I2C transmission
  Wire.endTransmission();
  delay(300);

  // Start I2C Transmission
  Wire.beginTransmission(Addr);
  // Stop I2C Transmission
  Wire.endTransmission();

  // Request 6 bytes of data
  Wire.requestFrom(Addr, 6);

  // Read 6 bytes of data
  // temp msb, temp lsb, temp crc, hum msb, hum lsb, hum crc
  if (Wire.available() == 6)
  {
    data[0] = Wire.read();
    data[1] = Wire.read();
    data[2] = Wire.read();
    data[3] = Wire.read();
    data[4] = Wire.read();
    data[5] = Wire.read();
  }
  // Convert the data
  int temp = (data[0] * 256) + data[1];
  //cTemp = -45.0 + (175.0 * temp / 65535.0);
  temperature = -45.0 + (175.0 * temp / 65535.0);
  //fTemp = (cTemp * 1.8) + 32.0;
  humidity = (100.0 * ((data[3] * 256.0) + data[4])) / 65535.0;

  // Output data to serial monitor
  Serial.print("Temperature in Celsius :");
  //Serial.print(cTemp);
  Serial.print(temperature);
  Serial.println(" C");
  //Serial.print("Temperature in Fahrenheit :");
  //Serial.print(fTemp);
  //Serial.println(" F");
  Serial.print("Relative Humidity :");
  Serial.print(humidity);
  Serial.println(" %RH");
  delay(500);
}*/
