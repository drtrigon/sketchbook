/***************************************************************************
  MultiWii_HardwarePlatform_Test

  Hardware:
    - Arduino Nano (328)
    - Adafruit_LSM9DS0 (ACCeleration, MAGnetometer, GYROscope, Temperature)
        I2C address: 0x1D, 0x6B
    - Adafruit_BME280 (Temperature, PRESSure, Humidity)
        I2C address: 0x77

  Wiring using I2C:
    - Nano A4 to LSM9DS0 SDA and BME280 SDI
    - Nano A5 to LSM9DS0 SCL and BME280 SCK
    - 5V and GND

  See also:
    - Adafruit BME280 Breakout: http://www.adafruit.com/products/2650
    - MultiWii can be compiled from Arduino IDE: http://www.instructables.com/id/DIY-Custom-Multicopter-Flight-Controller/

  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

//#define ENABLE_LSM9DS0
//#define ENABLE_BME280
#define ENABLE_MultiWii

#if defined(ENABLE_LSM9DS0) || defined(ENABLE_BME280)
//I2C (Wire) library occupies A4 (SDA), A5 (SCL)
#include <Wire.h>

#include <Adafruit_Sensor.h>
#endif

#if defined(ENABLE_BME280)
// based on Adafruit_BME280 sensor driver, address: 0x77
#include <SPI.h>  // SPI not needed but referenced in BME280 library
#include <Adafruit_BME280.h>

Adafruit_BME280 bme; // I2C

//#define SEALEVELPRESSURE_HPA (1013.25)
#endif

#if defined(ENABLE_LSM9DS0)
// based on Adafruit_LSM9DS0 sensor driver, address: 0x1D, 0x6B
#include <Adafruit_LSM9DS0.h>

Adafruit_LSM9DS0 lsm = Adafruit_LSM9DS0(1000);  // Use I2C, ID #1000

sensors_event_t event, accel, mag, gyro, temp;
#endif

#if defined(ENABLE_MultiWii)
// Test MultiWii Sensors.cpp/.h implementations
#include "Arduino.h"
#include "Sensors.h"  // <-- add LSM9DS0 and BME280 functions to Sensors.cpp
#include "config.h"
#include "def.h"
#include "MultiWii.h"

// externals from MultiWii.cpp
int32_t baroPressure;
int16_t baroTemperature;
int16_t  i2c_errors_count = 0;
uint32_t currentTime = 0;
int32_t baroPressureSum;
imu_t imu;
global_conf_t global_conf;
conf_t conf;
uint16_t calibratingA = 0;
int16_t gyroZero[3] = {0,0,0};
uint16_t calibratingG;
flags_struct_t f;
uint8_t alarmArray[ALRM_FAC_SIZE];

// dummy as replacement for EEPROM.cpp/.h
void writeGlobalSet(uint8_t b) {}
#endif

void setup()
{
    // Start serial port at 9600 bps and wait for port to open:
    Serial.begin(9600);

#if defined(ENABLE_LSM9DS0)
    /* Initialise the LSM9DS0 sensor */
    if(!lsm.begin()) {
        /* There was a problem detecting the LSM9DS0 ... check your connections */
        Serial.println(F("Ooops, no LSM9DS0 detected ... Check your wiring or I2C ADDR!"));
        while(1);
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
#endif

#if defined(ENABLE_BME280)
    /* Initialise the BME280 sensor */
    if(!bme.begin()) {
        /* There was a problem detecting the BME280 ... check your connections */
        Serial.println(F("Could not find a valid BME280 sensor, check wiring!"));
        while(1);
    }
#endif

#if defined(ENABLE_MultiWii)
    // Test MultiWii Sensors.cpp/.h implementations
    initSensors();
#endif

    delay(100); // let sensor boot up
}

void loop()
{
    // put your main code here, to run repeatedly:

#if defined(ENABLE_LSM9DS0)
    // print out accelleration data [m/s^2]
    lsm.getEvent(&accel, &mag, &gyro, &temp);
    Serial.print(accel.acceleration.x);
    Serial.print(F(" "));

    Serial.print(accel.acceleration.y);
    Serial.print(F(" "));

    Serial.print(accel.acceleration.z);
    Serial.print(F("  "));

    // print out magnetometer data [gauss]
    Serial.print(mag.magnetic.x);
    Serial.print(F(" "));

    Serial.print(mag.magnetic.y);
    Serial.print(F(" "));

    Serial.print(mag.magnetic.z);
    Serial.print(F("  "));

    // print out gyroscopic data [dps]
    Serial.print(gyro.gyro.x);
    Serial.print(F(" "));

    Serial.print(gyro.gyro.y);
    Serial.print(F(" "));

    Serial.print(gyro.gyro.z);
    Serial.print(F("  "));

    // print out temperature data [*C]
    Serial.print(temp.temperature);
    Serial.print(F("  "));
#endif

#if defined(ENABLE_BME280)
    /* Display atmospheric pressue in Hecto-Pascals [mbar] */
    Serial.print(bme.readPressure() / 100.0F);
    //Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.print(F(" "));

    /* First we get the current temperature from the BMP085 [*C] */
    Serial.print(bme.readTemperature());
    Serial.print(F(" "));

    Serial.print(bme.readHumidity());
    Serial.print(F(" "));
#endif

#if defined(ENABLE_MultiWii)
    Serial.println();

    // Test MultiWii Sensors.cpp/.h implementations
    ACC_getADC();
    Serial.print(imu.accADC[ROLL]);
    Serial.print(F(" "));
    Serial.print(imu.accADC[PITCH]);
    Serial.print(F(" "));
    Serial.print(imu.accADC[YAW]);
    Serial.print(F("  "));

    Gyro_getADC();
    Serial.print(imu.gyroADC[ROLL]);
    Serial.print(F(" "));
    Serial.print(imu.gyroADC[PITCH]);
    Serial.print(F(" "));
    Serial.print(imu.gyroADC[YAW]);
    Serial.print(F("  "));

    currentTime = micros();
    Serial.print(Mag_getADC());
    Serial.print(F(" "));
    Serial.print(imu.magADC[ROLL]);
    Serial.print(F(" "));
    Serial.print(imu.magADC[PITCH]);
    Serial.print(F(" "));
    Serial.print(imu.magADC[YAW]);
    Serial.print(F("  "));

    Serial.print(Baro_update());
    Serial.print(F(" "));
    Serial.print(baroTemperature);
    Serial.print(F(" "));
    Serial.print(baroPressure);
    Serial.print(F(" "));
#endif

    Serial.println();

    delay(1000);
}

