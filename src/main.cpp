#include <Arduino.h>
#include <Wire.h>

void setup() {
  Serial.begin(115200);
}

void loop() {
  uint16_t value = analogRead(35);
  Serial.printf("%d\n", value);
  delay(500);
}