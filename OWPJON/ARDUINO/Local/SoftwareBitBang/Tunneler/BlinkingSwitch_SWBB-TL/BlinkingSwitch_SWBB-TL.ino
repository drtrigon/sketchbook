/* Similar to the Switch but with a blinking led indicating packets
   forwarded with on and off for each direction respectively. */
/* Dargino PJON SWBB-TL Tunneler
 *
 * - disable Dargino wifi; like Yun use WebInterface to configure
 *   LAN, DHCP, DISABLE WIFI
 *   also see comments in Yun_Dragino_Server_LoRa_WAN
 * - even though it is recognized as "Yun" the dargino contains
 *   an "Uno" (probably m328p) - so select "Arduino/Genuino Uno"
 *   even better is to use
 *   http://www.dragino.com/downloads/downloads/YunShield/package_dragino_yun_test_index.json
 *   and select "Dragino Yun + UNO or LG01/OLG01"
 *   also see http://192.168.11.16/cgi-bin/luci//admin/sensor/mcu
 *   as port use the network port (e.g. ...16)
 * - test PJON SWBB; use OWPJON/ARDUINO/Local/SoftwareBitBang/DeviceGeneric/
 * - 3.3-5V bidir. shifting http://vusb.wikidot.com/hardware - may be use own SWBB bus, see BlinkingSwitch_3way
 * - ...
 *
 *   * Heartbeat (Blink); replace LED_BUILTIN by HEART_LED
 */
/* Dragino LG01-S (Uno/Yun combo)
 * https://github.com/gioblu/PJON/tree/master/src/strategies/ThroughLoRa
 */
/**
 * needs doxygen docu - derived from BlinkingSwitch.ino using Yun_Dragino... and https://github.com/gioblu/PJON/tree/master/src/strategies/ThroughLoRa
 * ...
 */

#define DRAGINO  // enable for Dragino tunnel, disable for others

#ifdef DRAGINO
#define HEART_LED    A2
#define BUILTIN_LED  HEART_LED
#else
#define BUILTIN_LED  LED_BUILTIN
#endif
#define OWPJON_PIN    4

#define PJON_INCLUDE_TL

#define PJON_MAX_PACKETS 3
#include <PJONInteractiveRouter.h>

StrategyLink<SoftwareBitBang> link1;
StrategyLink<ThroughLora> link2;

PJONAny bus1(&link1);
PJONAny bus2(&link2);

PJONInteractiveRouter<PJONVirtualBusRouter<PJONSwitch>> router(2, (PJONAny*[2])
{
  &bus1, &bus2
});

void setup()
{
  //Serial.begin(115200);
  link1.strategy.set_pin(OWPJON_PIN);
  // Obligatory to initialize Radio with correct frequency
//  link2.strategy.setPins(ss, reset, dio0);
  link2.strategy.setFrequency(868100000UL);
  // Optional
  link2.strategy.setSignalBandwidth(250E3);  // default is 125E3
  link2.strategy.setTxPower(13);             // default is 17
  //link2.strategy.setSpreadingFactor(7);      // default is 7
  //link2.strategy.setCodingRate4(5);          // default is 5
  router.set_sendnotification(sendnotification_function);
//  router.set_error(error_handler);
  router.set_virtual_bus(0); // Enable virtual bus
  router.begin();

  // Init pin for LED
  pinMode(BUILTIN_LED, OUTPUT);
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
    digitalWrite(BUILTIN_LED, HIGH);
  };
  case 1: {
    digitalWrite(BUILTIN_LED, LOW);
  };
  }
}

/*void error_handler(uint8_t code, uint16_t data, void *custom_ptr) {
  digitalWrite(ERROR_LED_PIN, HIGH);
  error_on_time = millis();
}*/
