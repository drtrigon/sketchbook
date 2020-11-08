/**
 * @brief Code that uses the Danger Shield as a children/baby toy (using FreeRTOS)
 *
 * @file Uno_DangerShield-Toy_FreeRTOS/Uno_DangerShield-Toy_FreeRTOS.ino
 *
 * @author drtrigon
 * @date 2020-10-10
 * @version 1.0
 *   @li first version featuring 2 programs
 *   @li first version derived from Uno_DangerShield-Toy (Danger Shield Example Sketch)
 *       and 'Uno_ePaper_Test' (which was derived from "epd2in9-demo")
 *
 * @ref Uno_DangerShield-Toy/Uno_DangerShield-Toy.ino
 * @ref File > Examples > FreeRTOS > Blink_AnalogRead
 *
 * @see https://raw.githubusercontent.com/sparkfun/DangerShield/master/Firmware/DangerShield/DangerShield.ino
 * @see https://www.freertos.org/a00106.html
 *
 * @verbatim
 * Configurations (default is prog 0):
 *   1. prog 0: "Enhanced default config" similar to Danger Shield demo with some additions:
 *     - TaskBlink
 *     - TaskSliderLED
 *     - TaskSliderNumber
 *     - TaskButtonLED
 *     - TaskSliderButtonBuzzer (BUTTON3, SLIDER3)
 *     - TaskLightAlarm
 *     - TaskCapSenseWarning
 *     - TaskButtonBuzzer
 *   2. prog 1 "Sound Machine"
 *     - TaskSliderButtonBuzzer (BUTTON1, SLIDER1)
 *     - TaskSliderButtonBuzzer (BUTTON2, SLIDER2)
 *     - TaskSliderButtonBuzzer (BUTTON3, SLIDER3)
 * FreeRTOS hints:
 *   - memory is VERY limited, in order to make "all" tasks work, disable serial
 *     (in order to use serial, disable at least one task)
 *   - in order to check stack (memory) requirements use:
 *     - uxTaskGetStackHighWaterMark()
 *     - vApplicationStackOverflowHook callback (fast blinking LED)
 *   - for shared ressources (e.g. Buzzer or Alarm tasks) use either:
 *     - full blocking: vTaskSuspendAll(), xTaskResumeAll()
 *     - non-blocking: xSemaphoreTake(), xSemaphoreGive()
 *                     (seems you can use both delay() or vTaskDelay() here...)
 *
 * Pinout:
 *   Danger Shield v1.7 pin definitions (see code below)
 *
 * Thanks to:
 * Danger Shield Example Sketch
 * Chris Taylor, Nathan Seidle
 * Spark Fun Electronics
 * @endverbatim
 */

// Shift register bit values to display 0-9 on the seven-segment display
const PROGMEM byte ledCharSet[10] = {
  B00111111, 
  B00000110, 
  B01011011, 
  B01001111, 
  B01100110, 
  B01101101, 
  B01111101, 
  B00000111, 
  B01111111, 
  B01101111
};

// Pin definitions

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//These pin definitions are for v1.7 of the board
#define SLIDER1  A2 //Matches button 1
#define SLIDER2  A1 
#define SLIDER3  A0 //Matches button 3
#define LIGHT    A3
#define TEMP     A4

#define BUZZER   3
#define DATA     4
#define LED1     5
#define LED2     6
#define LATCH    7
#define CLOCK    8
#define BUTTON1  10
#define BUTTON2  11
#define BUTTON3  12
//v1.7 uses CapSense
//This relies on the Capactive Sensor library here: http://playground.arduino.cc/Main/CapacitiveSensor
#include <CapacitiveSensor.h>

CapacitiveSensor capPadOn92 = CapacitiveSensor(9, 2);   //Use digital pins 2 and 9,
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include <Arduino_FreeRTOS.h>
#include <semphr.h>

#define configCHECK_FOR_STACK_OVERFLOW    2

SemaphoreHandle_t xSemaphore = xSemaphoreCreateMutex();  // the Buzzer is a shared ressource - use Semaphore to protect it

int avgLightLevel;

// the setup function runs once when you press reset or power the board
void setup() {

  // ATTENTION: IN ORDER TO BE ABLE TO USE Serial (e.g. for checking stack usage with uxTaskGetStackHighWaterMark)
  //            DISABLE AT LEAST 1 TASK BELOW OTHERWISE YOU WILL GET A STACK OVERFLOW REPORTED BY FAST BLINKING PIN13 LED
/*  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  }*/

  //Initialize inputs and outputs
  pinMode(SLIDER1, INPUT);
  pinMode(SLIDER2, INPUT);
  pinMode(SLIDER3, INPUT);
  pinMode(LIGHT, INPUT);
  pinMode(TEMP, INPUT);

  //Enable internal pullups
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(BUTTON3, INPUT_PULLUP);

  pinMode(BUZZER, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  pinMode(LATCH, OUTPUT);
  pinMode(CLOCK, OUTPUT);
  pinMode(DATA, OUTPUT);

  //Initialize the capsense
  capPadOn92.set_CS_AutocaL_Millis(0xFFFFFFFF); // Turn off autocalibrate on channel 1 - From Capacitive Sensor example sketch

  //Take 16 readings from the light sensor and average them together
  avgLightLevel = 0;
  for(int x = 0 ; x < 16 ; x++)
    avgLightLevel += analogRead(LIGHT);
  avgLightLevel /= 16;

  int val2 = analogRead(SLIDER2);
  //int numToDisplay = map(val2, 0, 1023, 0, 9); //Map the slider value to a displayable value
  int numToDisplay = map(val2, 0, 1000, 0, 9); //Map the slider value to a displayable value

  // select (sub)program to run
  switch(numToDisplay) {
    case 1:   // prog 1: Sound Machine
      xTaskCreate(
        TaskSliderButtonBuzzer
        ,  "SliderButtonBuzzer"   // A name just for humans
        ,  90  // This stack size can be checked & adjusted by reading the Stack Highwater
        ,  ( void * ) BUTTON1  // Parameter passed into the task.
        ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        ,  NULL );

      xTaskCreate(
        TaskSliderButtonBuzzer
        ,  "SliderButtonBuzzer"   // A name just for humans
        ,  90  // This stack size can be checked & adjusted by reading the Stack Highwater
        ,  ( void * ) BUTTON2  // Parameter passed into the task.
        ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        ,  NULL );

      xTaskCreate(
        TaskSliderButtonBuzzer
        ,  "SliderButtonBuzzer"   // A name just for humans
        ,  90  // This stack size can be checked & adjusted by reading the Stack Highwater
        ,  ( void * ) BUTTON3  // Parameter passed into the task.
        ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        ,  NULL );

      break;
    default:  // prog 0 (default): Danger Shield demo with some additions
      // Now set up tasks to run independently.
      xTaskCreate(
        TaskBlink
        ,  "Blink"   // A name just for humans
        ,  80  // This stack size can be checked & adjusted by reading the Stack Highwater
        ,  NULL
        ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        ,  NULL );/**/

      xTaskCreate(
        TaskSliderLED
        ,  "SliderLED"   // A name just for humans
        ,  80  // This stack size can be checked & adjusted by reading the Stack Highwater
        ,  NULL
        ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        ,  NULL );

      xTaskCreate(
        TaskSliderNumber
        ,  "SliderNumber"   // A name just for humans
        ,  80  // This stack size can be checked & adjusted by reading the Stack Highwater
        ,  NULL
        ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        ,  NULL );

      xTaskCreate(
        TaskButtonLED
        ,  "ButtonLED"   // A name just for humans
        ,  80  // This stack size can be checked & adjusted by reading the Stack Highwater
        ,  NULL
        ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        ,  NULL );

      xTaskCreate(
        TaskSliderButtonBuzzer
        ,  "SliderButtonBuzzer"   // A name just for humans
        ,  90  // This stack size can be checked & adjusted by reading the Stack Highwater
        ,  ( void * ) BUTTON3  // Parameter passed into the task.
        ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        ,  NULL );

      xTaskCreate(
        TaskLightAlarm
        ,  "LightAlarm"   // A name just for humans
        ,  96  // This stack size can be checked & adjusted by reading the Stack Highwater (10 free)
        ,  NULL
        ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        ,  NULL );

    /*  xTaskCreate(
        TaskTempWarning
        ,  "TempWarning"   // A name just for humans
        ,  96  // This stack size can be checked & adjusted by reading the Stack Highwater
        ,  NULL
        ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        ,  NULL );*/

      xTaskCreate(
        TaskCapSenseWarning
        ,  "CapSenseWarning"   // A name just for humans
        ,  96  // This stack size can be checked & adjusted by reading the Stack Highwater (10 free)
        ,  NULL
        ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        ,  NULL );/**/

    /*  xTaskCreate(
        TaskCapSenseLED
        ,  "CapSenseLED"   // A name just for humans
        ,  80  // This stack size can be checked & adjusted by reading the Stack Highwater
        ,  NULL
        ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        ,  NULL );*/

      xTaskCreate(
        TaskButtonBuzzer
        ,  "ButtonBuzzer"   // A name just for humans
        ,  96  // This stack size can be checked & adjusted by reading the Stack Highwater (10 free)
        ,  NULL
        ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        ,  NULL );

      break;
  }

  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}

void loop()
{
  // Empty. Things are done in Tasks.
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName)
{
  // initialize digital LED_BUILTIN on pin 13 as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  for (;;) // A Task shall never return or exit.
  {
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    vTaskDelay( 100 / portTICK_PERIOD_MS ); // wait for one second
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    vTaskDelay( 100 / portTICK_PERIOD_MS ); // wait for one second
  }
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskBlink(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, LEONARDO, MEGA, and ZERO 
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN takes care 
  of use the correct LED pin whatever is the board used.
  
  The MICRO does not have a LED_BUILTIN available. For the MICRO board please substitute
  the LED_BUILTIN definition with either LED_BUILTIN_RX or LED_BUILTIN_TX.
  e.g. pinMode(LED_BUILTIN_RX, OUTPUT); etc.
  
  If you want to know what pin the on-board LED is connected to on your Arduino model, check
  the Technical Specs of your board  at https://www.arduino.cc/en/Main/Products
  
  This example code is in the public domain.

  modified 8 May 2014
  by Scott Fitzgerald
  
  modified 2 Sep 2016
  by Arturo Guadalupi
*/
  // initialize digital LED_BUILTIN on pin 13 as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  for (;;) // A Task shall never return or exit.
  {
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second
  }
}

void TaskSliderLED(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  for (;;) // A Task shall never return or exit.
  {
    int val1 = analogRead(SLIDER1);
    //Set the brightness on LED #2 (D6) based on slider 1
    int ledLevel = map(val1, 0, 1023, 0, 255); //Map the slider level to a value we can set on the LED
    analogWrite(LED2, ledLevel); //Set LED brightness
    vTaskDelay( 50 / portTICK_PERIOD_MS ); // wait for one second
  }
}

void TaskSliderNumber(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  for (;;) // A Task shall never return or exit.
  {
    int val2 = analogRead(SLIDER2);
    //Set 7 segment display based on the 2nd slider
    //int numToDisplay = map(val2, 0, 1023, 0, 9); //Map the slider value to a displayable value
    int numToDisplay = map(val2, 0, 1000, 0, 9); //Map the slider value to a displayable value
    digitalWrite(LATCH, LOW);
    if(digitalRead(BUTTON2) == LOW)
//      shiftOut(DATA, CLOCK, MSBFIRST, (ledCharSet[numToDisplay]));
      shiftOut(DATA, CLOCK, MSBFIRST, (pgm_read_word_near(ledCharSet + numToDisplay)));
    else
//      shiftOut(DATA, CLOCK, MSBFIRST, ~(ledCharSet[numToDisplay]));
      shiftOut(DATA, CLOCK, MSBFIRST, ~(pgm_read_word_near(ledCharSet + numToDisplay)));
    digitalWrite(LATCH, HIGH);
    vTaskDelay( 50 / portTICK_PERIOD_MS ); // wait for one second
  }
}

void TaskButtonLED(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  for (;;) // A Task shall never return or exit.
  {
    //If user is hitting button 1, turn on LED 1
    if(digitalRead(BUTTON1) == LOW)
    {
      //Serial.print(" Button 1!");
      digitalWrite(LED1, HIGH);
    }
    else
      digitalWrite(LED1, LOW);
    vTaskDelay( 50 / portTICK_PERIOD_MS ); // wait for one second
  }
}

void TaskSliderButtonBuzzer(void *pvParameters)  // This is a task.
{
//  (void) pvParameters;

//  /* The parameter value is expected to be 1 as 1 is passed in the
//  pvParameters value in the call to xTaskCreate() below. */
//  configASSERT( ( ( uint32_t ) pvParameters ) == 1 );

  int button, slider;
  switch(( ( uint8_t ) pvParameters )) {
    case BUTTON1:   // BUTTON1
      button = BUTTON1;
      slider = SLIDER1;
      break;
    case BUTTON2:   // BUTTON2
      button = BUTTON2;
      slider = SLIDER3;
      break;
    default:        // BUTTON3
      button = BUTTON3;
      slider = SLIDER3;
      break;
  }

  for (;;) // A Task shall never return or exit.
  {
    int val3 = analogRead(slider);
    //Set the sound based on the 3rd slider
    long buzSound = map(val3, 0, 1023, 1000, 10000); //Map the slider value to an audible frequency
    if(xSemaphoreTake( xSemaphore, (TickType_t)10) == pdTRUE) {
      if((buzSound > 1100) && (digitalRead(button) == LOW))
        tone(BUZZER, buzSound); //Set sound value
      else
        noTone(BUZZER);

      xSemaphoreGive( xSemaphore );
    }
    vTaskDelay( 50 / portTICK_PERIOD_MS ); // wait for one second

    /* Calling the function will have used some stack space, we would
    therefore now expect uxTaskGetStackHighWaterMark() to return a
    value lower than when it was called on entering the task. */
/*    UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    Serial.println(uxHighWaterMark);*/
  }
}

void TaskLightAlarm(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  for (;;) // A Task shall never return or exit.
  {
    int lightLevel = analogRead(LIGHT);
    int numToDisplay; //Map the slider value to a displayable value
    long buzSound; //Map the slider value to an audible frequency

    vTaskSuspendAll();

    //If light sensor is less than 3/4 of the average (covered up) then freak out
    while(lightLevel < (avgLightLevel * 3 / 4))
    {
  
      //Blink the status LEDs back and forth
      if(digitalRead(LED1) == LOW)
      {
        digitalWrite(LED1, HIGH);
        digitalWrite(LED2, LOW);
      }
      else {
        digitalWrite(LED1, LOW);
        digitalWrite(LED2, HIGH);
      }
  
      //Display an increasing number on the 7-segment display
      digitalWrite(LATCH, LOW);
      shiftOut(DATA, CLOCK, MSBFIRST, ~(ledCharSet[numToDisplay]));
      digitalWrite(LATCH, HIGH);
      numToDisplay++; //Goto next number
      if(numToDisplay > 9) numToDisplay = 0; //Loop number
  
      //Play a horrendously annoying sound
      //Frequency increases with each loop
      if(digitalRead(BUTTON3) == LOW)
        tone(BUZZER, buzSound);
      else
        noTone(BUZZER);
      buzSound += 100;
      if(buzSound > 3000) buzSound = 1000;
  
      //vTaskDelay( 25 / portTICK_PERIOD_MS ); // wait for one second
      delay(25);
  
      lightLevel = analogRead(LIGHT); //We need to take another reading to be able to exit the while loop
    }
 
    xTaskResumeAll();

    vTaskDelay( 50 / portTICK_PERIOD_MS ); // wait for one second
  }
}

void TaskTempWarning(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  for (;;) // A Task shall never return or exit.
  {
    long temperature = analogRead(TEMP);
  
    //Do conversion of temp from analog value to digital
    temperature *= 5000; //5V is the same as 1023 from the ADC (12-bit) - example, the ADC returns 153 * 5000 = 765,000
    temperature /= 1023; //It's now in mV - 765,000 / 1023 = 747
    //The TMP36 reports 500mV at 0C so let's subtract off the 500mV offset
    temperature -= 500; //747 - 500 = 247
    //Temp is now in C where 247 = 24.7

    vTaskSuspendAll();

    //If light sensor is less than 3/4 of the average (covered up) then freak out
    while(((float)temperature/10) > 30.0)
    {

      digitalWrite(LED1, LOW);
      digitalWrite(LED2, LOW);
      digitalWrite(LATCH, LOW);
      shiftOut(DATA, CLOCK, MSBFIRST, ~B00000000);
      digitalWrite(LATCH, HIGH);
      noTone(BUZZER);

      //vTaskDelay( 25 / portTICK_PERIOD_MS ); // wait for one second
      delay(25);
  
      temperature = analogRead(TEMP);
    
      //Do conversion of temp from analog value to digital
      temperature *= 5000; //5V is the same as 1023 from the ADC (12-bit) - example, the ADC returns 153 * 5000 = 765,000
      temperature /= 1023; //It's now in mV - 765,000 / 1023 = 747
      //The TMP36 reports 500mV at 0C so let's subtract off the 500mV offset
      temperature -= 500; //747 - 500 = 247
      //Temp is now in C where 247 = 24.7
    }
 
    xTaskResumeAll();

    vTaskDelay( 500 / portTICK_PERIOD_MS ); // wait for one second
   }
}

void TaskCapSenseWarning(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  for (;;) // A Task shall never return or exit.
  {
    long capLevel = capPadOn92.capacitiveSensor(30);

    vTaskSuspendAll();  // instead of using Semaphore just stop all other tasks (lot of ressources)

    //If light sensor is less than 3/4 of the average (covered up) then freak out
    while(capLevel > 1000)
    {

      digitalWrite(LED1, LOW);
      digitalWrite(LED2, LOW);
      digitalWrite(LATCH, LOW);
      shiftOut(DATA, CLOCK, MSBFIRST, ~B00000000);
      digitalWrite(LATCH, HIGH);
      noTone(BUZZER);

      //vTaskDelay( 25 / portTICK_PERIOD_MS ); // wait for one second
      delay(25);

      capLevel = capPadOn92.capacitiveSensor(30);
    }

    xTaskResumeAll();

    vTaskDelay( 500 / portTICK_PERIOD_MS ); // wait for one second
   }
}

void TaskCapSenseLED(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  for (;;) // A Task shall never return or exit.
  {
    //Read the cap sense pad
    long capLevel = capPadOn92.capacitiveSensor(30);
    //Set the brightness on LED #2 (D6) based on slider 1
    int ledLevel = map(max(capLevel, 20), 20, 1250, 0, 255); //Map the slider level to a value we can set on the LED
    analogWrite(LED1, ledLevel); //Set LED brightness
    vTaskDelay( 50 / portTICK_PERIOD_MS ); // wait for one second
  }
}

void TaskButtonBuzzer(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  for (;;) // A Task shall never return or exit.
  {
    if(xSemaphoreTake( xSemaphore, (TickType_t)10) == pdTRUE) {
      if(digitalRead(BUTTON1) == LOW) {
          tone(BUZZER, 1000);
          delay( 500 ); // wait for one second
          noTone(BUZZER);
          delay( 100 ); // wait for one second
          tone(BUZZER, 1000);
          delay( 500 ); // wait for one second
          noTone(BUZZER);
          delay( 100 ); // wait for one second
          tone(BUZZER, 1000);
          delay( 500 ); // wait for one second
          noTone(BUZZER);
          delay( 100 ); // wait for one second
      }

      xSemaphoreGive( xSemaphore );
    }
    vTaskDelay( 50 / portTICK_PERIOD_MS ); // wait for one second
  }
}
