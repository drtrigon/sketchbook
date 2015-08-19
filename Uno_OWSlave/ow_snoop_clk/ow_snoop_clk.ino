/********************************************************************************

  OWsnoop 2.0 (clock based)

  (this version is not necessarily better or more stable than the interrupt
   based one 1.x series)

*********************************************************************************

?  DOES NOT WORK ON LEONARDO; since attachInterrupt and Serial interfere such
?                             that the virtual serial port does not get initialized
?                             and thus does not appear on the (linux) PC at all!

********************************************************************************/

#define pinIR     2
#define pinLED   13

unsigned long m_cal, c_cal, m, c;
unsigned int i;

//storage variables
volatile boolean toggle1 = 0;

volatile unsigned int hi = 0;
volatile unsigned int lo = 0;
#define CODE_LEN 300           // > 445 gives strange issues (memory overflow)
volatile unsigned int durations[CODE_LEN];
volatile unsigned int indx = 0;
volatile unsigned int indx_max = 0;

byte j = 0;
byte inByte = 0;         // incoming serial byte


void setup(){

  pinMode(pinIR , INPUT);

  Serial.begin(115200);
  Serial.println("OWsnoop 2.0 (timer/clock based)");
  delay(1000);

  // http://www.instructables.com/id/Arduino-Timer-Interrupts/

  cli();//stop interrupts

  //set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1Mhz increments (16 increments @ 16 MHz = 1 MHz)
//  OCR1A = 16;// = 16 (must be <65536)
  // set compare match register for 200kHz increments (5*16 increments @ 16 MHz = 1/5 MHz = 200 kHz)
  OCR1A = 5*16;// = 16 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
//  // Set CS10 and CS12 bits for 1024 prescaler
//  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // Set CS00 bit for no prescaler (16 MHz)
  TCCR1B |= (1 << CS00);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  sei();//allow interrupts
  
}//end setup

ISR(TIMER1_COMPA_vect){//timer1 interrupt
  if ((PIND & B00000100) == 0) {
    ++lo;
    if (hi != 0) {
//      durations[indx] = hi;
      hi = 0;
//      indx = (++indx % CODE_LEN);
    }
  } else {
    ++hi;
    if (lo != 0) {
      durations[indx] = lo;
      lo = 0;
      indx = (++indx % CODE_LEN);
    }
  }
}

void loop(){
  if((hi > 10000) && (indx_max != indx)){
    TIMSK1 &= ~(1<<OCIE1A);   // disable compare match interrupt - to enable Serial
    Serial.print("\n");
    while (indx_max != indx) {
      if(durations[indx_max] == 0) {
        Serial.print("ES");  // Error: Short
//      } else if((  2 <= durations[indx_max]) && (durations[indx_max] <= 4)) {  // WRITE 1 (Std.): 6us (too short for this method?! use FALLING and pusein)
      } else if((  1 <= durations[indx_max]) && (durations[indx_max] <= 5)) {  // WRITE 1 (Std.): 6us (too short for this method?! use FALLING and pusein)
        ++j;
//        Serial.print("W1");
        inByte = (inByte >> 1) | B10000000;
//      } else if(( 14 <= durations[indx_max]) && (durations[indx_max] <= 16)) {  // WRITE 0 (Std.): 60us (min) - 60+10us (max)
      } else if(( 6 <= durations[indx_max]) && (durations[indx_max] <= 16)) {  // WRITE 0 (Std.): 60us (min) - 60+10us (max)
        ++j;
//        Serial.print("W0");
        inByte = (inByte >> 1);
      } else if(( 19 <= durations[indx_max]) && (durations[indx_max] <= 21)) {  // PRESENCE (Std.): ?
        Serial.print("P");
//        Serial.print(durations[indx_max]);
      } else if(96 <= durations[indx_max]) {                            // RESET (Std.): 480us (min)
        Serial.print("\n");
        Serial.print(j);
        Serial.print("R");
//        Serial.print(durations[indx_max]);
        j=0;
        inByte='.';
      } else {
        Serial.print(" ?");
        Serial.print(durations[indx_max]);
      }
      if(((j % 8) == 0) && (j != 0)) {
        Serial.print(" ");
        Serial.print(inByte, HEX);
      }
//      Serial.print(" ");

      indx_max = (++indx_max % CODE_LEN);
    }
    TIMSK1 |= (1 << OCIE1A);  // enable compare match interrupt - don't need Serial anymore
  }
}

