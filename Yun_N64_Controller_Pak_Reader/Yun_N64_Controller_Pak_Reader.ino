/**********************************************************************************
Nintendo 64 Controller Pak Reader for Arduino (Yun, Uno+SDcard)

https://github.com/sanni/cartreader/issues/16

Author: sanni, DrTrigon
Date: 2018-04-24
Version: V1B

SD lib:
  https://github.com/greiman/SdFat
Needs EEPROMAnything.h copied into sketch folder:
  https://github.com/sanni/cartreader/blob/master/Cart_Reader/EEPROMAnything.h

Pinout:
  https://github.com/sanni/cartreader/issues/16#issuecomment-383758731
  view on controller connector:
    _____
   /     \    1: GND        -> Arduino GND
  | 1 2 3 |   2: DATA       -> Arduino Pin 2
  |_______|   3: VCC (3.3V) -> Arduino 3.3V

// TODO: note about how to format usb stick

Thanks to:
Andrew Brown/Peter Den Hartog - N64 send/get functions
Shaun Taylor - address/data CRC functions

**********************************************************************************/

/******************************************
Build Configuration
******************************************/
#define ARDUINO_YUN    // switch to Arduino Yun SDcard/USBstick handling
//#define ENABLE_WRITE   // be cautious not to overwrite your data
//#define ENABLE_EEPROM  // enable use of EEPROM (for unique ids)
#define EEPROM_SIZE_EMULATED  8

/******************************************
Pinout
******************************************/
//N64 Controller 3.3V to Arduino 3.3V
//N64 Controller Data to Arduino Uno Pin 2
//N64 Controller GND to Arduino GND
//1K Resistor from Arduino Pin 2 to 3.3V

/******************************************
Libraries
*****************************************/
// SD Card
#ifndef ARDUINO_YUN
#include <SdFat.h>
#define chipSelectPin 10
SdFat sd;
SdFile myFile;
#else
#include <Bridge.h>
#include <Process.h>
#include <FileIO.h>  // on Yun supports SD cards and USB sticks
#endif

// AVR Eeprom
#ifdef ENABLE_EEPROM
#include <EEPROM.h>
#include "EEPROMAnything.h"
#endif

/******************************************
Defines
*****************************************/
// Pin for controller data
#define N64_PIN 2

// these two macros set arduino pin 2 to input or output, which with an
// external 1K pull-up resistor to the 3.3V rail, is like pulling it high or
// low. These operations translate to 1 op code, which takes 2 cycles
#define N64_HIGH DDRD &= ~0x04
#define N64_LOW DDRD |= 0x04
#define N64_QUERY (PIND & 0x04)

/******************************************
Variables
*****************************************/
// 256 bits of received Controller data
char N64_raw_dump[257];
// Array that holds one Controller Pak block of 32 bytes
byte myBlock[33];

//remember folder number to create a new folder for every save
int foldern;
#ifndef ARDUINO_YUN
char fileName[26];
#else
char fileName[46];
#endif

// Variable to count errors
unsigned long writeErrors;

// For incoming serial data
int incomingByte;

// Array that holds the data
byte sdBuffer[32];

/******************************************
Setup
*****************************************/
void setup() {
  // Communication with controller on this pin
  // Don't remove these lines, we don't want to push +5V to the controller
  digitalWrite(N64_PIN, LOW);
  pinMode(N64_PIN, INPUT);

  // Serial Begin (and wait for port to open)
  Serial.begin(9600);
#ifdef ARDUINO_YUN
  while (!Serial)
    delay(10);     // will pause Zero, Leonardo, etc until serial console opens
#endif
  Serial.println(F("N64 Controller Pak Reader - 2018 sanni drtrigon"));
  Serial.println("");

#ifdef ENABLE_EEPROM
  // Read current folder number out of eeprom
  EEPROM_readAnything(0, foldern);
#endif

  // Init SD card
#ifndef ARDUINO_YUN
  if (!sd.begin(chipSelectPin, SPI_FULL_SPEED)) {
    Serial.println(F("SD Error"));
    while (1);
  }
#else
  // Bridge startup
  Bridge.begin();

  FileSystem.begin();
#endif

  // Print SD Info
#ifndef ARDUINO_YUN
  Serial.print(F("SD Card: "));
  Serial.print(sd.card()->cardSize() * 512E-9);
  Serial.print(F("GB FAT"));
  Serial.println(int(sd.vol()->fatType()));
  Serial.println("");
  Serial.println(F("Files:"));
  // Print all files in root of SD
  Serial.println(F("Name - Size"));
  sd.vwd()->rewind();
  sd.mkdir("MPK", true);
  sd.chdir("MPK");
  while (myFile.openNext(sd.vwd(), O_READ)) {
    if (myFile.isHidden()) {
    }
    else {
      if (myFile.isDir()) {
        // Indicate a directory.
        Serial.write('/');
      }
      myFile.printName(&Serial);
      Serial.write(' ');
      myFile.printFileSize(&Serial);
      Serial.println();
    }
    myFile.close();
  }
#else
  Process shell;  // initialize a new process
  Serial.print(F("SD Card / USB stick: "));
//  shell.runShellCommand(F("/bin/df --output=size,fstype --block-size=G /mnt/sda1"));  // command you want to run
////  shell.begin(F("df"));
////  shell.addParameter(F("--output=size,fstype --block-size=G /mnt/sda1"));
////  shell.addParameter(F("--help"));
////  shell.run();  // run the command
//  // while there's any characters coming back from the
//  // process, print them to the serial monitor:
//  while (shell.available() > 0) {
//    Serial.print(shell.readString());
////    Serial.print(shell.read());
//  }
//  Serial.print(shell.exitValue());
//  shell.close();
//  Serial.println("");
  Serial.println("no idea ...");
  Serial.println(F("Files:"));
  // Print all files in root of SD
  //Serial.println(F("Name - Size"));
  Serial.println(F("Perm - Group - User - Size - Date - Name"));
  FileSystem.mkdir("/mnt/sda1/MPK/");
  // there is a File.rewindDirectory() structure for Yun, but too complex to use here; use Process
  //shell.runShellCommand(F("/usr/bin/stat -c '%n %s' /mnt/sda1/MPK/*"));  // command you want to run
  shell.runShellCommand(F("/bin/ls -l /mnt/sda1/MPK/"));  // command you want to run
  // while there's any characters coming back from the
  // process, print them to the serial monitor:
  while (shell.available() > 0) {
    Serial.print(shell.readString());
  }
  shell.close();
#endif
  Serial.println("");

#ifndef ENABLE_EEPROM
  // init EEPROM emulation if not existing already
  if (!FileSystem.exists("/mnt/sda1/MPK/EEPROM")) {
#ifndef ARDUINO_YUN
    // Change to root
    sd.chdir("/");
    // Change to MPK directory
    sd.chdir("MPK");
    if (!myFile.open("EEPROM", O_RDWR | O_CREAT)) {
#else
    File myFile = FileSystem.open("/mnt/sda1/MPK/EEPROM", FILE_WRITE);
    if (!myFile) {
#endif
      Serial.println(F("SD Error"));
      while (1);
    }
    for (unsigned int i = 0; i < EEPROM_SIZE_EMULATED; i++) {
#ifndef ARDUINO_YUN
      // no idea...
#else
      myFile.seek(i);
#endif
      myFile.write(0);
    }
    myFile.close();
  }

  // Read current folder number out of eeprom
  EEPROM_readAnything(0, foldern);
#endif
}

/******************************************
Helper functions
*****************************************/
// Prompt a filename from the Serial Monitor
#ifdef ENABLE_WRITE
void getfilename() {
  Serial.println(F("Please enter a filename in 8.3 format: _"));
  Serial.println("");
  while (Serial.available() == 0) {}
  String strBuffer;
  strBuffer = Serial.readString();
  strBuffer.toCharArray(fileName, 13);
}
#endif

#ifndef ENABLE_EEPROM
// Emulate EEPROM write function with help of storage on SD
template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
  const byte* p = (const byte*)(const void*)&value;
  unsigned int i;
  //open file on sd card
#ifndef ARDUINO_YUN
  // Change to root
  sd.chdir("/");
  // Change to MPK directory
  sd.chdir("MPK");
  if (!myFile.open("EEPROM", O_RDWR | O_CREAT)) {
#else
  File myFile = FileSystem.open("/mnt/sda1/MPK/EEPROM", FILE_WRITE);
  if (!myFile) {
#endif
    Serial.println(F("EEPROM/SD Error"));
    while (1);
  }
  // EEPROM_writeAnything emulation
  for (i = 0; i < sizeof(value); i++) {
#ifndef ARDUINO_YUN
    // no idea...
#else
    myFile.seek(ee++);
#endif
    myFile.write(*p++);
  }
  myFile.close();
  return i;
}

// Emulate EEPROM read function with help of storage on SD
template <class T> int EEPROM_readAnything(int ee, T& value)
{
  byte* p = (byte*)(void*)&value;
  unsigned int i;
  //open file on sd card
#ifndef ARDUINO_YUN
  // Change to root
  sd.chdir("/");
  // Change to MPK directory
  sd.chdir("MPK");
  if (!myFile.open("EEPROM", O_READ)) {
#else
  File myFile = FileSystem.open("/mnt/sda1/MPK/EEPROM", FILE_READ);
  if (!myFile) {
#endif
    Serial.println(F("EEPROM/SD Error"));
    while (1);
  }
  // EEPROM_readAnything emulation
  for (i = 0; i < sizeof(value); i++) {
#ifndef ARDUINO_YUN
    // no idea...
#else
    myFile.seek(ee++);
#endif
    *p++ = myFile.read();
  }
  myFile.close();
  return i;
}
#endif

/******************************************
CRC Functions
*****************************************/
static word addrCRC(word address) {
  // CRC table
  word xor_table[16] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x15, 0x1F, 0x0B, 0x16, 0x19, 0x07, 0x0E, 0x1C, 0x0D, 0x1A, 0x01 };
  word crc = 0;
  // Make sure we have a valid address
  address &= ~0x1F;
  // Go through each bit in the address, and if set, xor the right value into the output
  for (int i = 15; i >= 5; i--) {
    // Is this bit set?
    if ( ((address >> i) & 0x1)) {
      crc ^= xor_table[i];
    }
  }
  // Just in case
  crc &= 0x1F;
  // Create a new address with the CRC appended
  return address | crc;
}

static byte dataCRC(byte *data) {
  byte ret = 0;
  for (byte i = 0; i <= 32; i++) {
    for (byte j = 7; j >= 0; j--) {
      int tmp = 0;
      if (ret & 0x80) {
        tmp = 0x85;
      }
      ret <<= 1;
      if ( i < 32 ) {
        if (data[i] & (0x01 << j)) {
          ret |= 0x1;
        }
      }
      ret ^= tmp;
    }
  }
  return ret;
}

/******************************************
N64 Controller Protocol Functions
*****************************************/
void N64_send(unsigned char *buffer, char length) {
  // Send these bytes
  char bits;
  bool bit;

  // This routine is very carefully timed by examining the assembly output.
  // Do not change any statements, it could throw the timings off
  //
  // We get 16 cycles per microsecond, which should be plenty, but we need to
  // be conservative. Most assembly ops take 1 cycle, but a few take 2
  //
  // I use manually constructed for-loops out of gotos so I have more control
  // over the outputted assembly. I can insert nops where it was impossible
  // with a for loop

  asm volatile (";Starting outer for loop");
  outer_loop:
  {
    asm volatile (";Starting inner for loop");
    bits = 8;
    inner_loop:
    {
      // Starting a bit, set the line low
      asm volatile (";Setting line to low");
      N64_LOW; // 1 op, 2 cycles

      asm volatile (";branching");
      if (*buffer >> 7) {
        asm volatile (";Bit is a 1");
        // 1 bit
        // remain low for 1us, then go high for 3us
        // nop block 1
        asm volatile ("nop\nnop\nnop\nnop\nnop\n");

        asm volatile (";Setting line to high");
        N64_HIGH;

        // nop block 2
        // we'll wait only 2us to sync up with both conditions
        // at the bottom of the if statement
        asm volatile ("nop\nnop\nnop\nnop\nnop\n"
                      "nop\nnop\nnop\nnop\nnop\n"
                      "nop\nnop\nnop\nnop\nnop\n"
                      "nop\nnop\nnop\nnop\nnop\n"
                      "nop\nnop\nnop\nnop\nnop\n"
                      "nop\nnop\nnop\nnop\nnop\n"
                     );

      }
      else {
        asm volatile (";Bit is a 0");
        // 0 bit
        // remain low for 3us, then go high for 1us
        // nop block 3
        asm volatile ("nop\nnop\nnop\nnop\nnop\n"
                      "nop\nnop\nnop\nnop\nnop\n"
                      "nop\nnop\nnop\nnop\nnop\n"
                      "nop\nnop\nnop\nnop\nnop\n"
                      "nop\nnop\nnop\nnop\nnop\n"
                      "nop\nnop\nnop\nnop\nnop\n"
                      "nop\nnop\nnop\nnop\nnop\n"
                      "nop\n");

        asm volatile (";Setting line to high");
        N64_HIGH;

        // wait for 1us
        asm volatile ("; end of conditional branch, need to wait 1us more before next bit");

      }
      // end of the if, the line is high and needs to remain
      // high for exactly 16 more cycles, regardless of the previous
      // branch path

      asm volatile (";finishing inner loop body");
      --bits;
      if (bits != 0) {
        // nop block 4
        // this block is why a for loop was impossible
        asm volatile ("nop\nnop\nnop\nnop\nnop\n"
                      "nop\nnop\nnop\nnop\n");
        // rotate bits
        asm volatile (";rotating out bits");
        *buffer <<= 1;

        goto inner_loop;
      } // fall out of inner loop
    }
    asm volatile (";continuing outer loop");
    // In this case: the inner loop exits and the outer loop iterates,
    // there are /exactly/ 16 cycles taken up by the necessary operations.
    // So no nops are needed here (that was lucky!)
    --length;
    if (length != 0) {
      ++buffer;
      goto outer_loop;
    } // fall out of outer loop

  }
}

void N64_stop() {
  // send a single stop (1) bit
  // nop block 5
  asm volatile ("nop\nnop\nnop\nnop\n");
  N64_LOW;
  // wait 1 us, 16 cycles, then raise the line
  // 16-2=14
  // nop block 6
  asm volatile ("nop\nnop\nnop\nnop\nnop\n"
                "nop\nnop\nnop\nnop\nnop\n"
                "nop\nnop\nnop\nnop\n");
  N64_HIGH;
}

void N64_get(word bitcount) {
  // listen for the expected bitcount/8 bytes of data back from the controller and
  // blast it out to the N64_raw_dump array, one bit per byte for extra speed.
  asm volatile (";Starting to listen");
  unsigned char timeout;
  char *bitbin = N64_raw_dump;

  // Again, using gotos here to make the assembly more predictable and
  // optimization easier (please don't kill me)
  read_loop:
  timeout = 0x3f;
  // wait for line to go low
  while (N64_QUERY) {
    if (!--timeout)
      return;
  }
  // wait approx 2us and poll the line
  asm volatile (
                "nop\nnop\nnop\nnop\nnop\n"
                "nop\nnop\nnop\nnop\nnop\n"
                "nop\nnop\nnop\nnop\nnop\n"
                "nop\nnop\nnop\nnop\nnop\n"
                "nop\nnop\nnop\nnop\nnop\n"
                "nop\nnop\nnop\nnop\nnop\n"
  );
  *bitbin = N64_QUERY;
  ++bitbin;
  --bitcount;
  if (bitcount == 0)
    return;

  // wait for line to go high again
  // it may already be high, so this should just drop through
  timeout = 0x3f;
  while (!N64_QUERY) {
    if (!--timeout)
      return;
  }
  goto read_loop;
}

/******************************************
N64 Controller Pak Functions
*****************************************/
// read 32bytes from controller pak
void readBlock(word myAddress) {
  // Calculate the address CRC
  word myAddressCRC = addrCRC(myAddress);

  // Read Controller Pak command
  unsigned char command[] = {0x02};
  // Address Command
  unsigned char addressHigh[] = {myAddressCRC >> 8};
  unsigned char addressLow[] = {myAddressCRC & 0xff};

  // don't want interrupts getting in the way
  noInterrupts();
  // send those 3 bytes
  N64_send(command, 1);
  N64_send(addressHigh, 1);
  N64_send(addressLow, 1);
  N64_stop();
  // read in data
  N64_get(256);
  // end of time sensitive code
  interrupts();

  // Empty N64_raw_dump into myBlock
  for (word i = 0; i < 256; i += 8) {
    boolean byteFlipped[9];

    // Flip byte order
    byteFlipped[0] = N64_raw_dump[i + 7];
    byteFlipped[1] = N64_raw_dump[i + 6];
    byteFlipped[2] = N64_raw_dump[i + 5];
    byteFlipped[3] = N64_raw_dump[i + 4];
    byteFlipped[4] = N64_raw_dump[i + 3];
    byteFlipped[5] = N64_raw_dump[i + 2];
    byteFlipped[6] = N64_raw_dump[i + 1];
    byteFlipped[7] = N64_raw_dump[i + 0];

    // Join bits into one byte
    unsigned char myByte = 0;
    for (byte j = 0; j < 8; ++j) {
      if (byteFlipped[j]) {
        myByte |= 1 << j;
      }
    }
    // Save byte into block array
    myBlock[i / 8] = myByte;

  }
}

// reads the MPK file to the sd card
void readMPK() {
#ifndef ARDUINO_YUN
  // Change to root
  sd.chdir("/");
  // Change to MPK directory
  sd.chdir("MPK");
#endif

  // Get name, add extension and convert to char array for sd lib
  EEPROM_readAnything(0, foldern);
#ifndef ARDUINO_YUN
  sprintf(fileName, "%d", foldern);
#else
  sprintf(fileName, "/mnt/sda1/MPK/%d", foldern);
#endif
  strcat(fileName, ".mpk");

  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  //open file on sd card
#ifndef ARDUINO_YUN
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
#else
  File myFile = FileSystem.open(fileName, FILE_WRITE);
  if (!myFile) {
#endif
    Serial.println(F("SD Error"));
    while (1);
  }

  Serial.println(F("Please wait..."));

  // Controller paks, which all have 32kB of space, are mapped between 0x0000 – 0x7FFF
  for (word i = 0x0000; i < 0x8000; i += 32) {
    // Read one block of the Controller Pak into array myBlock
    readBlock(i);
    // Write block to SD card
    for (byte j = 0; j < 32; j++) {
      myFile.write(myBlock[j]);
      Serial.print(myBlock[j], HEX);  // read data
    }
    Serial.println(i, HEX);  // progress
  }
  // Close the file:
  myFile.close();
  Serial.print(F("Saved as /MPK/"));
  Serial.println(fileName);
}

#ifdef ENABLE_WRITE
void writeMPK() {
#ifndef ARDUINO_YUN
  // Change to root
  sd.chdir("/");
  // Change to MPK directory
  sd.chdir("MPK");
#endif

  Serial.print(F("Writing "));
  Serial.print(fileName);
  Serial.println(F("..."));

  // Open file on sd card
#ifndef ARDUINO_YUN
  if (myFile.open(fileName, O_READ)) {
#else
  File myFile = FileSystem.open(fileName, FILE_READ);
  if (myFile) {
#endif
    for (word myAddress = 0x0000; myAddress < 0x8000; myAddress += 32) {
      // Read 32 bytes into SD buffer
      myFile.read(sdBuffer, 32);

      // Calculate the address CRC
      word myAddressCRC = addrCRC(myAddress);

      // Write Controller Pak command
      unsigned char command[] = {0x03};
      // Address Command
      unsigned char addressHigh[] = {myAddressCRC >> 8};
      unsigned char addressLow[] = {myAddressCRC & 0xff};

      // don't want interrupts getting in the way
      noInterrupts();
      // Send write command
      N64_send(command, 1);
      // Send block number
      N64_send(addressHigh, 1);
      N64_send(addressLow, 1);
      // Send data to write
      N64_send(sdBuffer, 32);
      // Send stop
      N64_stop();
      // Enable interrupts
      interrupts();

      Serial.println(myAddress, HEX);  // progress
    }
    // Close the file:
    myFile.close();
    Serial.println(F("Done"));

  }
  else {
    Serial.println(F("SD Error"));
    while (1);
  }
}

// verifies if write was successful
void verifyMPK() {
  writeErrors = 0;

  Serial.println(F("Verifying..."));

  //open file on sd card
#ifndef ARDUINO_YUN
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
#else
  File myFile = FileSystem.open(fileName, FILE_READ);
  if (!myFile) {
#endif
    Serial.println(F("SD Error"));
    while (1);
  }

  // Controller paks, which all have 32kB of space, are mapped between 0x0000 – 0x7FFF
  for (word i = 0x0000; i < 0x8000; i += 32) {
    // Read one block of the Controller Pak into array myBlock
    readBlock(i);
    // Check against file on SD card
    for (byte j = 0; j < 32; j++) {
      if (myFile.read() != myBlock[j]) {
        writeErrors++;
      }
      Serial.println(i, HEX);  // progress
    }
  }
  // Close the file:
  myFile.close();
  if (writeErrors == 0) {
    Serial.println(F("OK"));
  }
  else {
    Serial.print(writeErrors);
    Serial.println(F(" errors"));
  }
}
#endif

/******************************************
Main loop
*****************************************/
void loop() {
  // Print menu to serial monitor
  Serial.println(F("Menu:"));
  Serial.println(F("(0)Read (1)Write"));
  Serial.println("");

  // Wait for user input
  Serial.println(F("Please enter a single digit or letter: _"));
  Serial.println("");

  while (Serial.available() == 0) {
  }

  // Read the incoming byte:
  incomingByte = Serial.read();

  // Execute user choice
  switch (incomingByte) {
    case 48:  // '0'
      Serial.println(F("Reading Controller Pak"));
      readMPK();
      break;

    case 49:  // '1'
#ifdef ENABLE_WRITE
      Serial.println(F("Writing Controller Pak"));
      getfilename();
      writeMPK();
      verifyMPK();
#else
      Serial.println(F("Disabled for security - set ENABLE_WRITE to enable."));
#endif
      break;

  }
  Serial.println("");
}

/******************************************
End of File
******************************************/
