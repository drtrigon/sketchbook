/**
 * @brief Switch/Tunnel for ThroughLoRa-SoftwareBitBang
 *
 * @file OWPJON/ARDUINO/Local/SoftwareBitBang/Tunneler/BlinkingSwitch_SWBB-TL/BlinkingSwitch_SWBB-TL.ino
 *
 * @author drtrigon
 * @date 2018-08-08
 * @version 1.1
 *   @li added missing default to switch cases
 *   @li added watchdog for recovery from heavy failures
 *   @li changed LoRa frequency/channel
 * @version 1.0
 *   @li first version derived from
 *       @ref OWPJON/ARDUINO/Local/SoftwareBitBang/Tunneler/BlinkingSwitch/BlinkingSwitch.ino
 *       @ref Yun_Dragino_Server_LoRa_WAN/Yun_Dragino_Server_LoRa_WAN.ino
 *       @see https://github.com/gioblu/PJON/tree/master/src/strategies/ThroughLoRa
 *
 * @verbatim
 * OneWire PJON Generic "OWPG" scheme:
 *   @ref OWPJON/LINUX/Local/LocalUDP/RemoteWorker/DeviceGeneric/DeviceGeneric.cpp
 * For isolated remote SWBB networks on the same bus use e.g. LoRa and setup 2 tunnels/switches
 *   OWPJON/ARDUINO/Local/SoftwareBitBang/Tunneler/BlinkingSwitch_SWBB-TL/ (this sketch)
 *
 * Compatible with: atmega328 (Uno, Nano, Uno/Yun combo Dragino LG01-S), atmega32u4 (Yun)
 *
 * Pinout:
 *   1wire PJON data bus (OWPJON SWBB):
 *        1WIRE DATA    -> Arduino Pin D4
 *        GND black     -> Arduino GND
 *   1wire Sniffer/Master/Client Shield
 *   Dragino Lora Shield Shield or Dragino LG01-S
 *   @see https://github.com/gioblu/PJON/tree/master/src/strategies/ThroughLoRa#supported-shieldsmodules
 *
 * Dragino LG01-S:
 *   - Dragino is recognized as "Yun" but contains an "Uno" (m328p); select "Arduino/Genuino Uno"
 *     @see http://192.168.11.16/cgi-bin/luci//admin/sensor/mcu
 *     correct but not needed would be to use
 *     http://www.dragino.com/downloads/downloads/YunShield/package_dragino_yun_test_index.json
 *     and select "Dragino Yun + UNO or LG01/OLG01"
 *   - Disable Dargino wifi; like Yun use WebInterface to configure LAN, DHCP, DISABLE WIFI
 *     also see comments, hints, etc. in @ref Yun_Dragino_Server_LoRa_WAN/Yun_Dragino_Server_LoRa_WAN.ino
 *   - Bidirectional Level Shifting 3.3V<->5V needed for SWBB; 3v6 Zener and 4k7 resistor voltage divider
 *     (consider using @ref OWPJON/ARDUINO/Local/SoftwareBitBang/Tunneler/BlinkingSwitch_3way/BlinkingSwitch_3way.ino
 *     to isolate 3.3V and 5V bus from each other - also consider LUDP-SWBB-TL 3way tunnel)
 *     @ref OWPJON/README.md
 *     @see http://vusb.wikidot.com/hardware
 *     @see http://www.partsim.com/simulator/#148247
 *   - LoRa tx power 10..17 dBm; @see https://www.digitalairwireless.com/articles/blog/wifi-transmit-power-calculations-made-simples
 *   - LoRa settings vs. datarate; @see http://www.rfwireless-world.com/calculators/LoRa-Data-Rate-Calculator.html
 *   - LoRa settings for max. datarate; lowest SF, CR - highest BW. For max. range; highest SF, (CR for reliability) - lowest BW
 *   - LoRa frequency and debug; scan for free before using, @see Uno_Serial_LoRa_Sniffer/Uno_Serial_LoRa_Sniffer.ino
 *   - Debug Dragino; @see http://www.dragino.com/downloads/downloads/YunShield/YUN_SHIELD_QUICK_START_GUIDE_v1.0.pdf
 *                    For reboot via network @see http://192.168.11.16/cgi-bin/luci//admin.
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
 * Matheus-Garbelini - support
 * @endverbatim
 */

//#define DRAGINO  // enable for Dragino tunnel, disable for others

//#define ENABLE_DEBUG

#ifdef DRAGINO
#define HEART_LED    A2
#define BUILTIN_LED  HEART_LED    // Heartbeat (Blink) LED
#define SERIAL       Console      // Console replaces Serial for network connection
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

unsigned long time_last_message;

#define PJON_INCLUDE_TL

#define PJON_MAX_PACKETS 3
#include <PJONInteractiveRouter.h>

StrategyLink<SoftwareBitBang> link1;
StrategyLink<ThroughLora> link2;

PJONAny bus1(&link1);
PJONAny bus2(&link2);
//PJONAny bus2(&link2, PJON_NOT_ASSIGNED, 10000);

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
  // need a FREE and ALLOWED frequency (channel)
  //link2.strategy.setFrequency(868100000UL);  // 868.1 MHz
  //link2.strategy.setFrequency(865400000UL);  // 865.4 MHz
  link2.strategy.setFrequency(869200000UL);  // 869.2 MHz
  // Optional
  link2.strategy.setSignalBandwidth(250E3);  // default is 125E3
  link2.strategy.setTxPower(10);             // default is 17
  //link2.strategy.setSpreadingFactor(7);      // default is 7
  //link2.strategy.setCodingRate4(5);          // default is 5
  router.set_sendnotification(sendnotification_function);
#ifdef ENABLE_DEBUG
  router.set_error(error_handler);
#endif
  router.set_virtual_bus(0); // Enable virtual bus
  router.begin();

  // Init pin for LED
  pinMode(BUILTIN_LED, OUTPUT);

  time_last_message = millis() + 180000;

  // set watchdog for reset
  wdt_reset();
  wdt_enable(WDTO_8S);
  //wdt_enable(WDTO_15MS);
}

void loop()
{
  router.loop();

  // https://playground.arduino.cc/Code/TimingRollover (after approximately 50 days)
  if( (long)( millis() - time_last_message ) < 0 )  // no update for >180'000ms = 3min (see 1wire OLED)
    wdt_reset();
}

void sendnotification_function(const uint8_t * const payload, const uint16_t length, const uint8_t receiver_bus,
                               const uint8_t sender_bus, const PJON_Packet_Info &packet_info)
{
  switch(sender_bus) {
  case 0: {
    digitalWrite(BUILTIN_LED, HIGH);
//    time_last_message = millis() + 180000;
    break;
  };
  case 1: {
    digitalWrite(BUILTIN_LED, LOW);
    time_last_message = millis() + 180000;
#ifdef ENABLE_DEBUG
    SERIAL.print(link2.strategy.packetRssi());
    SERIAL.print(" ");
    SERIAL.print(link2.strategy.packetSnr());
    SERIAL.print(" ");
#endif
    break;
  };
  default:
    break;
  }
#ifdef ENABLE_DEBUG
  SERIAL.println(sender_bus);
#endif
}

#ifdef ENABLE_DEBUG
void error_handler(uint8_t code, uint16_t data, void *custom_pointer)
{
//  digitalWrite(ERROR_LED_PIN, HIGH);
  if(code == PJON_CONNECTION_LOST) {
    SERIAL.print(F("Connection with device ID "));
    SERIAL.print(router.get_bus(router.get_callback_bus()).packets[data].content[0], DEC);
    SERIAL.println(F(" is lost."));
  }
  if(code == PJON_PACKETS_BUFFER_FULL) {
    SERIAL.print(F("Packet buffer is full, has now a length of "));
    SERIAL.println(data);
    SERIAL.println(F("Possible wrong bus configuration!"));
    SERIAL.println(F("higher PJON_MAX_PACKETS in PJONDefines.h if necessary."));
  }
  if(code == PJON_CONTENT_TOO_LONG) {
    SERIAL.print(F("Content is too long, length: "));
    SERIAL.println(data);
  }
}
#endif
