/*
  Blink

  see File > Examples > 01.Basics > Blink

  Usage:
  http://dokuwiki.ehajo.de/bausaetze:gscheiduino:anleitung

  Program:
  http://dokuwiki.ehajo.de/artikel:arduino-boardmanager
  1. Add in File > Preferences > Additional Board manager URLs:
     http://www.ehajo.de/package_ehajo_index.json
  2. Add board in Tools > Board: ... > Board Manager...:
     search 'gscheiduino'
  3. Select 'Takt', 'Prozessor', etc. and Upload by pressing
     RST (reset) on the board manually
  4. Closing the jumper on the board puts it into the AVR mkII
     mode and thus allows to the Burn Bootloader (and by that
     set flags)
  5. Running the Arduino IDE as 'sudo' might be needed especially
     for the AVR mkII mode (needed for burning bootloaders e.g.)
  5. Basically Arduino/Genuino Uno or Nano as well as MiniCore
     (for user specific settings/flags) should work as well for 328
     ... what about tiny's, etc.?
*/

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(100);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(100);                       // wait for a second
}
