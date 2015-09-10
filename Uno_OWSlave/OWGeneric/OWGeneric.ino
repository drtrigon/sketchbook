#include <OneWireSlave.h>
/********************************************************************************
    
    Program. . . . OWGeneric
    Author . . . . Ursin Solèr (according to design by Ian Evans)
    Written. . . . 1 Aug 2015.
    Description. . Act as One Wire Slave (emulates RTC DS2415)
                   Read all the analog inputs (A0-A5) and return values on demand
                       Returns Value (5 Bytes) depending on value written last (5 Bytes)
                       1 Device Control Byte and 4 Data Bytes
    Hardware . . . Arduino Uno/Nano (w/ ATmega328)
    Hardware . . .
            AT.... Pin Layout (Arduino UNO)
                  ---_---
                  .     .
              RST |?   ?| PD13       LED
              3V3 |?   ?| PD12
 VCC    B_5V   5V |?   ?| PD11 
              GND |?   ?| PD10 
 Ground BGND  GND |?   ?| PD09 
              Vin |?   ?| PD08 
 Sens ? B_S??  A0 |?   ?| PD07 B_S?? Sens ??
                  .     .
 Sens ? B_S??  A3 |?   ?| PD02       One Wire
 Sens ? B_S??  A4 |?   ?| PD01      
 Sens ? B_S??  A5 |?   ?| PD00      
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
//    One Wire Slave Data
//                          {Family, <---, ----, ----, ID--, ----, --->,  CRC}
unsigned char rom[8]      = {DS2415, 0xE2, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00};
//                          {     CTRL, MEM0, MEM1, MEM2, MEM3}   CTRL = U4 U3 U2 U1 OSC OSC 0 0
  char rtccountr_in[5]    = {     0x00, 0x00, 0x00, 0x00, 0x00};
  char rtccountr_out[5]   = {B11110000, 0x01, 0x23, 0xCD, 0xEF};
// If the oscillator is intentionally stopped the 
// real time clock counter behaves as a 4-byte nonvolatile
// memory. -> OSC = 0 ALWAYS / U4:U1 memory

// In DEBUG mode 1wire interface is replaced by serial ouput
//#define DEBUG

#define SoftwareVer 1.0
#define SensorCount   3

//    Pin Layouts
#define LEDPin       13
#define OWPin         2

//  LED Flash Parameters
#define flashPause  100    // LED between flash delay
#define flashLength  50    // Flash length

// OneWire Slave class object
OneWireSlave ds(OWPin); 

/********************************************************************************
    Initiate the Environment
********************************************************************************/
void setup() {
    // Initialise the Pin usage
    pinMode(LEDPin, OUTPUT); 
    pinMode(OWPin, INPUT); 

    // Initialise the Pin output
    digitalWrite(LEDPin, LOW);    

#ifdef DEBUG
    // Start serial port at 9600 bps and wait for port to open:
    Serial.begin(9600);
    Serial.println("Uno_OWSlave/OWGeneric:");
    Serial.println("(send any byte to trigger proccessing/conversion)");
#endif
    
    // Initialise the One Wire Slave Library
    ds.init(rom);
    ds.setPower(SOURCED);
    //ds.setDebug();
    ds.setRTCCounter(rtccountr_out);
    ds.attach99h(process);

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
        rtccountr_in[1] = 0xFF;
        for(int i=0; i<4; ++i) {
            rtccountr_in[2] = i+1;
            int tic = millis();
            process();
            int toc = millis();
            debugPrint(toc-tic);
        }
        // test sensors
        for(int i=0; i<(SensorCount+1); ++i) {
            rtccountr_in[1] = i+1;
            int tic = millis();
            process();
            int toc = millis();
            debugPrint(toc-tic);
        }
    }
#endif
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
                /*unsigned int val = SensorCount;
                rtccountr_out[1] = rtccountr_in[1];
                rtccountr_out[2] = rtccountr_in[2];
                rtccountr_out[3] = (val >> 8) & 0xFF;
                rtccountr_out[4] = val & 0xFF;*/
                pack(SensorCount);
            } else if (rtccountr_in[2] == 0x02) {  // ISC: Software Version
                pack(SoftwareVer);
            } else if (rtccountr_in[2] == 0x03) {  // ISC: "RTC"
                pack(millis()/1000.);
            } else if (rtccountr_in[2] == 0x04) {  // ISC: Arduino Supply Voltage Level
                /* Measure and calculate supply voltage on Arduino 168 or 328 [V] */
                pack(readVcc()/1000.);
            }
            // no error handling here (yet)
            break;
        case 0x00:                                 // nop/pass
            break;
        case 0x01:                                 // A0
            pack(analogRead(A0)/1023.);
            // unit ppm, calibration needed
            break;
        case 0x02:                                 // A1
            pack(analogRead(A1)/1023.);
            // unit ppm, calibration needed
            break;
        case 0x03:                                 // A2
            pack(analogRead(A2)/1023.);
            // unit ppm, calibration needed
            break;
//        case 0x04:                                 // A3
//          ...
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
    Flash the LED
    [in a blocking manner]
********************************************************************************/
void FlashLED_blocking(void) {
//    digitalWrite(LEDPin, LOW);
//    delay(flashPause);
    digitalWrite(LEDPin, HIGH);
    delay(flashLength);
    digitalWrite(LEDPin, LOW);
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

