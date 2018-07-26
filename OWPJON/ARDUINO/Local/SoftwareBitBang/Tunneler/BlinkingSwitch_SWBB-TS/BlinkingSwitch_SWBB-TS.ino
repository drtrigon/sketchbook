/**
 * @brief Switch/Tunnel for ThroughSerial-SoftwareBitBang
 *
 * @file OWPJON/ARDUINO/Local/SoftwareBitBang/Tunneler/BlinkingSwitch_SWBB-TS/BlinkingSwitch_SWBB-TS.ino
 *
 * @author drtrigon
 * @date 2018-07-06
 * @version 1.0
 *   @li first version replacing SoftwareBitBangSurrogate derived from
 *       @ref OWPJON/ARDUINO/Local/SoftwareBitBang/Tunneler/BlinkingSwitch/BlinkingSwitch.ino
 *
 * @verbatim
 * OneWire PJON Generic "OWPG" scheme:
 *   Server e.g. linux machine or raspi
 *      OWPJON/LINUX/Local/LocalUDP/RemoteWorker/DeviceGeneric/
 *      OWPJON/LINUX/Local/ThroughSerial/RemoteWorker/DeviceGeneric/
 *   Tunnel(er) similar to 1wire master (similar cause we are on a multi-master bus) e.g. AVR
 *      OWPJON/ARDUINO/Local/SoftwareBitBang/Tunneler/BlinkingSwitch/
 *      OWPJON/ARDUINO/Local/SoftwareBitBang/Tunneler/BlinkingSwitch_SWBB-TS/ (this sketch)
 *      OWPJON/ARDUINO/Local/ThroughSerial/SoftwareBitBangSurrogate/Surrogate/ (obsolete)
 *   Devices e.g. AVR
 *      OWPJON/ARDUINO/Local/SoftwareBitBang/DeviceGeneric/
 *      OWPJON/ARDUINO/Local/SoftwareBitBang/OWP_DG_LCD_Sensors/
 *      ...
 *
 * Compatible with: atmega328 (Uno, Nano), atmega32u4 (Yun)
 *
 * Pinout:
 *   1wire PJON data bus (OWPJON SWBB):
 *        1WIRE DATA    -> Arduino Pin D7
 *        GND black     -> Arduino GND
 *   1wire Sniffer/Master/Client Shield
 *
 * Test on Ubuntu or Raspberry Pi Server (owpshell) confer the docu of
 * following files:
 *   - @ref OWPJON/LINUX/Local/LocalUDP/RemoteWorker/DeviceGeneric/DeviceGeneric.cpp
 *   - @ref OWPJON/LINUX/Local/ThroughSerial/RemoteWorker/DeviceGeneric/DeviceGeneric.cpp
 *
 * Thanks to:
 * gioblu - PJON 11.0 and support
 *          @see https://www.pjon.org/
 *          @see https://github.com/gioblu/PJON
 * fredilarsen - support
 * @endverbatim
 */

//#define PJON_MAX_PACKETS 3
#include <PJONInteractiveRouter.h>

StrategyLink<SoftwareBitBang> link1;
StrategyLink<ThroughSerial> link2;

PJONAny bus1(&link1);
PJONAny bus2(&link2);

PJONInteractiveRouter<PJONVirtualBusRouter<PJONSwitch>> router(2, (PJONAny*[2])
{
  &bus1, &bus2
});

void setup()
{
  //Serial.begin(115200);
  Serial.begin(9600);
  link1.strategy.set_pin(7);
  link2.strategy.set_serial(&Serial);
//  bus2.set_synchronous_acknowledge(false);
//  router.set_sendnotification(sendnotification_function);
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
