#include <OneWireSlave.h>
/********************************************************************************

    Program. . . . OWGeneric_SensorStation
    Author . . . . Ursin Solèr (according to design by Ian Evans)
    Written. . . . 1 Aug 2015.
    Description. . Act as One Wire Slave (emulates RTC DS2415)
                   Read all the analog inputs (A0-A5) and return values on demand
                       Returns Value (5 Bytes) depending on value written last (5 Bytes)
                       1 Device Control Byte and 4 Data Bytes
                   Danger Shield Example Sketch, SparkFun Electronics, Chris Taylor
    Hardware . . . Arduino Uno/Nano (w/ ATmega328)
    Hardware . . .
            AT.... Pin Layout (Arduino UNO) & Danger Shield v1.7
                  ---_---
                  .     .
              RST |?   ?| PD13       LED
              3V3 |?   ?| PD12
 VCC    B_5V   5V |?   ?| PD11 B_DS0 TCS3200D S0 (Output scaling/gain)
              GND |?   ?| PD10 B_DS1 TCS3200D S1 (Output scaling/gain)
 Ground BGND  GND |?   ?| PD09 B_DS2 TCS3200D S2 (Photodiode type/color)
              Vin |?   ?| PD08 B_DS3 TCS3200D S3 (Photodiode type/color)
 Sens 1 B_S01  A0 |?   ?| PD07 B_S11 Sens 11 (TCS3200D)
                  .     .
 Sens 4 B_S04  A3 |?   ?| PD02       
 Sens 5 B_S05  A4 |?   ?| PD01      
OneWire        A5 |?   ?| PD00      
                  -------

Most simple read/write device with 4 byte (32 bit) memory: RTC DS2415
http://pdfserv.maximintegrated.com/en/ds/DS2415.pdf
http://owfs.sourceforge.net/DS2415.3.html

********************************************************************************

(there is a status byte first - in both write and read)

   Write 0x99    Read  0x66      Function

0x 00 00 00 00   [last value]    nop/pass

0x 01 00 00 00   -- float --   read ??? (A0) and return float / [description???]
0x 02 00 00 00   -- float --   read ??? (A1) and return float / [description???]
0x 03 00 00 00   -- float --   read ??? (A2) and return float / [description???]
//0x 04 00 00 00   -- float --   read ??? (A3) and return float / [description???]

0x FF 01 00 00   -- float --   get number of sensors available
0x FF 02 00 00   -- float --   get "RTC" counts: millis()/1000.
0x FF 03 00 00   -- float --   measure supply voltage Vcc

********************************************************************************/
//  One Wire Slave Data
//                          {Family, <---, ----, ----, ID--, ----, --->,  CRC}
unsigned char rom[8]      = {DS2415, 0xE2, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00};
//                          {     CTRL, MEM0, MEM1, MEM2, MEM3}   CTRL = U4 U3 U2 U1 OSC OSC 0 0
  char rtccountr_in[5]    = {     0x00, 0x00, 0x00, 0x00, 0x00};
  char rtccountr_out[5]   = {B11110000, 0x01, 0x23, 0xCD, 0xEF};
// "If the oscillator is intentionally stopped the 
// real time clock counter behaves as a 4-byte nonvolatile
// memory." -> OSC = 0 ALWAYS / U4:U1 memory

// Shift register bit values to display 0-9 on the seven-segment display: DP G F E D C B A
//  *-A-*
//  F   B           ledCharSet[0]...ledCharSet[9]:  numbers 0..9
//  *-G-*           ledCharSet[10]..ledCharSet[15]: letters A..F (distinct to numbers due to decimal point)
//  E   C                           ledCharSet[16]: all OFF
//  *-D-* DP
const byte ledCharSet[17] = {

  B00111111,B00000110,B01011011,B01001111,B01100110,B01101101,B01111101,B00000111,B01111111,B01101111, 
  B11110111,B11111111,B10111001,B10111111,B11111001,B11110001,
  B00000000
};

// In DEBUG mode 1wire interface is replaced by serial ouput
//#define DEBUG

#define SoftwareVer 1.0
#define SensorCount   9

//  Pin Layouts
#define LEDPin       13
#define OWPin        A5

//  LED Flash Parameters
#define flashPause  100    // LED between flash delay
#define flashLength  50    // Flash length

#include <CapacitiveSensor.h>

//CapSense   cs_9_2 = CapSense(9,2);   //Initializes CapSense pins
CapacitiveSensor   cs_9_2 = CapacitiveSensor(9,2);   //Initializes CapSense pins

// Pin definitions
#define SLIDER1   0
#define SLIDER2   1
#define SLIDER3   2
//#define KNOCK     5
#define BUTTON1  10
#define BUTTON2  11
#define BUTTON3  12
#define LED1      5
#define LED2      6
#define BUZZER    3
#define TEMP      4
#define LIGHT     3
#define LATCH     7
#define CLOCK     8
#define DATA      4

#include <TimerOne.h>
  
volatile unsigned long ctr1 = 0;
volatile unsigned long ctr2 = 0;
volatile unsigned long ctr3 = 0;
unsigned long d, t;

// OneWire Slave class object
OneWireSlave ds(OWPin); 

/********************************************************************************
    Initiate the Environment
********************************************************************************/
void setup() {
    // Initialise the Pin usage
//    pinMode(LEDPin, OUTPUT);
    pinMode(OWPin, INPUT);

    // Initialise the Pin output
//    digitalWrite(LEDPin, LOW);

#ifdef DEBUG
    // Start serial port at 9600 bps and wait for port to open:
    Serial.begin(9600);
    Serial.println("Uno_OWSlave/OWGeneric_DangerShield:");
    Serial.println("(send any byte to trigger proccessing/conversion)");
#endif

  cs_9_2.set_CS_AutocaL_Millis(0xFFFFFFFF); // Calibrates CapSense pin timing
  
  pinMode(BUTTON1,INPUT);
  pinMode(BUTTON2,INPUT);
  pinMode(BUTTON3,INPUT);
  
  digitalWrite(BUTTON1,HIGH);
  digitalWrite(BUTTON2,HIGH);
  digitalWrite(BUTTON3,HIGH);
  
  pinMode(BUZZER, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LATCH, OUTPUT);
  pinMode(CLOCK, OUTPUT);
  pinMode(DATA,OUTPUT);

  Timer1.initialize(100000); // set a timer of length 100000 microseconds (or 0.1 sec - or 10Hz => the led will blink 5 times, 5 cycles of on-and-off, per second)
  Timer1.attachInterrupt( timerIsr ); // attach the service routine here

    // Initialise the One Wire Slave Library
    ds.init(rom);
    ds.setPower(SOURCED);
    //ds.setDebug();
    ds.setRTCCounter(rtccountr_out);
    ds.attach99h(process);

    // Test all LEDs
    digitalWrite(LED1,HIGH);
    digitalWrite(LED2,HIGH);
    SetBCD(0x0B);
    delay(500);
    digitalWrite(LED1,LOW);
    digitalWrite(LED2,LOW);
    SetBCD(16);

    // Status output: init done, ready now.
    FlashLED_blocking();
    delay(flashPause);
    FlashLED_blocking();
}

/********************************************************************************
    Repeatedly process One Wire Handling
********************************************************************************/
void loop() {

    // dss.waitForRequest(false) returns in case of:
    //  * error - dss.waitForRequest(true) prevents this and keeps executing
    //  * successfully executed command (SKIP ROM) with TRUE
    //  * successfully executed SEARCH ROM, ALARM SEARCH or unkown command with FALSE
    //  * not successfully executed MATCH ROM (address not matching) with FALSE
    //  (SKIP ROM and MATCH ROM execute duty functions like 99h:process)
    //  (sucessfull MATCH ROM does not return and continues executing)
    //
    // TODO; in the case of THIS sketch it looks like:
    //  * at the moment the sensor listing loop gets struc (<10) with true here and then flashes
    //  * ds.waitForRequest(false) does NOT return with TRUE for the first about 7 iterations (from 0x66, 0x99, else ...?)
    //  * when it finally returns with TRUE (and enters the if condition) a 1wire communication error has occured
    //    (otherwise - no error occured - it does NEVER return!)
#ifndef DEBUG
    ds.waitForRequest(false);
#else
    if (Serial.available() > 0) {
        // get incoming byte to empty the buffer
        int inByte = Serial.read();

        // test ISC
        int tic, toc;
        rtccountr_in[1] = 0xFF;
        for(int i=0; i<4; ++i) {
            rtccountr_in[2] = i+1;
            tic = millis();
            process();
            toc = millis();
            debugPrint(toc-tic);
        }
        // test sensors
        //for(int i=0; i<(SensorCount+1); ++i) {
        for(int i=0; i<(SensorCount); ++i) {
            rtccountr_in[1] = i+1;
            tic = millis();
            process();
            toc = millis();
            debugPrint(toc-tic);
        }
        // Activates buzzer
        rtccountr_in[1] = 0x0A;
        rtccountr_in[3] = 50;    // "duration" (50*10=500 cycles/pulses)
        rtccountr_in[4] = 1;     // "frequency" [1..100]
        tic = millis();
        process();
        toc = millis();
        debugPrint(toc-tic);
        // Set LED state
        rtccountr_in[1] = 0x0B;
        rtccountr_in[3] = 26;
        rtccountr_in[4] = 255;
        tic = millis();
        process();
        toc = millis();
        debugPrint(toc-tic);
        // Set the seven-segment display
        rtccountr_in[1] = 0x0C;
        rtccountr_in[4] = 0x0F;
        tic = millis();
        process();
        toc = millis();
        debugPrint(toc-tic);
        // Test error handling
        rtccountr_in[1] = SensorCount+4;
        tic = millis();
        process();
        toc = millis();
        debugPrint(toc-tic);
    }
#endif
}

/// --------------------------
/// Custom ISR Timer Routine
/// (will reduce 1wire stability and performance - but by how much - usable for RTC relais?)
/// --------------------------
void timerIsr()
{
    ctr1 += int(!(digitalRead(BUTTON1)));
    ctr2 += int(!(digitalRead(BUTTON2)));
    ctr3 += int(!(digitalRead(BUTTON3)));
    // speed up and improve stability by using: https://www.arduino.cc/en/Reference/PortManipulation
}

/********************************************************************************
    Process WRITE CLOCK 0x99 and prepare sensor readings
********************************************************************************/
void process(){
//    digitalWrite(LEDPin, HIGH);

#ifndef DEBUG
    ds.getRTCCounter(rtccountr_in);
#endif

    // process LSB only ... 256 are enough commands for now (with ISC 256**2)
    // (otherwise unpack it to long)
    switch ((unsigned char)rtccountr_in[1]) {
        case 0xFF:                                 // Internal "System" Commands (ISC)
            if (rtccountr_in[2] == 0x01) {         // ISC: get number of sensors
                SetBCD(10);
                /*unsigned int val = SensorCount;
                rtccountr_out[1] = rtccountr_in[1];
                rtccountr_out[2] = rtccountr_in[2];
                rtccountr_out[3] = (val >> 8) & 0xFF;
                rtccountr_out[4] = val & 0xFF;*/
                pack(SensorCount);
            } else if (rtccountr_in[2] == 0x02) {  // ISC: Software Version
                SetBCD(11);
                pack(SoftwareVer);
            } else if (rtccountr_in[2] == 0x03) {  // ISC: "RTC"
                SetBCD(12);
                pack(millis()/1000.);
            } else if (rtccountr_in[2] == 0x04) {  // ISC: Arduino Supply Voltage Level
                SetBCD(13);
                /* Measure and calculate supply voltage on Arduino 168 or 328 [V] */
                pack(readVcc()/1000.);
            }
            // no error handling here (yet)
            break;
        case 0x00:                                 // nop/pass
            break;
        case 0x01:                                 // Read value of SLIDER1
            SetBCD(0);
            pack(analogRead(SLIDER1)/1023.);
            break;
        case 0x02:                                 // Read value of SLIDER2
            SetBCD(1);
            pack(analogRead(SLIDER2)/1023.);
            break;
        case 0x03:                                 // Read value of SLIDER3
            SetBCD(2);
            pack(analogRead(SLIDER3)/1023.);
            break;
        case 0x04:                                 // Tests/Reads CapSense pad
            SetBCD(3);
            pack(cs_9_2.capacitiveSensor(30));
            break;
        case 0x05:                                 // Reads temp sensor values
            SetBCD(4);
            pack(analogRead(TEMP)/1023.);
            break;
        case 0x06:                                 // Reads light sensor values
            SetBCD(5);
            pack(analogRead(LIGHT)/1023.);
            break;
        case 0x07:                                 // Reads button1 state counter
            SetBCD(6);
            pack(ctr1);
            break;
        case 0x08:                                 // Reads button2 state counter
            SetBCD(7);
            pack(ctr2);
            break;
        case 0x09:                                 // Reads button3 state counter
            SetBCD(8);
            pack(ctr3);
            break;
        case 0x0A:                                 // Activates buzzer
            SetBCD(16);
            d = 10*(unsigned char)rtccountr_in[3];
            t = 100*(unsigned char)rtccountr_in[4];
            for(int x = 0; x < d; x++)
            {
                digitalWrite(BUZZER, HIGH);
                delayMicroseconds(t);
                digitalWrite(BUZZER, LOW);
                delayMicroseconds(t);
            }
            break;
        case 0x0B:                                 // Set LED state
            SetBCD(16);
            //digitalWrite(LED1,rtccountr_in[3]);
            //digitalWrite(LED2,rtccountr_in[4]);
            // use PWM to dim LEDs
            analogWrite(LED1,(unsigned char)rtccountr_in[3]);
            analogWrite(LED2,(unsigned char)rtccountr_in[4]);
            break;
        case 0x0C:                                 // Set the seven-segment display
            //SetBCD(16);
            SetBCD(rtccountr_in[4]);
            break;
        default: // Unknown command (DO NOT USE FFFFFFFF since that is the same as a dead bus!)
            rtccountr_out[1] = 0xFE;
            rtccountr_out[2] = 0xFE;
            rtccountr_out[3] = 0xFE;
            rtccountr_out[4] = 0xFE;
            break;
    }

    //ds.setRTCCounter(rtccountr_in);
    ds.setRTCCounter(rtccountr_out);

//    digitalWrite(LEDPin, LOW);
//    ToggleLED();
    FlashLED_blocking();
}

/********************************************************************************
    Pack float into 4 byte representation (compatible with python)
********************************************************************************/
void pack(float val) {
    // http://stackoverflow.com/questions/3784263/converting-an-int-into-a-4-byte-char-array-c
//    bytes[0] = (val >> 24) & 0xFF;
//    bytes[1] = (val >> 16) & 0xFF;
    /*unsigned int val = 4242;
    rtccountr_out[1] = rtccountr_in[1];
    rtccountr_out[2] = 0x00;
    rtccountr_out[3] = (val >> 8) & 0xFF;
    rtccountr_out[4] = val & 0xFF;*/
    // http://forum.arduino.cc/index.php?topic=42466.0
    byte  *ArrayOfFourBytes;
    ArrayOfFourBytes = (byte*) & val;
    rtccountr_out[1] = ArrayOfFourBytes[0];
    rtccountr_out[2] = ArrayOfFourBytes[1];
    rtccountr_out[3] = ArrayOfFourBytes[2];
    rtccountr_out[4] = ArrayOfFourBytes[3];
    // http://stackoverflow.com/questions/3991478/building-a-32bit-float-out-of-its-4-composite-bytes-c
}

/********************************************************************************
    Set the seven-segment display (BCD) values
********************************************************************************/
void SetBCD(byte val) {
    digitalWrite(LATCH,LOW);
    shiftOut(DATA,CLOCK,MSBFIRST,~(ledCharSet[val]));
    digitalWrite(LATCH,HIGH);
}


/********************************************************************************
    Flash the LED
    [in a blocking manner]
********************************************************************************/
void FlashLED_blocking(void) {
//    digitalWrite(LEDPin, LOW);
//    delay(flashPause);
/*    digitalWrite(LEDPin, HIGH);
    delay(flashLength);
    digitalWrite(LEDPin, LOW);*/
}


/********************************************************************************
    Print debug info to serial output
********************************************************************************/
void debugPrint(int time) {
    Serial.print(byte(rtccountr_in[1]), HEX);
    Serial.print("  ");

    Serial.print(byte(rtccountr_out[1]), HEX);
    Serial.print(" ");
    Serial.print(byte(rtccountr_out[2]), HEX);
    Serial.print(" ");
    Serial.print(byte(rtccountr_out[3]), HEX);
    Serial.print(" ");
    Serial.print(byte(rtccountr_out[4]), HEX);
    Serial.print("  ");

    Serial.print(time);
    Serial.print("  ");

    float val = *(float*) (rtccountr_out+1);
    Serial.print(val);
    Serial.print("\n");
}


/********************************************************************************
    Accessing the secret voltmeter on the Arduino 168 or 328
    http://code.google.com/p/tinkerit/wiki/SecretVoltmeter
********************************************************************************/
long readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return result;
}

