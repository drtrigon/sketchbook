/********************************************************************************
    
    Program. . . . Uno_HP_C4280_Enabler
    Author . . . . Ursin Solèr (according to design by Ian Evans)
    Written. . . . 5 Sep 2015.
    Description. . Hack HP Photosmart C4280 to work in case of faulty ink cartridge
    Hardware . . . Arduino Uno/Nano (w/ ATmega328)
    Hardware . . . ATtiny84 (needs Arduino IDE 1.6.5)
                     IDE: http://highlowtech.org/?p=1695
                          Programming an ATtiny w/ Arduino 1.6 (or 1.0)
                          (20150809)
                     Platine: ATtiny
                     Prozessor: ATtiny84
                     Clock: 8MHz (internal)
                     Port: /dev/ttyACM0 (Arduino Uno)
                     Programmer: Arduino as ISP
                     (upload first: Datei > Beispiele > ArduinoISP)
    Schematic. . . Uno_HP-C4280_Enabler.fzz

********************************************************************************

HP Photosmart C4280 Frontpanel (front view):

Lowercase letters are the soldered connections.

Power:  A B     A = ???
         1      B = ??? (variable, mean around 1.2-2.2V)
        A B        

        C D     C = ???
         2      D = ???
        C D        
        E F     E = ???
         3      F = ???
        E F        
        G H     G = ???
         4      H = ???
        G H        
Cancel: I J     i = CANCEL_COMmon = blue
         5      j = signal = P11 = A0 = green
        i j       (MOS (FET) RELAIS simulates switch connecting i & j)

Blue:   k l     k = COMmon = yellow
         6      l = signal = P10 = orange
        K L       (MOS (FET) RELAIS connecting k & l)
Gray:   K n     K = COMmon
         7      n = signal = P09 = red
        K N       (MOS (FET) RELAIS connecting k & n)
Green:  K p     K = COMmon
         8      p = signal = P08 = brown
        K P       (MOS (FET) RELAIS connecting k & p)

MOS (FET) RELAIS inputs are connected to pin on positive side
and all together to same 1kOhm that goes to GND. So one button
should be used at a time only! (using multiple does no harm, but
is not supposed to work either)

********************************************************************************

Unblock Sequence is started by holding POWER & CANCEL down for > 3s.

This button press is so long that is does not need to be detected by
interrupt but can be done by simple polling in the main loop.

( may be needed to connect POWER & CANCEL to analog inputs in order
  to read their state porperly!? )

********************************************************************************/

// In ATTINY mode Serial interface is disabled
#define ATTINY

// Buttons controlled by I/O pins
//#define POWER       12
#define CANCEL      10
#define CANCEL_IN   A1
#define BLUE         9
#define GRAY         8
#define GREEN        7

#define pushDelay   30    // delay in 1/10th seconds (30*0.1 = 3s)
// we have to check/poll the line state every 1/100th
// second since the line idle state is oscillating

char inByte;

void setup() {
  // initialize the digital pin as an output.
//  pinMode(POWER, INPUT);
  pinMode(CANCEL,    OUTPUT);
  pinMode(CANCEL_IN, INPUT);
  pinMode(BLUE,      OUTPUT);
  pinMode(GRAY,      OUTPUT);
  pinMode(GREEN,     OUTPUT);

//  digitalWrite(POWER, LOW);
  digitalWrite(CANCEL, LOW);
  digitalWrite(  BLUE, LOW);
  digitalWrite(  GRAY, LOW);
  digitalWrite( GREEN, LOW);

  // start serial port at 9600 bps and wait for port to open:
#ifndef ATTINY
  Serial.begin(9600);
  Serial.println("HP C4280 Unblock Sequence Chip:");
#endif
}

// the loop routine runs over and over again forever:
void loop() {
  // detect button press (need to poll every 1/10th second - 100ms since line is oscillating)
  for(int i=0; i<pushDelay; ++i) {
#ifndef ATTINY
    delay(100);
#else
    delay(10);
#endif
//    if (!((digitalRead(POWER) == LOW) && (digitalRead(CANCEL) == LOW))) {
//    if (!(digitalRead(CANCEL) == LOW)) {
    if (digitalRead(CANCEL_IN) == LOW) {
//    if (analogRead(CANCEL_IN) < 500) {
      return;
    }
  }

  // do the "1xBLUE", "1xGREEN", "1xGRAY" sequence once ("blink the LED")
  // this results in a short screen flash, which you can recognize as "ready"
#ifndef ATTINY
  Serial.println("Ready to run sequence. Wait for button to release.");
  Serial.println("1xBLUE");
#endif
  pushButton(BLUE);
#ifndef ATTINY
  Serial.println("1xGREEN");
#endif
  pushButton(GREEN);
#ifndef ATTINY
  Serial.println("1xGRAY");
#endif
  pushButton(GRAY);

  // wait for button to release
//  while((digitalRead(POWER) == LOW) || (digitalRead(CANCEL) == LOW)) {
//  while(digitalRead(CANCEL) == LOW) {
  while(digitalRead(CANCEL_IN) == HIGH) {
#ifndef ATTINY
    delay(100);
#else
    delay(10);
#endif
  }
#ifndef ATTINY
  delay(1000);
#else
  delay(100);
#endif

  // run button press sequence
#ifndef ATTINY
  Serial.println("Sequence: ");
  Serial.println("1xBLUE");
#endif
  pushButton(BLUE);
#ifndef ATTINY
  Serial.println("1xGREEN");
#endif
  pushButton(GREEN);
#ifndef ATTINY
  Serial.println("1xGRAY");
#endif
  pushButton(GRAY);
#ifndef ATTINY
  Serial.println("");
  Serial.println("1xBLUE");
#endif
  pushButton(BLUE);
#ifndef ATTINY
  Serial.println("");
  Serial.println("1xGREEN");
#endif
  pushButton(GREEN);
#ifndef ATTINY
  Serial.println("");
//  Serial.println("11xBLUE");
  Serial.println("5xGRAY");
#endif
  pushButton(GRAY);
  pushButton(GRAY);
  pushButton(GRAY);
  pushButton(GRAY);
  pushButton(GRAY);
#ifndef ATTINY
  Serial.println("");
  Serial.println("1xGREEN");
#endif
  pushButton(GREEN);
#ifndef ATTINY
  Serial.println("");
  Serial.println("3xCANCEL");
#endif
  pushButton(CANCEL);
  pushButton(CANCEL);
  pushButton(CANCEL);
#ifndef ATTINY
  Serial.println("OK");
#endif
}

void pushButton(int pin) {
  // OUTPUT is pre-configured to LOW, INPUT is floating
  // by switching to OUTPUT we pull the line down and by
  // switching back we let it float again
  // protective resistor is a good idea to limit pin input
  // current (470-1k Ohm)
/*  pinMode(pin, OUTPUT);
  delay(100);
  pinMode(pin, INPUT);
  delay(500);*/
  digitalWrite(pin, HIGH);
#ifndef ATTINY
  delay(100);
#else
  delay(10);
#endif
  digitalWrite(pin, LOW);
#ifndef ATTINY
  delay(200);
#else
  delay(20);
#endif
}
