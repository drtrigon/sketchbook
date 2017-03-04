#include <TimerOne.h>

// include Yun_Log_BatteryDisCharging functionallity:
// * voltage measurement
// * current measurement
// * web api interface
// * OLED display (I2C)
// * need to put the charge pump code externally to an
//   ATtiny because the fast update rate "Timer1.initialize(50);"
//   makes it inpossible to do more sophisticated stuff on the same
//   processor - use I2C interface and connect the PWR pin to 5V

// Pin 13 has an LED connected on most Arduino boards.
#define LED   13  // LED Pin

/*
 * Dickson Charge Pump Driver
 */

// Yun change port from D to B
// Uno: use port D; PORTD, DDRD; P1=D2=PD2=0x04, P2=D3=PD3=0x08, PWR=D4=PD4=0x10
// Yun: use port B; PORTB, DDRB; P1=D8=PB4=0x10, P2=D9=PB5=0x20, PWR=D10=PB6=0x40
#define P1     0x10  // Pin D8 ON
#define P2     0x20  // Pin D9 ON
#define PWR    0x40  // Pin D10 ON
//#define OFF 0x20  // Pin D5 ON
//#define REF 404
// Uno measured: (Yun similar)
// 100 =  2.93V
// 150 =  4.73V  --- (130) begin linear range
// 200 =  5.93V  ---
// 300 =  8.17V  ---
// 400 = 10.43V  ---
// 500 = 12.71V  ---
// 600 = 14.86V  --- (610) end linear range
// 700 = 15.17V
// 800 = 15.17V
volatile unsigned long REF = 0;

char phase = 0;
////char onOff = 0;
//char onOff = 1;

void ticker () {
////  if (onOff) {
    int volts = analogRead(A0);  // Fast Analog Read (see 'setup()')
    if (volts <= REF) {
      if (phase) {
        PORTB = P1 | PWR;
////        PORTD = P1;
      } else {
        PORTB = P2 | PWR;
      }
      phase ^= 1;
    }
////  } else {
////    DDRD = OFF;
////    PORTD = OFF;
////  }
}

void setup() {
//  DDRB = P1 | P2 | PWR | OFF;
  DDRB = P1 | P2 | PWR;

  // initialize the digital pin as an output.
  pinMode(LED, OUTPUT);

  // http://forum.arduino.cc/index.php?topic=6549.0 (Faster Analog Read?)
// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif
  // set prescale to 16
  sbi(ADCSRA,ADPS2) ;
  cbi(ADCSRA,ADPS1) ;
  cbi(ADCSRA,ADPS0) ;

//  analogReference(DEFAULT);
//  Timer1.initialize(250);
  Timer1.initialize(50);  // Fast Analog Read (see 'setup()')
  Timer1.attachInterrupt(ticker);


  // start serial interface and wait for port to open
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.println("./sketch_oct02a:");
  Serial.println("(send number in 0...1023 to set voltage)");
}

void loop() {
  if (Serial.available() > 0) {
    // get incoming byte to empty the buffer
//    int inByte = Serial.read();
    int inByte = Serial.parseInt();
//    Serial.println(inByte);

    // set charge pump output voltage
    if((0 <= inByte) && (inByte <= 1023)) {
      REF = inByte;
      Serial.print(REF);
    } else {
      Serial.print("(invalid)");
    }
    Serial.print(" ");
    Serial.print(0.0225233*REF + 1.40082);  // linearization good in 130...610
    Serial.print("V\n");

    digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(1000);               // wait for a second
    digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW
//    delay(1000);               // wait for a second
  }
}
