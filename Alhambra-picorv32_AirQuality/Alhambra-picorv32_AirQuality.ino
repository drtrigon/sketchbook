/**
 * @brief Code for picorv32 in an Alhambra II for determing and displaying Air Quality
 *
 * @file Alhambra-picorv32_AirQuality/Alhambra-picorv32_AirQuality.ino
 *
 * @author drtrigon
 * @date 2020-10-18
 * @version 1.0
 *   @li connect AREF (AR) to 5V
 *   @li switch1 changes to display average (of last 100 values taken over 1000s)
 *   @li first version derived from Fade example
 *
 * @see https://github.com/drtrigon/fpgarduino-icestorm
 * @see https://github.com/FPGAwars/Alhambra-II-FPGA/tree/master/doc
 * @see https://www.codrey.com/electronic-circuits/how-to-use-mq-135-gas-sensor/
 *
 * @verbatim
 * Pinout:
 *   connect AREF (AR) to 5V
 *   MQ 135:
 *        5V            -> Alhambra 5V
 *       GND            -> Alhambra GND
 *     SENSE            -> Alhambra A0
 *
 * Tested with:
 *   - lighter gas
 *   - breath
 *   (no calibration available)
 *
 * Notes (future todo):
 * - is there an adc block that works with 2 channels (ADC0 and ADC1)? else
 *   use a multiplexer
 * - also add flammable (which?) sensor to detect "Diesel Dämpfe von Heizung"
 * - Diesel:
 *   - To put this in perspective, less than 4% of the molecules in a bucket
 *     of diesel fuel are small enough to pass through the flame
 *     arrestor and enter the sensor.
 *     @see http://www.goodforgas.com/documents/appnotes/AP1001_Choosing_best_technologies_for_comb_gas_and_voc_measurement_1_15_13.pdf
 *   - Die Hauptbestandteile des Dieselkraftstoffes sind vorwiegend Alkane,
 *     Cycloalkane und aromatische Kohlenwasserstoffe mit jeweils etwa 9 bis
 *     22 Kohlenstoff-Atomen pro Molekül [...]
 *     @see https://de.wikipedia.org/wiki/Dieselkraftstoff
 *
 * Thanks to:
 * fpgawars, icestorm, icestudio
 * @endverbatim
 */

int led = LED_BUILTIN;           // the PWM pin the LED is attached to
int brightness = 0;    // how bright the LED is
int fadeAmount = 5;    // how many points to fade the LED by

/*#define STEP  5  // steps of 5%
const byte ledBarSet[21] = {  // default
  0B00000000,  //  0%
  0B00000000,  //  5%
  0B00000000,  // 10%
  0B00000001,  // 15%
  0B00000011,  // 20%
  0B00000010,  // 25%
  0B00000110,  // 30%
  0B00000100,  // 35%
  0B00001100,  // 40%
  0B00001000,  // 45%
  0B00011000,  // 50%
  0B00010000,  // 55%
  0B00110000,  // 60%
  0B00100000,  // 65%
  0B01100000,  // 70%
  0B01000000,  // 75%
  0B11000000,  // 80%
  0B10000000,  // 85%
  0B11111111,  // 90%
  0B11111111,  // 95%
  0B11111111   //100%
};*/

/*#define STEP  5  // steps of 5%
const byte ledBarSet[21] = {  // sensitive on low values
  0B00000000,  //  0%
  0B00000001,  //  5%
  0B00000011,  // 10%
  0B00000010,  // 15%
  0B00000110,  // 20%
  0B00000100,  // 25%
  0B00001100,  // 30%
  0B00001000,  // 35%
  0B00011000,  // 40%
  0B00010000,  // 45%
  0B00110000,  // 50%
  0B00100000,  // 55%
  0B01100000,  // 60%
  0B01000000,  // 65%
  0B11000000,  // 70%
  0B10000000,  // 75%
  0B11111111,  // 80%
  0B11111111,  // 85%
  0B11111111,  // 90%
  0B11111111,  // 95%
  0B11111111   //100%
};*/

#define STEP  4  // steps of 4%
const byte ledBarSet[26] = {  // more sensitive on low values
  0B00000000,  //  0%
  0B00000001,  //  4%
  0B00000011,  //  8%
  0B00000010,  // 12%
  0B00000110,  // 16%
  0B00000100,  // 20%
  0B00001100,  // 24%
  0B00001000,  // 28%
  0B00011000,  // 32%
  0B00010000,  // 36%
  0B00110000,  // 40%
  0B00100000,  // 44%
  0B01100000,  // 48%
  0B01000000,  // 52%
  0B11000000,  // 56%
  0B10000000,  // 60%
  0B11111111,  // 64%
  0B11111111,  // 68%
  0B11111111,  // 72%
  0B11111111,  // 76%
  0B11111111,  // 80%
  0B11111111,  // 84%
  0B11111111,  // 88%
  0B11111111,  // 92%
  0B11111111,  // 96%
  0B11111111   //100%
};

int sensorPin = A0;    // select the input pin for the potentiometer
int sensorValue = 0;  // variable to store the value coming from the sensor

#define SW1  14  // switch1

int avg_buf[100] = {0};  // store value all 10s thus average over 1000s
int avg_idx = 0;
unsigned long avg_time = 0;

// the setup routine runs once when you press reset:
void setup() {
  // declare pin 9 to be an output:
  pinMode(led, OUTPUT);

  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
}

// the loop routine runs over and over again forever:
void loop() {
/*  // set the brightness of pin 9:
  analogWrite(led, brightness);

  // change the brightness for next time through the loop:
  brightness = brightness + fadeAmount;

  // reverse the direction of the fading at the ends of the fade:
  if (brightness <= 0 || brightness >= 255) {
    fadeAmount = -fadeAmount;
  }
  // wait for 30 milliseconds to see the dimming effect
  delay(30);*/

  if(digitalRead(SW1) == HIGH) {  // display average value
    sensorValue = 0;
    for (int i = 0; i < 100; i++) {
        sensorValue += avg_buf[i];
    }
    sensorValue /= 100;
  } else {                        // display current value
    sensorValue = analogRead(sensorPin);
//    sensorValue = analogRead(sensorPin)/2+50;  // debug
    if((millis() - avg_time) > 10000) {  // store value for average
      avg_buf[avg_idx] = sensorValue;
      avg_idx = (avg_idx + 1) % 100;
      avg_time = millis();
      Serial.println("val stored");
    }
  }

  digitalWrite(led, ledBarSet[(sensorValue*100/1023)/STEP]);

  Serial.print(sensorValue);
  Serial.print(" ");
  Serial.println(sensorValue*100/1023);

  delay(200);  // ADC has 4Hz clock

// TODO:
// - use fade mode to display error (or something else)
// (- can CPU be used with faster clock for speed-up?)
// (- can SPI display be used by using SPI master (block)?)
}
