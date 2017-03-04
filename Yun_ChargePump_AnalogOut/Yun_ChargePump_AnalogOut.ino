#include <toneAC.h>  // use PWMs for charge pump ;)
#include <PID_v1.h>

// * used to work up to ~15V - now with regular updates (once per sec)
//   it's only about 11V...
// * Yun get quite warm...

// https://sites.google.com/site/wayneholder/12-volt-charge-pump
// http://playground.arduino.cc/Code/ToneAC
// https://bitbucket.org/teckel12/arduino-toneac/wiki/Home
// http://playground.arduino.cc/Code/PIDLibrary
// http://playground.arduino.cc/Code/PIDAutotuneLibrary

// include Yun_Log_BatteryDisCharging functionallity:
// * improved voltage measurement
// * current measurement
// * web api interface
// * OLED display (I2C)

// Pin 13 has an LED connected on most Arduino boards.
#define LED   13  // LED Pin

/*
 * Dickson Charge Pump Driver
 */
// Yun change port from D to B
// Uno: use port D; PORTD, DDRD; P1=D2=PD2=0x04, P2=D3=PD3=0x08, PWR=D4=PD4=0x10
// Yun: use port B; PORTB, DDRB; P1=D8=PB4=0x10, P2=D9=PB5=0x20, PWR=D10=PB6=0x40
// toneAC: use Pin 8 for PWR and for charge pump signal use 2 PWMs
//         on Pins 9 & 10 - ATmega328, ATmega128, ATmega640, ATmega8, Uno, Leonardo, etc. 
//         D2 and D3 can be swapped, but when playing with volume there
//         are differences for the given pin assignments below a volume
//         between 9 and 10 is optimal (for swapped pins is it 10)
#define P1         10  // Pin D10 (was D2 but can also be D3)
#define P2          9  // Pin D9 (was D3 but can also be D2)
#define PWR         8  // Pin D8 (was D4)
#define PIN_INPUT  A0

volatile unsigned long REF = 0;
// calibration of ADC and voltage divider with DMM
// 5 times measured the frequency 2000 (Hz): 512, 497, 510, 497, 491
// -> average: 501.4 -> 501
// -> DMM reading: 11.72 V

//Define Variables we'll be connecting to
double Setpoint, Input, Output;
//Specify the links and initial tuning parameters
//double Kp=2, Ki=5, Kd=1;
//double Kp=0, Ki=750, Kd=0;
double Kp=0, Ki=2.5, Kd=0;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

void setup() {
  pinMode(PWR, OUTPUT);

  // initialize the digital pin as an output.
  pinMode(LED, OUTPUT);

  toneAC(0); // Turn off toneAC, can also use noToneAC().
  digitalWrite(PWR, LOW);

  analogReference(DEFAULT);

  //initialize the variables we're linked to
  Input = analogRead(PIN_INPUT);
  Setpoint = 0;
  //turn the PID on
  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(0, log(30000));

  // start serial interface and wait for port to open
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.println("./Yun_ChargePump_AnalogOut:");
  Serial.println("(send number in 0...15 to set voltage)");
}

void loop() {
  if (Serial.available() > 0) {
    // get incoming byte to empty the buffer
//    int inByte = Serial.read();
    int in = Serial.parseInt();
//    Serial.println(in);

    // set charge pump output voltage
//    if((0 <= in) && (in <= 30000)) {
    if((0 <= in) && (in <= 15)) {
      REF = in;
      Setpoint = in;
      Serial.print(REF);
    } else {
      Serial.print("(invalid)");
    }
    Serial.print(" ");
    /*Serial.print(0.0225233*REF + 1.40082);  // linearization good in 130...610
    Serial.print("V\n");*/

    if(REF == 0) {
      toneAC(0); // Turn off toneAC, can also use noToneAC().
      digitalWrite(PWR, LOW);
    } /*else {
      digitalWrite(PWR, HIGH);
      toneAC(REF); // Play the frequency 'in' (Herz)
      //toneAC(1000, in); // Play 1kHz with the volume 'in' (1/10)
    }*/

    digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(1000);               // wait for a second
    digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW
//    delay(1000);               // wait for a second

    int volt = analogRead(PIN_INPUT);
//    Serial.println(volt);
    Serial.print(11.72 * volt / 501);
    Serial.print("V\n");
  }
  else if(REF != 0) {
    Input = 11.72 * analogRead(PIN_INPUT) / 501;
    myPID.Compute();
    toneAC(exp(Output));
//    Serial.println(exp(Output));
    delay(1000);               // wait for a second
  }
}
