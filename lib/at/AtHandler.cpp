#include "AtHandler.h"

#include <Arduino.h>
#include <Wire.h>
#include <esp32-hal-log.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

// reading states
#define READING_CHARS 2
#define TIMEOUT 5

#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "1.0"
#endif

#ifndef PIN_SOILM
#define PIN_SOILM 0
#endif

#ifndef PIN_PUMP
#define PIN_PUMP 0
#endif

AtHandler::AtHandler() {
  if (PIN_SOILM != 0) {
    pinMode(PIN_SOILM, INPUT);
    log_i("soil sensor configured on %d", PIN_SOILM);
  }
  if (PIN_PUMP != 0) {
    pinMode(PIN_PUMP, OUTPUT);
    log_i("pump configured on %d", PIN_PUMP);
  }
  this->loadConfig();
}

void AtHandler::handle(Stream *in, Stream *out) {
  size_t length = read_line(in);
  if (length == 0) {
    return;
  }
  if (strcmp("AT", this->buffer) == 0) {
    out->print("OK\r\n");
    return;
  }
  if (strcmp("AT+GMR", this->buffer) == 0) {
    out->print(FIRMWARE_VERSION);
    out->print("\r\n");
    out->print("OK\r\n");
    return;
  }

  if (strcmp("AT+SOILM?", this->buffer) == 0) {
    this->handleSoilMoisture(out);
    return;
  }

  if (strcmp("AT+MINSOILM", this->buffer) == 0) {
    this->handleMinSoilMoistureSetup(out);
    return;
  }

  if (strcmp("AT+MAXSOILM", this->buffer) == 0) {
    this->handleMaxSoilMoistureSetup(out);
    return;
  }

  if (strcmp("AT+PUMPOFF", this->buffer) == 0) {
    this->handlePumpOff(out);
    return;
  }

  int timeout;
  int matched = sscanf(this->buffer, "AT+PUMP=%d", &timeout);
  if (matched == 1) {
    this->handlePump(timeout, out);
    return;
  }

  //FIXME pump with autolevel

  out->print("unknown command\r\n");
  out->print("ERROR\r\n");
}

void AtHandler::handlePumpOff(Stream *out) {
  if (PIN_PUMP == 0) {
    out->printf("pump pin (PIN_PUMP) is not configured\r\n");
    out->print("ERROR\r\n");
    return;
  }
  digitalWrite(PIN_PUMP, HIGH);
  out->print("OK\r\n");
}

void AtHandler::handlePump(int timeout, Stream *out) {
  if (PIN_PUMP == 0) {
    out->printf("pump pin (PIN_PUMP) is not configured\r\n");
    out->print("ERROR\r\n");
    return;
  }
  digitalWrite(PIN_PUMP, LOW);
  delay(timeout);
  digitalWrite(PIN_PUMP, HIGH);
  out->print("OK\r\n");
}

size_t AtHandler::read_line(Stream *in) {
  //Check to see if anything is available in the serial receive buffer
  while (in->available() > 0) {
    static unsigned int message_pos = 0;
    //Read the next available byte in the serial receive buffer
    char inByte = in->read();
    if (inByte == '\r') {
      continue;
    }
    //Message coming in (check not terminating character) and guard for over message size
    if (inByte != '\n' && (message_pos < BUFFER_LENGTH - 1)) {
      //Add the incoming byte to our message
      this->buffer[message_pos] = inByte;
      message_pos++;
    } else {
      //Add null character to string
      this->buffer[message_pos] = '\0';
      size_t result = message_pos;
      message_pos = 0;
      return result;
    }
  }
  return 0;
}

void AtHandler::handleSoilMoisture(Stream *out) {
  if (PIN_SOILM == 0) {
    out->printf("soil moisture pin (PIN_SOILM) is not configured\r\n");
    out->print("ERROR\r\n");
    return;
  }
  int value = analogRead(PIN_SOILM);
  out->printf("%d\r\n", value);
  out->print("OK\r\n");
}

void AtHandler::handleMaxSoilMoistureSetup(Stream *out) {
  if (PIN_SOILM == 0) {
    out->printf("soil moisture pin (PIN_SOILM) is not configured\r\n");
    out->print("ERROR\r\n");
    return;
  }
  int value = analogRead(PIN_SOILM);

  preferences.begin("plant-watering", false);
  preferences.putInt("maxsoilm", value);
  preferences.end();

  this->maxSoilMoisture = value;
  out->printf("%d\r\n", value);
  out->print("OK\r\n");
}

void AtHandler::handleMinSoilMoistureSetup(Stream *out) {
  if (PIN_SOILM == 0) {
    out->printf("soil moisture pin (PIN_SOILM) is not configured\r\n");
    out->print("ERROR\r\n");
    return;
  }
  int value = analogRead(PIN_SOILM);

  preferences.begin("plant-watering", false);
  preferences.putInt("minsoilm", value);
  preferences.end();

  this->minSoilMoisture = value;
  out->printf("%d\r\n", value);
  out->print("OK\r\n");
}

void AtHandler::loadConfig() {
  if (!preferences.begin("plant-watering", true)) {
    return;
  }
  this->minSoilMoisture = preferences.getInt("minsoilm", 0);
  this->maxSoilMoisture = preferences.getInt("maxsoilm", 0);
  preferences.end();
}
