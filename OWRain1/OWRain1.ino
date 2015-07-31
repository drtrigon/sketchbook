#include <OWSlave.h>
//#include <PinChangeInterrupt.h>
/********************************************************************************
    
    Program. . . . OWRain
    Author . . . . Ian Evans
    Written. . . . 27 May 2014.
    Description. . Act as One Wire Slave
                   Count interrupts and convert to mm * 100
                   Returns Rain mm * 100 (4 Bytes)
                           Not used      (4 Bytes)

       ATTiny85 Pin Layout
            ---_---
        PB5 |1   8| VCC
        PB3 |2   7| PB2 Counter
        PB4 |3   6| PB1 One Wire
        GND |4   5| PB0 LED
            -------
            
********************************************************************************/
//    One Wire Slave Data
//                     {Fami, <---, ----, ----, ID--, ----, --->,  CRC} 
unsigned char rom[8] = {0xE1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00};
//                     {MM0,  MM1,  MM2,  MM3,  NIU0, NIU1, NIU2, NIU3, CRC}
  char scratchpad[9] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


//  Convertion to mm
#define countsPerMM 1;

//    Pin Layouts
#define LEDPin     0
#define OWPin      1
#define CounterPin 2

// Counter Variable Definition
volatile unsigned long counter    = 0;    

// LED Flash Variables
volatile long    flash       = 0;         // Flash achnowledgement counter
long             flashPause  = 100;       // LED between flash delay
#define          flashLength 50           // Flash length
long             flashStart  = 0;
long             flashStop   = 0;

// OW Interrupt Variables
volatile long prevInt    = 0;      // Previous Interrupt micros
volatile boolean owReset = false;

OWSlave ds(OWPin); 

/********************************************************************************
    Initiate the Environment
********************************************************************************/
void setup() {
    // Initialise the Pn usage
    pinMode(LEDPin, OUTPUT); 
    pinMode(CounterPin, INPUT);
    pinMode(OWPin, INPUT); 

    digitalWrite(LEDPin, LOW);    

//    attachPcInterrupt(CounterPin,incCounter,FALLING);
    attachInterrupt(CounterPin,incCounter,FALLING);
//    attachPcInterrupt(OWPin,onewireInterrupt,CHANGE);
    attachInterrupt(OWPin,onewireInterrupt,CHANGE);
    
    // Initialise the One Wire Slave Library
    ds.setRom(rom);

    flash +=2;
}

/********************************************************************************
    Loop checking for any actions to be taken
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
    detachInterrupt(OWPin);
    
    if (ds.presence()) {
        if (ds.recvAndProcessCmd()) {
            uint8_t cmd = ds.recv();
            if (cmd == 0x44) {
                float mm = counter;   // Convert the counter to mm
                mm /= countsPerMM;    
                unsigned long mm100 = mm * 100;      // return as mm * 100
                // Convert the long to 4 bytes
                scratchpad[0] = mm100 & 0xFF;
                scratchpad[1] = (mm100 >> 8) & 0xFF;
                scratchpad[2] = (mm100 >> 16) & 0xFF;
                scratchpad[3] = (mm100 >> 24) & 0xFF;
                scratchpad[8] = ds.crc8(scratchpad,8);    // Calculate the CRC
            }
            if (cmd == 0xBE) {
                for( int i = 0; i < 9; i++) {
                    ds.send(scratchpad[i]);
                }
                flash++;
            }
        }
    }
//    attachPcInterrupt(OWPin,onewireInterrupt,CHANGE);
    attachInterrupt(OWPin,onewireInterrupt,CHANGE);
}

//************************************************************
// Counter Interrupt handling 
//************************************************************
void incCounter(void) {
    counter++;
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

