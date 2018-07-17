/* Similar to the Switch but with a blinking led indicating packets
   forwarded with on and off for each direction respectively. */
/* Dargino PJON SWBB-TL Tunneler
 *
 * - disable Dargino wifi; like Yun use WebInterface to configure
 *   LAN, DHCP, DISABLE WIFI
 *   also see comments, hints and all in "Yun_Dragino_Server_LoRa_WAN"
 * - even though it is recognized as "Yun" the dargino contains
 *   an "Uno" (probably m328p) - so select "Arduino/Genuino Uno"
 *   even better is to use
 *   http://www.dragino.com/downloads/downloads/YunShield/package_dragino_yun_test_index.json
 *   and select "Dragino Yun + UNO or LG01/OLG01"
 *   also see http://192.168.11.16/cgi-bin/luci//admin/sensor/mcu
 *   as port use the network port (e.g. ...16)
 * - test PJON SWBB; use OWPJON/ARDUINO/Local/SoftwareBitBang/DeviceGeneric/
 * - 3.3-5V bidir. shifting http://vusb.wikidot.com/hardware - may be use own SWBB bus, see BlinkingSwitch_3way
 *   http://www.partsim.com/simulator/#148247
 * - LoRa tx power: https://www.digitalairwireless.com/articles/blog/wifi-transmit-power-calculations-made-simples
 * - debug: http://www.dragino.com/downloads/downloads/YunShield/YUN_SHIELD_QUICK_START_GUIDE_v1.0.pdf
 *          https://github.com/gioblu/PJON/wiki/Error-handling
 *          http://192.168.11.16/cgi-bin/luci//admin  (reboot dragino)
 * - ...
 *
 *   * Heartbeat (Blink); replace LED_BUILTIN by HEART_LED
 *   * Console replaces Serial
 */
/* Dragino LG01-S (Uno/Yun combo)
 * https://github.com/gioblu/PJON/tree/master/src/strategies/ThroughLoRa
 */
/**
 * needs doxygen docu - derived from BlinkingSwitch.ino using Yun_Dragino... and https://github.com/gioblu/PJON/tree/master/src/strategies/ThroughLoRa
 * ...
 */

#define DRAGINO  // enable for Dragino tunnel, disable for others

//#define ENABLE_DEBUG

#ifdef DRAGINO
#define HEART_LED    A2
#define BUILTIN_LED  HEART_LED
#define SERIAL       Console
#else
#define BUILTIN_LED  LED_BUILTIN
#define SERIAL       Serial
#endif
#define OWPJON_PIN    4

#include <avr/wdt.h>     // watchdog (used as work-a-round to recover from error)
#ifdef ENABLE_DEBUG
#ifdef DRAGINO
#include <Console.h>     // Console lib, used to show debug info in Arduino IDE
//#include <Bridge.h>
#define BAUDRATE 115200  //For product: LG01.
#endif
#endif

#define PJON_INCLUDE_TL

#define PJON_MAX_PACKETS 3
//#define PJON_MAX_PACKETS 0
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
  wdt_disable();  // disable watchdog

#ifdef ENABLE_DEBUG
#ifdef DRAGINO
  Bridge.begin(BAUDRATE);
  Console.begin();
  //Console.buffer(64);
  //while (!Console);     // wait for Network Serial to open
#else
  Serial.begin(115200);
#endif
  SERIAL.println(F(__FILE__));
#endif
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
//#ifdef ENABLE_DEBUG
  router.set_error(error_handler);
//#endif
  router.set_virtual_bus(0); // Enable virtual bus
  router.begin();

  // Init pin for LED
  pinMode(BUILTIN_LED, OUTPUT);
}

void loop()
{
  router.loop();
}

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
#ifdef ENABLE_DEBUG
  SERIAL.println(sender_bus);
#endif
  //wdt_disable();
  //wdt_reset();
}

void error_handler(uint8_t code, uint16_t data, void *custom_ptr) {
//  digitalWrite(ERROR_LED_PIN, HIGH);
#ifdef ENABLE_DEBUG
  if(code == PJON_CONNECTION_LOST) {
    SERIAL.print(F("Connection with device ID "));
    SERIAL.print(data);
    SERIAL.println(F(" is lost."));
  }
  if(code == PJON_PACKETS_BUFFER_FULL) {
    SERIAL.print(F("Packet buffer is full, has now a length of "));
    SERIAL.println(data, DEC);
    SERIAL.println(F("Possible wrong bus configuration!"));
    SERIAL.println(F("higher PJON_MAX_PACKETS in PJONDefines.h if necessary."));
  }
  if(code == PJON_CONTENT_TOO_LONG) {
    SERIAL.print(F("Content is too long, length: "));
    SERIAL.println(data);
  }
#endif
  // set watchdog for reset
  wdt_reset();
  //wdt_enable(WDTO_8S);  // give time (8s) to recover
  wdt_enable(WDTO_15MS);  // reset as fast as possible (15ms)
}
