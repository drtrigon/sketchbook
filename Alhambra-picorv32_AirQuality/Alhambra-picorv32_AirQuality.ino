/*
  Fade

  This example shows how to fade an LED on pin 9 using the analogWrite()
  function.

  The analogWrite() function uses PWM, so if you want to change the pin you're
  using, be sure to use another PWM capable pin. On most Arduino, the PWM pins
  are identified with a "~" sign, like ~3, ~5, ~6, ~9, ~10 and ~11.

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/Fade
*/

int led = LED_BUILTIN;           // the PWM pin the LED is attached to
int brightness = 0;    // how bright the LED is
int fadeAmount = 5;    // how many points to fade the LED by

/*const byte ledBarSet[21] = {  // default
  0B00000000,  //  0%
  0B00000000,  //  5%
  0B00000000,  // 10%
  0B00000001,  // 15%
  0B00000011,  // 20%
  0B00000010,  // 25%
  0B00000110,  // 30%
  0B00000100,  // 35%
  0B00001100,  // 40%
  0B00001000,  // 45%
  0B00011000,  // 50%
  0B00010000,  // 55%
  0B00110000,  // 60%
  0B00100000,  // 65%
  0B01100000,  // 70%
  0B01000000,  // 75%
  0B11000000,  // 80%
  0B10000000,  // 85%
  0B11111111,  // 90%
  0B11111111,  // 95%
  0B11111111   //100%
};*/

const byte ledBarSet[21] = {  // sensitive on low values
  0B00000000,  //  0%
  0B00000001,  //  5%
  0B00000011,  // 10%
  0B00000010,  // 15%
  0B00000110,  // 20%
  0B00000100,  // 25%
  0B00001100,  // 30%
  0B00001000,  // 35%
  0B00011000,  // 40%
  0B00010000,  // 45%
  0B00110000,  // 50%
  0B00100000,  // 55%
  0B01100000,  // 60%
  0B01000000,  // 65%
  0B11000000,  // 70%
  0B10000000,  // 75%
  0B11111111,  // 80%
  0B11111111,  // 85%
  0B11111111,  // 90%
  0B11111111,  // 95%
  0B11111111   //100%
};

int sensorPin = A0;    // select the input pin for the potentiometer
int sensorValue = 0;  // variable to store the value coming from the sensor

// the setup routine runs once when you press reset:
void setup() {
  // declare pin 9 to be an output:
  pinMode(led, OUTPUT);

  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
}

// the loop routine runs over and over again forever:
void loop() {
/*  // set the brightness of pin 9:
  analogWrite(led, brightness);

  // change the brightness for next time through the loop:
  brightness = brightness + fadeAmount;

  // reverse the direction of the fading at the ends of the fade:
  if (brightness <= 0 || brightness >= 255) {
    fadeAmount = -fadeAmount;
  }
  // wait for 30 milliseconds to see the dimming effect
  delay(30);*/

  sensorValue = analogRead(sensorPin);
//  sensorValue = analogRead(sensorPin)/2+50;  // debug

  digitalWrite(led, ledBarSet[(sensorValue*100/1023)/5]);

  Serial.print(sensorValue);
  Serial.print(' ');
  Serial.println(sensorValue*100/1023);

  delay(30);

// TODO:
// - check ADC ref/max voltage (3.3V or 5V?) if 3.3V use "Spannungsteiler von 5V auf 3.3V"
// - check how are 2 power lanes (1.8A vs 3A) connected
// - use the 2 buttons
// - use fade mode to display error (or something else)
// (- can CPU be used with faster clock for speed-up?)
// (- can SPI display be used by using SPI master (block)?)
}
