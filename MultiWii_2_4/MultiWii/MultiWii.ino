
/*
 * Welcome to MultiWii.
 *
 * If you see this message, chances are you are using the Arduino IDE. That is ok.
 * To get the MultiWii program configured for your copter, you must switch to the tab named 'config.h'.
 * Maybe that tab is not visible in the list at the top, then you must use the drop down list at the right
 * to access that tab. In that tab you must enable your baord or sensors and optionally various features.
 * For more info go to http://www.multiwii.com/wiki/index.php?title=Main_Page
 *
 * Have fun, and do not forget MultiWii is made possible and brought to you under the GPL License.
 *
 * Added drivers:
 *   * I2C gyroscope: LSM9DS0 (combo + ACC + MAGN)
 *   * I2C barometer: BME280
 *   * Receiver: ORANGERX R614XN DIY NANO (Spektrum) in 1 wire per channel PWM mode (not cPPM)
 *
 * Hints:
 *   * running MultiWiiConf in VM is too slow, but not needed as it needs java only:
 *     ursin@ThinkPad-T440-ursin:~/Downloads/sketchbook/MultiWii_2_4/MultiWiiConf/application.linux64$ sudo ./MultiWiiConf 
 *   * Binding procedure; 1. connect Vcc (red) to SPEK_BIND_POWER (A0) and cPPM/BIND (yellow) to SPEK_BIND_DATA (A1)
 *                        2. bind by pressing "RX BIND" button and carrying out the bind procedure of transmitter
 *                        3. Restore flying setup by connecting Vcc (red) to +5V and disconnect cPPM/BIND (yellow)
 *     (actually the best would be a hardware button for binding, because it always needs hardware intervention,
 *      see MultiWii_2_4/FlightController-bind-RX_PartSim.pdf)
 *
 */

