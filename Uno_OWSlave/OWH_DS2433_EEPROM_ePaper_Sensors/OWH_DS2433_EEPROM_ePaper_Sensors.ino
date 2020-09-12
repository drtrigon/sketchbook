// TODO:
// - fine tune LED front light angle (note or draw position/geometry somewhere)
// - add picturs of device/setup for docu purposes (of old oled "lcd" device also!)
//
// - are both libraries needed? SPI AND Wire?
// - more lines and columns available as on oled -> use that!!! (ds2433 can work with up to 512 bytes of data, not just 144 / may be use bigger font?)

/**
 * @brief Code that emulates a 1wire LCD/OLED Display (DS2433 4096 bits EEPROM)
 *
 * @file Uno_OWSlave/OWH_DS2433_EEPROM_LCD_Sensors/OWH_DS2433_EEPROM_LCD_Sensors.ino
 *
 * @author drtrigon
 * @date 2020-08-04
 * @version 2.0
 *   @li change display to ePaper (less power consumption, bigger display)
 *       (derived from 'Uno_ePaper_Test' which was derived from "epd2in9-demo")
 * @version 1.1
 *   @li add led feedback indicating missing update for more than 3 mins.
 *       confer @ref FREQ_BLINK_OK and @ref FREQ_BLINK_ERR
 * @version 1.0
 *   @li add supply voltage and chip temperature sensors
 *   @li first version providing all basic features
 *
 * @see https://www.waveshare.com/wiki/E-Paper_Shield
 * @see https://www.bastelgarage.ch/universal-raw-driver-e-paper-shield-fur-arduino-nucleo?search=epaper
 * @see https://www.waveshare.com/w/upload/e-Paper_Shield_Code.7z
 * @see http://www.seeedstudio.com/wiki/Grove_-_OLED_Display_1.12%22
 * @see http://www.seeedstudio.com/wiki/File:Stem-diagram-sign.jpg
 * @see http://www.instructables.com/id/Arduino-1-wire-Display-144-Chars/
 *
 * @verbatim
 * OneWireHub library compilation for v2.2.0:
 *   Needs for compilation Arduino IDE version >= 1.8.3 due to used syntax.
 * OneWireHub library settings for v2.2.0:
 *   In order to make OneWireHub library v2.2.0 work properly with my 1wire network
 *   (Raspberry Pi Server with iButton LinkHub-E) I need to adopt
 *     'libraries/OneWireHub/OneWireHub_config.h':
 *       constexpr timeOW_t ONEWIRE_TIME_MSG_HIGH_TIMEOUT     = { 150000_us };
 *     this is 10x the default value (may be smaller works too).
 *     @see https://github.com/orgua/OneWireHub/issues/43
 *   Optionally we can also change
 *     'libraries/OneWireHub/OneWireHub_config.h':
 *       #define HUB_SLAVE_LIMIT     8
 *     as we need only 3 slots and the lower the value the faster are responses.
 *   These settings might cause issues with other sketches - be aware of that.
 *
 * Pinout:
 *   ePaper (Waveshare ePaper Shield):
 *     (uses more or less all pins from pin 5 upward)
 *     (therefore pins for staus led and 1wire data had to be changed)
 *     https://www.waveshare.com/w/upload/c/c8/E-Paper_Shield_User_Manual_en.pdf
 *     https://www.waveshare.com/w/upload/b/bb/E-Ppaer_Shield_Schematic.pdf
 *   Status LED:
 *        LIGHT LED     -> LIGHT BTN
 *                      -> Arduino 5V via 3.3k resistor eg.
 *        STATUS LED    -> Arduino Pin D3
 *        (opt.)        -> Arduino GND via 4.7k resistor eg.
 *   Control Buttons or Switch:
 *        RESET BTN     -> Arduino Pin RST
 *                      -> Arduino GND
 *        LIGHT BTN     -> LIGHT LED (other contact of LED to 5V via 3.3k resistor eg.)
 *                      -> Arduino GND
 *   1wire data bus (MicroLAN):
 *        1WIRE DATA    -> Arduino Pin D4
 *
 * Test on Raspberry Pi Server using OWFS (owshell):
 *   $ /opt/owfs/bin/owwrite 2D.00003124DA00/memory 'hello world!'
 *   $ /opt/owfs/bin/owget 2D.00003124DA00/memory
 *
 * Tested with:
 *   - 1wire LCD/OLED Display:
 *     LinkHubE-Master, atmega328@16MHz (Arduino Uno) as Slave
 *   - OneWire HubDS2433 4096 bits EEPROM:
 *     DS9490R-Master, atmega328@16MHz and teensy3.2@96MHz as Slave
 *
 * Notes:
 * - main issue was memory footprint:
 *   a) disable all serial library (debug) in 'Waveshare_ePaper' rename it to 'Waveshare_ePaper_minimal' (Debug.h, DEV_Config.cpp)
 *   b) DO NOT use String class or other with dynamic memory usage (stability problems)
 *      -> needs at least ~300 bytes free SRAM in order to work
 *      -> using char array buffers and 'sprintf' works even with ~20 bytes only (as no dynamic allocations are done and nedeed)
 *   c) put strings into flash using 'strcpy_P' and 'PSTR' similar like F macro
 *      -> also use 'sprintf_P' and 'PSTR' (use '%S' instead of '%s'!)
 *      -> if needed 'strcpy_P' can be replaced by 'sprintf_P' not needing any other change
 *   see: https://forum.arduino.cc/index.php?topic=243268.0
 *        https://learn.adafruit.com/memories-of-an-arduino/optimizing-sram
 *        https://andybrown.me.uk/2011/01/01/debugging-avr-dynamic-memory-allocation/
 *        (https://ww1.microchip.com/downloads/en/AppNotes/doc8453.pdf)
 *        (https://create.arduino.cc/projecthub/john-bradnam/reducing-your-memory-usage-26ca05)
 *        (https://www.thecoderscorner.com/electronics/microcontrollers/efficiency/evaluating-static-memory-usage-in-arduino-sketch/)
 *   d) look on Arduin IDE output "Using library" parts at the end; SPI, Wire, SD - are all of them needed?
 *      also analyze SRAM usage: $ /home/osboxes/.arduino15/packages/arduino/tools/avr-gcc/5.4.0-atmel3.6.1-arduino2/bin/avr-nm -Crtd --size-sort /tmp/arduino_build_71303/OWH_DS2433_EEPROM_ePaper_Sensors.ino.elf | grep -i ' [dbv] '
 *      -> free >500 bytes by disabling all SD usage by deleting the .cpp and .h source files (in 'Waveshare_ePaper_minimal')
 * - in order to keep the working principle using line based partial update a ePaper
 *   supporting partial update is needed (however full update should work as well when
 *   following the hints about memory footprint above - ban String class, use
 *   sprintf - also full update suppresses ghosting better)
 *   -> see 'Uno_ePaper_Test' for more info on how to use full/partial update etc.
 * - casing:
 *   a) prototype done with LEGO - may suitable be for longer use also
 *   b) external bright (red) LED added as ePaper front-light
 *      (directly connected to 5V/GND via switch, not software action needed)
 *
 * Thanks to:
 * orgua - OneWireHub OneWire slave device emulator
 *         @see https://github.com/orgua/OneWireHub
 * @endverbatim
 */

#include "OneWireHub.h"
#include "DS2433.h"
#include "DS18B20.h"

#include "EPD_2in9.h"
#include "GUI_Paint.h"
#include <Wire.h>

// https://bigdanzblog.wordpress.com/2014/10/24/arduino-watchdog-timer-wdt-example-code/
#include <avr/wdt.h>    // watchdog
#include <EEPROM.h>

#define FREQ_BLINK_OK   2000  // fast blink 0.5 Hz all OK
#define FREQ_BLINK_ERR  4000  // slow blink 0.25 Hz no update for >3min
// hint: bigger interval makes the 1wire bus more responsive

//constexpr uint8_t pin_onewire   { 8 };
//constexpr uint8_t pin_led       { 13 };
constexpr uint8_t pin_onewire   { 4 };
constexpr uint8_t pin_led       { 3 };
//constexpr uint8_t pin_button    { 2 };

uint8_t mem_read[16];
//uint8_t display_mode = 0;  // 0: normal, 1: inverse; 2: off
uint8_t mem_buffer[144];

unsigned long RAM_lRebootCount0 __attribute__ ((section (".noinit")));  // these variables retain uncanged in memory during reboot
unsigned long RAM_lRebootCount1 __attribute__ ((section (".noinit")));  // copy for checksum (power-up detection)
unsigned long EEPROM_lPowerCount0;                                      // these variables retain uncanged in eeprom during power-cycle
unsigned long EEPROM_lPowerCount1;                                      // copy for checksum (first use detection)

unsigned long time_last_update;
bool display_update;
bool status_update;

uint32_t interval = FREQ_BLINK_OK;  // interval at which to blink (milliseconds)
//uint32_t nextMillis = millis();     // will store next time LED will updated

auto hub = OneWireHub(pin_onewire);
auto ds2433 = DS2433(DS2433::family_code, 0x00, 0x00, 0x33, 0x24, 0xDA, 0x00);  // LCD/OLED
auto ds18b0 = DS18B20(0x28, 0x00, 0x00, 0x33, 0x24, 0xDA, 0x00);                // supply voltage Vcc
auto ds18b1 = DS18B20(0x28, 0x01, 0x00, 0x33, 0x24, 0xDA, 0x00);                // chip temperature Ti
////auto ds18b1 = DS18B20(DS18B20::family_code, 0x01, 0x00, 0x33, 0x24, 0xDA, 0x00);  // chip temperature Ti

GUIPAINT paint;
EPD2IN9 epd;

bool blinking(void);

void EPD_Print(UWORD, UWORD , const char *, sFONT*, UWORD, UWORD);

/**
 *  Arduino IDE: put your setup code here, to run once.
 */
void setup()
{
    // - immediately disable watchdog timer so set will not get interrupted
    // - any 'slow' activity needs to be completed before enabling the watchdog timer.
    // - the following forces a pause before enabling WDT. This gives the IDE a chance to
    //   call the bootloader in case something dumb happens during development and the WDT
    //   resets the MCU too quickly. Once the code is solid, remove this.
    wdt_disable();

    mem_read[13] = 0;
//    memset(mem_read, 0, 16);

//    Serial.begin(115200);
//    Wire.begin();
//    Serial.println("OneWire-Hub DS2433");
    //epd.EPD_Init(lut_full_update);     // full update ("mode"?)
    epd.EPD_Init(lut_partial_update);  // partial update ("mode"?)
    //SeeedGrayOled.setInverseDisplay();    // Set display to inverse mode
    //paint.Paint_NewImage(IMAGE_BW, EPD_WIDTH, EPD_HEIGHT, IMAGE_ROTATE_90, IMAGE_COLOR_POSITIVE);   // Horizontal screen
    paint.Paint_NewImage(IMAGE_BW, EPD_WIDTH, EPD_HEIGHT, IMAGE_ROTATE_180, IMAGE_COLOR_POSITIVE);  // Vertical screen: IMAGE_ROTATE_0, IMAGE_ROTATE_180
    paint.Paint_Clear(BLACK);  // reset/reduce ghosting
    epd.EPD_Display();
    paint.Paint_Clear(WHITE);  // clear display
    epd.EPD_Display();
    //epd.EPD_Init(lut_partial_update);  // partial update ("mode"?)
    //paint.Paint_DrawString_EN(0*20+10, 0*20+10, "OWHub DS2433", &Font12, BLACK, WHITE);  // needs ~20 pixel in height (last line ~250)
    //EPD_Print(0*10+10, 0*20+10, "OWHub DS2433", &Font12, BLACK, WHITE);  // needs ~20 pixel in height (last line ~250)
    strcpy_P(&mem_read[0], PSTR("OWHub DS2433"));  // like F() macro
    EPD_Print(0*10+10, 0*20+10, mem_read, &Font12, BLACK, WHITE);  // needs ~20 pixel in height (last line ~250)
    //paint.Paint_DrawString_EN(10, 10, "OWHub DS2433", &Font16, BLACK, WHITE);  // needs ~30 pixel in height (last line ~250)

    //String stringOne = String(ds2433.ID[0], HEX) + String(ds2433.ID[1], HEX) + String(ds2433.ID[2], HEX) + String(ds2433.ID[3], HEX) + String(ds2433.ID[4], HEX) + String(ds2433.ID[5], HEX) + String(ds2433.ID[6], HEX);
    ////paint.Paint_DrawString_EN(0*20+10, 1*20+10, stringOne.c_str(), &Font12, WHITE, BLACK);  // needs ~20 pixel in height (last line ~250)
    //EPD_Print(0*20+10, 1*20+10, stringOne.c_str(), &Font12, WHITE, BLACK);  // needs ~20 pixel in height (last line ~250)
//    sprintf(&mem_read[0],"%02X%02X%02X%02X%02X%02X%02X", ds2433.ID[0], ds2433.ID[1], ds2433.ID[2], ds2433.ID[3], ds2433.ID[4], ds2433.ID[5], ds2433.ID[6]);
    //sprintf(&mem_read[0],"%X%X%X%X%X%X%X", ds2433.ID[0], ds2433.ID[1], ds2433.ID[2], ds2433.ID[3], ds2433.ID[4], ds2433.ID[5], ds2433.ID[6]);
    sprintf_P(&mem_read[0], PSTR("%X%X%X%X%X%X%X"), ds2433.ID[0], ds2433.ID[1], ds2433.ID[2], ds2433.ID[3], ds2433.ID[4], ds2433.ID[5], ds2433.ID[6]);
    EPD_Print(0*10+10, 1*20+10, mem_read, &Font12, WHITE, BLACK);  // needs ~20 pixel in height (last line ~250)

    strcpy_P(&mem_read[0], PSTR("Version: 2.0"));  // like F() macro
    EPD_Print(0*10+10, 2*20+10, mem_read, &Font12, WHITE, BLACK);  // needs ~20 pixel in height (last line ~250)

    strcpy_P(&mem_read[0], PSTR("OWHub: 2.2.0"));  // like F() macro
    EPD_Print(0*10+10, 3*20+10, mem_read, &Font12, WHITE, BLACK);  // needs ~20 pixel in height (last line ~250)

    strcpy_P(&mem_read[0], PSTR(__DATE__));  // like F() macro
    EPD_Print(0*10+10, 4*20+10, mem_read, &Font12, WHITE, BLACK);  // needs ~20 pixel in height (last line ~250)
    strcpy_P(&mem_read[0], PSTR(__TIME__));  // like F() macro
    EPD_Print(0*10+10, 5*20+10, mem_read, &Font12, WHITE, BLACK);  // needs ~20 pixel in height (last line ~250)
//    Serial.println(__FILE__);

    //sprintf(&mem_read[0],"%i/%s", ARDUINO, __VERSION__);
    sprintf_P(&mem_read[0], PSTR("%i/%S"), ARDUINO, PSTR(__VERSION__));  // need to use '%S' with uppercase S
    EPD_Print(0*10+10, 6*20+10, mem_read, &Font12, WHITE, BLACK);  // needs ~20 pixel in height (last line ~250)

    sprintf_P(&mem_read[0], PSTR("Vs: %i"), readVcc());
    EPD_Print(0*10+10, 7*20+10, mem_read, &Font12, WHITE, BLACK);  // needs ~20 pixel in height (last line ~250)

    sprintf_P(&mem_read[0], PSTR("Ti: %ld"), (long)(GetTemp()*1000));
    EPD_Print(0*10+10, 8*20+10, mem_read, &Font12, WHITE, BLACK);  // needs ~20 pixel in height (last line ~250)

    // https://www.arduino.cc/en/Reference/EEPROM
    EEPROM.get(sizeof(unsigned long)*0, EEPROM_lPowerCount0);
    EEPROM.get(sizeof(unsigned long)*1, EEPROM_lPowerCount1);
    if (EEPROM_lPowerCount0 != EEPROM_lPowerCount1) {  // setup-up reset
        EEPROM_lPowerCount0 = 0;
        EEPROM_lPowerCount1 = 0;
    }
    // https://forum.arduino.cc/index.php?topic=232362.0
    if (RAM_lRebootCount0 != RAM_lRebootCount1) {      // power-up reset
        RAM_lRebootCount0 = 0;
        RAM_lRebootCount1 = 0;
        //++EEPROM_lPowerCount0; ++EEPROM_lPowerCount1;
        ++EEPROM_lPowerCount1;
        //EEPROM.put(sizeof(unsigned long)*0, EEPROM_lPowerCount0);
// writing to eeprom limited to ~100'000 times thus disabled
//        EEPROM.put(sizeof(unsigned long)*0, EEPROM_lPowerCount1);
//        EEPROM.put(sizeof(unsigned long)*1, EEPROM_lPowerCount1);
// writing to eeprom disabled
    }
// writing to eeprom limited to ~100'000 times thus disabled
//    sprintf(&mem_read[0],"P: %2ld  R: %2ld", EEPROM_lPowerCount0, RAM_lRebootCount0);
    sprintf_P(&mem_read[0], PSTR("P: %2ld  R: %2ld"), EEPROM_lPowerCount1, RAM_lRebootCount0);  // while eeprom writing disabled
// writing to eeprom disabled
    EPD_Print(0*10+10, 9*20+10, mem_read, &Font12, WHITE, BLACK);  // needs ~20 pixel in height (last line ~250)
    ++RAM_lRebootCount0;
    ++RAM_lRebootCount1;
// PSTR used here is used 3 times in whole code - how to re-use it? by declaring a global const (expr) pointer?

    pinMode(pin_led, OUTPUT);

//    pinMode(pin_button, INPUT_PULLUP);

    // Setup OneWire
    hub.attach(ds2433);
    hub.attach(ds18b0);
    hub.attach(ds18b1);

    /*// Test-Cases: the following code is just to show basic functions, can be removed any time
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
    strcpy_P(&mem_read[0], PSTR("config done"));  // like F() macro
    EPD_Print(0*10+10, 10*20+10, mem_read, &Font12, WHITE, BLACK);  // needs ~20 pixel in height (last line ~250)

    //3.Refresh the picture in RAM to e-Paper (needed for full update only)
    epd.EPD_Display();
    epd.EPD_Sleep();      // prevent ghosting and other damages, e.g. due to high-voltage for long time

    delay(3000);

    paint.Paint_Clear(WHITE);  // clear display

    sprintf_P(&mem_read[0], PSTR("P: %2ld  R: %2ld"), EEPROM_lPowerCount1, RAM_lRebootCount0);  // while eeprom writing disabled
    EPD_Print(0*10+10, 12*20+10, mem_read, &Font12, BLACK, WHITE);  // needs ~20 pixel in height (last line ~250)

    //3.Refresh the picture in RAM to e-Paper (needed for full update only)
    epd.EPD_Display();
    epd.EPD_Sleep();      // prevent ghosting and other damages, e.g. due to high-voltage for long time

    time_last_update = millis();

    wdt_reset();
    wdt_enable(WDTO_8S);
}

/**
 *  Arduino IDE: put your main code here, to run repeatedly.
 */
void loop()
{
    wdt_reset();
    // following function must be called periodically
    hub.poll();

    // Blink triggers the state-change
    if (blinking()) {
        /*// clear screen all millis() timer overflow in order to reset shadows (ghosting?)
        if ((nextMillis <= interval) && (interval == FREQ_BLINK_OK)) {  // timer overflow (clear display to prevent ghosting)
            // set buffer to empty/cleared display (display filled with whitespaces) - triggers print of all data after clear
            memset(&mem_buffer[0], ' ', 144);

            wdt_reset();

            // clear display
            paint.Paint_Clear(BLACK);  // reset/reduce ghosting
            epd.EPD_Display();
            paint.Paint_Clear(WHITE);  // clear display
            epd.EPD_Display();

            // restore power-on/reboot status display
            sprintf_P(&mem_read[0], PSTR("P: %2ld  R: %2ld"), EEPROM_lPowerCount1, RAM_lRebootCount0);  // while eeprom writing disabled
            EPD_Print(0*10+10, 12*20+10, mem_read, &Font12, BLACK, WHITE);  // needs ~20 pixel in height (last line ~250)

            display_update = true;
        }*/

        // pages are 32 bytes each, but we read in blocks of 12 byte due to the LCD
        for(char i=0; i < 12 ; i++) {
            ds2433.readMemory(mem_read, 12, i*12);
            if(memcmp(mem_read, &mem_buffer[i*12], 12) != 0) {  // update display on change only otherwise 1wire becomes unresponsive due to slow output!!
                wdt_reset();

                time_last_update = millis();
                display_update = true;

                EPD_Print(0*10+10, i*20+10, reinterpret_cast<const char *>(mem_read), &Font12, WHITE, BLACK);  // needs ~20 pixel in height (last line ~250)

                memcpy(&mem_buffer[i*12], mem_read, 12);
            }
        }

//        sprintf_P(&mem_read[0], PSTR("tmr: %ld"), millis());
//        EPD_Print(0*20+10, 13*20+10, reinterpret_cast<const char *>(mem_read), &Font12, WHITE, BLACK);  // needs ~20 pixel in height (last line ~250)

        if (status_update) {
            wdt_reset();

            if (interval == FREQ_BLINK_ERR) {  // no update for >180'000ms = 3min
                strcpy_P(&mem_read[0], PSTR("E: no update"));  // like F() macro
                paint.Paint_DrawString_EN(0*10+10, 13*20+10, mem_read, &Font12, BLACK, WHITE);  // needs ~20 pixel in height (last line ~250)
            } else
                paint.Paint_ClearWindows(0*10+10, 13*20+10, 0*10+10 + Font12.Width * 12, 13*20+10 + Font12.Height, WHITE);
        }

        if (display_update or status_update) {
            wdt_reset();

            //3.Refresh the picture in RAM to e-Paper (needed for full update only)
            epd.EPD_Display();
            epd.EPD_Sleep();      // prevent ghosting and other damages, e.g. due to high-voltage for long time

            display_update = false;
            status_update = false;
        }

        ds18b0.setTemperature(static_cast<float>(readVcc()/1000.));    // -55...125 allowed
        //ds18b0.setTemperatureRaw(static_cast<int16_t>((readVcc()/1000.) * 16.0f));
        ds18b1.setTemperature(static_cast<float>(GetTemp()));          // -55...125 allowed
        //ds18b1.setTemperatureRaw(static_cast<int16_t>(GetTemp() * 16.0f));
    }
}

/**
 *  OneWireHub OneWire slave status update function.
 *
 * @param void
 * @return Integer indicating whether status updated with 1
 */
bool blinking(void)
{
    //static uint32_t interval    = FREQ_BLINK_OK; // interval at which to blink (milliseconds)
    static uint32_t nextMillis  = millis();     // will store next time LED will updated

    if (millis() > nextMillis) {
        //if (nextMillis > (time_last_update + 180000))  // (would be faster...)
        if (millis() > (time_last_update + 180000)) {  // no update for >180'000ms = 3min
            status_update = (interval != FREQ_BLINK_ERR);
            interval = FREQ_BLINK_ERR;        // ERR: interval at which to blink (milliseconds)
        } else {
            status_update = (interval != FREQ_BLINK_OK);
            interval = FREQ_BLINK_OK;         //  OK: interval at which to blink (milliseconds)
        }

        nextMillis += interval;             // save the next time you blinked the LED
        static uint8_t ledState = LOW;      // ledState used to set the LED
        if (ledState == LOW)    ledState = HIGH;
        else                    ledState = LOW;
        //digitalWrite(pin_led, ledState);
        return 1;
    }
    return 0;
}

/**
 *  ...
 *
 * @return (nothing)
 */
void EPD_Print(UWORD Xstart, UWORD Ystart, const char * pString,
                                   sFONT* Font, UWORD Color_Background, UWORD Color_Foreground)
{
    // ClearWindows needed for partial update
    paint.Paint_ClearWindows(Xstart, Ystart, Xstart + Font->Width * 12, Ystart + Font->Height, WHITE);

    paint.Paint_DrawString_EN(Xstart, Ystart, pString, Font, Color_Background, Color_Foreground);  // needs ~20 pixel in height (last line ~250)

//    //3.Refresh the picture in RAM to e-Paper
//    epd.EPD_Display();
//    epd.EPD_Sleep();      // prevent ghosting and other damages, e.g. due to high-voltage for long time
}

/**
 *  Accessing the secret voltmeter on the Arduino 168 or 328.
 *
 * @see loop()
 * @see http://code.google.com/p/tinkerit/wiki/SecretVoltmeter
 * @return The supply voltage in [mV]
 */
long readVcc()
{
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

/**
 *  Internal Temperature Sensor for ATmega328 types.
 *
 * @param void
 * @see loop()
 * @see https://playground.arduino.cc/Main/InternalTemperatureSensor
 * @return The chip temperature in [°C]
 */
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
//    t = (wADC - 324.31 ) / 1.22;
    t = (wADC - 330.81 ) / 1.22;    // Arduino Uno R3 (mega328)

    // The returned temperature is in degrees Celsius.
    return (t);
}
