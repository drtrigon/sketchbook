/*
  LoRa Simple Client for Arduino :
  Support Devices: LoRa Shield + Arduino 
  
  Example sketch showing how to create a simple messageing client, 
  with the RH_RF95 class. RH_RF95 class does not provide for addressing or
  reliability, so you should only use RH_RF95 if you do not need the higher
  level messaging abilities.

  It is designed to work with the other example LoRa Simple Server
  User need to use the modified RadioHead library from:
  https://github.com/dragino/RadioHead

  modified 16 11 2016
  by Edwin Chen <support@dragino.com>
  Dragino Technology Co., Limited

  Features:
  * unique sensor id
  * time stamp
  * send re-try up to 5 times until confirmation (that transmision) ok
  * voltage (Vs) and temperature (Ti) data transmission
  * watchdog 8s
*/

#define ID "device-1"  // should be unique

//#include <SPI.h>
#include <RH_RF95.h>

// https://bigdanzblog.wordpress.com/2014/10/24/arduino-watchdog-timer-wdt-example-code/
#include <avr/wdt.h>    // watchdog

// Singleton instance of the radio driver
RH_RF95 rf95;

#define frequency 868.0

float voltage = 0.0;
float temperature = 0.0;

void setup() 
{
  // - immediately disable watchdog timer so set will not get interrupted
  // - any 'slow' activity needs to be completed before enabling the watchdog timer.
  // - the following forces a pause before enabling WDT. This gives the IDE a chance to
  //   call the bootloader in case something dumb happens during development and the WDT
  //   resets the MCU too quickly. Once the code is solid, remove this.
  wdt_disable();

  Serial.begin(9600);
  //while (!Serial) ; // Wait for serial port to be available
  Serial.println("Start LoRa Client");
  if (!rf95.init())
    Serial.println("init failed");
  // Setup ISM frequency
  rf95.setFrequency(frequency);
  // Setup Power,dBm
  rf95.setTxPower(13);

  // Setup Spreading Factor (6 ~ 12)
  rf95.setSpreadingFactor(7);
  
  // Setup BandWidth, option: 7800,10400,15600,20800,31200,41700,62500,125000,250000,500000
  //Lower BandWidth for longer distance.
  rf95.setSignalBandwidth(125000);
  
  // Setup Coding Rate:5(4/5),6(4/6),7(4/7),8(4/8) 
  rf95.setCodingRate4(5);

  wdt_reset();
  wdt_enable(WDTO_8S);
}

void loop()
{
  wdt_reset();

  Serial.println("Sending to LoRa Server");
  // Send a message to LoRa Server
//  uint8_t data[] = "Hello, this is device 1";
//  rf95.send(data, sizeof(data));
  voltage = readVcc()/1000.;  // Vs
  temperature = GetTemp();    // Ti
  String data = String(ID) + String(" ") 
              + String(millis(), DEC) + String(" ")
              + String(voltage) + String(" ")
              + String(temperature);
  bool state = false;
  for(int i=0; (!state) && (i < 5); ++i)  // re-try up to 5 times until confirmation ok
  {
    rf95.send(data.c_str(), strlen(data.c_str()));
  
    wdt_reset();

    rf95.waitPacketSent();
    // Now wait for a reply
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.waitAvailableTimeout(3000))
    { 
      // Should be a reply message for us now   
      if (rf95.recv(buf, &len))
      {
        buf[len] = '\0';
        Serial.print("got reply: ");
        Serial.println((char*)buf);
        Serial.print("RSSI: ");
        Serial.println(rf95.lastRssi(), DEC);
        state = (strncmp(buf, data.c_str(), strlen(data.c_str())) == 0)
             && (strncmp(&(buf[len-2]), "OK", 2) == 0);
      }
    }
    else
    {
      Serial.println("No reply, is LoRa server running?");
    }
    Serial.print("Transmission confirmed: ");
    Serial.println(state);
    delay(100);
  }
//    else
  if (!state)
  {
    Serial.println("recv confirmation failed");
  }
  wdt_reset();
  delay(5000);
}


/********************************************************************************
    Accessing the secret voltmeter on the Arduino 168 or 328
    http://code.google.com/p/tinkerit/wiki/SecretVoltmeter
********************************************************************************/
long readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return result;
}

/********************************************************************************
    Internal Temperature Sensor for ATmega328 types
    https://playground.arduino.cc/Main/InternalTemperatureSensor
********************************************************************************/
double GetTemp(void)
{
  unsigned int wADC;
  double t;

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
