// TODO: add storage capabilities (sd card needed? try with usb stick first!)

/*
  Arduino Yún Battery Dis-/Charge Monitor and Logger
  (derived from Arduino Yún Bridge example, SD card datalogger)

 This example for the Arduino Yún shows how to use the
 Bridge library to access the digital and analog pins
 on the board through REST calls. It demonstrates how
 you can create your own API when using REST style
 calls through the browser.

 Possible commands created in this shetch:

 (* "/arduino/digital/13"     -> digitalRead(13) )
 (* "/arduino/digital/13/1"   -> digitalWrite(13, HIGH) )
 (* "/arduino/analog/2/123"   -> analogWrite(2, 123) )
 (* "/arduino/analog/2"       -> analogRead(2) )
 (* "/arduino/mode/13/input"  -> pinMode(13, INPUT) )
 (* "/arduino/mode/13/output" -> pinMode(13, OUTPUT) )

 * "/arduino/adc/01"         -> ads.readADC_Differential_0_1()
 * "/arduino/adc/23"         -> ads.readADC_Differential_2_3()
 * "/arduino/adc/0"          -> ads.readADC_SingleEnded(0)
 * "/arduino/adc/1"          -> ads.readADC_SingleEnded(1)
 * "/arduino/adc/2"          -> ads.readADC_SingleEnded(2)
 * "/arduino/adc/3"          -> ads.readADC_SingleEnded(3)

 * "/arduino/mon/U"          -> return calibrated voltage reading
 * "/arduino/mon/I"          -> return calibrated current reading
 * "/arduino/mon/R"          -> return calculated resistance
 * "/arduino/mon/P"          -> return calculated power
 * "/arduino/mon/C"          -> return integrated capacity
 * "/arduino/mon/E"          -> return integrated energy
 * "/arduino/mon/t"          -> return integrated time
 * "/arduino/mon/reset"      -> set t, C, E integrators to zero (by lt=t)
 * "/arduino/mon/log"        -> return most recent log
 (* "/arduino/mon/calib"      -> set U and I offset)

 This example code is part of the public domain

 http://www.arduino.cc/en/Tutorial/Bridge

// 1.) wire groove connector to Ardino:
//             UNO   Yun
//      black  GND   GND
//        red   5V    5V
//      white   A4    P2   SDA
//     yellow   A5    P3   SCL
// 2.) connect to ArduinoYun via USB to PC
// 3.) use arduino IDE (new version, e.g. 1.6.5) to upload this sketch
// 4.) connect to 'ArduinoYun-XXXXXXXXXXXX' wifi/wlan
//     (maybe make it later connect to usual wifi)
// 5.) use browser: http://arduino.local/ (password: 'arduino')
// 6.) click to 'CONFIGURE' and set 'REST API access' to 'OPEN' (default: 'WITH PASSWORD')
// 7.) use browser: http://arduino.local/arduino/digital/13
//     > Pin D13 set to 1
//     http://arduino.local/arduino/mode/13/output
//     http://arduino.local/arduino/digital/13/1
// 8.) use browser: http://arduino.local/arduino/adc/01
//                  http://arduino.local/arduino/adc/23
//                  http://arduino.local/arduino/mon/log

// https://www.arduino.cc/en/Guide/ArduinoYun
// https://www.arduino.cc/en/Tutorial/Bridge
// https://www.arduino.cc/en/reference/wire
// http://www.open-electronics.org/using-rest-with-arduino-yun/

 */

#include <Bridge.h>
#include <YunServer.h>
#include <YunClient.h>

//#include <FileIO.h>

#include <Wire.h>
#include <Adafruit_ADS1015.h>

//#define DEBUG

#define AVERAGE    10      // NOISY thus average! investigate why hardware is noisy!
#define LOG_SIZE   30
#ifndef DEBUG
#define LOG_TIME   300.0
#else
#define LOG_TIME   30.0
#endif
/* Be sure to update this value based on the IC and the gain settings! */
//#define multiplier 3.0F    /* ADS1015 @ +/- 6.144V gain (12-bit results) */
#define multiplier 0.1875F /* ADS1115  @ +/- 6.144V gain (16-bit results) */

// tested for discharge (single-ended mode)
//static inline float calibration_V(float V) { return 10.3 * (V - 57.)   * 1.E-3; }  //  ~0.1V/V, offset ~50mV (why?) [returns V]
// tested for charge (single-ended mode)
static inline float calibration_V(float V) { return 9.9 * (V - 79.)   * 1.E-3; }  //  ~0.1V/V, offset ~50mV (why?) [returns V]
// tested for dis-/charge (single-ended mode)
static inline float calibration_I(float I) { return (I - 2523.) / 245. * 1.E0;  }  // ~200mV/A, offset ~2500mV [returns A]
// it looks like we have an additional offset of about +22mV on each channel when running stand-alone (not on PC USB)

unsigned long const MAX_unsigned_long = -1;

float ADC_SE_0, ADC_SE_1;
float U, I, R, P, C, E, T, lI, lT;
float W = 0.0;
unsigned long t  = 0;
unsigned long lt = 0;
int dsindex = LOG_SIZE - 1;  // such that +1 -> 0

// Listen to the default port 5555, the Yún webserver
// will forward there all the HTTP requests you send
YunServer server;

Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
// Adafruit_ADS1015 ads;     /* Use thi for the 12-bit version */

void setup() {
  // Bridge startup
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  Bridge.begin();
  digitalWrite(13, LOW);

  // Listen for incoming connection only from localhost
  // (no one from the external network could connect)
  server.listenOnLocalhost();
  server.begin();

//  Serial.begin(9600);
//  Serial.println("Hello!");
//  
//  Serial.println("Getting differential reading from AIN0 (P) and AIN1 (N)");
//  Serial.println("Getting single-ended readings from AIN0..3");
//  Serial.println("ADC Range: +/- 6.144V (1 bit = 3mV/ADS1015, 0.1875mV/ADS1115)");
//  
  // The ADC input range (or gain) can be changed via the following
  // functions, but be careful never to exceed VDD +0.3V max, or to
  // exceed the upper and lower limits if you adjust the input range!
  // Setting these values incorrectly may destroy your ADC!
  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
  
  ads.begin();

//  FileSystem.begin();
}

void loop() {
  // Get clients coming from server
  YunClient client = server.accept();

  // There is a new client?
  if (client) {
    // Process request
    process(client);

    // Close connection and free resources.
    client.stop();
  }

  // Measure data
  monitor_func();
  // Log data
  log_func();

  delay(50); // Poll every 50ms
}

void process(YunClient client) {
  // read the command
  String command = client.readStringUntil('/');

/*  // is "digital" command?
  if (command == "digital") {
    digitalCommand(client);
  }

  // is "analog" command?
  if (command == "analog") {
    analogCommand(client);
  }

  // is "mode" command?
  if (command == "mode") {
    modeCommand(client);
  }*/

  // is "adc" command?
  if (command == "adc") {
    adcCommand(client);
  }

  // is "mon" command?
  if (command == "mon") {
    monCommand(client);
  }
}

/*void digitalCommand(YunClient client) {
  int pin, value;

  // Read pin number
  pin = client.parseInt();

  // If the next character is a '/' it means we have an URL
  // with a value like: "/digital/13/1"
  if (client.read() == '/') {
    value = client.parseInt();
    digitalWrite(pin, value);
  }
  else {
    value = digitalRead(pin);
  }

  // Send feedback to client
  client.print(F("Pin D"));
  client.print(pin);
  client.print(F(" set to "));
  client.println(value);

  // Update datastore key with the current pin value
  String key = "D";
  key += pin;
  Bridge.put(key, String(value));
}

void analogCommand(YunClient client) {
  int pin, value;

  // Read pin number
  pin = client.parseInt();

  // If the next character is a '/' it means we have an URL
  // with a value like: "/analog/5/120"
  if (client.read() == '/') {
    // Read value and execute command
    value = client.parseInt();
    analogWrite(pin, value);

    // Send feedback to client
    client.print(F("Pin D"));
    client.print(pin);
    client.print(F(" set to analog "));
    client.println(value);

    // Update datastore key with the current pin value
    String key = "D";
    key += pin;
    Bridge.put(key, String(value));
  }
  else {
    // Read analog pin
    value = analogRead(pin);

    // Send feedback to client
    client.print(F("Pin A"));
    client.print(pin);
    client.print(F(" reads analog "));
    client.println(value);

    // Update datastore key with the current pin value
    String key = "A";
    key += pin;
    Bridge.put(key, String(value));
  }
}

void modeCommand(YunClient client) {
  int pin;

  // Read pin number
  pin = client.parseInt();

  // If the next character is not a '/' we have a malformed URL
  if (client.read() != '/') {
    client.println(F("error"));
    return;
  }

  String mode = client.readStringUntil('\r');

  if (mode == "input") {
    pinMode(pin, INPUT);
    // Send feedback to client
    client.print(F("Pin D"));
    client.print(pin);
    client.print(F(" configured as INPUT!"));
    return;
  }

  if (mode == "output") {
    pinMode(pin, OUTPUT);
    // Send feedback to client
    client.print(F("Pin D"));
    client.print(pin);
    client.print(F(" configured as OUTPUT!"));
    return;
  }

  client.print(F("error: invalid mode "));
  client.print(mode);
}*/

// derived from 'analogCommand' and 'modeCommand'
void adcCommand(YunClient client) {
  int16_t results;

  // Read pin number
  String pins = client.readStringUntil('\r');

  if (pins == "01") {
    // Read adc diff. pins
    results = ads.readADC_Differential_0_1();

    // Send feedback to client
    client.print(F("Pin ADC"));
    client.print(pins);
    client.print(F(" reads analog diff "));
    client.print(results);
    client.print(F(" (")); client.print(results * multiplier); client.println(F("mV)"));

    // Update datastore key with the current pin value
    String key = "ADC";
    key += pins;
    Bridge.put(key, String(results));

    return;
  }

  if (pins == "23") {
    // Read adc diff. pins
    results = ads.readADC_Differential_2_3();

    // Send feedback to client
    client.print(F("Pin ADC"));
    client.print(pins);
    client.print(F(" reads analog diff "));
    client.print(results);
    client.print(F(" (")); client.print(results * multiplier); client.println(F("mV)"));

    // Update datastore key with the current pin value
    String key = "ADC";
    key += pins;
    Bridge.put(key, String(results));

    return;
  }

  if (pins == "0") {
    // Read adc single-end. pins
    results = ads.readADC_SingleEnded(0);

    // Send feedback to client
    client.print(F("Pin ADC"));
    client.print(pins);
    client.print(F(" reads analog diff "));
    client.print(results);
    client.print(F(" (")); client.print(results * multiplier); client.println(F("mV)"));

    // Update datastore key with the current pin value
    String key = "ADC";
    key += pins;
    Bridge.put(key, String(results));

    return;
  }

  if (pins == "1") {
    // Read adc single-end. pins
    results = ads.readADC_SingleEnded(1);

    // Send feedback to client
    client.print(F("Pin ADC"));
    client.print(pins);
    client.print(F(" reads analog diff "));
    client.print(results);
    client.print(F(" (")); client.print(results * multiplier); client.println(F("mV)"));

    // Update datastore key with the current pin value
    String key = "ADC";
    key += pins;
    Bridge.put(key, String(results));

    return;
  }

  if (pins == "2") {
    // Read adc single-end. pins
    results = ads.readADC_SingleEnded(2);

    // Send feedback to client
    client.print(F("Pin ADC"));
    client.print(pins);
    client.print(F(" reads analog diff "));
    client.print(results);
    client.print(F(" (")); client.print(results * multiplier); client.println(F("mV)"));

    // Update datastore key with the current pin value
    String key = "ADC";
    key += pins;
    Bridge.put(key, String(results));

    return;
  }

  if (pins == "3") {
    // Read adc single-end. pins
    results = ads.readADC_SingleEnded(3);

    // Send feedback to client
    client.print(F("Pin ADC"));
    client.print(pins);
    client.print(F(" reads analog diff "));
    client.print(results);
    client.print(F(" (")); client.print(results * multiplier); client.println(F("mV)"));

    // Update datastore key with the current pin value
    String key = "ADC";
    key += pins;
    Bridge.put(key, String(results));

    return;
  }

  client.print(F("error: invalid pins "));
  client.print(pins);
}

// derived from 'adcCommand'
void monCommand(YunClient client) {
  // Read pin number
  String pins = client.readStringUntil('\r');

  if (pins == "U") {
    // Send feedback to client
    client.print(F("Voltage reads "));
    client.print(U, 3);
    client.println(F("V"));

    // Update datastore key with the current pin value
    Bridge.put("U", String(U, 3));

    return;
  }

  if (pins == "I") {
    // Send feedback to client
    client.print(F("Current reads "));
    client.print(I, 3);
    client.println(F("A"));

    // Update datastore key with the current pin value
    Bridge.put("I", String(I, 3));

    return;
  }

  if (pins == "R") {
    // Send feedback to client
    client.print(F("Resistance reads "));
    client.print(R, 3);
    client.println(F("Ohm"));

    // Update datastore key with the current pin value
    Bridge.put("R", String(R, 3));

    return;
  }

  if (pins == "P") {
    // Send feedback to client
    client.print(F("Power reads "));
    client.print(P, 3);
    client.println(F("W"));

    // Update datastore key with the current pin value
    Bridge.put("P", String(P, 3));

    return;
  }

  if (pins == "C") {
    // Send feedback to client
    client.print(F("Capacity reads "));
    client.print(C, 3);
    client.println(F("mAh"));

    // Update datastore key with the current pin value
    Bridge.put("C", String(C, 3));

    return;
  }

  if (pins == "E") {
    // Send feedback to client
    client.print(F("Energy reads "));
    client.print(E, 3);
    client.println(F("J"));

    // Update datastore key with the current pin value
    Bridge.put("E", String(E, 3));

    return;
  }

  if (pins == "W") {
    // Send feedback to client
    client.print(F("Watch reads "));
    client.print(W, 3);
    client.println(F("s"));

    // Update datastore key with the current pin value
    Bridge.put("W", String(W, 3));

    return;
  }

  if ((pins == "t") || (pins == "reset")) {
    if (pins == "reset") {
      lt = t;  // trigger reset in func 'monitor_func'
    }

    // Send feedback to client
    client.print(F("Time reads "));
    client.print(T, 3);
    client.println(F("s"));

    // Update datastore key with the current pin value
    Bridge.put("t", String(T, 3));

    return;
  }

  if (pins == "log") {
    // Send feedback to client
    int i = (dsindex + 1) % LOG_SIZE;
    char myData[10];
    while (true) {
      // Retrieve value from datastore by key
      // https://www.arduino.cc/en/Reference/YunGet
      Bridge.get((String("bW")+i).c_str(), myData, 10);
      //Serial.println(myData);
      client.print(myData);
      client.print(F(", "));
      Bridge.get((String("bT")+i).c_str(), myData, 10);
      client.print(myData);
      client.print(F(", "));
      Bridge.get((String("bU")+i).c_str(), myData, 10);
      client.print(myData);
      client.print(F(", "));
      Bridge.get((String("bI")+i).c_str(), myData, 10);
      client.print(myData);
      client.print(F(", "));
      Bridge.get((String("bR")+i).c_str(), myData, 10);
      client.print(myData);
      client.print(F(", "));
      Bridge.get((String("bP")+i).c_str(), myData, 10);
      client.print(myData);
      client.print(F(", "));
      Bridge.get((String("bC")+i).c_str(), myData, 10);
      client.print(myData);
      client.print(F(", "));
      Bridge.get((String("bE")+i).c_str(), myData, 10);
      client.println(myData);

      if (i == dsindex) {
        return;
      }
      i = (i + 1) % LOG_SIZE;
    }

    return;
  }

  client.print(F("error: invalid value "));
  client.print(pins);
}

void monitor_func(void) {
  // Measure data (with averaging?)
  int16_t results;
  float dt, dC, dE;

  // Read adc single-end. pins
  ADC_SE_0 = 0.0;
  for (int i=0; i < AVERAGE; ++i) {
    results = ads.readADC_SingleEnded(0);
    ADC_SE_0 += results * multiplier;       // NOISY thus average! investigate why hardware is noisy!
  }
  ADC_SE_0 /= AVERAGE;
  // Read adc single-end. pins
  ADC_SE_1 = 0.0;
  for (int i=0; i < AVERAGE; ++i) {
    results = ads.readADC_SingleEnded(1);
    ADC_SE_1 += results * multiplier;       // NOISY thus average! investigate why hardware is noisy!
  }
  ADC_SE_1 /= AVERAGE;

  U = calibration_V(ADC_SE_0);           // unit: V
  lI = I;
  I = calibration_I(ADC_SE_1);           // unit: A
  R = U / I;                             // unit: Ohm
  P = U * I;                             // unit: W
  //if ((t == lt) || ((sgn(lI) != sgn(I)) && (abs(I - lI) > 0.010))) {  // reset "counters"
  if ((t == lt) || ((sgn(lI) != sgn(I)) && (abs(I - lI) > 0.020))) {  // reset "counters" (e.g. start/end of dis/charge cycles)
  //if ((t == lt) || (abs(I - lI) > 0.050)) {  // reset "counters" and edge detection for start/end of dis/charge cycles
    t = millis() - 1;
    C = 0.0;
    E = 0.0;
    T = 0.0;
    lT = -LOG_TIME;   // -5min
  }
  lt = t;                                // unit: ms
  t  = millis();                         // unit: ms
  dt = (t - lt) * 1E-3;                  // unit: s
  if (dt < 0.) {
    // handle wrap of millis()
    dt = (t - (lt-MAX_unsigned_long-1)) * 1E-3;
  }
  dC = I * dt / 3.6;                     // unit: mAh
  dE = P * dt;                           // unit: J
  C += dC;                               // unit: mAh
  E += dE;                               // unit: J
  T += dt;                               // unit: s
  W += dt;                               // unit: s
}

void log_func(void) {
  if ((T - lT) < LOG_TIME) {  // storage/buffer delay: 5mins (since 50ms is too fast)
    return;
  }
  lT = T;

  // Update datastore key with the data to log (store data on the Linux processor)
  // https://www.arduino.cc/en/Reference/YunPut
  // Reason to prefer over array is here we have virtually infinite memory and does not interfere with program memory
  dsindex = (dsindex + 1) % LOG_SIZE;    // ring buffer
  Bridge.put(String("bU")+dsindex, String(U, 3));
  Bridge.put(String("bI")+dsindex, String(I, 3));
  Bridge.put(String("bR")+dsindex, String(R, 3));
  Bridge.put(String("bP")+dsindex, String(P, 3));
  Bridge.put(String("bC")+dsindex, String(C, 3));
  Bridge.put(String("bE")+dsindex, String(E, 3));
  Bridge.put(String("bT")+dsindex, String(T, 3));
  Bridge.put(String("bW")+dsindex, String(W, 3));

  // Write to SD card or USB stick
  // https://www.arduino.cc/en/Reference/YunBridgeLibrary
  // https://www.arduino.cc/en/Tutorial/YunDatalogger
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  // The FileSystem card is mounted at the following "/mnt/FileSystema1"
/*  File dataFile = FileSystem.open("/mnt/sd/datalog.txt", FILE_APPEND);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }*/
  // ursin@ursin-ThinkPad-T420:~$ ssh root@arduino.local
  // (password: arduino)
  // root@Arduino:~# ls -la /mnt/
  // http://forum.arduino.cc/index.php?topic=218307.0
  // http://wiki.openwrt.org/doc/howto/usb.storage
  // root@Arduino:~# ls -la /dev/ 
}

//static inline int8_t sgn(int val) {
static inline int8_t sgn(float val) {
  // http://forum.arduino.cc/index.php?topic=37804.msg279218#msg279218
 if (val < 0) return -1;
// if (val==0) return 0;
 return 1;
}


