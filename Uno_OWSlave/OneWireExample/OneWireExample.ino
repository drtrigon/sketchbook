/*
** http://forum.arduino.cc/index.php?topic=65706.msg1756727#msg1756727
** ...
*/

#include <OneWireSlave.h>

char rom[8] = {0x01, 0xAD, 0xDA, 0xCE, 0x0F, 0x00, 0x00, 0x00};
OneWireSlave ds(8);

void setup() {
    ds.setRom(rom);
}

void loop() {
    ds.waitForRequest(false);
}

