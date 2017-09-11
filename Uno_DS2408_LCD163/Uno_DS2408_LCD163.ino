#include <OneWire.h>

// OneWire DS18S20, DS18B20, DS1822 Temperature Example
//
// http://www.pjrc.com/teensy/td_libs_OneWire.html
//
// The DallasTemperature library can do all this work for you!
// http://milesburton.com/Dallas_Temperature_Control_Library
/*
 * DS2408 8-Channel Addressable Switch
 *
 * Writte by Glenn Trewitt, glenn at trewitt dot org
 *
 * Some notes about the DS2408:
 *   - Unlike most input/output ports, the DS2408 doesn't have mode bits to
 *       set whether the pins are input or output.  If you issue a read command,
 *       they're inputs.  If you write to them, they're outputs.
 *   - For reading from a switch, you should use 10K pull-up resisters.

esera/eservice LCD 163

https://www.eservice-online.de/shop/1-wire-smart-home/display-lcd/113/usb-adapter-fertigmodul
https://www.eservice-online.de/media/archive/4a/cf/43/Programmbeispiel-1-Wire-Display-163559d6f87339f9.zip
https://www.eservice-online.de/media/pdf/2e/78/67/Bedienungsanleitung%20LCD%20Display%20163.pdf
https://drtrigon.ddns.net/owncloud/index.php/apps/files/?dir=%2Fdocuments%2FBibliothek%2FBetriebsanleitungen
https://datasheets.maximintegrated.com/en/ds/DS2408.pdf
http://www.lcd-module.de/pdf/doma/dog-m.pdf
 */

char ST7036[] = "\x33\x32\x21\x1c\x52\x69\x74\x20\x0c\x01\x06";     // InitString für ST7036   1-zeilig
/* private static $ctab3  = array('ä'=>0x84, 'ö'=>0x94, 'ü'=>0x81,
                                'Ä'=>0x8e, 'Ö'=>0x99, 'Ü'=>0x9a, 'ß'=>0x07);  // Umlaut-Übersetzungstabelle für ST7036
private static $ssharp = "\x06\x09\x11\x12\x11\x11\x16\x10";                 // Zeichendefinition 'ß'*/

//   case "ST7036":
char * InitString = ST7036;

/*void PrintBytes(uint8_t* addr, uint8_t count, bool newline=0) {
  for (uint8_t i = 0; i < count; i++) {
    Serial.print(addr[i]>>4, HEX);
    Serial.print(addr[i]&0x0f, HEX);
  }
  if (newline)
    Serial.println();
}

void ReadAndReport(OneWire* net, uint8_t* addr) {
  Serial.print("  Reading DS2408 ");
  PrintBytes(addr, 8);
  Serial.println();
  
  uint8_t buf[13];  // Put everything in the buffer so we can compute CRC easily.
  buf[0] = 0xF0;    // Read PIO Registers
  buf[1] = 0x88;    // LSB address
  buf[2] = 0x00;    // MSB address
  net->write_bytes(buf, 3);
  net->read_bytes(buf+3, 10);     // 3 cmd bytes, 6 data bytes, 2 0xFF, 2 CRC16
  net->reset();

  if (!OneWire::check_crc16(buf, 11, &buf[11])) {
    Serial.print("CRC failure in DS2408 at ");
    PrintBytes(addr, 8, true);
    return;
  }
  Serial.print("  DS2408 data = ");
  // First 3 bytes contain command, register address.
  Serial.println(buf[3], BIN);
}*/

void pio_write(OneWire* ds, uint8_t* addr, byte b) {
//now you can write to the output latches using PIO WRITE command (0x5a)
  ds->reset();
  ds->select(addr);
  ds->write(0x5a);
  ds->write(b);
  ds->write(~b);
  
  //the colde now is optional, but since there is no error checking while setting status registers, it's important to read them back
  delay(100);
  ds->write(0xaa);
  byte regpio = ds->read();
  if (!(regpio & b)) {
   Serial.println("Writing to PIO register went wrong");
   Serial.println(b, BIN);
   Serial.println(regpio, BIN);
  }
  //Write byte to PIO (next byte)
  //Repeat the previous 4 steps with more PIO 
  //output data as needed in the application.
  //Reset pulse
  //Presence pulse  
  // see Example 1 p33-34 in https://datasheets.maximintegrated.com/en/ds/DS2408.pdf
}

 // LCD-Initialisierung --------------------------------------------------------
void SendLCDInit(OneWire* ds, uint8_t* addr) {
//  TMEX_F29_SetStrobe($this->InstanceID, TRUE);
  delay(100);
  char sendbuf[256];
  MakeCmd(sendbuf, InitString, 11);
  // Init-String ausgeben
  WriteLCDData(ds, addr, sendbuf, 2*11);
//  // deutsche Sonderzeichen definieren
//  // case "ST7036":
//    $this->SendLCDCharDef(7, self::$ssharp); // ß */

}

 // Command-String erzeugen ----------------------------------------------------
void MakeCmd(char * string_dest, const char * string_source, uint8_t num) {
  MakeNibbles(string_source, string_dest, num, true);
}

 // Bytes in Nibbles (Halb-Bytes) umwandeln -------------------------------------------------
 // R/S-Bit für Text setzen, für Befehle löschen
void MakeNibbles(const char * string, char * sendbuf, uint8_t num, uint8_t cmd) {
  byte rs = (cmd) ? 0 : 0x10;
  //char sendbuf[256];
  for (int i=0; i<num; ++i) {
    byte s = string[i];
    if (!cmd && (s >= 0xc0)) {
      //case "ST7036":
//      s = ctab3[chr(s)];
      Serial.println("ctab3 not defined - needs to be done NOW!");
    }
    sendbuf[2*i]   = ((s >> 4) | rs);
    sendbuf[2*i+1] = ((s & 0x0f) | rs);
  }
//  sendbuf[2*strlen(string)] = 0x00;
  return;
}
/*// https://stackoverflow.com/questions/19891962/c-how-to-append-a-char-to-char
char* appendCharToCharArray(char* array, char a)
{
    size_t len = strlen(array);

    char* ret = new char[len+2];

    strcpy(ret, array);    
    ret[len] = a;
    ret[len+1] = '\0';

    return ret;
}*/
 
 // Ausgabe der Befehls- oder Text-Sequenz -------------------------------------
void WriteLCDData(OneWire* ds, uint8_t* addr, const char * string, uint8_t num) {
  //TMEX_F29_WriteBytesMasked($this->InstanceID, $string, 0x1f);
  for (int i=0; i<num; ++i) {
    pio_write(ds, addr, string[i] & 0x1f);
    delay(100);
  }
}

 // Kontrasteinstellung beim ST7036-Controller ---------------------------------
void SendLCDContrast(OneWire* ds, uint8_t* addr, uint8_t ContrastValue) {
  uint8_t contrast = ContrastValue & 0x3f;
  char cstrg[4];
  cstrg[0] = 0x29; // umschalten auf Befehlstabelle 1
  cstrg[1] = (contrast >> 4) | 0x50;
  cstrg[2] = (contrast & 0x0f) | 0x70;
  cstrg[3] = 0x28; // zurückschalten auf Befehlstabelle 0
  char sendbuf[256];
  MakeCmd(sendbuf, cstrg, 4);
  // Command-String ausgeben
  WriteLCDData(ds, addr, sendbuf, 2*4);
}

/* // Ausgabe auf das gesamte LCD ------------------------------------------------
void SendLCDScreen(const char * text) {
  char sendbuf[256];
  uint8_t currentline = 0;
  $lines = explode("|", $text);

  // Display löschen
  $sendbuf .= $this->ClearDisplay();

  // Cursor ausschalten
  $sendbuf .= $this->SwitchCursor(false);
  
  foreach($lines as $line) {
    // Cursor positionieren
    $sendbuf .= $this->CalcCursorPos($currentline, 0);

    // Text-String erzeugen
    $line = substr($line, 0, $this->cols);
    $sendbuf .=  $this->MakeTxt($line);

    $currentline++;
    if ($currentline >= $this->rows) break;
  }
  // Text-String ausgeben
  $this->WriteLCDData($sendbuf);
}*/

OneWire  ds(8);  // on pin 8 (a 4.7K resistor is necessary)

void setup(void) {
  Serial.begin(9600);

  InitString[2] = 0x29;                 // InitString-Ergänzung für 2-zeilige Displays
  InitString[7] = 0x28;
  InitString[3] = 0x1d;  // InitString-Ergänzung für 3-zeilige Displays
}

void loop(void) {
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;
  
  ds.reset_search();
  delay(250);
  if ( !ds.search(addr)) {
    Serial.println("No more addresses.");
    Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }
  
  Serial.print("ROM =");
  for( i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
  Serial.println();
 
  if (addr[0] != 0x29) {
    Serial.print(" is not a DS2408.\n");
    return;
  } else {
    Serial.print("Found DS2408.\n");
  }

/*//Konfiguration fuer 1-Wire Display 163
//Setze den Pin 5 auf Eingang (Tastereingang)
TMEX_SetF29PinType ($InstanceID, 5, true);

//Setze alle anderen PinŽs auf Ausgang (Datenausgabe)
TMEX_SetF29PinType ($InstanceID, 0, false);
TMEX_SetF29PinType ($InstanceID, 1, false);
TMEX_SetF29PinType ($InstanceID, 2, false);
TMEX_SetF29PinType ($InstanceID, 3, false);
TMEX_SetF29PinType ($InstanceID, 4, false);
TMEX_SetF29PinType ($InstanceID, 6, false);
TMEX_SetF29PinType ($InstanceID, 7, false);*/

  // http://forum.arduino.cc/index.php?topic=178437.0
  // strobe?
  ds.reset();
  ds.select(addr);
  ds.write(0xcc); //command to write to registers
  ds.write(0x8d); //select the control status register
  ds.write(0x00);
  ds.write(0b00000100); //set RSTZ mode to STRB
  //the colde now is optional, but since there is no error checking while setting status registers, it's important to read them back
  ds.reset();
  ds.select(addr);
  ds.write(0xf0);
  ds.write(0x8d);
  ds.write(0x00);
  byte reg0x8d = ds.read();
  if (!(reg0x8d & 0b100)) {
   Serial.println("Writing to status register went wrong");
  } else {
   Serial.println("Writing to status register OK");
  }
  //now you can write to the output latches using PIO WRITE command (0x5a)

  // Steuerung Hintergrundbeleuchtung 1-Wire Display 163

  Serial.println("Backlight auf 0% abdimmen und LED ausschalten");
  pio_write(&ds, addr, ~0b00000000);  //Backlight auf 0% abdimmen und LED ausschalten

  delay(1000);
  Serial.println("Backlight auf 50% aufdimmen");
  pio_write(&ds, addr, ~0b01000000);  //Backlight auf 50% aufdimmen

  delay(1000);
  Serial.println("Backlight auf 100% aufdimmen");
  pio_write(&ds, addr, ~0b10000000);  //Backlight auf 100% aufdimmen

  delay(1000);
  Serial.println("Backlight blinken");
  pio_write(&ds, addr, ~0b11000000);  //Backlight blinken

  delay(1000);
  Serial.println("Backlight auf 0% abdimmen und LED ausschalten");
  pio_write(&ds, addr, ~0b00000000);  //Backlight auf 0% abdimmen und LED ausschalten

  delay(1000);
  Serial.println("Backlight auf 100% aufdimmen und LED wieder einschalten");
  pio_write(&ds, addr, ~0b10000000);  //Backlight auf 100% aufdimmen und LED wieder einschalten

  // LCD-Initialisierung

  delay(1000);
  Serial.println("Try to INIT LCD...");
  SendLCDInit(&ds, addr);

/*$text0 = "E-Service Online|1-Wire         |LCD-Display 16x3       |vierte Zeile ";
$lcd->SendLCDScreen($text0);

IPS_Sleep(2000);
$text1 = "1-Wire         |Display 163    |         |vierte Zeile";
$lcd->SendLCDScreen($text1);



//******************************************************************************
// LCD-Textausgabe an einer bestimmten Cursor-Position
// ===================================================
// Zeilenüberlauf wird nicht berücksichtigt!
//******************************************************************************
$text = date("d.m.y H:i");
$lcd->SendLCDText(2, 1, $text); // 3. Zeile, 2. Spalte*/



  // Kontrasteinstellung beim ST7036 Controller
  // Zulässige Werte für $ContrastValue: 0...63
  delay(1000);
  Serial.println("Try to SET CONTRAST...");
  int ContrastValue = 35;
//  int ContrastValue = 60;
  SendLCDContrast(&ds, addr, ContrastValue);

// Contrast works!!! :))

// On LCD line1 the first few chars (3) do get a horizontal line on top most pixels... ;))) it alive :)
// (think the pixel are written during backlight stuff - assume strobe issue, no idea...)

// Button 5 is input ... how to manage?

  Serial.println("DONE.");
  delay(10000);
  return;



/*  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  Serial.print("  Data = ");
  Serial.print(present, HEX);
  Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.print(" CRC=");
  Serial.print(OneWire::crc8(data, 8), HEX);
  Serial.println();

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
  Serial.print("  Temperature = ");
  Serial.print(celsius);
  Serial.print(" Celsius, ");
  Serial.print(fahrenheit);
  Serial.println(" Fahrenheit");*/
}
