#include <Arduino.h>
#include "serial_cmd.h"

SerialCmd serialCmd;

void setup() {
  Serial.begin(115200);
  serialCmd.cmdScanf();
}

void loop() {}