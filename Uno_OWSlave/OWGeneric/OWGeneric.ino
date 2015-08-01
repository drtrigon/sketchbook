#include <OWSlave.h>
//#include <OneWireSlave.h>
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
 VCC    B_5V   5V |?   ?| PD13       LED
 Ground BGND  GND |?   ?| PD12
                  .     .
 Sens 1 B_S01  A0 |?   ?| PD07
 Sens 2 B_S02  A1 |?   ?| PD06 
                  .     .
 Sens 4 B_S04  A3 |?   ?| PD02       One Wire
 Sens 5 B_S05  A4 |?   ?| PD01      
 Sens 6 B_S06  A5 |?   ?| PD00      
                  -------

Most simple read/write device with 4 byte (32 bit) memory: RTC DS2415
http://pdfserv.maximintegrated.com/en/ds/DS2415.pdf
http://owfs.sourceforge.net/DS2415.3.html

********************************************************************************

Alternative more sophisticated devices:
ADC -> 1-Wire Quad A/D Converter: http://datasheets.maximintegrated.com/en/ds/DS2450.pdf
                                  http://owfs.sourceforge.net/DS2450.3.html
GPIO -> 1-Wire 8-Channel Addressable Switch: http://datasheets.maximintegrated.com/en/ds/DS2408.pdf
                                             http://owfs.sourceforge.net/DS2408.3.html
(when emulating multiple devices, ALL presence pulses have to be sent
 at the same time/in parallel which is not easily possible with 1 Arduino)

Family codes:
http://owfs.sourceforge.net/commands.html

********************************************************************************/
//    One Wire Slave Data
//                          {Fami, <---, ----, ----, ID--, ----, --->,  CRC} 
//unsigned char rom2450a[8] = {0x20, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00};   // DS2450; A0-A3
//unsigned char rom2408a[8] = {0x29, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00};   // DS2408; D0-D7
//unsigned char rom2408b[8] = {0x29, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x00};   // DS2408; D8-D15
//unsigned char rom[8]      = {0xE2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00};
unsigned char rom[8]      = {0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00};
//         char rom[8]      = {0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00};
////                          {HUM0, HUM1, HUM2, HUM3, SEN0, SEN1, SUP0, SUP1,  CRC}
  char scratchpad[9]      = {0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
//                          {MEM0, MEM1, MEM2, MEM3}
  char rtccountr_in[4]    = {0x00, 0x00, 0x00, 0x00};
//  char rtccountr_out[4]   = {0x01, 0x00, 0x00, 0x04};
  char rtccountr_out[4]   = {0x00, 0x00, 0x00, 0x00};
//char ctrlbyte = B10000001;    // U4 U3 U2 U1 OSC OSC 0 0
char ctrlbyte = B00000000;    // U4 U3 U2 U1 OSC OSC 0 0
// If the oscillator is intentionally stopped the 
// real time clock counter behaves as a 4-byte nonvolatile
// memory. -> OSC = 0 ALWAYS / U4:U1 memory


//  Convertion to mm
#define countsPerMM 1;

//    Pin Layouts
#define LEDPin    13
#define OWPin      2
#define OWPinInt   0

// LED Flash Variables
volatile long    flash       = 0;         // Flash achnowledgement counter
long             flashPause  = 100;       // LED between flash delay
#define          flashLength 50           // Flash length
long             flashStart  = 0;
long             flashStop   = 0;

// OW Interrupt Variables
volatile long prevInt    = 0;      // Previous Interrupt micros
volatile boolean owReset = false;

//OWSlave ds2450a(OWPin); 
//OWSlave ds2408a(OWPin); 
//OWSlave ds2408b(OWPin); 
OWSlave ds(OWPin); 
//OneWireSlave ds(OWPin); 

/********************************************************************************
    Initiate the Environment
********************************************************************************/
void setup() {
    // Initialise the Pn usage
    pinMode(LEDPin, OUTPUT); 
    pinMode(OWPin, INPUT); 
    pinMode(supplyPin, INPUT); 
    pinMode(sensorPin, INPUT); 

    digitalWrite(LEDPin, LOW);    

//    attachPcInterrupt(OWPin,onewireInterrupt,CHANGE);
    attachInterrupt(OWPinInt,onewireInterrupt,CHANGE);
    
    // Initialise the One Wire Slave Library
//    ds2450a.setRom(rom2450a);
//    ds2408a.setRom(rom2408a);
//    ds2408b.setRom(rom2408b);
    ds.setRom(rom);

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
    
    if (ds.presence()) {
        if (ds.recvAndProcessCmd()) {
//    if (ds.waitForRequest(false)) {
            uint8_t cmd = ds.recv();
            //emuDS2450(cmd);
            //emuDS2408(cmd);
            //emuDS2408(cmd);
            emuDS2415(cmd);
        }
    }
//    attachPcInterrupt(OWPin,onewireInterrupt,CHANGE);
    pinMode(OWPin, INPUT);   // recover the output mode in case of error in ds.send (should work without; error test)
    attachInterrupt(OWPinInt,onewireInterrupt,CHANGE);
}

/********************************************************************************
    Emulate DS2450
    http://datasheets.maximintegrated.com/en/ds/DS2450.pdf

  TRANSACTION SEQUENCE 
  The protocol for accessing the DS2450 via the 1-Wire port is as follows: 
  * Initialization 
  * ROM Function Command 
  * Memory/Convert Function Command 
  * Transaction/Data 
  ...
********************************************************************************/
void emuDS2450(uint8_t cmd){
/*    if (cmd == 0x44) {
        float humidity = getHumidity();
        unsigned long Humidity100 = humidity * 100; // return as Humidity * 100
        // Convert the long to 4 bytes
        scratchpad[0] = Humidity100 & 0xFF;
        scratchpad[1] = (Humidity100 >> 8) & 0xFF;
        scratchpad[2] = (Humidity100 >> 16) & 0xFF;
        scratchpad[3] = (Humidity100 >> 24) & 0xFF;

        scratchpad[8] = ds.crc8(scratchpad,8);    // Calculate the CRC
    }
    if (cmd == 0xBE) {
        for( int i = 0; i < 9; i++) {
            ds.send(scratchpad[i]);
        }
        flash++;
    }
    if (cmd == 0xBE) {
        // ...
    }
    if (cmd == 0xAA) {   // Read Memory Command
        // ...
    }
    if (cmd == 0x55) {   // Write Memory Command
        // ...
    }
    if (cmd == 0x3C) {   // Convert Command
        // ...
    }*/
}

/********************************************************************************
    Emulate DS2408
    http://datasheets.maximintegrated.com/en/ds/DS2408.pdf

  TRANSACTION SEQUENCE
  The protocol for accessing the DS2408 through the 1-Wire port is as follows:
  * Initialization
  * ROM Function Command
  * Control Function Command
  * Transaction/Data
  Illustrations of the transaction sequence for the various control function commands are found later in this document. 
********************************************************************************/
void emuDS2408(uint8_t cmd){
/*    if (cmd == 0xF0) {   // Read PIO Registers
        // ...
    }
    if (cmd == 0xF5) {   // Channel-Access Read
        // ...
    }
    if (cmd == 0x5A) {   // Channel-Access Write
        // ...
    }
    if (cmd == 0xCC) {   // Write Conditional Search Register
        // ...
    }
    if (cmd == 0xC3) {   // Reset Activity Latches
        // ...
    }*/
}

/********************************************************************************
    Emulate DS2415
    http://pdfserv.maximintegrated.com/en/ds/DS2415.pdf

  Transaction Sequence
  The protocol for accessing the DS2415 via the 1-Wire port is as follows:
  * Initialization
  * ROM Function Command
  * Clock Function Command
  (no crc... ;)
********************************************************************************/
void emuDS2415(uint8_t cmd){
    if (cmd == 0x66) {          // READ CLOCK 
        // send device control byte
        delayMicroseconds(100);
        ds.send(ctrlbyte);
        // send 4 bytes of the RTC counter "4-byte nonvolatile memory"
        //ds.sendData(rtccountr_out, 4);
        for( int i = 0; i < 4; i++) {
            delayMicroseconds(75);
            ds.send(rtccountr_out[i]);
        }
        flash++;
    } else if (cmd == 0x99) {   // WRITE CLOCK
        // receive device control byte
        ctrlbyte = ds.recv();
        // receive 4 bytes of the RTC counter "4-byte nonvolatile memory"
        for( int i = 0; i < 4; i++) {
            rtccountr_in[i] = ds.recv();
//            rtccountr_out[i] = ds.recv();
        }
        flash++;
        // process device control byte
        //ctrlbyte = (ctrlbyte>>4)<<4;  // set most-right 4 bits to 0
        ctrlbyte = ctrlbyte & 0xF0;  // set most-right 4 bits to 0
        // process cmd given in rtccountr_in memory
        process();
    }
}

/********************************************************************************
    Process sensor readings (rtccountr_in and prepare rtccountr_out)
********************************************************************************/
void process(void){
    // process LSB only ... 256 are enough commands for now
    // (rtccountr_in[3] == 00)           // nop/pass
    if (rtccountr_in[3] == 01) {         // A0
        unsigned int val = analogRead(A0);
        // http://stackoverflow.com/questions/3784263/converting-an-int-into-a-4-byte-char-array-c
//        bytes[0] = (val >> 24) & 0xFF;
//        bytes[1] = (val >> 16) & 0xFF;
        rtccountr_out[0] = rtccountr_in[3];
        rtccountr_out[1] = 0x00;
        rtccountr_out[2] = (val >> 8) & 0xFF;
        rtccountr_out[3] = val & 0xFF;
    } else if (rtccountr_in[3] == 02) {  // A1
        unsigned int val = analogRead(A1);
        rtccountr_out[0] = rtccountr_in[3];
        rtccountr_out[1] = 0x00;
        rtccountr_out[2] = (val >> 8) & 0xFF;
        rtccountr_out[3] = val & 0xFF;
    } else if (rtccountr_in[3] == 03) {  // A2
        unsigned int val = analogRead(A2);
        rtccountr_out[0] = rtccountr_in[3];
        rtccountr_out[1] = 0x00;
        rtccountr_out[2] = (val >> 8) & 0xFF;
        rtccountr_out[3] = val & 0xFF;
    } else if (rtccountr_in[3] == 04) {  // A3
        unsigned int val = analogRead(A3);
        rtccountr_out[0] = rtccountr_in[3];
        rtccountr_out[1] = 0x00;
        rtccountr_out[2] = (val >> 8) & 0xFF;
        rtccountr_out[3] = val & 0xFF;
    } else if (rtccountr_in[3] == 05) {  // A4
        unsigned int val = analogRead(A4);
        rtccountr_out[0] = rtccountr_in[3];
        rtccountr_out[1] = 0x00;
        rtccountr_out[2] = (val >> 8) & 0xFF;
        rtccountr_out[3] = val & 0xFF;
    } else if (rtccountr_in[3] == 06) {  // A5
        unsigned int val = analogRead(A5);
        rtccountr_out[0] = rtccountr_in[3];
        rtccountr_out[1] = 0x00;
        rtccountr_out[2] = (val >> 8) & 0xFF;
        rtccountr_out[3] = val & 0xFF;
    }
}

/********************************************************************************
    Convert a voltage into % Humidity
********************************************************************************/
float getHumidity(void){

    unsigned int Sensor = analogRead(sensorPin);    // Humidity Sensor Channel
    unsigned int Supply = analogRead(supplyPin);    // Power Sensor Channel

    scratchpad[4] = Sensor & 0xFF;
    scratchpad[5] = (Sensor >> 8) & 0xFF;
    scratchpad[6] = Supply & 0xFF;
    scratchpad[7] = (Supply >> 8) & 0xFF;
    
//    float SensorV = float(Supply) / 1023.0 * 5.0;     // Convert the Power to a Voltage
//    float SupplyV = float(Sensor) / 1023.0 * 5.0;     // Convert the Sensor to a Voltage

    // convert the voltage to a relative humidity
    // - the equation is derived from the HIH-4030/31 datasheet
    // - it is not calibrated to your individual sensor
    //  Table 2 of the sheet shows the may deviate from this line

    // VOUT=(VSUPPLY)(0.0062(sensor RH) + 0.16), typical at 25 ºC
    // float sensorRH = voltage / supplyVolt / 0.0062 - 0.16;
    float sensorRH = 161.29 * Sensor / Supply - 25.81;

    // True RH = (Sensor RH)/(1.0546 – 0.00216T), T in ºC
    float degreesCelsius = 25.0;
    float trueRH = sensorRH / (1.0546 - 0.0026 * degreesCelsius); //temperature adjustment 

    // return trueRH;
    return sensorRH;
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

