/*
*    Example-Code that emulates a DS2433 4096 bits EEPROM
*
*   Tested with
*    - DS9490R-Master, atmega328@16MHz and teensy3.2@96MHz as Slave
*
*    // add 2 ds18b20 for data like; source voltage, chip temp
*/
/*
*    1wire LCD/OLED Display
*
*   In order to make it work with my 1wire network
*   (Raspberry Pi Server with iButton LinkHub-E) I need
*   to adopt 'libraries/OneWireHub/OneWireHub_config.h'
*    ONEWIRE_TIME_MSG_HIGH_TIMEOUT     = { 150000_us };
*   this is 10x the default value (may be smaller works too).
*
*   Grove Shield I2C/Wire:
*    GND GND   black
*    VCC  5V   red
*    SDA  A4   white
*    SCL  A5   yellow
*
*   See also:
*    http://www.seeedstudio.com/wiki/Grove_-_OLED_Display_1.12%22
*    http://www.seeedstudio.com/wiki/File:Stem-diagram-sign.jpg
*
*   Test from Raspberry Pi Server:
*    ursin@ThinkPad-T440-ursin:~$ ssh pi@192.168.11.41
*    ...
*    pi@raspberrypi ~ $ /opt/owfs/bin/owwrite 2D.00003124DA00/memory 'hallo LEA!'
*    pi@raspberrypi ~ $ /opt/owfs/bin/owget 2D.00003124DA00/memory
*
*    Button: Pin2 to GND (with internal pullup)
*
*    Needs for compilation Arduino IDE >= 1.8.3 !!!
*/

#include "OneWireHub.h"
#include "DS2433.h"

#include <Wire.h>
#include <SeeedGrayOLED.h>

constexpr uint8_t pin_onewire   { 8 };
constexpr uint8_t pin_led       { 13 };
constexpr uint8_t pin_button    { 2 };

uint8_t mem_read[16];
uint8_t display_mode = 0;  // 0: normal, 1: inverse; 2: off
uint8_t mem_buffer[144];

auto hub = OneWireHub(pin_onewire);
auto ds2433 = DS2433(DS2433::family_code, 0x00, 0x00, 0x33, 0x24, 0xDA, 0x00);
//auto ds18b0 = DS18B20(0x28, 0x00, 0x55, 0x33, 0x24, 0xDA, 0x11); // 0x FF 03 00 00   -- float --   measure supply voltage Vcc
//auto ds18b1 = DS18B20(0x28, 0x01, 0x55, 0x33, 0x24, 0xDA, 0x11); //              ?   -- float --   measure chip temperature

bool blinking(void);

void setup()
{
//    Serial.begin(115200);
    Wire.begin();
//    Serial.println("OneWire-Hub DS2433");
    SeeedGrayOled.init();
    //SeeedGrayOled.setInverseDisplay();    // Set display to inverse mode
    SeeedGrayOled.setNormalDisplay();       //Set display to Normal mode
    SeeedGrayOled.clearDisplay();           //clear the screen and set start position to top left corner
    SeeedGrayOled.setVerticalMode();        // Set to vertical mode for displaying text
    SeeedGrayOled.setTextXY(0,0);           //Set the cursor to 0th line, 0th Column
    //SeeedGrayOled.setGrayLevel(15); //Set Grayscale level. Any number between 0 - 15.
    //SeeedGrayOled.putNumber(123);           //Print number
//    SeeedGrayOled.putString("OneWire-Hub DS2433"); //Print Hello World
    SeeedGrayOled.putString("OWHub DS2433"); //Print Hello World
//    SeeedGrayOled.setTextXY(1,0);           //Set the cursor to 1st line, 0th Column
//    SeeedGrayOled.putNumber(0xFFFF);        //Print number
    // Bitmap and Scroll modes available also

    SeeedGrayOled.setTextXY(1,0);
    String stringOne =  String(ds2433.ID[0], HEX) + String(ds2433.ID[1], HEX) + String(ds2433.ID[2], HEX) + String(ds2433.ID[3], HEX) + String(ds2433.ID[4], HEX) + String(ds2433.ID[5], HEX) + String(ds2433.ID[6], HEX);
    SeeedGrayOled.putString(stringOne.c_str());

    SeeedGrayOled.setTextXY(2,0);
    SeeedGrayOled.putString("Version: 1");

    SeeedGrayOled.setTextXY(3,0);
    SeeedGrayOled.putString("OWHub: 2.2.0");

    SeeedGrayOled.setTextXY(4,0);
    SeeedGrayOled.putString(__DATE__);
    SeeedGrayOled.setTextXY(5,0);
    SeeedGrayOled.putString(__TIME__);
//    Serial.println(__FILE__);

    SeeedGrayOled.setTextXY(6,0);
    stringOne =  String(ARDUINO, DEC);
    SeeedGrayOled.putString(stringOne.c_str());
    SeeedGrayOled.putString("/");
    SeeedGrayOled.putString(__VERSION__);

    SeeedGrayOled.setTextXY(7,0);
    SeeedGrayOled.putString("Vs: ");
    SeeedGrayOled.putNumber(readVcc());

    SeeedGrayOled.setTextXY(8,0);
    SeeedGrayOled.putString("Ti: ");
    SeeedGrayOled.putNumber(GetTemp()*1000);

    pinMode(pin_led, OUTPUT);

    pinMode(pin_button, INPUT_PULLUP);

    mem_read[13] = 0;

    // Setup OneWire
    hub.attach(ds2433);
//    hub.attach(ds18b0);
//    hub.attach(ds18b1);

/*    // Test-Cases: the following code is just to show basic functions, can be removed any time
    Serial.println("Test Write Text Data to page 0");
    constexpr char memory[] = "abcdefg-test-data full ASCII:-?+";
    ds2433.writeMemory(reinterpret_cast<const uint8_t *>(memory),sizeof(memory),0x00);

    Serial.println("Test Write binary Data to page 1");
    constexpr uint8_t mem_dummy[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    ds2433.writeMemory(mem_dummy, sizeof(mem_dummy), 1*32);

    Serial.print("Test Read binary Data to page 1: 0x");
    uint8_t mem_read[16];
    ds2433.readMemory(mem_read, 16, 31); // begin one byte earlier than page 1
    Serial.println(mem_read[2],HEX); // should read 0x11

    // ds2433.clearMemory(); // begin fresh after doing some work*/

//    Serial.println("config done");
    SeeedGrayOled.setTextXY(9,0);
    SeeedGrayOled.putString("config done");

    delay(2000);

    for(char i=0; i < 10 ; i++)
    {
        SeeedGrayOled.setTextXY(i,0);           //Set the cursor to (i+4)th line, 0th Column
        SeeedGrayOled.clearDisplay();           //clear the screen and set start position to top left corner
    }
}

void loop()
{
    // following function must be called periodically
    hub.poll();

    // Blink triggers the state-change
    if (blinking())
    {
        // Keep in mind the pull-up means the pushbutton's logic is inverted. It goes
        // HIGH when it's open, and LOW when it's pressed.
        if (digitalRead(pin_button) == LOW) {
            display_mode = (display_mode+1) % 3;
            //SeeedGrayOled.setTextXY(0,0);           //Set the cursor to (i+4)th line, 0th Column
            //SeeedGrayOled.putNumber(display_mode);

            switch (display_mode)  // changes mode only for newly/periodically printed stuff
            {
                case 1:
                    SeeedGrayOled.setInverseDisplay();    // Set display to inverse mode
                    SeeedGrayOled.sendCommand(SeeedGrayOLED_Display_On_Cmd);
                    break;
                case 2:
                    SeeedGrayOled.setNormalDisplay();    // Set display to inverse mode
                    SeeedGrayOled.sendCommand(SeeedGrayOLED_Display_Off_Cmd);
                    break;
                default:
                    SeeedGrayOled.setNormalDisplay();       //Set display to Normal mode
                    SeeedGrayOled.sendCommand(SeeedGrayOLED_Display_On_Cmd);
                    break;
            }
        }

        // pages are 32 bytes each, but we read in blocks of 12 byte due to the LCD
        for(char i=0; i < 12 ; i++)
        {
            //SeeedGrayOled.setTextXY(0,1);           //Set the cursor to (i+4)th line, 0th Column
            //SeeedGrayOled.putNumber(i);
            ds2433.readMemory(mem_read, 12, i*12);
            //SeeedGrayOled.setTextXY(0,2);           //Set the cursor to (i+4)th line, 0th Column
            //SeeedGrayOled.putNumber(i);
            if(memcmp(mem_read, &mem_buffer[i*12], 12) != 0)  // update display on change only otherwise 1wire becomes unresponsive due to slow output!!
            {
                //SeeedGrayOled.setTextXY(0,0);           //Set the cursor to (i+4)th line, 0th Column
                //SeeedGrayOled.putNumber(i);

                SeeedGrayOled.setTextXY(i,0);           //Set the cursor to (i+4)th line, 0th Column
                SeeedGrayOled.putString(reinterpret_cast<const char *>(mem_read));

                memcpy(&mem_buffer[i*12], mem_read, 12);
            }
        }
    }
}

bool blinking(void)
{
    const  uint32_t interval    = 2000;          // interval at which to blink (milliseconds)
    static uint32_t nextMillis  = millis();     // will store next time LED will updated

    if (millis() > nextMillis)
    {
        nextMillis += interval;             // save the next time you blinked the LED
        static uint8_t ledState = LOW;      // ledState used to set the LED
        if (ledState == LOW)    ledState = HIGH;
        else                    ledState = LOW;
        digitalWrite(pin_led, ledState);
        return 1;
    }
    return 0;
}

/********************************************************************************
    Accessing the secret voltmeter on the Arduino 168 or 328
    http://code.google.com/p/tinkerit/wiki/SecretVoltmeter
********************************************************************************/
long readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return result;
}

/********************************************************************************
    Internal Temperature Sensor for ATmega328 types
    https://playground.arduino.cc/Main/InternalTemperatureSensor
********************************************************************************/
double GetTemp(void)
{
  unsigned int wADC;
  double t;

  // The internal temperature has to be used
  // with the internal reference of 1.1V.
  // Channel 8 can not be selected with
  // the analogRead function yet.

  // Set the internal reference and mux.
  ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
  ADCSRA |= _BV(ADEN);  // enable the ADC

  delay(20);            // wait for voltages to become stable.

  ADCSRA |= _BV(ADSC);  // Start the ADC

  // Detect end-of-conversion
  while (bit_is_set(ADCSRA,ADSC));

  // Reading register "ADCW" takes care of how to read ADCL and ADCH.
  wADC = ADCW;

  // The offset of 324.31 could be wrong. It is just an indication.
//  t = (wADC - 324.31 ) / 1.22;
  t = (wADC - 330.81 ) / 1.22;    // Arduino Uno R3 (mega328)

  // The returned temperature is in degrees Celsius.
  return (t);
}
