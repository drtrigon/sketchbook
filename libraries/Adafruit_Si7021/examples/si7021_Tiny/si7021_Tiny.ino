#include "Adafruit_Si7021_Tiny.h"

Adafruit_Si7021_Tiny sensor = Adafruit_Si7021_Tiny();

void setup() {
  Serial.begin(115200);
  Serial.println("Si7021 test");
  sensor.begin();
}

void loop() {
  Serial.print("Humidity:    "); Serial.print(sensor.readHumidity(), 2);
  Serial.print("\tTemperature: "); Serial.println(sensor.readTemperature(), 2);
  delay(100);
}
