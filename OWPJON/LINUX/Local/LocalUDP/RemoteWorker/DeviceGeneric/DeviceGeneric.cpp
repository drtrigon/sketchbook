/**
 * @brief Example-Code for a generic Server (e.g. Sensor, Display)
 *
 * @file OWPJON/LINUX/Local/LocalUDP/RemoteWorker/DeviceGeneric/DeviceGeneric.cpp
 *
 * @author drtrigon
 * @date 2018-07-31
 * @version 1.2
 *   @li make it more reliable by considering
 *     @li network delay/latency LUDP_RESPONSE_TIMEOUT (especially when on battery powered wifi)
 *     @li a lot of switches in row (3 for outdoor part of bus)
 *     @li possibility of blocked lora channels/freq (increasing delays)
 *   @li having (u)sleep in order to reduce cpu load and give os and hardware time
 *   @li clean bus from old (hanging) messages
 * @version 1.1
 *   @li use different IDs for linux/ubuntu and raspi builds (use `make raspi`)
 *   @li status and error handling added
 *   @li debug output to stderr added
 *   @li retrying disabled in favour of the former changes
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
 *      OWPJON/ARDUINO/Local/SoftwareBitBang/OWP_DG_1w-adaptor/
 *      ...
 *
 * Reliability and connection issues:
 *   It seems that battery powering the notebook (connected via wifi) has heavy influence
 *   on UDP packet send delay (and thus PJON health).
 *   Increase LUDP_RESPONSE_TIMEOUT to 1000000 (1s) in @ref libraries/PJON/src/strategies/LocalUDP/LocalUDP.h
 *   Apply patch: .../sketchbook$ patch -p0 < OWPJON/20180731-udp_latency.patch
 *   Also adopting poll timings might help ... for more info in general @ee https://github.com/gioblu/PJON/issues/222
 *
 * Compatible with: ubuntu 14.04, raspi (look at the Makefile)
 *   ID 45: owpshell on linux/ubuntu (testing)
 *   ID 46: owpshell on raspi .41 (productive)
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

#define BLOCK_SIZE 256
//uint8_t buffer[BLOCK_SIZE];
char buffer[BLOCK_SIZE];

//static void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info)
void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info)
{
  fwrite(payload, sizeof(char), length, stdout);
  printf("\n");  // print newline - causes also a flush
};

void error_handler(uint8_t code, uint16_t data, void *custom_pointer)
{
  if(code == PJON_CONNECTION_LOST) {
    fprintf(stderr, "Connection with device ID %i is lost.\n", bus.packets[data].content[0]);
  }
  if(code == PJON_PACKETS_BUFFER_FULL) {
    fprintf(stderr, "Packet buffer is full, has now a length of %i\n", data);
    fprintf(stderr, "Possible wrong bus configuration!\n");
    fprintf(stderr, "higher PJON_MAX_PACKETS in PJONDefines.h if necessary.\n");
  }
  if(code == PJON_CONTENT_TOO_LONG) {
    fprintf(stderr, "Content is too long, length: %i\n", data);
  }
}

/*void loop()
{
  bus.update();
  bus.receive(1000);
  //bus.receive();
}*/

int main(int argc, char* argv[]) // or char** argv
{
  if((argc == 2) && (strcmp(argv[1], "--id") == 0)) {
    printf("%s %s\n", __DATE__, __TIME__);
    printf("ID: %i\n", ID);
    return 0;
  }

  // Welcome to RemoteWorker 1 (Transmitter)
//  bus.set_receiver(receiver_function);
  bus.set_error(error_handler);

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
  uint16_t ret;
//  for(int i = 0; i < 3; ++i) {  // try 3 times (using timeout), then exit
  {
    // clean bus from old (hanging) messages
    usleep(100000);
    bus.update();
    bus.receive(1000);
  bus.set_receiver(receiver_function);

    bus.send(atoi(argv[3]), buffer, l);
    // Attempting to roll bus...
    //bus.update();
    // Attempting to receive from bus...
    //bus.receive();
    // Success!

    time_t timer0, timer1;
    time(&timer0);  /* get current time; same as: timer = time(NULL)  */
    time(&timer1);

    //while (true) loop();
    //while (!EXIT) {
    //  loop();
    //  time(&timer1);
    //  EXIT = EXIT || (difftime(timer1, timer0) > 5.);  // 5s timeout
    //}

//    for(int j = 0; j < 3000; ++j) {  // do timeout 3000 times ~ 3s
    while (difftime(timer1, timer0) < 3.) {  // 3s timeout
      usleep(10000);                 // multi-threading - give os and socket time as we have hardware buffer for UDP
      bus.update();
      ret = bus.receive(1000);       // 1ms timeout
      switch (ret) {
      case PJON_ACK:
        return 0;                    // Success!
        break;
      case PJON_NAK:
        fprintf(stderr, "NAK: %i\n", ret);
        break;
      case PJON_BUSY:   // wait 1s and restart timeout - allow bus to cool down
        fprintf(stderr, "BUSY: %i\n", ret);
        usleep(1000000);
//        j = 0;
// TODO: restric maximum number of timeout resets
        time(&timer0);  // reset timeout
        break;
      case PJON_FAIL:
      default:
        break;
      }
      time(&timer1);
    }
  }
  fprintf(stderr, "FAILED: %i\n", ret);
  return 1;  // failed
}
