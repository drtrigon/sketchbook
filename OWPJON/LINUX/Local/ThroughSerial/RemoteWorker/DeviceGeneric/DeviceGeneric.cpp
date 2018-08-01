/**
 * @brief Example-Code for a generic Server (e.g. Sensor, Display)
 *
 * @file OWPJON/LINUX/Local/ThroughSerial/RemoteWorker/DeviceGeneric/DeviceGeneric.cpp
 *
 * @author drtrigon
 * @date 2018-07-06
 * @version 1.0
 *   @li first version derived from PJON 11.0 examples
 *       examples/LINUX/Local/EthernetTCP/RemoteWorker/Transmitter/Transmitter.cpp
 *
 * TODO: THIS CODE NEEDS AN UPDATE - ADOPT IT TO THE LATEST LUDP CODE (LINUX/Local/LocalUDP/...)
 *       Also considering LUDP_RESPONSE_TIMEOUT versus TS_RESPONSE_TIME_OUT.
 *
 * It sends REQUESTS to device id 44 which is on a SWBB bus connected to the
 * Tunnel, and receives replies back.
 * Device 44 is the example:
 * OWPJON/ARDUINO/Local/SoftwareBitBang/DeviceGeneric/
 *
 * This demonstrates the concept with a process on a PC or a RPI transparently
 * being part of a SWBB bus via a Tunnel. The Tunnel/Surrogate will "phone home"
 * to the RemoteWorker to create a permanent link.
 *
 * @see https://github.com/gioblu/PJON/blob/master/examples/LINUX/Local/EthernetTCP/RemoteWorker/Transmitter/Transmitter.cpp
 *
 * @verbatim
 * OneWire PJON Generic "OWPG" scheme:
 *   Server e.g. linux machine or raspi
 *      OWPJON/LINUX/Local/LocalUDP/RemoteWorker/DeviceGeneric/
 *      OWPJON/LINUX/Local/ThroughSerial/RemoteWorker/DeviceGeneric/ (this sketch)
 *   Tunnel(er) similar to 1wire master (similar cause we are on a multi-master bus) e.g. AVR
 *      OWPJON/ARDUINO/Local/SoftwareBitBang/Tunneler/BlinkingSwitch/
 *      OWPJON/ARDUINO/Local/SoftwareBitBang/Tunneler/BlinkingSwitch_SWBB-TS/
 *      OWPJON/ARDUINO/Local/ThroughSerial/SoftwareBitBangSurrogate/Surrogate/ (obsolete)
 *   Devices e.g. AVR
 *      OWPJON/ARDUINO/Local/SoftwareBitBang/DeviceGeneric/
 *      OWPJON/ARDUINO/Local/SoftwareBitBang/OWP_DG_LCD_Sensors/
 *      ...
 *
 * Compatible with: ubuntu 14.04, raspi
 *
 * Compile wiringPi using:
 * $ git clone git://git.drogon.net/wiringPi
 * $ cd wiringPi
 * $ ./build
 *
 * Example sending READ_INFO 0x01 to device id 44:
 * $ printf "\x01" | sudo ./owpshell /dev/ttyACM0 9600 44
 * owp:dg:v1
 * $ printf "\x11" | sudo ./owpshell /dev/ttyACM0 9600 44 | unpack.py f
 * 4.771999835968018,
 * $ printf "\x12" | sudo ./owpshell /dev/ttyACM0 9600 44 | unpack.py f
 * 28.02459144592285,
 * More info can be found in @ref OWPJON/README.md.
 *
 * Thanks to:
 * gioblu - PJON 11.0 and support
 *          @see https://www.pjon.org/
 *          @see https://github.com/gioblu/PJON
 * fredilarsen - support
 * @endverbatim
 */

// For printf used below
#include <stdio.h>
// PJON library
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
// RPI serial interface
#include <wiringPi.h>

#ifndef RPI
#define RPI true
#endif

#define TS_RESPONSE_TIME_OUT 35000
/* Maximum accepted timeframe between transmission and synchronous
   acknowledgement. This timeframe is affected by latency and CRC computation.
   Could be necessary to higher this value if devices are separated by long
   physical distance and or if transmitting long packets. */

//// Define one of these modes
//#define ETCP_SINGLE_DIRECTION
////#define ETCP_SINGLE_SOCKET_WITH_ACK

#define PJON_INCLUDE_TS true // Include only ThroughSerial
#include <PJON.h>

//PJON<EthernetTCP> bus(45);
PJON<ThroughSerial> bus(45);

//bool EXIT = false;

#define BLOCK_SIZE 256
//uint8_t buffer[BLOCK_SIZE];
char buffer[BLOCK_SIZE];

static void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info)
{
  fwrite(payload, sizeof(char), length, stdout);
  printf("\n");  // print newline - causes also a flush

//  EXIT = true;
};

void loop()
{
  bus.update();
  bus.receive(1000);
  //bus.receive();

//  // Show the number of sockets created after startup
//  // (Try disconnecting the Ethernet cable for a while to see it increase when reconnected.)
//  static uint32_t last = millis();
//  if (millis() - last > 5000) {
//    last = millis();
//    printf("CONNECT COUNT: %d\n", bus.strategy.link.get_connection_count());
//  }
}

int main(int argc, char* argv[]) // or char** argv
{
  // Welcome to RemoteWorker 1 (Transmitter)
//  bus.strategy.link.set_id(bus.device_id());
//bus.strategy.link.keep_connection(true);
//#ifdef ETCP_SINGLE_DIRECTION
//  bus.strategy.link.single_initiate_direction(true);
//#elif ETCP_SINGLE_SOCKET_WITH_ACK
//  bus.strategy.link.single_socket(true);
//#endif
  uint32_t baud_rate = atoi(argv[2]);
  // Opening serial...
  int s = serialOpen(argv[1], baud_rate);
  if(int(s) < 0) {
    printf("Serial open fail!\n");
    return 1;
  }
  // Setting serial...
  bus.strategy.set_serial(s);
  bus.strategy.set_baud_rate(baud_rate);
  bus.set_receiver(receiver_function);
//  bus.strategy.link.start_listening();
//  bus.set_synchronous_acknowledge(false);

  // Opening bus...
  bus.begin();
  // Attempting to send a packet...
  //bus.send(44, "B", 1);
  size_t l;
  if(argc < 5)  // get data to send from stdin or command line (stdin allows \0)
    //l = fread(buffer, BLOCK_SIZE, sizeof(char), stdin);
    l = fread(buffer, sizeof(*buffer), sizeof(buffer)/sizeof(*buffer), stdin);
  else {
    strcpy(buffer, argv[4]);
    l = strlen(argv[4]);
  }
  for(int i = 0; i < 3; ++i) {  // try 3 times (using timeout), then exit
    //bus.send(argv[3][0], buffer, l);
    bus.send(atoi(argv[3]), buffer, l);
    // Attempting to roll bus...
    //bus.update();
    // Attempting to receive from bus...
    //bus.receive();
    // Success!

    //while (true) loop();
    //while (!EXIT) loop();

    for(int j = 0; j < 5000; ++j) {  // do timeout 5000 times ~ 5s
      bus.update();
// TODO: use error handler callback
      switch (bus.receive(1000)) {   // 1ms timeout
      case PJON_ACK:
        return 0;
        break;
//      case PJON_NAK:   // re-try; send data again
//        break;
//      case PJON_BUSY:
//        // ...
//        break;
//      case PJON_FAIL:
//        // ...
//        break;
      default:
        break;
      }
    }
  }
  return 0;
}
