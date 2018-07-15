/**
 * @brief Example-Code for a generic Server (e.g. Sensor, Display)
 *
 * @file OWPJON/LINUX/Local/LocalUDP/RemoteWorker/DeviceGeneric/DeviceGeneric.cpp
 *
 * @author drtrigon
 * @date 2018-07-06
 * @version 1.0
 *   @li first version derived from
 *       @ref OWPJON/LINUX/Local/ThroughSerial/RemoteWorker/DeviceGeneric/DeviceGeneric.cpp
 *
 * @verbatim
 * OneWire PJON Generic "OWPG" scheme:
 *   Server e.g. linux machine or raspi
 *      OWPJON/LINUX/Local/LocalUDP/RemoteWorker/DeviceGeneric/ (this sketch)
 *      OWPJON/LINUX/Local/ThroughSerial/RemoteWorker/DeviceGeneric/
 *   Tunnel(er) similar to 1wire master (similar cause we are on a multi-master bus) e.g. AVR
 *      OWPJON/ARDUINO/Local/SoftwareBitBang/Tunneler/BlinkingSwitch/
 *      OWPJON/ARDUINO/Local/SoftwareBitBang/Tunneler/BlinkingSwitch_SWBB-TS/
 *      OWPJON/ARDUINO/Local/ThroughSerial/SoftwareBitBangSurrogate/Surrogate/ (obsolete)
 *   Devices e.g. AVR
 *      OWPJON/ARDUINO/Local/SoftwareBitBang/DeviceGeneric/
 *      OWPJON/ARDUINO/Local/SoftwareBitBang/OWP_DG_LCD_Sensors/
 *      ...
 *
 * Compatible with: ubuntu 14.04, raspi (look at the Makefile)
 *
 * Example sending READ_INFO 0x01 to device id 44:
 * $ printf "\x01" | ./owpshell - - 44
 * owp:dg:v1
 * $ printf "\x11" | ./owpshell - - 44 | ../../../ThroughSerial/RemoteWorker/DeviceGeneric/unpack.py f
 * 4.771999835968018,
 * $ printf "\x12" | ./owpshell - - 44 | ../../../ThroughSerial/RemoteWorker/DeviceGeneric/unpack.py f
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

// use different ID for different machines (linux, raspi, ...) - see Makefile
#ifndef ID
#define ID  45  // default ID for linux (raspi uses 46)
#endif

// Avoid using '#define RPI true' as this requires wiringPi !

#define PJON_INCLUDE_LUDP
#include <PJON.h>

// <Strategy name> bus(selected device id)
PJON<LocalUDP> bus(ID);

//bool EXIT = false;

#define BLOCK_SIZE 256
//uint8_t buffer[BLOCK_SIZE];
char buffer[BLOCK_SIZE];

//static void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info)
void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info)
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
  if((argc == 2) && (strcmp(argv[1], "--id") == 0)) {
    printf("ID: %i\n", ID);
    return 0;
  }

  // Welcome to RemoteWorker 1 (Transmitter)
  bus.set_receiver(receiver_function);

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

//    time_t timer0, timer1;
//    time(&timer0);  /* get current time; same as: timer = time(NULL)  */

    //while (true) loop();
    //while (!EXIT) loop();
//    while (!EXIT) {
//      loop();
//      time(&timer1);
//      EXIT = EXIT || (difftime(timer1, timer0) > 5.);  // 5s timeout
//    }

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
