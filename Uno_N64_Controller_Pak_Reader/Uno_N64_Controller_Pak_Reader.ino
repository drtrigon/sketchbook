//*********************************************************************************
/**
 * @brief Nintendo 64 Controller Pak Reader for Arduino (Uno+SD Card)
 *
 * @file Uno_N64_Controller_Pak_Reader/Uno_N64_Controller_Pak_Reader.ino
 *
 * @author sanni, drtrigon
 * @date 2018-04-24
 * @version V1C
 *
 * @see https://github.com/sanni/cartreader/issues/16
 * @see https://github.com/sanni/cartreader/blob/master/Cart_Reader/N64.ino
 * @see http://www.instructables.com/id/Use-an-Arduino-with-an-N64-controller/
 *
 * @verbatim
 * SD lib:
 *   https://github.com/greiman/SdFat
 * Needs EEPROMAnything.h copied into sketch folder:
 *   https://github.com/sanni/cartreader/blob/master/Cart_Reader/EEPROMAnything.h
 * DIY SD Card interface/adapter:
 *   http://www.instructables.com/id/Cheap-DIY-SD-card-breadboard-socket/
 *   http://www.bot-thoughts.com/2010/02/logging-data-to-sd-cards.html
 *   https://www.arduino.cc/en/Reference/SDCardNotes
 * Inspect (check/test) MPK data read:
 *   https://github.com/bryc/mempak
 *   https://rawgit.com/bryc/mempak/master/index.html
 *
 * Pinout:
 *   https://github.com/sanni/cartreader/issues/16#issuecomment-383758731
 *   N64 - view on controller connector:
 *     _____
 *    /     \    1: GND        -> Arduino GND
 *   | 1 2 3 |   2: DATA       -> Arduino Pin 2
 *   |_______|   3: VCC (3.3V) -> Arduino 3.3V
 *   (contacts start after about 5mm inside the hole)
 *   SD Card:    1: CS         -> Arduino Pin 10
 *               2: DI/MOSI    -> Arduino Pin 11
 *               3: VSS/GND    -> Arduino GND
 *               4: VDD/+3.3V  -> Arduino 3.3V
 *               5: SCK/CLK    -> Arduino Pin 13
 *               7: DO/MISO    -> Arduino Pin 12
 *     Format:
 *       $ lsblk
 *       $ sudo fdisk /dev/mmcblk0
 *         "d" untill all partitions deleted, "n" for new:
 *         Erster Sektor (2048-31116287, Vorgabe: 2048): 4096
 *         Last Sektor, +Sektoren or +size{K,M,G} (4096-31116287, Vorgabe: 31116287): +2G
 *         "w" to write changes
 *       $ sudo mkdosfs -F 16 /dev/mmcblk0p1
 *         use sketch to test SD Card: https://www.arduino.cc/en/Tutorial/CardInfo
 *   (SD Card does get recognized but writing - e.g. mkdir, open - does not work)
 *
 * AS I WAS NOT ABLE TO WRITE SUCCESSFULLY TO SD CARD I USED THE SIMPLE DUMP FEATURE:
 *   1. $ sudo miniterm.py /dev/ttyACM0 | tee n64-controllerpak-01.log
 *        in the menu chose '0' to dump to console
 *   2. open .log in text-editor (e.g. kate) and remove all lines except
 *        the ones containing the hex dump and store as it .hex - needs to be 1024 lines
 *   3. $ xxd -r -p n64-controllerpak-01.hex n64-controllerpak-01.mpk
 *   4. $ ls -la n64-controllerpak-01.mpk
 *        check size needs to be 32768 and compare the .hex with .mpk using e.g. mc
 *        finally inspect it on: https://rawgit.com/bryc/mempak/master/index.html
 *
 * Thanks to:
 * Andrew Brown/Peter Den Hartog - N64 send/get functions
 * Shaun Taylor - address/data CRC functions
 * @endverbatim
 */
//*********************************************************************************

/******************************************
Build Configuration
******************************************/
//#define ENABLE_SD      // enable use of SD Card (and Controller Pak)
//#define ENABLE_WRITE   // be cautious not to overwrite your data
//#define ENABLE_EEPROM  // enable use of EEPROM (for unique ids)
#define EEPROM_SIZE_EMULATED  8

#ifndef ENABLE_SD
#undef ENABLE_WRITE
#endif

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
#ifdef ENABLE_SD
#include <SdFat.h>
#define chipSelectPin 10  // SCK/CLK pin 13, DO/MISO pin 12, DI/MOSI pin 11, CS pin 10
SdFat sd;
SdFile myFile;

// AVR Eeprom
#ifdef ENABLE_EEPROM
#include <EEPROM.h>
#include "EEPROMAnything.h"
#endif
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
#ifdef ENABLE_SD
int foldern;
char fileName[26];

// Variable to count errors
unsigned long writeErrors;
#endif

// For incoming serial data
int incomingByte;

// Array that holds the data
byte sdBuffer[32];

// N64 Controller
String rawStr = ""; // above char array read into a string
struct {
  char stick_x;
  char stick_y;
} N64_status;
//stings that hold the buttons
String button = "N/A";

/******************************************
Setup
*****************************************/
void setup()
{
  // Communication with controller on this pin
  // Don't remove these lines, we don't want to push +5V to the controller
  digitalWrite(N64_PIN, LOW);
  pinMode(N64_PIN, INPUT);

  // Serial Begin (and wait for port to open)
  Serial.begin(9600);
  Serial.println(F("N64 Controller Pak Reader - 2018 sanni drtrigon"));
  Serial.println("");

#ifdef ENABLE_SD
#ifdef ENABLE_EEPROM
  // Read current folder number out of eeprom
  EEPROM_readAnything(0, foldern);
#endif

  // Init SD card
  if (!sd.begin(chipSelectPin, SPI_FULL_SPEED)) {
    Serial.println(F("SD Error"));
    while (1);
  }

  // Print SD Info
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
    } else {
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
  Serial.println("");

#ifndef ENABLE_EEPROM
  // Change to root
  sd.chdir("/");
  // Change to MPK directory
  sd.chdir("MPK");
  // init EEPROM emulation if not existing already
  if (!myFile.exists("EEPROM")) {
    //if (!myFile.open("EEPROM", O_READ)) {  // alternative to myFile.exists
    if (!myFile.open("EEPROM", O_RDWR | O_CREAT)) {
      Serial.println(F("EEPROM/SD Error"));
      while (1);
    }
    for (unsigned int i = 0; i < EEPROM_SIZE_EMULATED; i++) {
      myFile.seekSet(i);
      myFile.write(byte(0));
    }
    myFile.close();
  }

  // Read current folder number out of eeprom
  EEPROM_readAnything(0, foldern);
#endif
#endif

  // Initialize the gamecube controller by sending it a null byte.
  // This is unnecessary for a standard controller, but is required for the
  // Wavebird.
  unsigned char initialize = 0x00;
  noInterrupts();
  N64_send(&initialize, 1);

  // Stupid routine to wait for the gamecube controller to stop
  // sending its response. We don't care what it is, but we
  // can't start asking for status if it's still responding
  int x;
  for (x=0; x<64; x++) {
    // make sure the line is idle for 64 iterations, should
    // be plenty.
    if (!N64_QUERY)
      x = 0;
  }

  // Query for the gamecube controller's status. We do this
  // to get the 0 point for the control stick.
  unsigned char command[] = {0x01};
  N64_send(command, 1);
  // read in data and dump it to N64_raw_dump
  N64_get(32);
  interrupts();

#ifdef ENABLE_SD
  // Check/Test communication with N64 Controller
  Serial.print(F("Press any button on the N64 Controller to continue..."));
  while (button == "N/A") {
    delay(200);
    get_button();
  }
  Serial.println(button);
#else
  Serial.println(F("Disabled SD Card support (ENABLE_SD) - skipping controller test."));
#endif
}

/******************************************
Helper functions
*****************************************/
#ifdef ENABLE_SD
#ifdef ENABLE_WRITE
/// Prompt a filename from the Serial Monitor
void getfilename()
{
  Serial.println(F("Please enter a filename in 8.3 format: _"));
  Serial.println("");
  while (Serial.available() == 0) {}
  String strBuffer;
  strBuffer = Serial.readString();
  strBuffer.toCharArray(fileName, 13);
}
#endif

#ifndef ENABLE_EEPROM
/// Emulate EEPROM write function with help of storage on SD
template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
  const byte* p = (const byte*)(const void*)&value;
  unsigned int i;
  //open file on sd card
  // Change to root
  sd.chdir("/");
  // Change to MPK directory
  sd.chdir("MPK");
  if (!myFile.open("EEPROM", O_RDWR | O_CREAT)) {
    Serial.println(F("EEPROM/SD Error"));
    while (1);
  }
  // EEPROM_writeAnything emulation
  for (i = 0; i < sizeof(value); i++) {
    myFile.seekSet(ee++);
    myFile.write(*p++);
  }
  myFile.close();
  return i;
}

/// Emulate EEPROM read function with help of storage on SD
template <class T> int EEPROM_readAnything(int ee, T& value)
{
  byte* p = (byte*)(void*)&value;
  unsigned int i;
  //open file on sd card
  // Change to root
  sd.chdir("/");
  // Change to MPK directory
  sd.chdir("MPK");
  if (!myFile.open("EEPROM", O_READ)) {
    Serial.println(F("EEPROM/SD Error"));
    while (1);
  }
  // EEPROM_readAnything emulation
  for (i = 0; i < sizeof(value); i++) {
    myFile.seekSet(ee++);
    *p++ = myFile.read();
  }
  myFile.close();
  return i;
}
#endif
#endif

/******************************************
CRC Functions
*****************************************/
static word addrCRC(word address)
{
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

/*
static byte dataCRC(byte *data)
{
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
*/

/******************************************
N64 Controller Protocol Functions
*****************************************/
void N64_send(unsigned char *buffer, char length)
{
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
outer_loop: {
    asm volatile (";Starting inner for loop");
    bits = 8;
inner_loop: {
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

      } else {
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

void N64_stop()
{
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

void N64_get(word bitcount)
{
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
   N64 Controller Functions
 *****************************************/
void get_button()
{
  // Command to send to the gamecube
  // The last bit is rumble, flip it to rumble
  // yes this does need to be inside the loop, the
  // array gets mutilated when it goes through N64_send
  unsigned char command[] = {
    0x01
  };

  // don't want interrupts getting in the way
  noInterrupts();
  // send those 3 bytes
  N64_send(command, 1);
  N64_stop();
  // read in 32bits of data and dump it to N64_raw_dump
  N64_get(32);
  // end of time sensitive code
  interrupts();

  // The get_N64_status function sloppily dumps its data 1 bit per byte
  // into the get_status_extended char array. It's our job to go through
  // that and put each piece neatly into the struct N64_status
  int i;
  memset(&N64_status, 0, sizeof(N64_status));

  // bits: joystick x value
  // These are 8 bit values centered at 0x80 (128)
  for (i = 0; i < 8; i++) {
    N64_status.stick_x |= N64_raw_dump[16 + i] ? (0x80 >> i) : 0;
  }
  for (i = 0; i < 8; i++) {
    N64_status.stick_y |= N64_raw_dump[24 + i] ? (0x80 >> i) : 0;
  }

  // read char array N64_raw_dump into string rawStr
  rawStr = "";
  for (i = 0; i < 16; i++) {
    rawStr = rawStr + String(N64_raw_dump[i], DEC);
  }

  // Buttons (A,B,Z,S,DU,DD,DL,DR,0,0,L,R,CU,CD,CL,CR)
  button = "N/A";
  if (!(rawStr.substring(0, 16) == "0000000000000000")) {
    for (int i = 0; i < 16; i++) {
      // seems to be 16, 8 or 4 depending on what pin is used
      if (!(N64_raw_dump[i] == 0)) {
        switch (i) {
        case 7:
          button = "D-Right";
          break;

        case 6:
          button = "D-Left";
          break;

        case 5:
          button = "D-Down";
          break;

        case 4:
          button = "D-Up";
          break;

        case 3:
          button = "START";
          break;

        case 2:
          button = "Z";
          break;

        case 1:
          button = "B";
          break;

        case 0:
          button = "A";
          break;

        case 15:
          button = "C-Right";
          break;

        case 14:
          button = "C-Left";
          break;

        case 13:
          button = "C-Down";
          break;

        case 12:
          button = "C-Up";
          break;

        case 11:
          button = "R";
          break;

        case 10:
          button = "L";
          break;
        }
      }
    }
  }
}

/******************************************
N64 Controller Pak Functions
*****************************************/
/// read 32bytes from controller pak
void readBlock(word myAddress)
{
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

/// reads the MPK file to the sd card
void readMPK()
{
#ifdef ENABLE_SD
  // Change to root
  sd.chdir("/");
  // Change to MPK directory
  sd.chdir("MPK");

  // Get name, add extension and convert to char array for sd lib
  EEPROM_readAnything(0, foldern);
  sprintf(fileName, "%d", foldern);
  strcat(fileName, ".mpk");

  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  //open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    Serial.println(F("SD Error"));
    while (1);
  }
#endif

  Serial.println(F("Please wait..."));

  // Controller paks, which all have 32kB of space, are mapped between 0x0000 – 0x7FFF
  for (word i = 0x0000; i < 0x8000; i += 32) {
    //Serial.print(i, HEX);              // progress
    //Serial.print(F(": "));
    // Read one block of the Controller Pak into array myBlock
    readBlock(i);
    // Write block to SD card
    for (byte j = 0; j < 32; j++) {
#ifdef ENABLE_SD
      myFile.write(myBlock[j]);
#endif
      Serial.print(myBlock[j] < 16 ? "0" : "");
      Serial.print(myBlock[j], HEX);   // read data
      Serial.print(' ');
    }
    Serial.print('\n');
  }
#ifdef ENABLE_SD
  // Close the file:
  myFile.close();
  Serial.print(F("Saved as /MPK/"));
  Serial.println(fileName);
#else
  Serial.print(F("Done"));
#endif
}

#ifdef ENABLE_WRITE
void writeMPK()
{
  // Change to root
  sd.chdir("/");
  // Change to MPK directory
  sd.chdir("MPK");

  Serial.print(F("Writing "));
  Serial.print(fileName);
  Serial.println(F("..."));

  // Open file on sd card
  if (myFile.open(fileName, O_READ)) {
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

  } else {
    Serial.println(F("SD Error"));
    while (1);
  }
}

/// verifies if write was successful
void verifyMPK()
{
  writeErrors = 0;

  Serial.println(F("Verifying..."));

  //open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
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
      Serial.println(i, HEX);          // progress
    }
  }
  // Close the file:
  myFile.close();
  if (writeErrors == 0) {
    Serial.println(F("OK"));
  } else {
    Serial.print(writeErrors);
    Serial.println(F(" errors"));
  }
}
#endif

/******************************************
Main loop
*****************************************/
void loop()
{
  // Print menu to serial monitor
  Serial.println(F("Menu:"));
  Serial.println(F("(0)Read (1)Write (2)Test"));
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
    Serial.println(F("Disabled for security - set ENABLE_SD, ENABLE_WRITE to enable."));
#endif
    break;

  case 50:  // '2'
    Serial.println(F("Testing Controller (RESET to exit)"));
    while (1) {
      delay(100);
      get_button();

      Serial.print(rawStr);
      Serial.print(' ');
      Serial.print(N64_status.stick_x, DEC);
      Serial.print(' ');
      Serial.print(N64_status.stick_y, DEC);
      Serial.print(' ');
      Serial.println(button);
    }
    break;

  }
  Serial.println("");
}

/******************************************
End of File
******************************************/