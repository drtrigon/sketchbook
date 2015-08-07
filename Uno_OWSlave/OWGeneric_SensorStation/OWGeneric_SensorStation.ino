//#include <OWSlave.h>
#include <OneWireSlave.h>
//#include <PinChangeInterrupt.h>
/********************************************************************************
    
    Program. . . . OWGeneric
    Author . . . . Ursin Solèr (according to design by Ian Evans)
    Written. . . . 1 Aug 2015.
    Description. . Act as One Wire Slave (emulates RTC DS2415)
                   Read all the analog inputs (A0-A5) and return values on demand
                       Returns Value (5 Bytes) depending on value written last (5 Bytes)
                       1 Device Control Byte and 4 Data Bytes

            AT.... Pin Layout (Arduino UNO)
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
 Sens 4 B_S04  A3 |?   ?| PD02       One Wire
 Sens 5 B_S05  A4 |?   ?| PD01      
 Sens 6 B_S06  A5 |?   ?| PD00      
                  -------

Most simple read/write device with 4 byte (32 bit) memory: RTC DS2415
http://pdfserv.maximintegrated.com/en/ds/DS2415.pdf
http://owfs.sourceforge.net/DS2415.3.html

********************************************************************************

(there is a status byte first - in both write and read)

   Write 0x99    Read  0x66      Function

0x 00 00 00 00   [last value]    nop/pass

0x 01 00 00 00   01 00 MSB LSB   read MQ-135 (A0) and return int / Air Quality
0x 02 00 00 00   02 00 MSB LSB   read MQ-7 (A1) and return int / CO (!!! The heater uses an alternating voltage of 5V and 1.4V. !!! http://playground.arduino.cc/Main/MQGasSensors !!!)
0x 03 00 00 00   03 00 MSB LSB   read MQ-5 (A2) and return int / Natural gas
0x 04 00 00 00   04 00 MSB LSB   read MQ-2 (A3) and return int / Flamable/Combustible gas
0x 05 00 00 00   05 00 MSB LSB   read MQ-3 (A4) and return int / Alcohol (Smoke)
0x 06 00 00 00   06 00 MSB LSB   read MQ-8 (A5) and return int / Hydrogen

0x 07 00 00 00   07 00 MSB LSB   read TCS3200D (100% Red) and return int        ! LOW VALUES TAKE MORE THAN 100ms TO CONVERT
0x 08 00 00 00   08 00 MSB LSB   read TCS3200D (100% Blue) and return int       !
0x 09 00 00 00   09 00 MSB LSB   read TCS3200D (100% Clear/All) and return int  !
0x 0A 00 00 00   0A 00 MSB LSB   read TCS3200D (100% Green) and return int      !

********************************************************************************/
//    One Wire Slave Data
//                          {Fami, <---, ----, ----, ID--, ----, --->,  CRC} 
unsigned char rom[8]      = {0x24, 0xE2, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00};
//                          {     CTRL, MEM0, MEM1, MEM2, MEM3}   CTRL = U4 U3 U2 U1 OSC OSC 0 0
  char rtccountr_in[5]    = {     0x00, 0x00, 0x00, 0x00, 0x00};
  char rtccountr_out[5]   = {B11110000, 0x01, 0x23, 0xCD, 0xEF};
// If the oscillator is intentionally stopped the 
// real time clock counter behaves as a 4-byte nonvolatile
// memory. -> OSC = 0 ALWAYS / U4:U1 memory


//    Pin Layouts
#define LEDPin    13
#define OWPin      2
#define OWPinInt   0
// A0-A5: MQ-135, ...
#define TCS_S0    11
#define TCS_S1    10
#define TCS_S2     9
#define TCS_S3     8
#define TCS_OUT    7

// LED Flash Variables
volatile long    flash       = 0;         // Flash achnowledgement counter
long             flashPause  = 100;       // LED between flash delay
#define          flashLength 50           // Flash length
long             flashStart  = 0;
long             flashStop   = 0;

// OW Interrupt Variables
volatile long prevInt    = 0;      // Previous Interrupt micros
volatile boolean owReset = false;

//OWSlave ds(OWPin); 
OneWireSlave ds(OWPin); 

/********************************************************************************
    Initiate the Environment
********************************************************************************/
void setup() {
    // Initialise the Pn usage
    pinMode(LEDPin, OUTPUT); 
    pinMode(OWPin, INPUT); 

    pinMode(TCS_S0, OUTPUT); 
    pinMode(TCS_S1, OUTPUT); 
    pinMode(TCS_S2, OUTPUT); 
    pinMode(TCS_S3, OUTPUT); 
    pinMode(TCS_OUT, INPUT); 

    digitalWrite(LEDPin, LOW);    

    digitalWrite(TCS_S0, LOW);   // Output scaling/gain:
    digitalWrite(TCS_S1, LOW);   // Power down

//    attachPcInterrupt(OWPin,onewireInterrupt,CHANGE);
    attachInterrupt(OWPinInt,onewireInterrupt,CHANGE);
    
    // Initialise the One Wire Slave Library
    ds.init(rom);
    ds.setPower(SOURCED);
    //ds.setDebug();
    ds.setRTCCounter(rtccountr_out);
//    ds.attach99h(test);
    ds.attach99h(process);

    // Debug serial output
//    Serial.begin(9600);

    flash +=2;
}

/********************************************************************************
    Repeatedly check for action to execute
********************************************************************************/
void loop() {
    
    if (flash > 0) FlashLED();    // Control the flashing of the LED

    if (owReset) owHandler();    // Handle the OW reset that was received
}

//************************************************************
// Process One Wire Handling
//************************************************************
void owHandler(void) {

    owReset=false;
//    detachPcInterrupt(OWPin);
    detachInterrupt(OWPinInt);

    if (ds.waitForRequest(false)) {
//    if (ds.waitForRequestInterrupt(false)) {
// ds.waitForRequest(false) does NOT return with TRUE for the first about 7 iterations ! (from 0x66, 0x99, else ...?)
//        flash++;   // ...when it return with true and enters here the flash following seems to lag it so much that it crashes...
//        Serial.println("!");
// at the moment the sensor listing loop gets struc (<10) with true here and then flash++ and the following call...
    }

//    attachPcInterrupt(OWPin,onewireInterrupt,CHANGE);
    pinMode(OWPin, INPUT);   // recover the output mode in case of error in ds.send (should work without; error test)
    attachInterrupt(OWPinInt,onewireInterrupt,CHANGE);
}

/********************************************************************************
    Process WRITE CLOCK 0x99 and prepare sensor readings
********************************************************************************/
void process(){
    ds.getRTCCounter(rtccountr_in);

    // process LSB only ... 256 are enough commands for now
    // (rtccountr_in[1] == 0x00)           // nop/pass
    if (rtccountr_in[1] == 0x01) {         // A0 (MQ-135)
        unsigned int val = analogRead(A0);
        // http://stackoverflow.com/questions/3784263/converting-an-int-into-a-4-byte-char-array-c
//        bytes[0] = (val >> 24) & 0xFF;
//        bytes[1] = (val >> 16) & 0xFF;
        rtccountr_out[1] = rtccountr_in[1];
        rtccountr_out[2] = 0x00;
        rtccountr_out[3] = (val >> 8) & 0xFF;
        rtccountr_out[4] = val & 0xFF;
    } else if (rtccountr_in[1] == 0x02) {  // A1 (MQ-7)
        unsigned int val = analogRead(A1);
        rtccountr_out[1] = rtccountr_in[1];
        rtccountr_out[2] = 0x00;
        rtccountr_out[3] = (val >> 8) & 0xFF;
        rtccountr_out[4] = val & 0xFF;
    } else if (rtccountr_in[1] == 0x03) {  // A2 (MQ-5)
        unsigned int val = analogRead(A2);
        rtccountr_out[1] = rtccountr_in[1];
        rtccountr_out[2] = 0x00;
        rtccountr_out[3] = (val >> 8) & 0xFF;
        rtccountr_out[4] = val & 0xFF;
    } else if (rtccountr_in[1] == 0x04) {  // A3 (MQ-2)
        unsigned int val = analogRead(A3);
        rtccountr_out[1] = rtccountr_in[1];
        rtccountr_out[2] = 0x00;
        rtccountr_out[3] = (val >> 8) & 0xFF;
        rtccountr_out[4] = val & 0xFF;
    } else if (rtccountr_in[1] == 0x05) {  // A4 (MQ-3)
        unsigned int val = analogRead(A4);
        rtccountr_out[1] = rtccountr_in[1];
        rtccountr_out[2] = 0x00;
        rtccountr_out[3] = (val >> 8) & 0xFF;
        rtccountr_out[4] = val & 0xFF;
    } else if (rtccountr_in[1] == 0x06) {  // A5 (MQ-8)
        unsigned int val = analogRead(A5);
        rtccountr_out[1] = rtccountr_in[1];
        rtccountr_out[2] = 0x00;
        rtccountr_out[3] = (val >> 8) & 0xFF;
        rtccountr_out[4] = val & 0xFF;
    } else if (rtccountr_in[1] == 0x07) {  // P7 freq. (TCS3200D)
        /*digitalWrite(TCS_S0, LOW);    // Output scaling/gain:
        digitalWrite(TCS_S1, HIGH);   // 2%
        digitalWrite(TCS_S0, HIGH);   // Output scaling/gain:
        digitalWrite(TCS_S1, LOW);    // 20%*/
        digitalWrite(TCS_S0, HIGH);   // Output scaling/gain:
        digitalWrite(TCS_S1, HIGH);   // 100%
        digitalWrite(TCS_S2, LOW);    // Photodiode type/color:
        digitalWrite(TCS_S3, LOW);    // Red
        //freq = 500000/pulseIn(TCS_OUT, LOW);
        //unsigned int val = pulseIn(TCS_OUT, LOW);
        unsigned int val = 0;
        for( int i = 0; i < 10; i++) {
            val += pulseIn(TCS_OUT, LOW);
        }
        val = val/10;
        rtccountr_out[1] = rtccountr_in[1];
        //rtccountr_out[1] = (val >> 24) & 0xFF;
        rtccountr_out[2] = (val >> 16) & 0xFF;
        rtccountr_out[3] = (val >> 8) & 0xFF;
        rtccountr_out[4] = val & 0xFF;
        digitalWrite(TCS_S0, LOW);    // Output scaling/gain:
        digitalWrite(TCS_S1, LOW);    // Power down
    } else if (rtccountr_in[1] == 0x08) {  // P7 freq. (TCS3200D)
        digitalWrite(TCS_S0, HIGH);   // Output scaling/gain:
        digitalWrite(TCS_S1, HIGH);   // 100%
        digitalWrite(TCS_S2, LOW);    // Photodiode type/color:
        digitalWrite(TCS_S3, HIGH);   // Blue
        unsigned int val = 0;
        for( int i = 0; i < 10; i++) {
            val += pulseIn(TCS_OUT, LOW);
        }
        val = val/10;
        rtccountr_out[1] = rtccountr_in[1];
        rtccountr_out[2] = (val >> 16) & 0xFF;
        rtccountr_out[3] = (val >> 8) & 0xFF;
        rtccountr_out[4] = val & 0xFF;
        digitalWrite(TCS_S0, LOW);    // Output scaling/gain:
        digitalWrite(TCS_S1, LOW);    // Power down
    } else if (rtccountr_in[1] == 0x09) {  // P7 freq. (TCS3200D)
        digitalWrite(TCS_S0, HIGH);   // Output scaling/gain:
        digitalWrite(TCS_S1, HIGH);   // 100%
        digitalWrite(TCS_S2, HIGH);   // Photodiode type/color:
        digitalWrite(TCS_S3, LOW);    // Clear/All
        unsigned int val = 0;
        for( int i = 0; i < 10; i++) {
            val += pulseIn(TCS_OUT, LOW);
        }
        val = val/10;
        rtccountr_out[1] = rtccountr_in[1];
        rtccountr_out[2] = (val >> 16) & 0xFF;
        rtccountr_out[3] = (val >> 8) & 0xFF;
        rtccountr_out[4] = val & 0xFF;
        digitalWrite(TCS_S0, LOW);    // Output scaling/gain:
        digitalWrite(TCS_S1, LOW);    // Power down
    } else if (rtccountr_in[1] == 0x0A) {  // P7 freq. (TCS3200D)
        digitalWrite(TCS_S0, HIGH);   // Output scaling/gain:
        digitalWrite(TCS_S1, HIGH);   // 100%
        digitalWrite(TCS_S2, HIGH);   // Photodiode type/color:
        digitalWrite(TCS_S3, HIGH);   // Green
        unsigned int val = 0;
        for( int i = 0; i < 10; i++) {
            val += pulseIn(TCS_OUT, LOW);
        }
        val = val/10;
        rtccountr_out[1] = rtccountr_in[1];
        rtccountr_out[2] = (val >> 16) & 0xFF;
        rtccountr_out[3] = (val >> 8) & 0xFF;
        rtccountr_out[4] = val & 0xFF;
        digitalWrite(TCS_S0, LOW);    // Output scaling/gain:
        digitalWrite(TCS_S1, LOW);    // Power down
    }

    //ds.setRTCCounter(rtccountr_in);
    ds.setRTCCounter(rtccountr_out);
    flash++;
}

//************************************************************
// Onw WireInterrupt handling 
//************************************************************
void onewireInterrupt(void) {
    volatile long lastMicros = micros() - prevInt;
    prevInt = micros();
    if (lastMicros >= 410 && lastMicros <= 550) { //  OneWire Reset Detected
        owReset=true;
    }
}


//************************************************************
// Flash the LED
//************************************************************
void FlashLED(void) {
    if (flashStart == 0 && flashStop == 0) {
        flashStart = millis();
    }
    else if (flashStart > 0 && (millis()-flashStart > flashPause)) {
        digitalWrite(LEDPin, HIGH);
        flashStart = 0;
        flashStop = millis() ;
    }
    else if (flashStop > 0 && (millis() - flashStop > flashLength)) {
        digitalWrite(LEDPin, LOW);
        flashStop = 0;
        flash--;
    }
}

