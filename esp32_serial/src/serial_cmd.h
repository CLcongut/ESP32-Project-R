#pragma once
#include <Arduino.h>

#define SCANF_BUF_SIZE 256
#define CMD_CONFIG 5
#define HELPTIP " -> Enter \"/help\" For More Info"

#define GAPTIME_OPT_CNT 2
#define GAPTIME_OPT {0, 60}
#define RUNTIME_OPT_CNT 6
#define RUNTIME_OPT {0, 2, 5, 10, 20, 30}

typedef struct {
  char cmd[24];
  uint8_t cmdidx;
} CommandIndex;

typedef struct {
  uint8_t gapTime;
  uint8_t runTime;
  char ssid[24];
  char pswd[24];
  char ipv4[16];
  uint16_t port;
} ConfigValue;

class SerialCmd {
 private:
  char _rcvCommand[SCANF_BUF_SIZE];
  ConfigValue _configValue;
  const CommandIndex c_cmdIndexAry[CMD_CONFIG];
  const uint8_t c_GAPTIME[GAPTIME_OPT_CNT] = GAPTIME_OPT;
  const uint8_t c_RUNTIME[RUNTIME_OPT_CNT] = RUNTIME_OPT;

 public:
  SerialCmd();
  ~SerialCmd();
  size_t cmdScanf();
  uint8_t findIndex(char *cmd);
  void cmdGetHelp();
  void cmdSetRate(char *gaptime, char *runtime);
  void cmdSetWiFi(char *ssid, char *pswd);
  void cmdSetUDP(char *ipv4, char *port);
};