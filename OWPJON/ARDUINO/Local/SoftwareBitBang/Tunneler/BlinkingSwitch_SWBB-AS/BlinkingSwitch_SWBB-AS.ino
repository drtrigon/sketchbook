/* Similar to the Switch but with a blinking led indicating packets
   forwarded with on and off for each direction respectively. */
/* Use a couple of visible light / IR / UV LEDs as wireless bidirectional transceivers
   Connect LED + to A0
   Connect LED - to GND
   Place a 100KΩ-5MΩ pull down resistor between A0 and GND
   Try different resistor values to find the optimal to maximize range
   Higher resistance can higher the range but can also higher background noise.  */

//#define PJON_MAX_PACKETS 3
#include <PJONInteractiveRouter.h>

StrategyLink<SoftwareBitBang> link1;
StrategyLink<AnalogSampling> link2;

PJONAny bus1(&link1);
PJONAny bus2(&link2);

PJONInteractiveRouter<PJONVirtualBusRouter<PJONSwitch>> router(2, (PJONAny*[2])
{
  &bus1, &bus2
});

void setup()
{
  //Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); // Initialize LED 13 to be off

  /* Use internal 1.1v analog reference (not available on Arduino Mega)
  DEFAULT: Default analog reference 5v (5v Arduino boards) or 3.3v (3.3V Arduino boards)
  INTERNAL: Built-in 1.1v on ATmega168 or ATmega328 and 2.56v on ATmega8 (not available on Arduino Mega)
  INTERNAL1V1: Built-in 1.1v reference (Arduino Mega only)
  INTERNAL2V56: a built-in 2.56v reference (Arduino Mega only)
  EXTERNAL: Voltage applied to the AREF pin (0 to 5v) is used as the reference. */
  analogReference(INTERNAL);

  link1.strategy.set_pin(7);
  link2.strategy.set_pin(A0);
//  router.set_sendnotification(sendnotification_function);
//  router.set_error(error_handler);
  router.set_virtual_bus(0); // Enable virtual bus
  router.begin();
}

void loop()
{
  router.loop();
};

/*void sendnotification_function(const uint8_t * const payload, const uint16_t length, const uint8_t receiver_bus,
                               const uint8_t sender_bus, const PJON_Packet_Info &packet_info)
{
  switch(sender_bus) {
  case 0: {
    digitalWrite(LED_BUILTIN, HIGH);
  };
  case 1: {
    digitalWrite(LED_BUILTIN, LOW);
  };
  }
}*/

/*void error_handler(uint8_t code, uint16_t data, void *custom_ptr) {
  digitalWrite(ERROR_LED_PIN, HIGH);
  error_on_time = millis();
}*/
