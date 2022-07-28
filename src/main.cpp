#include <Arduino.h>
#include <esp32-hal-log.h>
#include <Wire.h>
#include <AtHandler.h>

#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "1.0"
#endif

AtHandler *handler;

void setup() {
  Serial.begin(115200);
  log_i("starting. firmware version: %s", FIRMWARE_VERSION);

  handler = new AtHandler();
  log_i("setup completed");
}

void loop() {
  handler->handle(&Serial, &Serial);
}