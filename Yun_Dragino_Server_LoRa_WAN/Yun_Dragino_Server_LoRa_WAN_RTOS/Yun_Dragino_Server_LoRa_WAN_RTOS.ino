/*
  Blink :
  For LG01: Turn on/off the HEART LED on LG01, 

  modified 8 May 2014
  by Edwin Chen <support@dragino.com>
  Dragino Technology Co., Limited

  ISSUE: LOW MEMORY -> STABILITY ISSUES (problem with String classes and code does not run)
                       Low memory available, stability problems may occur. (have 421 - need more than 500)

  Hints: 
  * see "LG01 LoRa Gateway User Manual" p12 (e.g. owncloud)
  * install arduino IDE 1.8.3
  * File > Preferences > Additionals Boards Manager URLs: add line with "http://www.dragino.com/downloads/downloads/YunShield/package_dragino_yun_test_index.json"
  * Tools > Board: ... > Boards Manager...: find "Dragino" and install
  * Sketch > Include Library > Manage Libraries...: find "rtos" and install
  * Tools > Board: ... > "Dragino Yun + UNO or LG01/OLG01"
  * Tools > Port: ... > "dragino-182d19 at 10.130.1.1 (Arduino Yun)"
  * upload File > Examples > Dragino > Basic > Blink: pasword is "dragino"
  * use with File > Examples > Dragino > LoRa > LoRa_Simple_Client_Arduino
    or modified variant ...
  * connect to http://10.130.1.1/ for dragino interface
               http://10.130.1.1:5555/ for restful interface

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
    https://www.arduino.cc/en/Tutorial/Bridge
    (has known issues that it does not recognize commands...)
    (also strings not possible - switch to bridge and write own or find other!)

? correct arest string handling once again and test with enough local memory free!!!
  ? lora_simple_client_arduino with vcc and ti data
  ? /var/iot/data
  ? Concurrent (polling instead of receiving all the time)
    See also manual p58 "12.4 How many nodes can the LG01 support?"
    Consider using the polling method to ensure that each time
    will only have a LoRa signal transmit in the frequency. Can
    support several hundred nodes or more. Examples can see:
    File > Examples > Dragino > LoRa > Concurrent > ...

  watchdog, bestätigung vom server an client (client wiederholt sonst)

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

#include <Arduino_FreeRTOS.h>

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
#include <aREST.h>

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
int temperature;
int humidity;
float sDATA01;
float sRSSI01;

// define two tasks for Blink & AnalogRead
void TaskBlink( void *pvParameters );
void TaskLoRaReceive( void *pvParameters );
void TaskRESTServer( void *pvParameters );

void setup() {
  Bridge.begin(BAUDRATE);
  Console.begin();
//  while (!Console) ; // Wait for console port to be available
  Console.println(F("Start Sketch"));

  // Now set up two tasks to run independently.
  xTaskCreate(
    TaskBlink
    ,  (const portCHAR *)"Blink"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

  xTaskCreate(
    TaskLoRaReceive
    ,  (const portCHAR *)"LoRaReceive"
    ,  256  // Stack size
    ,  NULL
    ,  2  // Priority
    ,  NULL );

  xTaskCreate(
    TaskRESTServer
    ,  (const portCHAR *)"RESTServer"
    ,  256  // Stack size
    ,  NULL
    ,  2  // Priority
    ,  NULL );

  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}

// the loop function runs over and over again forever
void loop()
{
  // Empty. Things are done in Tasks.
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskBlink(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, LEONARDO, MEGA, and ZERO 
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN takes care 
  of use the correct LED pin whatever is the board used.
  
  The MICRO does not have a LED_BUILTIN available. For the MICRO board please substitute
  the LED_BUILTIN definition with either LED_BUILTIN_RX or LED_BUILTIN_TX.
  e.g. pinMode(LED_BUILTIN_RX, OUTPUT); etc.
  
  If you want to know what pin the on-board LED is connected to on your Arduino model, check
  the Technical Specs of your board  at https://www.arduino.cc/en/Main/Products
  
  This example code is in the public domain.

  modified 8 May 2014
  by Scott Fitzgerald
  
  modified 2 Sep 2016
  by Arturo Guadalupi
*/

  // initialize digital pin  as an output.
  pinMode(HEART_LED, OUTPUT);

  for (;;) // A Task shall never return or exit.
  {
    digitalWrite(HEART_LED, HIGH);   // turn the HEART_LED on (HIGH is the voltage level)
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second
    digitalWrite(HEART_LED, LOW);    // turn the HEART_LED off by making the voltage LOW
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second
  }
}

void TaskLoRaReceive(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

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

  for (;;)
  {

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
      String data = String((char*)buf);
      Console.println(data);
// process and split string
      int i = data.indexOf(" ", 0);
      int j = data.indexOf(" ", i+1);
      String ID = data.substring(0, i);
      //String ts = data.substring(i+1, j);
      String val = data.substring(j+1, len);
//      sDATA01 = float(val);
      Console.println(ID);
      Console.println(data.substring(i+1, j));
      Console.println(val);
      Console.print(F("RSSI: "));
//      Console.println(rf95.lastRssi(), DEC);
      sRSSI01 = float(rf95.lastRssi());
      Console.println(sRSSI01);
      
      // Send a reply
//      uint8_t data[] = "And hello back to you";
//      rf95.send(data, sizeof(data));
      data += String("OK");
      rf95.send(data.c_str(), strlen(data.c_str()));
      rf95.waitPacketSent();
      Console.println(F("Sent a reply"));
      digitalWrite(HEART_LED, LOW);
    }
    else
    {
      Console.println(F("recv failed"));
    }
  }

    vTaskDelay(1);  // one tick delay (15ms) in between reads for stability
  }
}

void TaskRESTServer(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  // Init variables and expose them to REST API (ID should be 6 characters long)
  temperature = 24;
  humidity = 40;
  rest.variable("temperature",&temperature);
  rest.variable("voltage",&humidity);
  rest.variable("S01-data",&sDATA01);
  rest.variable("S01-RSSI",&sRSSI01);
  //rest.variable("S02-data",&sDATA02);
  //rest.variable("S02-RSSI",&sRSSI02);

  /*// Function to be exposed
  rest.function("led",ledControl);*/

  // Give name and ID to device
  rest.set_id("008");
  rest.set_name("mighty_cat");

  // Listen for incoming connection only from localhost
  server.begin();
  
  Console.print(F("RESTServer running"));
  
  for (;;)
  {
    // Handle REST calls
    YunClient client = server.accept();
    rest.handle(client);
    vTaskDelay(1);  // one tick delay (15ms) in between reads for stability
  }
}

/*// Custom function accessible by the API
int ledControl(String command) {

  // Get state from command
  int state = command.toInt();

  digitalWrite(7,state);
  return 1;
}*/
