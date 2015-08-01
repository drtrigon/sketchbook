#include <OWSlave.h>
//#include <PinChangeInterrupt.h>
/********************************************************************************
    
    Program. . . . OWHumidity2
    Author . . . . Ian Evans
    Written. . . . 27 May 2014.
    Description. . Act as One Wire Slave
                   Read the sensor and return the Relative humidity * 100
                   Returns Humidity% * 100 (4 Bytes)
                           Sensor value    (2 bytes)
                           Supply value    (2 bytes)

            ATTiny85 Pin Layout
                 ---_---
        ADC0 PB5 |1   8| VCC
 Supply ADC3 PB3 |2   7| PB2 ADC1
 Sensor ADC2 PB4 |3   6| PB1      One Wire
             GND |4   5| PB0      LED
                 -------
            
********************************************************************************/
//    One Wire Slave Data
//                     {Fami, <---, ----, ----, ID--, ----, --->,  CRC} 
unsigned char rom[8] = {0xE2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00};
//                     {HUM0, HUM1, HUM2, HUM3, SEN0, SEN1, SUP0, SUP1,  CRC}
  char scratchpad[9] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


//  Convertion to mm
#define countsPerMM 1;

//    Pin Layouts
#define LEDPin    0
#define OWPin     1
#define supplyPin A3
#define sensorPin A2

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
    pinMode(OWPin, INPUT); 
    pinMode(supplyPin, INPUT); 
    pinMode(sensorPin, INPUT); 

    digitalWrite(LEDPin, LOW);    

//    attachPcInterrupt(OWPin,onewireInterrupt,CHANGE);
    attachInterrupt(OWPin,onewireInterrupt,CHANGE);
    
    // Initialise the One Wire Slave Library
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
    detachInterrupt(OWPin);
    
    if (ds.presence()) {
        if (ds.recvAndProcessCmd()) {
            uint8_t cmd = ds.recv();
            if (cmd == 0x44) {
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
        }
    }
//    attachPcInterrupt(OWPin,onewireInterrupt,CHANGE);
    attachInterrupt(OWPin,onewireInterrupt,CHANGE);
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

