/********************************************************************************

  OWsnoop 1.0 (interrupt based)

*********************************************************************************

  DOES NOT WORK ON LEONARDO; since attachInterrupt and Serial interfere such
                             that the virtual serial port does not get initialized
                             and thus does not appear on the (linux) PC at all!

********************************************************************************/

#define pinOW     2
#define pinOWint  0
#define pinLED   13

#define CODE_LEN 300           // > 445 gives issues with serial output! (why?!?)
volatile unsigned int durations[CODE_LEN];
volatile unsigned long last = 0;
volatile unsigned long indx = 0;
volatile unsigned long indx_max = 0;
unsigned int j;
volatile unsigned long tic = 0;
volatile unsigned long toc = 0;

byte inByte = 0;         // incoming serial byte

void setup(){
  pinMode(pinOW , INPUT);

  Serial.begin(115200);
  Serial.println("OWsnoop 1.0 (interrupt based)");

  cli();//stop interrupts

  //set timer1 at 2MHz (prescaler 8 @ 16MHz) - resolution of 0.5us where micros() has only 4us (max. 62.5ns)
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // Set CS00 bit for no prescaler / CS11 prescaler 8 (0.5us @ 16MHz)
//  TCCR1B |= (1 << CS00);  
  TCCR1B |= (1 << CS11);  

  sei();//allow interrupts

  //LOW to trigger the interrupt whenever the pin is low,
  //CHANGE to trigger the interrupt whenever the pin changes value
  //RISING to trigger when the pin goes from low to high,
  //FALLING for when the pin goes from high to low. 
//  attachInterrupt(pinOWint, capture, FALLING);
  attachInterrupt(pinOWint, capture, LOW);
}

void loop(){
  // serial output if line is HIGH for > 10000 us
  if(((PIND & B00000100) == B00000100) && (((TCNT1 - last)>>1) > 10000)){
    while (indx_max != indx) {
      if(durations[indx_max] < 0) {
//        Serial.print("ES");  // Error: Short
//      } else if((  1 <= durations[indx_max]) && (durations[indx_max] <= 10)) {  // WRITE 1 (Std.): 6us (too short for this method?! use FALLING and pusein)
      } else if((  4 <= durations[indx_max]) && (durations[indx_max] <= 25)) {  // WRITE 1 (Std.): 6us (too short for this method?! use FALLING and pusein)
        ++j;
//        Serial.print("W1");
        inByte = (inByte >> 1) | B10000000;
      } else if(( 26 <= durations[indx_max]) && (durations[indx_max] <= 80)) {  // WRITE 0 (Std.): 60us (min) - 60+10us (max)
        ++j;
//        Serial.print("W0");
        inByte = (inByte >> 1);
/*      } else if(( 15 <= durations[indx_max]) && (durations[indx_max] <= 30)) {  // READ 0 (Std.): 6+9us (min) - 6+9+55us (max)
        Serial.print("R0");
//      } else if(( 45 < durations[indx_max]) && (durations[indx_max] <= 55)) {  // READ 1 (Std.): 6us (too short for this method?! use FALLING and pusein)
//        Serial.print("R1");*/
      } else if(( 82 <= durations[indx_max]) && (durations[indx_max] <= 105)) {  // PRESENCE (Std.): ?
        Serial.print("P");
//        Serial.print(durations[indx_max]);
      } else if(480 <= durations[indx_max]) {                            // RESET (Std.): 480us (min)
        Serial.print("\n");
        Serial.print(j);
        Serial.print("R");
//        Serial.print(durations[indx_max]);
        j=0;
        inByte=0;
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
  }
}

void capture(void){
//  delayMicroseconds(3);
  tic = TCNT1;
/*  if ((PIND & B00000100) == B00000100) {
    return;
  }*/
  // http://www.instructables.com/id/Fast-digitalRead-digitalWrite-for-Arduino/?ALLSTEPS
  while ((PIND & B00000100) == 0)
  toc = TCNT1;

  last = toc;
  durations[indx] = (toc - tic)>>1;
  indx = (++indx % CODE_LEN);
}

