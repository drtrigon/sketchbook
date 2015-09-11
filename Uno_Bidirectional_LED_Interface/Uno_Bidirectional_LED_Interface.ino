/********************************************************************************
  Very Low-Cost Sensing and Communication
  Using Bidirectional LEDs
  http://www.merl.com/publications/docs/TR2003-35.pdf

  2  The Bidirectional LED Interface
  Fig.5.  Emitting and sensing light with an LED

  1 red LED
  1 4k7Ohm
********************************************************************************/
 
// Pin 13 has an LED connected on most Arduino boards.
// give it a name:
int led  = 13;
int ledA =  3;
int ledB = 11;

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);

  // Start serial port at 9600 bps and wait for port to open:
  Serial.begin(9600);
  Serial.println("Uno_Bidirectional_LED_Interface:");
  Serial.println("(use keys; b=B, o, O, i=I)");

  // blink to signal; ready
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);               // wait for a second
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);               // wait for a second
}

// the loop routine runs over and over again forever:
void loop() {
    if (Serial.available() > 0) {
        // get incoming byte to empty the buffer
        int inByte = Serial.read();

        switch (inByte) {
            case 'b':
            case 'B':
                digitalWrite(led, !digitalRead(led));   // toggle the LED
                break;
            case 'o':
                // led output
                //writeLED(LOW);   // led: OFF
                writeLED(0);     // led: OFF
                break;
            case 'O':
                // led output
                //writeLED(HIGH);  // led: "emitting" ON
                writeLED(255);   // led: "emitting" ON
                break;
            case 'i':
            case 'I':
                // led input
                float val = 25000./readLED();
//                writeLED(10*val);
                writeLED(20*val + 10);
                Serial.println(val);
                break;
       }
    }
}

void writeLED(int state) {
    // led output
    pinMode(ledA, OUTPUT);
    pinMode(ledB, OUTPUT);
    digitalWrite(ledA, LOW);   // led:
    //digitalWrite(ledB, state); // LOW = OFF / HIGH = "emitting" ON
    analogWrite(ledB, state);  // PWM brightness control
}

unsigned long readLED(void) {
    // led input
    pinMode(ledA, OUTPUT);
    pinMode(ledB, OUTPUT);
    digitalWrite(ledA, HIGH);  // led:
    digitalWrite(ledB, LOW);   // "reverse bias" ON
    delay(100);
    pinMode(ledA, INPUT);      // led:
    digitalWrite(ledA, LOW);   // "discharge" to measure
    unsigned long i = 0;
    while(digitalRead(ledA)) {
        ++i;
    }
    return i;
}

