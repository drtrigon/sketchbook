#include <PJON.h>

// <Strategy name> bus(selected device id)
PJON<OverSampling> bus(44);

void setup() {
  Serial.begin(115200);

  pinMode(13, OUTPUT);
  digitalWrite(13, LOW); // Initialize LED 13 to be off

  /* When using more than one pin always use pins connected to
     a different port group to avoid cross-talk. */
  bus.strategy.set_pins(7, 12);

  bus.set_synchronous_acknowledge(false);
  bus.set_receiver(receiver_function);
  bus.begin();

  bus.send_repeatedly(45, "B", 1, 1000000);
};

void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info) {
  /* Make use of the payload before sending something, the buffer where payload points to is
     overwritten when a new message is dispatched */
  if((char)payload[0] == 'B') {
    digitalWrite(13, HIGH);
    delay(5);
    digitalWrite(13, LOW);
    delay(5);
  }
}

void loop() {
  bus.receive();
  bus.update();
};
