#ifndef AtHandler_h
#define AtHandler_h

#include <Stream.h>
#include <Preferences.h>

#define BUFFER_LENGTH 1024

class AtHandler {
 public:
  AtHandler();
  void handle(Stream *in, Stream *out);

 private:
  void handlePumpOff(Stream *out);
  void handlePump(int timeout, Stream *out);
  void handleMaxSoilMoistureSetup(Stream *out);
  void handleMinSoilMoistureSetup(Stream *out);
  void handleSoilMoisture(Stream *out);
  void loadConfig();
  size_t read_line(Stream *in);
  char buffer[BUFFER_LENGTH];
  bool receiving = false;
  Preferences preferences;
  int minSoilMoisture;
  int maxSoilMoisture;

  uint8_t config_version = 1;
};

#endif