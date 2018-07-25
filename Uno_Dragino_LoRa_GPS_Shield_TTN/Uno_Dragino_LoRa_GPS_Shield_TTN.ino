/**
 * @brief Send GPS and Sensor Data as valid LoRaWAN packet to TTN
 *
 * @file Uno_Dragino_LoRa_GPS_Shield_TTN/Uno_Dragino_LoRa_GPS_Shield_TTN.ino
 *
 * @author drtrigon
 * @date 2018-07-25
 * @version 1.0
 *   @li first version derived from
 *       @see https://github.com/goodcheney/Arduino-Profile-Examples/blob/patch-4/libraries/Dragino/examples/LoRa/LoRaWAN/LoRa_GPS_Shield_TTN
 *       @see https://github.com/dragino/Arduino-Profile-Examples/blob/master/libraries/Dragino/examples/LoRa/LoRaWAN/Arduino_LMIC/Arduino_LMIC.ino
 *       @see https://github.com/dragino/Arduino-Profile-Examples/blob/master/libraries/Dragino/examples/GPS/tinygps_example/tinygps_example.ino
 *       @see http://wiki.dragino.com/index.php?title=Connect_to_TTN#Create_LoRa_end_devices
 *
 * This example sends a valid LoRaWAN packet with binary payload,
 * using frequency and encryption settings matching those of
 * the (early prototype version of) The Things Network.
 *
 * Note: LoRaWAN per sub-band duty-cycle limitation is enforced (1% in g1,
 * 0.1% in g2).
 *
 * Change NWKSKEY, APPSKEY and DEVADDR to the keys given for your
 * account and device. Currently it is defined for
 * @see https://console.thethingsnetwork.org/applications/vitudurum-drtrigon/devices/test_01
 *
 * In order to retreive the data from TTN use the python script given
 * @ref Uno_Dragino_LoRa_GPS_Shield_TTN/request-data-ttn-restful.py
 *
 * Do not forget to define the radio type correctly in config.h.
 * Currently set for Dragino LoRa GPS Shield.
 *
 * Required Libraries:
 *   * LMIC (LoRaWAN): https://github.com/matthijskooijman/arduino-lmic
 *     (select "Version 1.5.0+arduino-2" otherwise sketch won't fit into 32K)
 *   * TinyGPS: http://arduiniana.org/libraries/tinygps/
 * 
 * Required Hardware:
 *   * LoRa Shield + Arduino
 *   * LoRa GPS Shield + Arduino
 *   * LoRa Mini etc.
 *
 * Hardware Connection:
 *   * Arduino UNO + LoRa GPS Shield (if use another shield, remove GPS parts)
 *   * Wire GPS_RXD to Arduino A1
 *     (SoftwareSerial TX is analog pin A1 - connect to RX of other device; GPS_RXD = RXD1)
 *   * Wire GPS_TXD to Arduino A2
 *     (SoftwareSerial RX is analog pin A2 - connect to TX of other device; GPS_TXD = TXD1)
 *   * Remove jumper in GPS_RXD/GPX_TXD 1x3 pin header
 *   a photo for hardware configuration is here: http://wiki.dragino.com/index.php?title=File:GPSshiwu.png
 *
 * Thanks to:
 * Thomas Telkamp - 2015
 * Matthijs Kooijman - 2015
 * Edwin Chen - modified 21 Mar 2017
 *              <support@dragino.com>
 *              Dragino Technology Co., Limited
 */

#include <lmic.h>
#include <hal/hal.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>

/* Application ID: vitudurum-drtrigon
   Device ID: test_01
   ttn*/

/* LoRaWAN NwkSKey, network session key
   This is the default Semtech key, which is used by the prototype TTN
   network initially.
   ttn*/
static const PROGMEM u1_t NWKSKEY[16] = { 0xC9,0xF6,0xE4,0xED,0x16,0xB6,0x8E,0x48,0x95,0x2F,0xC9,0x50,0x8A,0x09,0xB3,0x2E };

/* LoRaWAN AppSKey, application session key
   This is the default Semtech key, which is used by the prototype TTN
   network initially.
   ttn*/
static const u1_t PROGMEM APPSKEY[16] = { 0x41,0xF6,0x3D,0xC0,0x67,0xDD,0x01,0xF2,0x73,0x37,0x7C,0xA4,0x56,0x2C,0xD0,0x32 };

/*
 LoRaWAN end-device address (DevAddr)
 See http://thethingsnetwork.org/wiki/AddressSpace
 ttn*/
static const u4_t DEVADDR = 0x26011375;

/* Schedule TX every this many seconds (might become longer due to duty
 cycle limitations).*/
const unsigned TX_INTERVAL = 20;

unsigned int count = 1;        //For times count (reset frame counters in TTN!)


TinyGPS gps;
SoftwareSerial ss(A2, A1); // Arduino RX, TX to conenct to GPS module.


float flat, flon, falt;

float payload[5];        // single type payload of 5 * 4 bytes = 20 bytes
/*struct payload_t {       // mix different types in payload, e.g.
    float val1; uint8_t val2; char val3; // etc. ...
}
payload_t payload;*/

//static uint8_t mydata[] = "Hello, world!";      //For test using.


/* These callbacks are only used in over-the-air activation, so they are
  left empty here (we cannot leave them out completely unless
   DISABLE_JOIN is set in config.h, otherwise the linker will complain).*/
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

static osjob_t sendjob;

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 10,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 9,
    .dio = {2, 6, 7},
};

void do_send(osjob_t* j)
{
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println("OP_TXRXPEND, not sending");
    } else {
        Serial.println("");

        GPSRead();
        GPSPrint();
        count++;
        smartdelay(1000);

        payload[0] = flon;
        payload[1] = flat;
        payload[2] = falt;
        payload[3] = readVcc();
        payload[4] = readTemp();

        // Prepare upstream data transmission at the next possible time.
        //LMIC_setTxData2(1, mydata, sizeof(mydata)-1, 0);
        LMIC_setTxData2(1,(char*)(&payload),sizeof(payload),0);

        Serial.println("Packet queued");
        Serial.print("LMIC.freq:");
        Serial.println(LMIC.freq);
        Serial.println("");
        Serial.println("Receive data:");
        
    } 
    // Next TX is scheduled after TX_COMPLETE event.
}

void onEvent(ev_t ev)
{
    Serial.print(os_getTime());
    Serial.print(": ");
    Serial.println(ev);
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            break;
        case EV_RFU1:
            Serial.println(F("EV_RFU1"));
            break;
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if(LMIC.dataLen) {
                // data received in rx slot after tx
                Serial.print(F("Data Received: "));
                Serial.write(LMIC.frame+LMIC.dataBeg, LMIC.dataLen);
                Serial.println();
            }
            // Schedule next transmission
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
         default:
            Serial.println(F("Unknown event"));
            break;
    }
}

void GPSRead()
{
    unsigned long age;
    gps.f_get_position(&flat, &flon, &age);
    falt=gps.f_altitude();  //get altitude
    flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6;//save six decimal places
    flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6;
    falt == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : falt, 2;//save two decimal places
}

void GPSPrint()
{
    /*Convert GPS data to format*/
    if(flon!=1000.000000) {
        Serial.print("###########    ");
        Serial.print("NO.");
        Serial.print(count);
        Serial.println("    ###########");
        Serial.println("The longtitude and latitude and altitude are:");
        Serial.print("[");
        Serial.print(flon);
        Serial.print(",");
        Serial.print(flat);
        Serial.print(",");
        Serial.print(falt);
        Serial.println("]\n");
    }
}

static void smartdelay(unsigned long ms)
{
    unsigned long start = millis();
    do {
        while (ss.available()) {
            gps.encode(ss.read());
        }
    } while (millis() - start < ms);
}


void setup() {
    Serial.begin(9600);
    ss.begin(9600);
    //while(!Serial);
    Serial.println("LoRa GPS Example---- ");
    Serial.println("Connect to TTN");

    // LMIC init
    os_init();
    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();
    /*LMIC_setClockError(MAX_CLOCK_ERROR * 1/100);
     Set static session parameters. Instead of dynamically establishing a session
     by joining the network, precomputed session parameters are be provided.*/
    #ifdef PROGMEM
    /* On AVR, these values are stored in flash and only copied to RAM
       once. Copy them to a temporary buffer here, LMIC_setSession will
       copy them into a buffer of its own again.*/
    uint8_t appskey[sizeof(APPSKEY)];
    uint8_t nwkskey[sizeof(NWKSKEY)];
    memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
    memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
    LMIC_setSession (0x1, DEVADDR, nwkskey, appskey);
    #else
    // If not running an AVR with PROGMEM, just use the arrays directly 
    LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);
    #endif
    
    // Disable link check validation
    LMIC_setLinkCheckMode(0);

    // TTN uses SF9 for its RX2 window.
    LMIC.dn2Dr = DR_SF9;

    // Set data rate and transmit power (note: txpow seems to be ignored by the library)
    LMIC_setDrTxpow(DR_SF7,14);

    // Start job
    do_send(&sendjob);
}

void loop()
{
    os_runloop_once();
}


/**
 *  Accessing the secret voltmeter on the Arduino 168 or 328.
 *
 * @see receiver_function()
 * @see http://code.google.com/p/tinkerit/wiki/SecretVoltmeter
 * @return The supply voltage in [V]
 */
float readVcc()
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
  return result / 1000.;
}

/**
 *  Internal Temperature Sensor for ATmega328 types.
 *
 * @param void
 * @see receiver_function()
 * @see https://playground.arduino.cc/Main/InternalTemperatureSensor
 * @return The chip temperature in [°C]
 */
float readTemp(void)
{
  unsigned int wADC;
  float t;

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
