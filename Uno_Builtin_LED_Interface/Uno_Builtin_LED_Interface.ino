/* Builtin LED Photodetector Test (LED can still be used)
 *
 * Connect LED Pin D13 to Analog Input Pin A0.
 * 
 * This code allows to use the LED additionally e.g. as a button.
 *
 * Caution: This is a weak signal (high impedance) measurement.
 *          This means is is very sensitive to distortions like
 *          e.g. touching the wire (even if it is isolated).
 *
 * 2018: New Nano bootloader uses 115200 upload baudrate but
 *       IDE <= 1.8.5 have 57600 configured. Select "Uno" board
 *       to workaround as it has 115200 configured.
 * 2018: New Micro bootloader conflicts under linux with
 *       `modemmanager` thus remove it using:
 *       $ sudo apt-get remove modemmanager
 *       After this change it is possible to upload using same
 *       scheme as for Yun (thus may be add 2341:8037 and 2341:0037
 *       to VM as USB devices) - BUT ONCE ONLY during that upload
 *       the Micro changed name from 'Srl' to 'LLC' and now the
 *       bootloader does not react anymore - just serial output
 *       comes and messes up with avrdude...! Broken bootloader...?
 *       Fix:
 *       - use Uno/Nano as ISP with "ArduinoISP" example sketch
 *       - connect as follows:
 *         https://microtherion.github.io/ScratchMonkey/v1/Connection.html
 *         http://pinoutguide.com/Electronics/arduino_micro_pinout.shtml
 *         Uno / Nano    Micro ICSP    Arduino Micro
 *          Pin  13       Pin 3              -USB-
 *          Pin  12       Pin 1              |   |
 *          Pin  11       Pin 4              |12 |
 *          Pin  10       Pin 5         ICSP |34 |
 *          Pin  5V       Pin 2         pins |56 |
 *          Pin GND       Pin 6              -----
 *       - select as programmer "Arduino as ISP",
 *                as board "Arduino/Genuino Micro" and 
 *                as port e.g. "/dev/ttyACM0"
 *       - run Tools > Burn Bootloader (on issues reset both)
 */

// These constants won't change. They're used to give names to the pins used:
#define ANALOG_PIN  A0  // Analog input pin that the potentiometer is attached to

/* Read brightness/illumination voltage from builtin LED.
 *
 * mode: DEFAULT  - auto-select proper range
 *       INTERNAL - low brightness mode (high sensitivity)
 *
 * When it returns the BUILTIN LED pin is set to INPUT
 * and analogReference to DEFAULT (5V).
 */
float readBrightness(int mode = DEFAULT) {
  long value = 0;
  analogReference(mode);
  pinMode(LED_BUILTIN, INPUT);
  delay(10);
  analogRead(ANALOG_PIN);  // dump first reading
  for(int i = 0; i < 1000; ++i) {
    value += analogRead(ANALOG_PIN);
  }
  switch(mode) {
  case INTERNAL:
    analogReference(DEFAULT);
    return (value / 1000. * (1.1/5.));
    break;
  default:
    //if((mode == DEFAULT) && (value < (150L * 1000L))) {
    if(value < (190L * 1000L)) {
      //return readBrightness(INTERNAL) * -1.;  // debug
      return readBrightness(INTERNAL);
    }
    return (value / 1000.);
    break;
  }
}

bool isBright() {
  //return (readBrightness(INTERNAL) > 150.);  // max. ~219
  return (readBrightness(INTERNAL) > 190.);  // max. ~219
}

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
  /*while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }*/

  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, INPUT);
  analogReference(DEFAULT);
}

void loop() {
  // read the brightness from analog in
  Serial.println(readBrightness());
//  Serial.println(isBright());

  // the LED can still be used for blinking e.g.
  // 1. when the blinking sequence ends with LOW; set the
  //    pin to INPUT again and add a delay (otherwise
  //    reading brightness gets distorted)
  // 2. when the blinking sequence ends with HIGH; no need
  //    to set the pin to INPUT but adding a delay does help
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(LED_BUILTIN, INPUT);      // set pin to INPUT (when ending on LOW)
  delay(500);                       // allow LED to settle
}

