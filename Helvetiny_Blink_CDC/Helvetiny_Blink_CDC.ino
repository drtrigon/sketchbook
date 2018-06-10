/*
* Helvetiny85 / Digispark Test including USB Serial Debug
* (ATtiny85 eqipped with Micronucleus USB bootloader)
*
* Tested using micronucleus-1.11.hex bootloader, which results
* in 98% memory usage. Using micronucleus-2.03_t85_default.hex
* or even micronucleus-2.03_t85_aggressive.hex would free some
* memory for the sketch (user code). Arduino IDE support for 2.03?
* https://playground.boxtec.ch/doku.php/arduino/attinyusb/micronucleus
*
* Links:
*   https://digistump.com/wiki/digispark/tutorials/digicdc
*
* Setup:
*   see Helvetiny_Blink
*
* ATtiny85 Pin
*            4    via resistor (? ohms) to LED that is connected to Pin 6
*            6    LED (connected via resistor to Pin 4)
*
*/
#define LED_BUILTIN       1  // this LED is NOT builtin (see above on how to add)
//#define TEMP_OFFSET  -272.9  // offset of internal temp on chip in kelvin (deg celcius)
//#define TEMP_COEFF    1.075  // scaling of internal temp on chip
#define TEMP_OFFSET  -255
#define TEMP_COEFF    1.0

#include <DigiCDC.h>

void setup()
{
  // initialize the digital pin as an output.
  SerialUSB.begin();
  pinMode(LED_BUILTIN,OUTPUT);
}

// the loop routine runs over and over again forever:
void loop()
{

  //turns led on and off based on sending 0 or 1 from serial terminal
  if (SerialUSB.available()) {
    char input = SerialUSB.read();
    if(input == '0')
      digitalWrite(LED_BUILTIN,LOW);
    else if(input == '1')
      digitalWrite(LED_BUILTIN,HIGH);
    else if(input == '2')
      SerialUSB.println(F("TEST!")); //wrap your strings in F() to save ram!
    else if(input == '3')
      SerialUSB.println(getVcc());
    else if(input == '4')
      SerialUSB.println(getTemp());

  }

  SerialUSB.delay(100);               // keep usb alive // can alos use SerialUSB.refresh();
}


// https://github.com/cano64/ArduinoSystemStatus/blob/master/SystemStatus.cpp
// http://21stdigitalhome.blogspot.ch/2014/10/trinket-attiny85-internal-temperature.html
// (https://www.mikrocontroller.net/topic/315667)
// (https://hackaday.com/2014/02/01/atmega-attiny-core-temperature-sensors/)
float getVcc()
{
  //reads internal 1V1 reference against VCC
  ADMUX = _BV(MUX3) | _BV(MUX2); // For ATtiny85/45
  //ADMUX = 0xF | _BV( REFS1 );         // ADC4 (Temp Sensor) and Ref voltage = 1.1V;
  delay(10); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA, ADSC));
  //uint8_t low = ADCL;
  //unsigned int val = (ADCH << 8) | low;
  unsigned int val = ADC;
  //discard previous result
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA, ADSC));
  //low = ADCL;
  //val = (ADCH << 8) | low;
  val = ADC;

  //return ((long)1024 * 1100) / val;
  return ((float)1024 * 1.1) / val;
}

float getTemp()
{
//  ADCSRA &= ~(_BV(ADATE) |_BV(ADIE)); // Clear auto trigger and interrupt enable
//  ADCSRA |= _BV(ADEN);                // Enable AD and start conversion
  //ADMUX = (1<<REFS0) | (1<<REFS1) | (1<<MUX3); //turn 1.1V reference and select ADC8
  ADMUX = 0xF | _BV( REFS1 );         // ADC4 (Temp Sensor) and Ref voltage = 1.1V;
  delay(10); //wait for internal reference to settle
  // start the conversion
  ADCSRA |= bit(ADSC);
  //sbi(ADCSRA, ADSC);
  // ADSC is cleared when the conversion finishes
  while (ADCSRA & bit(ADSC));
  //while (bit_is_set(ADCSRA, ADSC));
  //uint8_t low  = ADCL;
  //uint8_t high = ADCH;
  int a = ADC;
  //discard first reading
  ADCSRA |= bit(ADSC);
  while (ADCSRA & bit(ADSC));
  //low  = ADCL;
  //high = ADCH;
  //a = (high << 8) | low;
  a = ADC;
  //ADCSRA &= ~(_BV(ADEN));        // disable ADC
  // Temperature compensation using the chip voltage would go here
  //return a - 272; //return temperature in C
  return(((float)a + TEMP_OFFSET) / TEMP_COEFF);
}