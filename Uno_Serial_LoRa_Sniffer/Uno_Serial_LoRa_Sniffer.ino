/*
 Name:      Uno_Serial_LoRa_Sniffer.ino
 Created:   2018-07-29, 7/31/2017 6:04:42 PM
 Author:    drtrigon, joe

TODO: need to implement scanning of CodingRate4, etc. !!!

 TTN EU863-870
   scan FREQ (see list), BW (125, 250, 500 kHz), SF (6..7) found channels:
   Freq: 868.10  BW: 250000  SF: 7  (12)
   Freq: 868.30  BW: 250000  SF: 7  (1)
   Freq: 867.50  BW: 125000  SF: 7  (1)
   (rest is basically free - test especially FREQ around 863 MHz)

   SF=7, CR=1 (4/5), BW=250 -> 10937.5 bps (=1.3671875 kB/s)
   SF=6, CR=1 (4/5), BW=500 -> 4.6875 kB/s

TODO: try higher datarate with SpreadingFactor(6) and setSignalBandwidth(500000) ... ?

 https://hackaday.com/2017/08/22/sniff-your-local-lora-packets/
 https://github.com/ImprobableStudios/Feather_TFT_LoRa_Sniffer/blob/master/Feather_TFT_LoRa_Sniffer.ino
 (use for Yun as it supports atmega32u4 and sd - see below)
 see also https://www.instructables.com/id/Turn-an-Arduino-Into-a-Quick-Easy-LoRa-Packet-Snif/
 and https://github.com/LooUQ/Arduino-LoRa-Sniffer

 https://www.semtech.com/uploads/documents/an1200.26.pdf

 @ref OWPJON  (for LoRa tunnel/switch and device frequencies)
 https://www.thethingsnetwork.org/docs/lorawan/frequency-plans.html  (for TTN frequencies)

 $ sudo miniterm.py /dev/ttyACM0 115200 | tee lora-sniffer-log.txt
 (may be first needed to connect with baud 9600 in order to trigger reset)
*/

#define ENABLE_BW_SCAN
#define ENABLE_SF_SCAN

#include <SPI.h>
//#include <SD.h>
//#include <Adafruit_GFX.h>
//#include <Adafruit_ILI9341.h>
#include <RH_RF95.h>
//#include <stdio.h>
//#include <stdarg.h>

/*#ifdef __AVR_ATmega32U4__
#define STMPE_CS  6
#define TFT_CS    9
#define TFT_DC    10
#define SD_CS     5

#define LED       13

#define RFM95_CS  8
#define RFM95_RST 4
#define RFM95_INT 7
#endif*/

#define LED       LED_BUILTIN

//#define PRINTFLN_MAXLEN         256

#define RH_FLAGS_ACK            0x80

//#define LOG_HEADER              "MS,Freq,FreqPacketCount,RSSI,From,To,MsgId,Flags,Ack,DataLen,DataBytes,Data"

// Define the frequencies to scan
// https://www.thethingsnetwork.org/docs/lorawan/frequency-plans.html
//#define FREQ_COUNT 17
#define FREQ_COUNT 3
float _frequencies[] =
{
    // OWPJON (edges of allowed band 863 and 870 MHz; use e.g. 864.2 MHz - enough BW not .1, .3, .5, .7 or .9)
    // no other free band in CH, see https://www.ofcomnet.ch/#/fatTable
//    865.0, 865.1, 865.2, 865.3, 865.4, 865.5, 865.6, 865.7, 865.8, 865.9,
    865.4,
    866.6,
    869.2,
    // From Adafruit Learn
    //868.0, 915.0,
    // TTN EU863-870
//    868.1, 868.3, 868.5, 867.1, 867.3, 867.5, 867.7, 867.9, 868.8,    // Uplink (and PJON)
//    869.525,                                                          // Downlink (+ Uplink channels 1-9)
    // TTN US902-928: Used in USA, Canada and South America
    //903.9, 904.1, 904.3, 904.5, 904.7, 904.9, 905.1, 905.3, 904.6,  // Uplink
    //923.3, 923.9, 924.5, 925.1, 925.7, 926.3, 926.9, 927.5          // Downlink
};
#ifdef ENABLE_BW_SCAN
//#define BW_COUNT 3
#define BW_COUNT 2
long int _bandwidths[] =
{
    500000,
    250000,
//    125000,
    // etc.
};
#else
#define BW_COUNT 1
long int _bandwidths[] =
{
    250000,
};
#endif
#ifdef ENABLE_SF_SCAN
//#define SF_COUNT 3
#define SF_COUNT 2
int _spreadingfactors[] =
{
    6,
    7,
//    8,
    // etc.
};
#else
#define SF_COUNT 1
int _spreadingfactors[] =
{
    7,
};
#endif

// How long should a frequency be monitored
//#define FREQ_TIME_MS            5000
#define FREQ_TIME_MS            4000  // as 5 * 12 = 60 s so minutely intervals get missed

// Used to track how many packets we have seen on each frequency above
uint16_t _packetCounts[FREQ_COUNT][BW_COUNT][SF_COUNT];

// Singleton instance of the radio driver
//RH_RF95 rf95(RFM95_CS, RFM95_INT);
RH_RF95 rf95;

//Adafruit_ILI9341 _tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

uint8_t  _rxBuffer[RH_RF95_MAX_MESSAGE_LEN];                // receive buffer
uint8_t  _rxRecvLen;                                        // number of bytes actually received
//char     _printBuffer[512]  = "\0";                         // to send output to the PC
uint32_t _freqExpire        = 0;                            // Millisecond at which frequency should change
uint32_t _freqIndex         = 0;                            // Which frequency is currently monitored
bool     _sdReady           = false;                        // Is a file open and ready for writing?
/*File     _logfile;                                          // Log file on SD card*/

uint32_t _bwIndex         = 0;                            // Which bandwidth is currently monitored
uint32_t _sfIndex         = 0;                            // Which spreading factor is currently monitored

void rf95_setFrequency(uint32_t index)
{
    if (!rf95.setFrequency(_frequencies[index]))
    {
        Serial.println(F("setFrequency failed"));
        while (1);
    }
//    rf95.setCodingRate4(5);

    Serial.print(F("Freq: "));
    Serial.print(_frequencies[index]);
//    Serial.print(F(" ("));
//    Serial.print(_packetCounts[index]);
//    Serial.println(F(")\t"));
    Serial.print(F("\t"));
}

void rf95_setBandwidth(uint32_t index)
{
    rf95.setSignalBandwidth(_bandwidths[index]);
//    rf95.setCodingRate4(5);

    Serial.print(F("BW: "));
    Serial.print(_bandwidths[index]);
    Serial.print(F("\t"));
}

void rf95_setSpreadingFactor(uint32_t index)
{
    rf95.setSpreadingFactor(_spreadingfactors[index]);
//    rf95.setCodingRate4(5);

    Serial.print(F("SF: "));
    Serial.print(_spreadingfactors[index]);
    Serial.print(F("\t"));
}

/*void sd_inititalize(void)
{
    if (SD.begin(SD_CS))
    {
        char filename[15];

        strcpy(filename, "LORA__00.TXT");

        for (uint8_t i = 0; i < 100; i++)
        {
            filename[6] = '0' + i / 10;
            filename[7] = '0' + i % 10;

            // create if does not exist, do not open existing, write, sync after write
            if (!SD.exists(filename))
            {
                break;
            }
        }

        _logfile = SD.open(filename, FILE_WRITE);

        if (!_logfile)
        {
            Serial.print("Couldnt create ");
            Serial.println(filename);
            tft_printfln(2, ILI9341_BLACK, ILI9341_RED, "File create failed  ");
        }
        else
        {
            Serial.print("Writing to ");
            Serial.println(filename);
            tft_printfln(2, ILI9341_BLACK, ILI9341_GREEN, "File: %12s  ", filename);

            // Set flag to indicate we can write to SD
            _sdReady = true;
        }
    }
    else
    {
        tft_printfln(2, ILI9341_BLACK, ILI9341_RED, "SD Init failed      ");
        Serial.println("SD.begin() failed!");
    }
}*/

void setup()
{
    for(int i=0; i < FREQ_COUNT; ++i)
        for(int j=0; j < BW_COUNT; ++j)
            for(int k=0; k < SF_COUNT; ++k)
                _packetCounts[i][j][k] = 0;

    // Configure pins
    pinMode(LED, OUTPUT);

    // Wait for serial to initialize before proceeding, so all output can be seen
    // This will block stand-alone operations
//    while (!Serial);

    // Initialize Serial
    Serial.begin(115200);
    delay(100);

    Serial.println(F("LoRa Network Probe"));
    Serial.println(F(__FILE__));

/*    // Initialize SD
    sd_inititalize();*/

    while (!rf95.init())
    {
        Serial.println(F("LoRa init failed"));
        while (1);
    }

    Serial.println(F("LoRa init OK"));

    rf95_setFrequency(_freqIndex);
////    rf95.setSpreadingFactor(7);
//    rf95.setSignalBandwidth(250000);
////    rf95.setCodingRate4(5);
    rf95_setBandwidth(_bwIndex);
    rf95_setSpreadingFactor(_sfIndex);
    Serial.print(F(" ("));
    Serial.print(_packetCounts[_freqIndex][_bwIndex][_sfIndex]);
    Serial.println(F(")"));
    rf95.setPromiscuous(true);

    // Update time for changing frequency
    _freqExpire = millis() + FREQ_TIME_MS;

/*    // Log the header for CSV output
    if (_sdReady)
    {
        _logfile.println(LOG_HEADER);
        _logfile.flush();
    }*/
}

void loop()
{
    _rxRecvLen = sizeof(_rxBuffer);

    digitalWrite(LED, LOW);

    rf95.setModeRx();

    // Handle incoming packet if available
    if (rf95.recv(_rxBuffer, &_rxRecvLen))
    {
        char isAck[4] = { "   " };

        digitalWrite(LED, HIGH);

        _packetCounts[_freqIndex][_bwIndex][_sfIndex]++;

        if (rf95.headerFlags() & RH_FLAGS_ACK)
        {
            memcpy(isAck, "Ack\0", 3);
        }

        _rxBuffer[_rxRecvLen] = '\0';

        // Output to TFT
        Serial.println(millis());

        Serial.print(F("Signal(RSSI)= "));
        Serial.print(rf95.lastRssi(), DEC);
        Serial.print(F(" (Freq: "));
        Serial.print((uint32_t)(_frequencies[_freqIndex] * 10), DEC);
        Serial.println(F(")"));

        Serial.print(F(" "));
        Serial.print(rf95.headerFrom(), DEC);
        Serial.print(F(" >> "));
        Serial.print(rf95.headerTo(), DEC);
        Serial.print(F(" MsgId:"));
        Serial.print(rf95.headerId(), DEC);
        Serial.print(F(" Flags:"));
        Serial.print(rf95.headerFlags(), HEX);
        Serial.print(F(" "));
        Serial.println(isAck);

        Serial.print(F(" "));
        Serial.print(_rxRecvLen, DEC);
        Serial.print(F(" => "));
        Serial.println((char*)_rxBuffer);

/*        // Prepare delimited output for log
        snprintf(_printBuffer, sizeof(_printBuffer), "%u,%u.%u,%04d,%d,%d,%d,%d,%02X,%s,%d,",
            millis(),
            (uint32_t)(_frequencies[_freqIndex] * 10) / 10,
            (uint32_t)(_frequencies[_freqIndex] * 10) % 10,
            _packetCounts[_freqIndex],
            rf95.lastRssi(),
            rf95.headerFrom(),
            rf95.headerTo(),
            rf95.headerId(),
            rf95.headerFlags(),
            isAck,
            _rxRecvLen
        );*/

        // Add bytes received as hex values
        for (int i = 0; i < _rxRecvLen; i++)
        {
//            snprintf(_printBuffer, sizeof(_printBuffer), "%s %02X", _printBuffer, _rxBuffer[i]);
            Serial.print(_rxBuffer[i], HEX);
            Serial.print(" ");
        }

        // Add bytes received as string - this is usually ugly and useless, but
        // it is here just in case. Maybe someone will send something in plaintext
//        snprintf(_printBuffer, sizeof(_printBuffer), "%s,%s", _printBuffer, _rxBuffer);
        Serial.println((char*)_rxBuffer);

/*        Serial.println(_printBuffer);

        if (_sdReady)
        {
            _logfile.println(_printBuffer);

            // Flushing adds overhead to the write time, but do it. FAT is VERY sensitive
            // to corruption.  If you don't flush regularly you WILL corrupt the File Allocation Table -
            // it is very difficult to avoid
            _logfile.flush();
        }*/
    }

    // Change frequency if it is time
    if (millis() > _freqExpire)
    {
        rf95.setModeIdle();

        _freqExpire = millis() + FREQ_TIME_MS;
        _freqIndex++;

        if (_freqIndex == FREQ_COUNT)
        {
            _freqIndex = 0;
            ++_bwIndex;
        }

        if (_bwIndex == BW_COUNT)
        {
            _bwIndex = 0;
            ++_sfIndex;
        }

        if (_sfIndex == SF_COUNT)
        {
            _sfIndex = 0;
        }

        rf95_setFrequency(_freqIndex);
        rf95_setBandwidth(_bwIndex);
        rf95_setSpreadingFactor(_sfIndex);
//    rf95.setCodingRate4(5);

        Serial.print(F(" ("));
        Serial.print(_packetCounts[_freqIndex][_bwIndex][_sfIndex]);
        Serial.println(F(")"));
    }
}
