/*
 Controlling a servo position using a potentiometer (variable resistor)
 by Michal Rinott <http://people.interaction-ivrea.it/m.rinott>

 modified on 8 Nov 2013
 by Scott Fitzgerald
 http://www.arduino.cc/en/Tutorial/Knob
*/

//#define DEBUG

#include <Servo.h>

Servo myservo;  // create servo object to control a servo

int potpin = 0;  // analog pin used to connect the potentiometer
int val;    // variable to read the value from the analog pin

void setup() {
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
  pinMode(LED_BUILTIN, OUTPUT);
#ifdef DEBUG
  Serial.begin(115200);
#endif
}

void loop() {
  val = analogRead(potpin);            // reads the value of the potentiometer (value between 0 and 1023)
  val *= 1.168;
  val = map(val, 0, 1023, 1000, 2000);     // scale it to use it with the servo (value between 0 and 180)
//  val = map(val, 0, 1023, 700, 2300);     // scale it to use it with the servo (value between 0 and 180)
  myservo.writeMicroseconds(val);                  // sets the servo position according to the scaled value
//  digitalWrite(LED_BUILTIN, (abs(val-1500) < 10));   // turn the LED on (HIGH is the voltage level)
  digitalWrite(LED_BUILTIN, ((val-1500) >= 0));   // turn the LED on (HIGH is the voltage level)
#ifdef DEBUG
  Serial.println(val);
#endif
  delay(15);                           // waits for the servo to get there
}

