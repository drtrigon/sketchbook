/*
  Blink :
  For LG01: Turn on/off the HEART LED on LG01, 

  modified 8 May 2014
  by Edwin Chen <support@dragino.com>
  Dragino Technology Co., Limited

  Hints: 
  * see "LG01 LoRa Gateway User Manual" p12 (e.g. owncloud)
  * install arduino IDE 1.8.3
  * File > Preferences > Additionals Boards Manager URLs: add line with "http://www.dragino.com/downloads/downloads/YunShield/package_dragino_yun_test_index.json"
  * Tools > Board: ... > Boards Manager...: find "Dragino" and install
  * Tools > Board: ... > "Dragino Yun + UNO or LG01/OLG01"
  * Tools > Port: ... > "dragino-182d19 at 10.130.1.1 (Arduino Yun)"
  * upload File > Examples > Dragino > Basic > Blink: pasword is "dragino"
  * use with File > Examples > Dragino > LoRa > LoRa_Simple_Client_Arduino
    or modified variant ...
  * connect to http://10.130.1.1/ for dragino interface
               http://10.130.1.1:5555/ for restful interface:
               http://192.168.11.14:5555/variables or http://192.168.11.14:5555/arduino/variables
  * ISSUES:
    * mainly (dynamic) memory issues, most prominent
      in combination of Console.print, REST library's
      url calls and string pointers ('new', memory leak)
      turns out that we are working hard on (dynamic) memory
      limit - creating a few extra pointers (with 'new') and
      not using them already crashes the code - caution memory leaks!
    * mainly visible/appearing as output of scrabled strings
      in Console.print or REST url (variables)
    * REST string pointer need to be created from literals or
      using 'new' in order to preserve them (keep valid) - using
      'new' makes sure that we do have to delete the pointer
      intentionally/manually (no garbe collection when out of scope!)
      - not to become invalid while beeing on the rest class internal list
    * REST string variables are not still not working yet, also because of 
      to the pointer 'new' confusion
    * using RTOS for the 3 tasks (blink, REST server, LoRa server)
      needs about 5KB more of program memory and shrinks the local
      memory (for dynamic allocation) to to below 500B with is critical
    * RH_RF95_MAX_MESSAGE_LEN > 250B
    * code works with freeMemory()<550 (currently =571..572)

  Configure Dragino:
  * disable AP and switch to WiFi client mode
    http://wiki.dragino.com/index.php?title=IoT_Mesh_Firmware_User_Manual#WiFi_Client_Mode
    * change to 'dragino2-xxxxxx'/'dragino-182d19' network: http://10.130.1.1/ ('root', 'dragino')
    * Network > Internet Access:
      * Access Internet via WiFi Client
      * Way to Get IP: DHCP
      * Input correct SSID, Password and Encryption. 
      * [Save & Apply]
    * change network to 'Buffalo' and open: http://192.168.11.14/
    * Network > LAN and DHCP
      * Enable DHCP server in its LAN port
      * (was all ok)
    * Network > Access Point
      * Disable WiFi AP
      * (turned off autamtically after switch to WiFi Client and [Save & Apply])
    * Network > Mesh Network
      * Disable WiFi Mesh Network
      * (was off too)
  * disable WiFi and switch to WAN (client) mode)
    * Network > Internet Access:
      * Access Internet via WAN PORT or USB Modem

  Features:
  * Heartbeat (Blink)
    File > Examples > Dragino > Basic > Blink
    https://github.com/dragino/Arduino-Profile-Examples/tree/master/libraries/Dragino/examples/Basic/Blink/Blink.ino
  * LoRa Simple Yun Server
    File > Examples > Dragino > LoRa > LoRa_Simple_Server_Yun
    https://github.com/dragino/Arduino-Profile-Examples/blob/master/libraries/Dragino/examples/LoRa/LoRa_Simple_Server_Yun/LoRa_Simple_Server_Yun.ino
  * RESTful (aREST or Bridge)
    File > Examples > aREST > Yun
    https://github.com/marcoschwartz/aREST/blob/master/examples/Yun/Yun.ino
    or
    File > Examples > Bridge > Bridge
    https://www.arduino.cc/en/Tutorial/Bridge (all commands supported & variables - output json)
    (has known issues that it does not recognize commands - therefore merged with Bridge variant; aREST_bridge.h)
    own rest interface; aREST_bridge.h
    * TODO: analog pins? how to change mode? A2 = 16 (Console.println(A2, DEC);)
    * TODO: http://myArduinoYun.local/arduino/digital -> read all digital pins? error? variables? what says the standard?
    * TODO: http://myArduinoYun.local/arduino/analog -> read all analog pins? error? variables? what says the standard?
    * TODO: http://myArduinoYun.local/variables/name -> read single variable? error? variables? what says the standard?
  * transmission of 2 values (Vs, Ti) with answer and re-try (up to 5 times until confirmation)
  * watchdog 8s
  * handling of multiple sensors
    * every sensor get 3 registred values in REST according to ID string
    * voltage, temperature and RSSI

  ? write data to /var/iot/data as that can be viewed in web gui
    (issues: no graphical display, how to clean/delete/reset, etc.)
  ? Concurrent (polling instead of receiving all the time)
    See also manual p58 "12.4 How many nodes can the LG01 support?"
    Consider using the polling method to ensure that each time
    will only have a LoRa signal transmit in the frequency. Can
    support several hundred nodes or more. Examples can see:
    File > Examples > Dragino > LoRa > Concurrent > ...
    (issues: need more battery/power on the node/sensor side since
             they need all to be on all the time)
    (implemented: sensor and packet identification such that
     handling multiple sensors should be possible; ID, timestamp)
*/
/*
  LoRa Simple Yun Server :
  Support Devices: LG01. 
  
  Example sketch showing how to create a simple messageing server, 
  with the RH_RF95 class. RH_RF95 class does not provide for addressing or
  reliability, so you should only use RH_RF95 if you do not need the higher
  level messaging abilities.

  It is designed to work with the other example LoRa Simple Client

  User need to use the modified RadioHead library from:
  https://github.com/dragino/RadioHead

  modified 16 11 2016
  by Edwin Chen <support@dragino.com>
  Dragino Technology Co., Limited
*/
/*
  This a simple example of the aREST Library working with
  the Arduino Yun. See the README file for more details.

  Written in 2014 by Marco Schwartz under a GPL license.
*/

//If you use Dragino IoT Mesh Firmware, uncomment below lines.
//For product: LG01. 
#define BAUDRATE 115200

//If you use Dragino Yun Mesh Firmware , uncomment below lines. 
//#define BAUDRATE 250000

#include <Console.h>
//#include <SPI.h>
#include <RH_RF95.h>

// Import required libraries
#include <Bridge.h>
#include <YunServer.h>
#include <YunClient.h>
//#include <aREST.h>
#include <aREST_bridge.h>

// https://bigdanzblog.wordpress.com/2014/10/24/arduino-watchdog-timer-wdt-example-code/
#include <avr/wdt.h>    // watchdog

#include <MemoryFree.h>

// Singleton instance of the radio driver
RH_RF95 rf95;

// Create aREST instance
aREST rest = aREST();

// Yun Server
//YunServer server(80);
YunServer server;  // default port 5555

#define HEART_LED A2
#define frequency 868.0

// Variables to be exposed to the API
//float fvoltage;
//float ftemperature;
float fRSSI;

void setup() {
  // - immediately disable watchdog timer so set will not get interrupted
  // - any 'slow' activity needs to be completed before enabling the watchdog timer.
  // - the following forces a pause before enabling WDT. This gives the IDE a chance to
  //   call the bootloader in case something dumb happens during development and the WDT
  //   resets the MCU too quickly. Once the code is solid, remove this.
  wdt_disable();

  // initialize digital pin  as an output.
  pinMode(HEART_LED, OUTPUT);

  Bridge.begin(BAUDRATE);
// ATTENTION: THIS 'Console' (not beeing 'Serial') has issues, especially
//            in combination with low memory (REST libraries url calls,
//            string pointers, etc.) and the Console output gets scrambled!
  Console.begin();
//  while (!Console) ; // Wait for console port to be available
  Console.println(F("Start Sketch"));
  if (!rf95.init())
    Console.println(F("init failed"));
  // Setup ISM frequency
  rf95.setFrequency(frequency);
  // Setup Power,dBm
  rf95.setTxPower(13);
  
  // Setup Spreading Factor (6 ~ 12)
  rf95.setSpreadingFactor(7);
  
  // Setup BandWidth, option: 7800,10400,15600,20800,31200,41700,62500,125000,250000,500000
  rf95.setSignalBandwidth(125000);
  
  // Setup Coding Rate:5(4/5),6(4/6),7(4/7),8(4/8) 
  rf95.setCodingRate4(5);
  
  Console.print(F("Listening on frequency: "));
  Console.println(frequency);

  /*// Init variables and expose them to REST API (ID should be 6 characters long)
  rest.variable("S01-Vs",&fvoltage);
  rest.variable("S01-Ti",&ftemperature);
  rest.variable("S01-RSSI",&fRSSI);
  //rest.variable("S02-RSSI",&sRSSI02);
  Bridge.put("S01-test", String(42.37));
  rest.variable("S01-test",(float*)NULL);*/

  /*// Function to be exposed
  rest.function("led",ledControl);*/

  // Give name and ID to device
  rest.set_id(F("008"));
//  rest.set_name(F("mighty_cat"));
  rest.set_name(F("dragino-LoRa"));

  // Listen for incoming connection only from localhost
  server.begin();
  
  Console.println(F("RESTServer running"));

  wdt_reset();
  wdt_enable(WDTO_8S);
}

// the loop function runs over and over again forever
void loop() {
  wdt_reset();

  if (rf95.available())
  {
    // Should be a message for us now   
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf95.recv(buf, &len))
    {
      digitalWrite(HEART_LED, HIGH);
//      RH_RF95::printBuffer("request: ", buf, len);
      Console.print(F("got request: "));
//      Console.println((char*)buf);
//      String data = String((char*)buf);
      String data = String((char*)buf).substring(0, len);  // in case of missing '\0' at the end
      Console.println(data);
      int i = data.indexOf(" ", 0);
      int j = data.indexOf(" ", i+1);
      int k = data.indexOf(" ", j+1);
      String ID = data.substring(0, i);
      //String ts = data.substring(i+1, j);
      String val_voltage = data.substring(j+1, k);
      String val_temperature = data.substring(k+1, len);
      /*fvoltage = val_voltage.toFloat();
      ftemperature = val_temperature.toFloat();
      Console.println(ID);
      Console.println(data.substring(i+1, j));
      Console.println(val);*/
      Console.print(F("RSSI: "));
//      Console.println(rf95.lastRssi(), DEC);
      fRSSI = float(rf95.lastRssi());
      Console.println(fRSSI);
      
      // check, register and populate variable with rest (if needed)
// ATTENTION: 'new' makes sure that I do have to delete the pointer
//            (no garbe collection when out of scope!) - not to
//            become invalid while beeing on the rest class internal list
//            (and then pointing to some random garbage in mem)
      String* key  = new String(ID + "_Vs");    // use 'new'
      String* key2 = new String(ID + "_Ti");    // use 'new'
      String* key3 = new String(ID + "_RSSI");  // use 'new'
      Console.print(F("variable: "));
      Console.print(key->c_str());
      Console.print(F(" "));
      Console.println(rest.is_variable(key->c_str()));
      Bridge.put(key->c_str(), val_voltage);
      Bridge.put(key2->c_str(), val_temperature);
      Bridge.put(key3->c_str(), String(fRSSI));
      if (!rest.is_variable(key->c_str())) {  // always register in group of 3
        rest.variable(key->c_str(), (float*)NULL);   // NULL -> use Bridge.get
        rest.variable(key2->c_str(), (float*)NULL);  // NULL -> use Bridge.get
        rest.variable(key3->c_str(), (float*)NULL);  // NULL -> use Bridge.get
      }
      else {  // already registred, don't need new ones - avoid memory leaks!
        delete key;
        delete key2;
        delete key3;
      }

      wdt_reset();

      // Send a confirmation reply (by adding "OK")
//      uint8_t data[] = "And hello back to you";
//      rf95.send(data, sizeof(data));
      data += String("OK");
      rf95.send(data.c_str(), strlen(data.c_str()));
      rf95.waitPacketSent();
      Console.println(F("Sent a confirmation reply"));
      Console.print(F("freeMemory()=")); Console.println(freeMemory());
      digitalWrite(HEART_LED, LOW);
      delay(100);              // wait for 1/10 second
    }
    else
    {
      Console.println(F("recv failed"));
    }
  }
  else
  {
    // Handle REST calls
    YunClient client = server.accept();
    rest.handle(client);

    wdt_reset();

    digitalWrite(HEART_LED, HIGH);   // turn the HEART_LED on (HIGH is the voltage level)
    delay(1000);              // wait for a second
    digitalWrite(HEART_LED, LOW);    // turn the HEART_LED off by making the voltage LOW
    delay(1000);              // wait for a second
  }
}

/*// Custom function accessible by the API
int ledControl(String command) {

  // Get state from command
  int state = command.toInt();

  digitalWrite(7,state);
  return 1;
}*/
