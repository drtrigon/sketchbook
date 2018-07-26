/**
 * needs doxygen docu - derived from BlinkingSwitch.ino (useful for LUDP-SWBB-TL or LUDP-SWBBx2 1 for 5V 1 for 3.3-5V buses)
 * ...
 */

#define PJON_MAX_PACKETS 3
#include <PJONInteractiveRouter.h>

// Ethernet configuration for this device, MAC must be unique!
byte mac[] = {0xDE, 0x34, 0x4E, 0xEF, 0xFE, 0xE1};

StrategyLink<SoftwareBitBang> link1;
StrategyLink<SoftwareBitBang> link2;
StrategyLink<LocalUDP> link3;

PJONAny bus1(&link1);
PJONAny bus2(&link2);
PJONAny bus3(&link3);

PJONInteractiveRouter<PJONVirtualBusRouter<PJONSwitch>> router(3, (PJONAny*[3])
{
  &bus1, &bus2, &bus3
});

void setup()
{
  //Serial.begin(115200);
  Ethernet.begin(mac); // Use DHCP
  link1.strategy.set_pin(7);
  link2.strategy.set_pin(8);
//  link3.strategy.set_port(7200); // Use a "private" UDP port
  router.set_sendnotification(sendnotification_function);
//  router.set_error(error_handler);
  router.set_virtual_bus(0); // Enable virtual bus
  router.begin();

  // Init pin for LED
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
  router.loop();
};

void sendnotification_function(const uint8_t * const payload, const uint16_t length, const uint8_t receiver_bus,
                               const uint8_t sender_bus, const PJON_Packet_Info &packet_info)
{
  switch(sender_bus) {
  case 0: {
    digitalWrite(LED_BUILTIN, HIGH);
    break;
  };
  case 1: {
    digitalWrite(LED_BUILTIN, LOW);
    break;
  };
  }
}

/*void error_handler(uint8_t code, uint16_t data, void *custom_ptr) {
  digitalWrite(ERROR_LED_PIN, HIGH);
  error_on_time = millis();
}*/
