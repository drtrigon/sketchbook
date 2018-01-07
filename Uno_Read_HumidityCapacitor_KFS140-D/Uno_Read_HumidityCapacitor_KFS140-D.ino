/*
* https://www.electronicshub.org/arduino-capacitance-meter/
* Capacitance Meter for Range 20 pF to 1000 nF
* https://gist.githubusercontent.com/elktros/0f38c598833477b1d6f211bad755beb8/raw/008574b7415c190b7b5d48d5b1340bd8ab2ce117/gistfile1.txt
* 
* Working Principle
* 
* For measuring smaller capacitances, we will use a different concept.
* For this we need to understand a little bit about the internal
* structure of ATmega328P.
* 
* All the I/O ports in ATmega328P Microcontroller have an internal
* pull–up resistor and an internal capacitor connected between the
* pin and ground. The following image shows the partial internal
* circuit of an I/O pin of ATmega328P Microcontroller.
* 
* We will be using the internal pull–up resistor and the stray
* capacitor in this circuit. Ignoring the diodes in the above circuit,
* the circuit for our project can be redrawn as follows.
* 
* Here, CT is the capacitor under test and CI is the internal
* capacitor. We need not worry about the internal capacitor and its
* value can be anywhere between 20 pF to 30 pF. The unknown capacitor
* is connected between A2 and A0 (Positive lead to A0, in case of
* polarized capacitor). Here, A2 acts as the charging pin and A0 acts
* as the discharging pin.
* 
* Initially, we will charge the unknown capacitor by setting A2 as
* HIGH and measure the voltage at A0 from the following formula.
* 
* VA0 = (VA2 X CT)/(CT+CI)
* 
* But we already know voltage at A0 with the help of analog read
* function. Hence, using that value in the above equation, we can get
* the unknown capacitance as follows.
* 
* CT = (CI X VA0)/(VA2-VA0 )
*/

#define sensor_mean  145.0  // KFS140-D (guessed value)

//#include <LiquidCrystal.h>
//LiquidCrystal lcd(6, 7, 2, 3, 4, 5);

const int analog_charge = A2;
const int analogPin = A0;

const float ck=24.42;

int voltage;
float cu;

void setup()
{
Serial.begin(9600);
//lcd.begin(16, 2);
//lcd.print(" 20pF<to>1000nF ");
Serial.println(F("Capacitance Meter for Range 20 pF to 1000 nF"));
//lcd.setCursor(0,1);
//lcd.print("place capacitor ");
Serial.println(F("place capacitor / KFS140-D"));
pinMode(analogPin,OUTPUT);      // low impedance (~0 ohms to GND)
pinMode(analog_charge,OUTPUT);  // low impedance (~0 ohms to GND)
}

void loop()
{
//     abc:
     // pF range
     pinMode(analogPin,INPUT);          // high impedance (Mohms to GND and/or VCC)
     digitalWrite(analog_charge,HIGH);  // charge against high impedance
     voltage=analogRead(analogPin);     // measure voltage
     digitalWrite(analog_charge,LOW);   // discharge
     //analog_charge = 998 || 999 || 1000 || 1001 and ! 1024
     pinMode(analogPin,OUTPUT);         // discharge and short/low impedance (~0 ohms to GND)
      if(voltage<1000)  // pF range
     {
      cu=((ck*voltage)/(1024.0-voltage));
      if(cu>20.0)
      {
//       lcd.setCursor(0,1);
//       lcd.print("                ");
//       lcd.setCursor(0,1);
//       lcd.print(cu,2);
       Serial.print(cu);
//       lcd.print("pF");
//       Serial.println(F(" pF"));
       Serial.print(F(" pF\t"));
       Serial.print(mapfloat(cu, sensor_mean-12.5, sensor_mean+12.5, 0., 100.));  // map pF to %RH; 0.25pF/%RH
       Serial.println(F(" %RH"));
      }
      else  // no cap present, or too small
      {
//       lcd.setCursor(0,1);
//       lcd.print("place capacitor ");
       Serial.println(F("place capacitor"));
       delay(200);
//       lcd.setCursor(0,1);
//       lcd.print("                ");
       delay(200);
//       goto abc;
       return;
      }
     }
    else  // nF range
    {
      // nF range
      voltage=0;
      pinMode(analogPin,OUTPUT);            // low impedance (~0 ohms to GND)
      delay(1);
      pinMode(analog_charge,INPUT_PULLUP);  // charge against pull-up resistor as impedance (kohms to VCC)
      unsigned long start_time = micros();
      unsigned long final_time=0;

      while((voltage < 1) && (final_time < 400000L))
      {
        voltage = digitalRead(analog_charge);
        unsigned long stop_time = micros();    // measure charge time
        final_time = stop_time > start_time ? stop_time - start_time : start_time - stop_time;
      }

      pinMode(analog_charge, INPUT);           // stop charging/high impedance (Mohms to GND and/or VCC)
      voltage = analogRead(analog_charge);     // measure voltage charged to
      digitalWrite(analogPin, HIGH);           // low impedance (~0 ohms to VCC); continue charge against high impedance
      int delay_T = (int)(final_time / 1000L) * 5;
      delay(delay_T);
      pinMode(analog_charge, OUTPUT);          // low impedance (~0 ohms to GND); continue charge against low impedance
      digitalWrite(analog_charge, LOW);
      digitalWrite(analogPin, LOW);            // discharge and short/low impedance (~0 ohms to GND)

      cu = -(float)final_time / 34.8 ;
      cu /= log(1.0 - (float)voltage / (float)1023);
      if(cu < 1000.0)
      {
//       lcd.setCursor(0,1);
//       lcd.print("                ");
//       lcd.setCursor(0,1);
//       lcd.print(cu,2);
       Serial.print(cu);
//       lcd.print("nF");
       Serial.println(F(" nF"));
      }
      else  // cap out of range, too big
      {
//       lcd.setCursor(0,1);
//       lcd.print("                ");
//       lcd.setCursor(0,1);
//       lcd.print("**Out of Range**");
       Serial.println(F("**Out of Range**"));
      }
    }
    delay(1000);
}

// https://forum.arduino.cc/index.php?topic=3922.msg30007#msg30007
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
 return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
