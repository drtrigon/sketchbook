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
 *
 *    Setup USB stick/drive:
 *
 *    ursin@ThinkPad-T440-ursin:~$ lsblk
 *    NAME   MAJ:MIN RM   SIZE RO TYPE MOUNTPOINT
 *    [...]
 *    sdb      8:16   1  1007M  0 disk
 *    └─sdb1   8:17   1  1006M  0 part
 *    sr0     11:0    1  1024M  0 rom
 *    ursin@ThinkPad-T440-ursin:~$ sudo fdisk /dev/sdb
 *
 *    Befehl (m für Hilfe): d
 *    Partition 1 ausgewählt
 *
 *    Befehl (m für Hilfe): p
 *
 *    Platte /dev/sdb: 1055 MByte, 1055916032 Byte
 *    33 Köpfe, 30 Sektoren/Spur, 2083 Zylinder, zusammen 2062336 Sektoren
 *    Einheiten = Sektoren von 1 × 512 = 512 Bytes
 *    Sector size (logical/physical): 512 bytes / 512 bytes
 *    I/O size (minimum/optimal): 512 bytes / 512 bytes
 *    Festplattenidentifikation: 0x89dd17da
 *
 *       Gerät  boot.     Anfang        Ende     Blöcke   Id  System
 *
 *    Befehl (m für Hilfe): n
 *    Partition type:
 *       p   primary (0 primary, 0 extended, 4 free)
 *       e   extended
 *    Select (default p):
 *    Using default response p
 *    Partitionsnummer (1-4, Vorgabe: 1):
 *    Benutze den Standardwert 1
 *    Erster Sektor (2048-2062335, Vorgabe: 2048):
 *    Benutze den Standardwert 2048
 *    Last Sektor, +Sektoren or +size{K,M,G} (2048-2062335, Vorgabe: 2062335):
 *    Benutze den Standardwert 2062335
 *
 *    Befehl (m für Hilfe): v
 *    Remaining 2047 unallocated 512-byte sectors
 *
 *    Befehl (m für Hilfe): w
 *    Die Partitionstabelle wurde verändert!
 *
 *    Rufe ioctl() um Partitionstabelle neu einzulesen.
 *    Synchronisiere Platten.
 *    ursin@ThinkPad-T440-ursin:~$ sudo mkfs.vfat /dev/sdb1
 *    mkfs.fat 3.0.26 (2014-03-07)
 *    ursin@ThinkPad-T440-ursin:~$ lsblk
 *    NAME   MAJ:MIN RM   SIZE RO TYPE MOUNTPOINT
 *    [...]
 *    sdb      8:16   1  1007M  0 disk
 *    └─sdb1   8:17   1  1006M  0 part /media/ursin/8DDC-2397
 *    sr0     11:0    1  1024M  0 rom
 *    ursin@ThinkPad-T440-ursin:~$ sudo fdisk /dev/sdb
 *    Befehl (m für Hilfe): p
 *
 *    Platte /dev/sdb: 1055 MByte, 1055916032 Byte
 *    6 Köpfe, 16 Sektoren/Spur, 21482 Zylinder, zusammen 2062336 Sektoren
 *    Einheiten = Sektoren von 1 × 512 = 512 Bytes
 *    Sector size (logical/physical): 512 bytes / 512 bytes
 *    I/O size (minimum/optimal): 512 bytes / 512 bytes
 *    Festplattenidentifikation: 0x89dd17da
 *
 *       Gerät  boot.     Anfang        Ende     Blöcke   Id  System
 *    /dev/sdb1            2048     2062335     1030144   83  Linux
 *
 *    Befehl (m für Hilfe): q
 *
 *    ursin@ThinkPad-T440-ursin:~$ touch /media/ursin/8DDC-2397/datalog.txt
 *
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
//#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

#include <Process.h>

#include <FileIO.h>

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
//BridgeServer server(80);  // NOTE: The use of YunServer is deprecated. Use BridgeServer instead.

Adafruit_SHT31 sht31 = Adafruit_SHT31();

//Adafruit_SSD1306 display(4);  // OLED_RESET = 4
SSD1306AsciiWire oled;

//#if (SSD1306_LCDHEIGHT != 64)
//#error("Height incorrect, please fix Adafruit_SSD1306.h!");
//#endif

// Variables to be exposed to the API
float temperature;
float humidity;
float Vs, Ti;

const String sDESC    = String("SHT31 I2C WiFi Sensor");
const String sDATE    = String(__DATE__);
const String sTIME    = String(__TIME__);
const String sIDE     = String(ARDUINO, DEC);
const String sVERSION = String(__VERSION__);
const String sFILE    = String(__FILE__);
#ifdef DEBUG
// DOES NOT WORK: causes strange issues and then firefox tries to download a bin-file!
//String sDEBUG   = String();
#endif

//// Declare functions to be exposed to the API
//int ledControl(String command);

void setup(void)
{
  static uint32_t startupMillis  = millis();  // time for startup

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

//  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
//  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
//  // Show image buffer on the display hardware.
//  // Since the buffer is intialized with an Adafruit splashscreen
//  // internally, this will display the splashscreen.
//  display.display();
//  delay(2000);
//  // Clear the buffer.
//  display.clearDisplay();
//
//  // text display tests
//  display.setTextSize(1);
//  display.setTextColor(WHITE);
//  //display.setTextColor(BLACK, WHITE); // 'inverted' text
//  display.setCursor(0,0);
  //Wire.begin();  // sht31.begin() contains Wire.begin()
  oled.begin(&Adafruit128x64, 0x3C);
  oled.setFont(System5x7);
  oled.clear();
  //oled.set2X();

  // Init variables and expose them to REST API
  //temperature = 24;
  //humidity = 40;
  temperature = sht31.readTemperature();
  humidity = sht31.readHumidity();
  Vs = getVcc();
  Ti = getTemp();
  rest.variable("temperature",&temperature);
  rest.variable("humidity",&humidity);
  rest.variable("Vs",&Vs);
// DOES NOT WORK: causes strange issues and then firefox tries to download a bin-file!
//                (like sDEBUG above - may be a data limit???)
//  rest.variable("Ti",&Ti);
#ifdef DEBUG
  Serial.print("Temp *C = "); Serial.println(temperature);
  Serial.print("Hum. % = "); Serial.println(humidity);
  Serial.print("Vs V = "); Serial.println(Vs);
  Serial.print("Ti *C = "); Serial.println(Ti);
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
//  display.println(sDESC);
//  ...
  oled.println(sDESC);
  oled.println(F("Version: 1"));
  oled.println(sDATE);
  oled.println(sTIME);
  oled.print(sIDE);
  oled.print("/");
  oled.println(sVERSION);
  oled.println(sFILE);

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

  startupMillis = (millis() - startupMillis);
  //Vs = float(startupMillis);  // trick for measuring/debug
  // startupMillis: power-up ~56269, rebooot/reset ~4588
  if (startupMillis > 10000)
  {
    // power-up (not rebooot/reset)

    // show temperature and humidity while waiting...
    // (...for about 20s)
    oled.set2X();
    oled.setCursor(0,0);
    oled.println(temperature);
    oled.setCursor(70,0);
    oled.println(humidity);
    oled.set1X();
    oled.setCursor(0,6);  // put it back to where it was

    // wait for the other processor (linux/wrt) to come up
    // (else IP below from wifiCheck will be empty)
    delay(75000.-startupMillis);
  }

  // Print the IP address
  // From: File > Examples > Bridge > WiFiStatus
//  Serial.println(WiFi.localIP());
  Process wifiCheck;  // initialize a new process
  wifiCheck.runShellCommand(F("/usr/bin/pretty-wifi-info.lua | grep 'IP address' | awk -F '[:/]' '{print substr($2,2)}'"));  // command you want to run
  // while there's any characters coming back from the
  // process, print them to the serial monitor:
  while (wifiCheck.available() > 0) {
    oled.print(wifiCheck.readString());
  }

  FileSystem.begin();

  oled.println(F("config done"));
#ifdef DEBUG
  Serial.println(F("config done"));
  Serial.flush();
  //sDEBUG = String("config done");
#endif

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

  oled.clear();
}

void loop() {

  // Handle REST calls
  YunClient client = server.accept();
  rest.handle(client);

  // Blink triggers the state-change
  if (blinking())
  {
    // Update Sensor Measurements
    temperature = sht31.readTemperature();
    humidity = sht31.readHumidity();
    Vs = getVcc();
    Ti = getTemp();
    oled.set2X();
    oled.setCursor(0,0);
    oled.println(temperature);
    oled.setCursor(70,0);
    oled.println(humidity);
    oled.setCursor(0,4);
    oled.println(Ti);  // internal temp
    oled.setCursor(70,4);
    oled.println(Vs);  // supply voltage
#ifdef DEBUG
    Serial.print("Temp *C = "); Serial.println(temperature);
    Serial.print("Hum. % = "); Serial.println(humidity);
    Serial.print("Vs V = "); Serial.println(Vs);
    Serial.print("Ti *C = "); Serial.println(Ti);
#endif

    // From: File > Examples > Bridge > Datalogger
    // make a string that start with a timestamp for assembling the data to log:
    String dataString;
    dataString += getTimeStamp();

    oled.set1X();
    oled.setCursor(0,7);
//    Serial.print(getTimeStamp());
    //oled.print(getTimeStamp());
    oled.print(dataString);

    //dataString += " = ";
    dataString += ",";  // separate the values with a comma
    // read sensors and append to the string:
    dataString += String(temperature);
    dataString += ",";  // separate the values with a comma
    dataString += String(humidity);
    dataString += ",";  // separate the values with a comma
    dataString += String(Vs);
    dataString += ",";  // separate the values with a comma
    dataString += String(Ti);
    //dataString += ",";  // separate the values with a comma

    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    // The FileSystem card is mounted at the following "/mnt/FileSystema1"
//    File dataFile = FileSystem.open("/mnt/sd/datalog.txt", FILE_APPEND);
    File dataFile = FileSystem.open("/mnt/sda1/datalog.txt", FILE_APPEND);

    // if the file is available, write to it:
    if (dataFile) {
      dataFile.println(dataString);
      dataFile.close();
#ifdef DEBUG
      // print to the serial port too:
      Serial.println(dataString);
    }
    // if the file isn't open, pop up an error:
    else {
      Serial.println("error opening datalog.txt");
#endif
    }
  }

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

// This function returns a string with the time stamp
String getTimeStamp() {
  String result;
  Process date;  // process used to get the date
  // date is a command line utility to get the date and the time
  // in different formats depending on the additional parameter
  date.begin(F("date"));
  //date.begin(F("/bin/date"));
  //date.addParameter(F("+%D-%T"));  // parameters: D for the complete date mm/dd/yy
  ////             T for the time hh:mm:ss
  date.addParameter(F("+%Y-%m-%d %H:%M:%S"));
  date.run();  // run the command

  // read the output of the command
  while (date.available() > 0) {
    char c = date.read();
    if (c != '\n') {
      result += c;
    }
////    Serial.print(date.readString());
//    oled.print(date.readString());
  }

  return result;
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
