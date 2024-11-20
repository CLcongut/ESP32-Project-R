#include <Arduino.h>
#include "serial_cmd.h"

SerialCmd serialCmd;

void setup() {
  Serial.begin(115200);
  serialCmd.begin();
  serialCmd.cmdScanf();
  ConfigValue cfv = serialCmd.cmdGetConfig(true);

  // Serial.println(cfv.gapTime);
  // Serial.println(cfv.runTime);
  // Serial.println(cfv.ssid);
  // Serial.println(cfv.pswd);
  // Serial.println(cfv.ipv4);
  // Serial.println(cfv.port);

  serialCmd.~SerialCmd();
}

void loop() {}