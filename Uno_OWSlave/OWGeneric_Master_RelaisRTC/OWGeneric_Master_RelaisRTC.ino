#include <OneWireSlave.h>
#include <OneWire.h>
/********************************************************************************
    
    Program. . . . OWGeneric_Master_RelaisRTC
                   derived from Uno_OWSlave/OWGeneric & Uno_OWMaster/OWReadRTC
    Author . . . . Ursin Solèr
    Written. . . . 1 Aug 2015.
    Description. . Act as One Wire Slave (emulates RTC DS2415/DS1904 RTC iButton)
                   Read DS1904 RTC iButton as Master and acts as Slave to
                   return these data - allows to debug the communication (between OWFS/ENET and iButton)
                       Returns Value (5 Bytes) from iButton (reads, buffers and relais the value)
                       1 Device Control Byte and 4 Data Bytes

            AT.... Pin Layout (Arduino Yun/Leonardo NOT Uno - see interrupt below)
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
               ?? |?   ?| PD04       One Wire OUT/Device (acts as master here)
                  .     .
 Sens ? B_S??  A3 |?   ?| PD02       One Wire IN/Master (acts as slave here)
 Sens ? B_S??  A4 |?   ?| PD01      
 Sens ? B_S??  A5 |?   ?| PD00      
                  -------

Most simple read/write device with 4 byte (32 bit) memory: RTC DS2415
http://owfs.org/uploads/DS1904.html
http://pdfserv.maximintegrated.com/en/ds/DS2415.pdf
http://owfs.sourceforge.net/DS2415.3.html

********************************************************************************/
//    One Wire Slave Data
//                          {Fami, <---, ----, ----, ID--, ----, --->,  CRC} 
unsigned char rom[8]      = {0x24, 0xE3, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00};
                            // take rom from iButton ... ? is not an iButton, so better not.
//                          {     CTRL, MEM0, MEM1, MEM2, MEM3}   CTRL = U4 U3 U2 U1 OSC OSC 0 0
   char rtccountr[5]      = {B00001100, 0x00, 0x00, 0x00, 0x00};
// If the oscillator is intentionally stopped the 
// real time clock counter behaves as a 4-byte nonvolatile
// memory. -> OSC = 0 ALWAYS / U4:U1 memory
// !!! ISSUE HERE: CANNOT SET 'RUNNING=1' !!!


#define DEBUG   TRUE
//#define DEBUG  FALSE

//    Pin Layouts
#define LEDPin    13
#define OWPin      2
#define OWPinInt   1                      // Uno: 0, Leonardo/Yun: 1
                                          // https://www.arduino.cc/en/Reference/AttachInterrupt

// LED Flash Variables
volatile long    flash       = 0;         // Flash achnowledgement counter
long             flashPause  = 100;       // LED between flash delay
#define          flashLength 50           // Flash length
long             flashStart  = 0;
long             flashStop   = 0;

// OW Interrupt Variables
volatile long prevInt    = 0;      // Previous Interrupt micros
volatile boolean owReset = false;

// OW Master stuff
byte i;
byte present = 0;
byte data[12];
byte addr[8];

long synctime, debugtime;

OneWireSlave dss(OWPin); 
OneWire dsm(4);                    // on pin 4

/********************************************************************************
    Initiate the Environment
********************************************************************************/
void setup() {
/*    // Delay to open serial console
    if (DEBUG) {
        delay(5000);
    }*/

    // Initialise the Pn usage
    pinMode(LEDPin, OUTPUT); 
    pinMode(OWPin, INPUT); 

/*    // Debug serial output
    if (DEBUG) {
        Serial.begin(9600);
    }*/

    digitalWrite(LEDPin, LOW);    

//    attachPcInterrupt(OWPin,onewireInterrupt,CHANGE);
    attachInterrupt(OWPinInt,onewireInterrupt,CHANGE);

    // Initialise the One Wire Master Library and get device address
    dsm.reset_search();
    if ( !dsm.search(addr)) {
        if (DEBUG) {
            Serial.print("No more addresses.\n");
        }
//        dsm.reset_search();
//        return;
    }
    else {
        if (DEBUG) {
            Serial.print("R =");
            for( i = 0; i < 8; i++) {
                Serial.print(" ");
                Serial.print(addr[i], HEX);
            }
            Serial.print("\n");
        }
    }
    if ( OneWire::crc8( addr, 7) != addr[7]) {
        Serial.print("CRC is not valid!\n");
//        return;
    }
    if ( addr[0] == 0x24) {
        Serial.print("Device is a DS2415 family device.\n");
    }
    else {
        Serial.print("Device family is not recognized: 0x");
        Serial.println(addr[0],HEX);
//        return;
    }
    
    // Initialise the One Wire Slave Library
    dss.init(rom);
    dss.setPower(SOURCED);
    //dss.setDebug();
//    dss.setRTCCounter(rtccountr);
//    dss.attach99h(process);

    if (DEBUG) {
        present = dsm.reset();
        dsm.select(addr);    
        dsm.write(0x99);             // Write Clock, 5 Bytes have to follow
        //dsm.write(0x99, 1);          // Write Clock, 5 Bytes have to follow WITH parasite power
        //dsm.write(B00001100);
        for ( i = 0; i < 5; i++) {
            dsm.write(rtccountr[i]);
        }

        /*Serial.print(0x99, HEX);
        for ( i = 0; i < 5; i++) {
            Serial.print(" ");
            Serial.print(rtccountr[i],HEX);
        }
        Serial.print("\n");*/
    }

    flash +=2;
}

/********************************************************************************
    Repeatedly check for action to execute
********************************************************************************/
void loop() {
    
    if (flash > 0) FlashLED();    // Control the flashing of the LED

    if (owReset) owHandler();    // Handle the OW reset that was received

    sync();                      // ...

/*    if (DEBUG) {
        if (Serial.available() > 0) {
            // read the incoming byte:
            byte incomingByte = Serial.read();
//            if (incomingByte == 'r') {
//                synctime = 0;
//                debugtime = 0;
//                sync();
//            } else if (incomingByte == 'w') {
            if (incomingByte == 'w') {
                process();
            }
        }
    }*/
}

//************************************************************
// Process One Wire Handling
//************************************************************
void owHandler(void) {

    owReset=false;
//    detachPcInterrupt(OWPin);
    detachInterrupt(OWPinInt);

    if (dss.waitForRequest(false)) {
//    if (dss.waitForRequestInterrupt(false)) {
// dss.waitForRequest(false) does NOT return with TRUE for the first about 7 iterations ! (from 0x66, 0x99, else ...?)
//        flash++;   // ...when it return with true and enters here the flash following seems to lag it so much that it crashes...
//        Serial.println("!");
// at the moment the sensor listing loop gets struc (<10) with true here and then flash++ and the following call...
    }
// !!! dss.waitForRequest(false) RETURNS IN CASE OF ERROR OR SUCCESSFULLY PRECCESSED COMMAND ONLY !!!

//    attachPcInterrupt(OWPin,onewireInterrupt,CHANGE);
    pinMode(OWPin, INPUT);   // recover the output mode in case of error in dss.send (should work without; error test)
    attachInterrupt(OWPinInt,onewireInterrupt,CHANGE);
}

/********************************************************************************
    Process READ CLOCK 0x66
********************************************************************************/
void sync(){
    long now = millis();
    if (((now - synctime) < 500) && (0 <= (now - synctime))) {
        return;
    }
    synctime = now;

    present = dsm.reset();
    dsm.select(addr);    
    dsm.write(0x66);             // Read Clock
//    Serial.print("P=");
//    Serial.print(present,HEX);
//    Serial.print(" ");
    for ( i = 0; i < 5; i++) {   // we need 5 bytes
        rtccountr[i] = dsm.read();
    }

    dss.setRTCCounter(rtccountr);
    flash++;

    /*if (DEBUG) {
        if (((now - debugtime) < 1000) && (0 <= (now - debugtime))) {
            return;
        }
        debugtime = now;

        Serial.print(0x66, HEX);
        for ( i = 0; i < 5; i++) {
            Serial.print(" ");
            Serial.print(rtccountr[i], HEX);
        }
        Serial.print("\n");
//    Serial.print(" CRC=");
//    Serial.print( OneWire::crc8( data, 8), HEX);
//    Serial.println();
    }*/
}

/********************************************************************************
    Process WRITE CLOCK 0x99
********************************************************************************/
void process(){
    dss.getRTCCounter(rtccountr);

    present = dsm.reset();
    dsm.select(addr);    
    dsm.write(0x99);             // Write Clock, 5 Bytes have to follow
/*    for ( i = 0; i < 5; i++) {
        dsm.write(rtccountr[i]);
    }*/
    dsm.write(B00001100);
    dsm.write(0x00);
    dsm.write(0x00);
    dsm.write(0x00);
    dsm.write(0x00);

    flash++;
    flash++;

    /*if (DEBUG) {
        Serial.print(0x99, HEX);
        for ( i = 0; i < 5; i++) {
            Serial.print(" ");
            Serial.print(rtccountr[i], HEX);
        }
        Serial.print("\n");
    }*/
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

