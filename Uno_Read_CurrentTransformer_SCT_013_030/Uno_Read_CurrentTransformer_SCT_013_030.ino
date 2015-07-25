/*
** Read SCT-013-030 CT from A4
**
** for how to connect see below, for datasheet the internet please.
*/

int analogPin = A4;    // CT Model: SCT-013-030 connected to analog pin 4 (middle pin of green connector)
                       // http://shop.boxtec.ch/beruehrungsloser-wechselstrom-messwandler-30a-max-p-40267.html (30A = 1V)

                       // outside (on green connector) leads to +3.3V

int val = 0;           // variable to store the value read
int vmin, vmax;        // variable to store the min. and max. values read


void setup()
{

  Serial.begin(9600);               //  setup serial

}


void loop()
{

  // amplitude measurement
  vmin = 1023;
  vmax = 0;
  for (int i=0; i <= 2047; i++){    // read about 10 50Hz periods (see below)
    val = analogRead(analogPin);    // read the input pin (at about 10kHz)
//    vmin = min(val, vmin);
//    vmax = max(val, vmax);
    if (val < vmin)
      vmin = val;
    if (val > vmax)
      vmax = val;
  } 

  // output
/*
  //Serial.println(vmax);             // max. value
  //Serial.println(vmin);             // min. value
  Serial.println(0.5*(vmax-vmin));  // amplitude/span value
  Serial.println(val);              // debug value
  //Serial.println((5.0*val)/1023);   // debug value in volts
  Serial.println("");               // debug newline
*/

  //Serial.print(0.5*(vmax-vmin));    // amplitude/span value
  Serial.print((30.0*5.0*0.5*(vmax-vmin))/1023 - 1.0); // current in amp
  Serial.print(" ");                // separator
  //Serial.print(val);                // last value
  Serial.print(5.0*val/1023);       // last value in volts
  Serial.print("\n");               // newline
  //delay(500);                       // debug

}

