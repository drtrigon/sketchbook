// monitor vs log - add storage capabilities! (sd card needed?)

/*
  Arduino Yún Bridge example

 This example for the Arduino Yún shows how to use the
 Bridge library to access the digital and analog pins
 on the board through REST calls. It demonstrates how
 you can create your own API when using REST style
 calls through the browser.

 Possible commands created in this shetch:

 * "/arduino/digital/13"     -> digitalRead(13)
 * "/arduino/digital/13/1"   -> digitalWrite(13, HIGH)
 * "/arduino/analog/2/123"   -> analogWrite(2, 123)
 * "/arduino/analog/2"       -> analogRead(2)
 * "/arduino/mode/13/input"  -> pinMode(13, INPUT)
 * "/arduino/mode/13/output" -> pinMode(13, OUTPUT)

 * "/arduino/adc/01"         -> ads.readADC_Differential_0_1()
 * "/arduino/adc/23"         -> ads.readADC_Differential_2_3()
 * "/arduino/adc/0"          -> ads.readADC_SingleEnded(0)
 * "/arduino/adc/1"          -> ads.readADC_SingleEnded(1)
 * "/arduino/adc/2"          -> ads.readADC_SingleEnded(2)
 * "/arduino/adc/3"          -> ads.readADC_SingleEnded(3)
 (* "/arduino/mon/U"          -> return calibrated voltage reading)
 (* "/arduino/mon/I"          -> return calibrated current reading)
 (* "/arduino/log/U"          -> return most recent voltage log)
 (* "/arduino/log/I"          -> return most recent current log)

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

// https://www.arduino.cc/en/Guide/ArduinoYun
// https://www.arduino.cc/en/Tutorial/Bridge
// https://www.arduino.cc/en/reference/wire
// http://www.open-electronics.org/using-rest-with-arduino-yun/

 */

#include <Bridge.h>
#include <YunServer.h>
#include <YunClient.h>

#include <Wire.h>
#include <Adafruit_ADS1015.h>

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

  delay(50); // Poll every 50ms
}

void process(YunClient client) {
  // read the command
  String command = client.readStringUntil('/');

  // is "digital" command?
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
  }

  // is "adc" command?
  if (command == "adc") {
    adcCommand(client);
  }
}

void digitalCommand(YunClient client) {
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
}

// derived from 'analogCommand' and 'modeCommand'
void adcCommand(YunClient client) {
  int16_t results;
  
  /* Be sure to update this value based on the IC and the gain settings! */
  //float   multiplier = 3.0F;    /* ADS1015 @ +/- 6.144V gain (12-bit results) */
  float multiplier = 0.1875F; /* ADS1115  @ +/- 6.144V gain (16-bit results) */

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


