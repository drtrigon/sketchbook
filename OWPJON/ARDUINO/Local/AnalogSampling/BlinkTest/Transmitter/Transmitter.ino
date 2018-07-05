// Predefine AS_MODE selecting communication mode if needed
#define AS_MODE 1 // 1024Bd  or 128B/s
//#define AS_MODE 2 // 1361Bd  or 170B/s
//#define AS_MODE 3 // 3773Bb  or 471B/s  (ADC prescale 32)
//#define AS_MODE 4 // 5547Bb  or 639B/s  (ADC prescale 16)
//#define AS_MODE 5 // 12658Bd or 1528B/s (ADC prescale  8)

/* Acknowledge maximum latency, 15000 microseconds default.
   Could be necessary to higher AS_RESPONSE_TIMEOUT if sending
   long packets because of the CRC computation time needed by
   receiver before transmitting its acknowledge  */
#define AS_RESPONSE_TIMEOUT 15000

/* Set the back-off exponential degree (default 5) */
#define AS_BACK_OFF_DEGREE      5

/* Set the maximum sending attempts (default 10) */
#define AS_MAX_ATTEMPTS        10

/* The values set above are the default producing a 3.2 seconds
   back-off timeout with 20 attempts. Higher SWBB_MAX_ATTEMPTS
   to higher the back-off timeout, higher SWBB_BACK_OFF_DEGREE
   to higher the interval between every attempt. */

#include <PJON.h>

// <Strategy name> bus(selected device id)
PJON<AnalogSampling> bus(45);

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); // Initialize LED 13 to be off

  /* Use internal 1.1v analog reference (not available on Arduino Mega)
  DEFAULT: Default analog reference 5v (5v Arduino boards) or 3.3v (3.3V Arduino boards)
  INTERNAL: Built-in 1.1v on ATmega168 or ATmega328 and 2.56v on ATmega8 (not available on Arduino Mega)
  INTERNAL1V1: Built-in 1.1v reference (Arduino Mega only)
  INTERNAL2V56: a built-in 2.56v reference (Arduino Mega only)
  EXTERNAL: Voltage applied to the AREF pin (0 to 5v) is used as the reference. */
  analogReference(INTERNAL);

  bus.strategy.set_pin(A0);
  bus.begin();
  Serial.begin(115200);
};

void loop() {
  if(bus.send_packet(44, "B",1) == PJON_ACK)
    digitalWrite(LED_BUILTIN, HIGH);
  delay(5);
  digitalWrite(LED_BUILTIN, LOW);
};

