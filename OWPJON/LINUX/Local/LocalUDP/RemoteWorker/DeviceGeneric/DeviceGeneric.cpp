/* This sketch is a modified copy of the examples:
LINUX/Local/EthernetTCP/RemoteWorker/Transmitter respective
LINUX/Local/ThroughSerial/RemoteWorker/DeviceGeneric

In this context it is a "RemoteWorker" for a corresponding "Surrogate" on a
SoftwareBitBang bus, connected to the Surrogate via Ethernet TCP.
The surrogate will connect to this device, allowing this device to talk to
all devices on the SWBB bus by their ids, and receiving all packets sent to
the id if this device on the SWBB bus.

It sends BLINK to device id 44 which is on a SWBB bus connected to the
Surrogate, and receives BLINK replies back.
Device 44 is the example:
ARDUINO/Local/SoftwareBitBang/BlinkWithResponse/Receiver.

This demonstrates the concept with a process on a PC or a RPI transparently
being part of a SWBB bus via a Surrogate. The Surrogate will "phone home"
to the RemoteWorker to create a permanent link.
The two alternative modes differ in the speed and the number of sockets used.
ETCP_SINGLE_DIRECTION is fastest and recommended if not on limited hardware.

Ethernet strategies and related concepts are contributed by Fred Larsen.

Example sending READ_INFO 0x01 to device id 44:
$ printf "\x01" | ./owpshell - - 44
*/

#define PJON_INCLUDE_LUDP
#include <PJON.h>

// <Strategy name> bus(selected device id)
PJON<LocalUDP> bus(45);

bool EXIT = false;

#define BLOCK_SIZE 256
//uint8_t buffer[BLOCK_SIZE];
char buffer[BLOCK_SIZE];

//static void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info)
void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info)
{
  fwrite(payload, sizeof(char), length, stdout);
  printf("\n");  // print newline - causes also a flush

  EXIT = true;
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
  //bus.send(argv[3][0], buffer, l);
  bus.send(atoi(argv[3]), buffer, l);
  // Attempting to roll bus...
  bus.update();
  // Attempting to receive from bus...
  bus.receive();
  // Success!

  time_t timer0, timer1;
  time(&timer0);  /* get current time; same as: timer = time(NULL)  */
  time(&timer1);

  //while (true) loop();
  //while (!EXIT) loop();
  while ((!EXIT) && (difftime(timer1, timer0) < 5)) {
    loop();
    time(&timer1);
  }
  return 0;
}
