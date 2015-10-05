#include <TimerOne.h>

// Pin 13 has an LED connected on most Arduino boards.
#define LED   13  // LED Pin

/*
 * Dickson Charge Pump Driver
 */
 
#define P1  0x04  // Pin D2 ON
#define P2  0x08  // Pin D3 ON
#define PWR 0x10  // Pin D4 ON
//#define OFF 0x20  // Pin D5 ON
#define OFF 0x00  // NO Pin ON
//#define REF 404
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
//char onOff = 0;
char onOff = 1;

void ticker () {
//  if (onOff) {
    DDRD = P1 | P2 | PWR | OFF;
    int volts = analogRead(A0);  // Fast Analog Read (see 'setup()')
    if (volts <= REF) {
      if (phase) {
        PORTD = P1 | PWR;
//        PORTD = P1;
      } else {
        PORTD = P2 | PWR;
      }
      phase ^= 1;
    }
//  } else {
//    DDRD = OFF;
//    PORTD = OFF;
//  }
}

void setup() {
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
//  Timer1.initialize(500);
  Timer1.initialize(100);
  Timer1.attachInterrupt(ticker);

  // start serial interface
  Serial.begin(9600);
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
