/*
** DEPRECIATED: Use Datei > Beispiele > Firmata > StandardFirmata
**              with http://www.pjrc.com/teensy/firmata_test/firmata_test.64bit
**
**          OR: Use Datei > Beispiele > 01.Basics > Blink
*/

int ledPin = 13; 

void setup()
{
  pinMode(ledPin, OUTPUT);

  digitalWrite(ledPin, HIGH);
  delay(10000);
  digitalWrite(ledPin, LOW);
  delay(10000);
  digitalWrite(ledPin, HIGH);
}

void loop()
{
/*  digitalWrite(ledPin, HIGH);
  delay(1000);
  digitalWrite(ledPin, LOW);
  delay(1000);*/
}
