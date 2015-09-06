/********************************************************************************
    
    Program. . . . Uno_HP_C4280_Enabler
    Author . . . . Ursin Solèr (according to design by Ian Evans)
    Written. . . . 5 Sep 2015.
    Description. . Hack HP Photosmart C4280 to work in case of faulty ink cartridge
    Hardware . . . Arduino Uno/Nano

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

// Buttons controlled by I/O pins
//#define POWER       12
#define CANCEL      11
#define CANCEL_IN   A0
#define BLUE        10
#define GRAY         9
#define GREEN        8

#define LED         13

#define pushDelay   30    // delay in 1/10th seconds (30*0.1 = 3s)
// we have to check/poll the line state every 1/100th
// second since the line idle state is oscillating

char inByte;

void setup() {
  // initialize the digital pin as an output.
  pinMode(LED, OUTPUT);

  digitalWrite(LED, LOW);

//  pinMode(POWER, INPUT);
/*  pinMode(CANCEL, INPUT);
  pinMode(  BLUE, INPUT);
  pinMode(  GRAY, INPUT);
  pinMode( GREEN, INPUT);*/
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
  Serial.begin(9600);
  Serial.println("HP C4280 Unblock Sequence Chip:");
}

// the loop routine runs over and over again forever:
void loop() {
  // detect button press (need to poll every 1/10th second - 100ms since line is oscillating)
  for(int i=0; i<pushDelay; ++i) {
    delay(100);
//    if (!((digitalRead(POWER) == LOW) && (digitalRead(CANCEL) == LOW))) {
//    if (!(digitalRead(CANCEL) == LOW)) {
    if (digitalRead(CANCEL_IN) == LOW) {
//    if (analogRead(CANCEL_IN) < 500) {
      return;
    }
  }

  // blink LED to report successfully recognized button press
  Serial.println("Ready to run sequence. Wait for button to release.");
  digitalWrite(LED, HIGH);
  delay(50);
  digitalWrite(LED, LOW);
// TODO: consider doing the "1xBLUE", "1xGREEN", "1xGRAY" sequence here
//       (does it work with CANCEL hold down) to signal the recognized
//       sequence start...?!

  // wait for button to release
//  while((digitalRead(POWER) == LOW) || (digitalRead(CANCEL) == LOW)) {
//  while(digitalRead(CANCEL) == LOW) {
  while(digitalRead(CANCEL_IN) == HIGH) {
    delay(100);
  }
  delay(1000);

  // run button press sequence
  Serial.println("Sequence: ");
  Serial.println("1xBLUE");
  pushButton(BLUE);
  Serial.println("1xGREEN");
  pushButton(GREEN);
  Serial.println("1xGRAY");
  pushButton(GRAY);
  Serial.println("");
  Serial.println("1xBLUE");
  pushButton(BLUE);
  Serial.println("");
  Serial.println("1xGREEN");
  pushButton(GREEN);
  Serial.println("");
//  Serial.println("11xBLUE");
  Serial.println("5xGRAY");
  pushButton(GRAY);
  pushButton(GRAY);
  pushButton(GRAY);
  pushButton(GRAY);
  pushButton(GRAY);
  Serial.println("");
  Serial.println("1xGREEN");
  pushButton(GREEN);
  Serial.println("");
  Serial.println("3xCANCEL");
  pushButton(CANCEL);
  pushButton(CANCEL);
  pushButton(CANCEL);
  Serial.println("OK");
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
  delay(100);
  digitalWrite(pin, LOW);
  delay(200);
}
