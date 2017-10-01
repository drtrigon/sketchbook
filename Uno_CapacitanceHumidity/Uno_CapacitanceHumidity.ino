/*
  Analog input, analog output, serial output
  for measuring very small capacitances
  (if charge/discharge on MOhms takes microseconds ~ pF)

 Sets a fixed PWM (analog out) value and measures the average
 voltage generated (depending on capacitance only).
 Also prints the results to the serial monitor.

 This is for capacitances that are to small to be measured by:
 https://www.arduino.cc/en/Tutorial/CapacitanceMeter

 The circuit:
 * 9,1MOhm connected to digital pin 9 (PWM)
 * humidity capacitor connected between 9,1MOhm and ground (GND)
 * connection between resistor and capacitor goes to analog pin 0
 -> this is a PWM analog out config with digital pin 9 as PWM out
    and the analog out (between resistor and capacitor) goes back
    to analog pin 0 for measurement
    resistor is 9,1M, capacitor is humidity sensor (~150pF)

 Source:
 File > Examples > Analog > AnalogInOutSerial
 created 29 Dec. 2008
 modified 9 Apr 2012
 by Tom Igoe
 This example code is in the public domain.

 */

// These constants won't change.  They're used to give names
// to the pins used:
const int analogInPin = A0;  // Analog input pin that the potentiometer is attached to
const int analogOutPin = 9; // Analog output pin that the LED is attached to

long sensorValue = 0;        // value read from the pot
int outputValue = 0;        // value output to the PWM (analog out)

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);

  outputValue = 40;    // (the lower the value the more sensitive it seams)
  // change the analog out value:
  analogWrite(analogOutPin, outputValue);
}

void loop() {
  // read the analog in value:
  sensorValue = 0;
  for(int i=0; i<1000; ++i)
  {
      sensorValue += analogRead(analogInPin);
  }

  // print the results to the serial monitor:
  Serial.print("sensor = ");
  Serial.print(sensorValue/4400.);    // this is just a rough guess!!
                                      // may be this is not linear
                                      // investigation using scope needed
  Serial.print("\t output = ");
  Serial.println(outputValue);

  // wait 2 milliseconds before the next loop
  // for the analog-to-digital converter to settle
  // after the last reading:
  delay(2);
}
