#include <OneWireSlave.h>
/********************************************************************************
 TODO: NEED error handling, at least some bit that show state (ok/error) of every sensor/channel
 TODO: does NOT work with OW-ENV-THPL on same master
 TODO: does NOT work with LinkHubE
 [increase version number when solving one of these issues!!]
 
    Program. . . . OWGeneric_SensorStation
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

0x 00 00 00 00   [last value]  nop/pass

0x 01 00 00 00   -- float --   read MQ-135 (A0) and return float / Air Quality
0x 02 00 00 00   -- float --   read MQ-7 (A1) and return float / CO (!!! The heater uses an alternating voltage of 5V and 1.4V. !!! http://playground.arduino.cc/Main/MQGasSensors !!!)
0x 03 00 00 00   -- float --   read Sharp dust sensor and return float / Smoke and dust
0x 04 00 00 00   -- float --   read MQ-2 (A3) and return float / Flamable/Combustible gas
0x 05 00 00 00   -- float --   read SEN113104 and return float / Water/Rain and damp (Grove)
0x 06 00 00 00   -- float --   read SEN02281P and return float / Loudness (Grove)
0x 07 00 00 00   -- float --   read TCS3200D color temperature in K and return float
0x 08 00 00 00   -- float --   read TCS3200D (100% Blue) and return float
0x 09 00 00 00   -- float --   read TCS3200D (100% Clear/All) and return float
0x 0A 00 00 00   -- float --   read TCS3200D (100% Green) and return float
0x 0B 00 00 00   -- float --   read UV sensor and return float
0x 0C 00 00 00   -- float --   read MIC "sensor" and return float
0x 0D 00 00 00   -- float --   read TSL2561 and return float
0x 0E 00 00 00   -- float --   read TSL2561 (broadband) and return float
0x 0F 00 00 00   -- float --   read TSL2561 (IR) and return float
0x 10 00 00 00   -- float --   read LSM9DS0 (Accel x) and return float
0x 11 00 00 00   -- float --   read LSM9DS0 (Accel y) and return float
0x 12 00 00 00   -- float --   read LSM9DS0 (Accel z) and return float
0x 13 00 00 00   -- float --   read LSM9DS0 (Magn x) and return float
0x 14 00 00 00   -- float --   read LSM9DS0 (Magn y) and return float
0x 15 00 00 00   -- float --   read LSM9DS0 (Magn z) and return float
0x 16 00 00 00   -- float --   read LSM9DS0 (Gyro x) and return float
0x 17 00 00 00   -- float --   read LSM9DS0 (Gyro y) and return float
0x 18 00 00 00   -- float --   read LSM9DS0 (Gyro z) and return float
0x 19 00 00 00   -- float --   read LSM9DS0 (Temp) and return float
0x 1A 00 00 00   -- float --   read BMP183 and return float
0x 1B 00 00 00   -- float --   read BMP183 (Temp) and return float

0x FF 01 00 00   -- float --   get number of sensors available
0x FF 02 00 00   -- float --   get "RTC" counts: millis()/1000.
0x FF 03 00 00   -- float --   measure supply voltage Vcc

********************************************************************************/
//  One Wire Slave Data
//                          {Family, <---, ----, ----, ID--, ----, --->,  CRC}
unsigned char rom[8]      = {DS2415, 0xE2, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00};
//                          {     CTRL, MEM0, MEM1, MEM2, MEM3}   CTRL = U4 U3 U2 U1 OSC OSC 0 0
  char rtccountr_in[5]    = {     0x00, 0x00, 0x00, 0x00, 0x00};
  char rtccountr_out[5]   = {B11110000, 0x01, 0x23, 0xCD, 0xEF};
// "If the oscillator is intentionally stopped the 
// real time clock counter behaves as a 4-byte nonvolatile
// memory." -> OSC = 0 ALWAYS / U4:U1 memory

// In DEBUG mode 1wire interface is replaced by serial ouput
//#define DEBUG

#define SoftwareVer 1.0
#define SensorCount  27

//  Pin Layouts
#define LEDPin       13
#define OWPin         2
//  MeasPin, MUX, TCS, software SPI (define your own pins; use A2 instead of 13 as clk)
//#define MeasPin      A0    // Uno & Nano compatible
#define MeasPin      A7    // Nano only (current config)
#define MUX_S0        6
#define MUX_S1        5
#define MUX_S2        4
#define MUX_S3        3
//#define TCS_S0        9
//#define TCS_S1       10
#define TCS_S0S1     A1    // use A1 as digital pin (but pulseIn does not work)
#define TCS_S2        8
#define TCS_S3        9
#define TCS_OUT       7
#define DUST_LED     A3
//I2C (Wire) library occupies A4 (SDA), A5 (SCL)
//#define BMP183_CLK  13
#define BMP183_CLK   A2    // CLK (hardware SPI needs pin 13 here!)
#define BMP183_SDO   12    // AKA MISO
#define BMP183_SDI   11    // AKA MOSI
#define BMP183_CS    10    // chip-select pin (use any pin)

//  LED Flash Parameters
#define flashPause  100    // LED between flash delay
#define flashLength  50    // Flash length
//  Frequency measurements Parameters
#define timeout   10000
//  Dust IR LED Parameters
#define dustDPreM   280    // pre-measurement delay
#define dustDPostM   40    // post-measurement delay
#define dustDOff   9680    // in-between-measurement/off delay

#include <Adafruit_Sensor.h>

// based on Adafruit_TSL2561 sensor driver
#include <Wire.h>
#include <Adafruit_TSL2561_U.h>
// based on Adafruit_LSM9DS0 sensor driver
#include <Adafruit_LSM9DS0.h>

// based on Adafruit_BMP183 sensor driver
#include <SPI.h>
#include <Adafruit_BMP183.h>

Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
Adafruit_LSM9DS0 lsm = Adafruit_LSM9DS0(1000);  // Use I2C, ID #1000
//Adafruit_BMP183 bmp = Adafruit_BMP183(BMP183_CS);    // initialize with hardware SPI
Adafruit_BMP183 bmp = Adafruit_BMP183(BMP183_CLK, BMP183_SDO, BMP183_SDI, BMP183_CS);    // or initialize with software SPI and use any 4 pins

//  Commonly used variables
unsigned int mmin, mmax, val, mavg;
float R, B, G, X, Y, Z, x, y, n, CCT;
uint16_t broadband, infrared;
sensors_event_t event, accel, mag, gyro, temp;

// OneWire Slave class object
OneWireSlave ds(OWPin);

/********************************************************************************
    Initiate the Environment
********************************************************************************/
void setup() {
    // Initialise the Pin usage
    pinMode(LEDPin, OUTPUT);
    pinMode(OWPin, INPUT);

    pinMode(MeasPin, INPUT);    // enables analogRead() and digitalRead() (but NOT pulseIn!)

    pinMode(MUX_S0, OUTPUT);
    pinMode(MUX_S1, OUTPUT);
    pinMode(MUX_S2, OUTPUT);
    pinMode(MUX_S3, OUTPUT);

    pinMode(TCS_S0S1, OUTPUT);
    pinMode(TCS_S2,   OUTPUT);
    pinMode(TCS_S3,   OUTPUT);
    pinMode(TCS_OUT,  INPUT);

    pinMode(DUST_LED, OUTPUT);

    // Initialise the Pin output
    digitalWrite(LEDPin, LOW);

    // mux does not need to be initialized, channel gets selected later

    digitalWrite(TCS_S0S1, LOW);   // Output scaling/gain: Power down

    digitalWrite(DUST_LED, HIGH);  // IR LED: off

#ifdef DEBUG
    // Start serial port at 9600 bps and wait for port to open:
    Serial.begin(9600);
    Serial.println("Nano_OWSlave/OWGeneric_SensorStation:");
    Serial.println("(send any byte to trigger proccessing/conversion)");
#endif

    /* Initialise the TSL2561 sensor */
    if(!tsl.begin())
    {
#ifdef DEBUG
        /* There was a problem detecting the ADXL345 ... check your connections */
        Serial.print("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!");
        while(1);
#endif
    }
    /* Display some basic information on this sensor */
//    displaySensorDetailsTSL();
    /* Setup the sensor gain and integration time */
    /* You can also manually set the gain or enable auto-gain support */
    // tsl.setGain(TSL2561_GAIN_1X);      /* No gain ... use in bright light to avoid sensor saturation */
    // tsl.setGain(TSL2561_GAIN_16X);     /* 16x gain ... use in low light to boost sensitivity */
    tsl.enableAutoRange(true);            /* Auto-gain ... switches automatically between 1x and 16x */
    /* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
    tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
    // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
    // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */

    /* Initialise the LSM9DS0 sensor */
    if(!lsm.begin())
    {
#ifdef DEBUG
        /* There was a problem detecting the LSM9DS0 ... check your connections */
        Serial.print(F("Ooops, no LSM9DS0 detected ... Check your wiring or I2C ADDR!"));
        while(1);
#endif
    }
    /* Display some basic information on this sensor */
//    displaySensorDetailsLSM();
    /* Setup the sensor gain and integration time */
    // 1.) Set the accelerometer range
    lsm.setupAccel(lsm.LSM9DS0_ACCELRANGE_2G);
    //lsm.setupAccel(lsm.LSM9DS0_ACCELRANGE_4G);
    //lsm.setupAccel(lsm.LSM9DS0_ACCELRANGE_6G);
    //lsm.setupAccel(lsm.LSM9DS0_ACCELRANGE_8G);
    //lsm.setupAccel(lsm.LSM9DS0_ACCELRANGE_16G);
    // 2.) Set the magnetometer sensitivity
    lsm.setupMag(lsm.LSM9DS0_MAGGAIN_2GAUSS);
    //lsm.setupMag(lsm.LSM9DS0_MAGGAIN_4GAUSS);
    //lsm.setupMag(lsm.LSM9DS0_MAGGAIN_8GAUSS);
    //lsm.setupMag(lsm.LSM9DS0_MAGGAIN_12GAUSS);
    // 3.) Setup the gyroscope
    lsm.setupGyro(lsm.LSM9DS0_GYROSCALE_245DPS);
    //lsm.setupGyro(lsm.LSM9DS0_GYROSCALE_500DPS);
    //lsm.setupGyro(lsm.LSM9DS0_GYROSCALE_2000DPS);

    /* Initialise the BMP183 sensor */
    if(!bmp.begin())
    {
#ifdef DEBUG
        /* There was a problem detecting the BMP183 ... check your connections */
        Serial.print("Ooops, no BMP183 detected ... Check your wiring!");
        while(1);
#endif
    }

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
void loop(void) {

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
        case 0x01:                                 // "A0" (MQ-135)
            selectMUXch(5);
            pack(analogRead(MeasPin)/1023.);
            // unit ppm, calibration needed
            break;
        case 0x02:                                 // "A1" (MQ-7)
            selectMUXch(4);
            pack(analogRead(MeasPin)/1023.);
            // unit ppm, calibration needed
            break;
        case 0x04:                                 // "A3" (MQ-2)
            selectMUXch(2);
            pack(analogRead(MeasPin)/1023.);
            // unit ppm, calibration needed
            break;
        case 0x03:                                 // Particles (dust, smoke, etc.) sensor: Sharp GP2Y1010AU0F
            selectMUXch(3);
            mmin = 1023;
            mmax = 0;
            mavg = 0;
            for(int i=0; i<3; ++i) {    // init (30ms)
                measDust();
            }
            for(int i=0; i<10; ++i) {   // 10 * 10ms = 100ms
                val = measDust();       // read the dust value
                mmin  = min(mmin, val);
                mmax  = max(mmax, val);
                mavg += val;
            }
            // print out data [ug/m^3]
            pack(172. * (5.*((mavg/100.)/1023.)) - 99.9);  // average value
//            pack(172. * (5.*((mmax-mmin)/1023.)));         // fluctuation between pulses (smoke or house dust)
            break;
        case 0x05:                                 // "A4" (SEN113104; Water/Rain and damp (Grove))
            selectMUXch(1);
            pack(analogRead(MeasPin)/1023.);
            // arbitrary unit
            break;
        case 0x06:                                 // "A5" (SEN02281P; Loudness (Grove))
            selectMUXch(0);
            pack(analogRead(MeasPin)/1023.);
            // arbitrary unit
            break;
        case 0x07:                                 // CCT calculated from RGB from freq. (TCS3200D)
            // * no (calibrated) white balance
            // * values from 0-10000 (us) expected, this fits roughly with 16-bits for the algo.
            // * calibration for "TCS3414CS"
            // http://ams.com/chi/content/download/251586/993227/version/2
            /*digitalWrite(TCS_S2, HIGH);     // Photodiode type/color:
            digitalWrite(TCS_S3, LOW);      // Clear/All
            float white = measFreq();*/       // (white balance)
            digitalWrite(TCS_S2, LOW);      // Photodiode type/color:
            digitalWrite(TCS_S3, LOW);      // Red
            //float R = measFreq()/(white+1); // values from 0-1 (can e.g. by multiplied by 255)
            R = timeout/(measFreq()+1);     // values from 0-10000 (us) expected <16bits
            digitalWrite(TCS_S2, HIGH);     // Photodiode type/color:
            digitalWrite(TCS_S3, HIGH);     // Green
            //float G = measFreq()/(white+1); // values from 0-1 (can e.g. by multiplied by 255)
            G = timeout/(measFreq()+1);     // values from 0-10000 (us) expected <16bits
            digitalWrite(TCS_S2, LOW);      // Photodiode type/color:
            digitalWrite(TCS_S3, HIGH);     // Blue
            //float B = measFreq()/(white+1); // values from 0-1 (can e.g. by multiplied by 255)
            B = timeout/(measFreq()+1);     // values from 0-10000 (us) expected <16bits
            // (calibration matrix for "TCS3414CS")
            X = (-0.14282)*R + (1.54924)*G + (-0.95641)*B;
            Y = (-0.32466)*R + (1.57837)*G + (-0.73191)*B;  // Illuminance
            Z = (-0.68202)*R + (0.77073)*G + (0.56332)*B;
            // map to 2D chromaticity diagram
            x = X/(X+Y+Z);
            y = Y/(X+Y+Z);
            // map to CCT using McCamy’s formula (+/-2K in 2,856 to 6,500 K), corresponding to CIE illuminants
            n = (x - 0.3320) / (0.1858 - y);
            CCT = 449*n*n*n + 3525*n*n + 6823.3*n + 5520.33;  // CCT in K
            pack(CCT);
            break;
        case 0x08:                                 // freq. (TCS3200D)
            digitalWrite(TCS_S2, LOW);    // Photodiode type/color:
            digitalWrite(TCS_S3, HIGH);   // Blue
            pack(measFreq());
            // avg. unit time counts, calibration needed
            break;
        case 0x09:                                 // freq. (TCS3200D)
            digitalWrite(TCS_S2, HIGH);   // Photodiode type/color:
            digitalWrite(TCS_S3, LOW);    // Clear/All
            pack(measFreq());
            // avg. unit time counts, calibration needed
            break;
        case 0x0A:                                 // freq. (TCS3200D)
            digitalWrite(TCS_S2, HIGH);   // Photodiode type/color:
            digitalWrite(TCS_S3, HIGH);   // Green
            pack(measFreq());
            // avg. unit time counts, calibration needed
            break;
        case 0x0B:                                 // UV sensor: Adafruit GUVA-S12SD
            selectMUXch(15);
            /* Display the results [mW/cm^2] or [UV index] */
            pack( (analogRead(MeasPin) * 0.01) );  // mW/cm^2
            //pack( (analogRead(MeasPin) * 0.049) ); // UV index
            break;
        case 0x0C:                                 // MIC "sensor": Sparkfun ...
            selectMUXch(14);
            mmin = 1023;
            mmax = 0;
            //for(unsigned long i=0; i<10000; ++i) {
            for(unsigned long i=0; i<1000; ++i) {
                val = analogRead(MeasPin);
                mmin = min(mmin, val);
                mmax = max(mmax, val);
                //delayMicroseconds(1);
            }
            pack((mmax-mmin)/1023.);
            // unit amplitude in rel. voltage, calibration needed
            break;
        case 0x0D:                                 // Light sensor: Adafruit_TSL2561
            /* Get a new sensor event */
            tsl.getEvent(&event);
            /* Display the results [lux] */
            pack(event.light);  // 0 = "sensor overload"
            break;
        case 0x0E:                                 // Light sensor: Adafruit_TSL2561
            /* Get a new sensor event */
            broadband = 0;
            infrared = 0;
            /* Populate broadband and infrared with the latest values */
            tsl.getLuminosity (&broadband, &infrared);
            pack(broadband);
            break;
        case 0x0F:                                 // Light sensor: Adafruit_TSL2561
            /* Get a new sensor event */
            broadband = 0;
            infrared = 0;
            /* Populate broadband and infrared with the latest values */
            tsl.getLuminosity (&broadband, &infrared);
            pack(infrared);
            break;
        case 0x10:                                 // Accel/Magn/Gyro/Temp sensor: Adafruit_LSM9DS0
            /* Get a new sensor event */
            lsm.getEvent(&accel, &mag, &gyro, &temp);
            // print out accelleration data [m/s^2]
            pack(accel.acceleration.x);
            break;
        case 0x11:                                 // Accel/Magn/Gyro/Temp sensor: Adafruit_LSM9DS0
            /* Get a new sensor event */
            lsm.getEvent(&accel, &mag, &gyro, &temp);
            // print out accelleration data [m/s^2]
            pack(accel.acceleration.y);
            break;
        case 0x12:                                 // Accel/Magn/Gyro/Temp sensor: Adafruit_LSM9DS0
            /* Get a new sensor event */
            lsm.getEvent(&accel, &mag, &gyro, &temp);
            // print out accelleration data [m/s^2]
            pack(accel.acceleration.z);
            break;
        case 0x13:                                 // Accel/Magn/Gyro/Temp sensor: Adafruit_LSM9DS0
            /* Get a new sensor event */
            lsm.getEvent(&accel, &mag, &gyro, &temp);
            // print out magnetometer data [gauss]
            pack(mag.magnetic.x);
            break;
        case 0x14:                                 // Accel/Magn/Gyro/Temp sensor: Adafruit_LSM9DS0
            /* Get a new sensor event */
            lsm.getEvent(&accel, &mag, &gyro, &temp);
            // print out magnetometer data [gauss]
            pack(mag.magnetic.y);
            break;
        case 0x15:                                 // Accel/Magn/Gyro/Temp sensor: Adafruit_LSM9DS0
            /* Get a new sensor event */
            lsm.getEvent(&accel, &mag, &gyro, &temp);
            // print out magnetometer data [gauss]
            pack(mag.magnetic.z);
            break;
        case 0x16:                                 // Accel/Magn/Gyro/Temp sensor: Adafruit_LSM9DS0
            /* Get a new sensor event */
            lsm.getEvent(&accel, &mag, &gyro, &temp);
            // print out gyroscopic data [dps]
            pack(gyro.gyro.x);
            break;
        case 0x17:                                 // Accel/Magn/Gyro/Temp sensor: Adafruit_LSM9DS0
            /* Get a new sensor event */
            lsm.getEvent(&accel, &mag, &gyro, &temp);
            // print out gyroscopic data [dps]
            pack(gyro.gyro.y);
            break;
        case 0x18:                                 // Accel/Magn/Gyro/Temp sensor: Adafruit_LSM9DS0
            /* Get a new sensor event */
            lsm.getEvent(&accel, &mag, &gyro, &temp);
            // print out gyroscopic data [dps]
            pack(gyro.gyro.z);
            break;
        case 0x19:                                 // Accel/Magn/Gyro/Temp sensor: Adafruit_LSM9DS0
            /* Get a new sensor event */
            lsm.getEvent(&accel, &mag, &gyro, &temp);
            // print out temperature data [*C]
            pack(temp.temperature);
            break;
        case 0x1A:                                 // Barometric pressure sensor: Adafruit_BMP183
            /* Display atmospheric pressue in Hecto-Pascals [mbar] */
            pack(bmp.getPressure() / 100);
            break;
        case 0x1B:                                 // Barometric pressure sensor: Adafruit_BMP183
            /* First we get the current temperature from the BMP085 [*C] */
            pack(bmp.getTemperature());
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
    Select Multiplexer channel to connect to ADC
********************************************************************************/
void selectMUXch(char ch) {
    // S0 = bit[0]
    digitalWrite(MUX_S0, ((ch & B00000001) != 0));
    // S1 = bit[1]
    digitalWrite(MUX_S1, ((ch & B00000010) != 0));
    // S2 = bit[2]
    digitalWrite(MUX_S2, ((ch & B00000100) != 0));
    // S3 = bit[3]
    digitalWrite(MUX_S3, ((ch & B00001000) != 0));

    // wait for MUX/switch to settle
    //delay(1);  // according to datasheet can switch with 10E7 Hz, so 0.1us is enough
    delayMicroseconds(10);  // according to datasheet can switch with 10E7 Hz, so 0.1us is enough

/*#ifdef DEBUG
    Serial.print(ch, BIN);
#endif*/
}

/********************************************************************************
    Measure frequency by averaging pulseIn results
********************************************************************************/
float measFreq(void) {
    digitalWrite(TCS_S0S1, HIGH); // Output scaling/gain: 100%
    delayMicroseconds(1000);      // give some time to turn on
    //freq = 500000/pulseIn(TCS_OUT, LOW);
    //unsigned int val = pulseIn(TCS_OUT, LOW);
    mavg = 0;
    for(int i = 0; i < 10; i++) {
        val = pulseIn(TCS_OUT, LOW, timeout);
        if (val == 0) {           // timeout results in 0 (!)
            mavg += timeout;
        } else {
            mavg += val;
        }
    }
    digitalWrite(TCS_S0S1, LOW);  // Output scaling/gain: Power down
    return (mavg/10.);
}

/********************************************************************************
    Measure pulse detector response for Sharp GP2Y1010AU0F
    (sensor for particles like dust, smoke, etc.)
********************************************************************************/
unsigned long measDust(void) {
    //need to set selectMUXch(8); before calling this function
    digitalWrite(DUST_LED, LOW);    //  (ON) POWER ON the LED via pin 3 on the sensor
    delayMicroseconds(dustDPreM);   //  (ON)
    val = analogRead(MeasPin);      //  (ON) read the dust value via pin 5 on the sensor
    delayMicroseconds(dustDPostM);  //  (ON)
    digitalWrite(DUST_LED, HIGH);   // (OFF) POWER OFF the LED
    delayMicroseconds(dustDOff);    // (OFF)
    // ...or use http://playground.arduino.cc/Code/ElapsedMillis alternatively
    return val;
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

