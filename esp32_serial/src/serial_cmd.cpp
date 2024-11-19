#include "serial_cmd.h"

SerialCmd::SerialCmd()
    : c_cmdIndexAry{{"/setRate", 1},
                    {"/setWiFi", 2},
                    {"/setUDP", 3},
                    {"/help", 4},
                    {"/close", 5}} {}

SerialCmd::~SerialCmd() {}

uint8_t SerialCmd::findIndex(char *cmd) {
  for (size_t i = 0; i < CMD_CONFIG; i++) {
    if (strcmp(c_cmdIndexAry[i].cmd, cmd) == 0) {
      return c_cmdIndexAry[i].cmdidx;
    }
  }
  return 0;
}

size_t SerialCmd::cmdScanf() {
  Serial.println("\r\n -> Console Activated!\r\n");
  while (true) {
    size_t cmdLen = Serial.available();
    if (cmdLen) {
      size_t index = 0;
      char cmdHead[24];
      char value1[24];
      char value2[24];
      for (;;) {
        if (index < cmdLen) {
          _rcvCommand[index++] = Serial.read();
        } else {
          _rcvCommand[index] = '\0';
          Serial.print("Rcv: ");
          Serial.println(_rcvCommand);
          if (_rcvCommand[0] == '/') {
            sscanf(_rcvCommand, "%23s %23s %23s", cmdHead, value1, value2);
            switch (findIndex(cmdHead)) {
              case 1:
                cmdSetRate(value1, value2);
                break;
              case 2:
                cmdSetWiFi(value1, value2);
                break;
              case 3:
                cmdSetUDP(value1, value2);
                break;
              case 4:
                cmdGetHelp();
                break;
              case 5:
                Serial.println(" -> Console Closed!\r\n");
                return 0;
                break;
              default:
                log_e("Invalid Command!\r\n" HELPTIP "\r\n");
                break;
            }
          }
          break;
        }
      }
    }
  }
  return 0;
}

void SerialCmd::cmdGetHelp() {
  Serial.println(" -> Enter \"/setRate <gapTime> <runTime>\" To Set Rate");
  Serial.println(" -> Enter \"/setWiFi <SSID> <PSWD>\" To Set WiFi Config");
  Serial.println(" -> Enter \"/setUDP <IPV4> <PORT>\" To Set UDP Traget");
  Serial.println(" -> Enter \"/help\" For More Info");
  Serial.println(" -> Enter \"/close\" To Close Console\r\n");
}

void SerialCmd::cmdSetRate(char *gaptime, char *runtime) {
  uint8_t t_gaptime = atoi(gaptime);
  uint8_t t_runtime = atoi(runtime);

  if (t_gaptime == c_GAPTIME[0] || t_runtime == c_RUNTIME[0]) {
    _configValue.gapTime = c_GAPTIME[0];
    _configValue.runTime = c_RUNTIME[0];

    Serial.println(" -> Set To Real-Time Mode!\r\n");
  } else {
    for (size_t i = 0; i < GAPTIME_OPT_CNT; i++) {
      if (t_gaptime == c_GAPTIME[i]) {
        _configValue.gapTime = t_gaptime;
        break;
      } else if (i == GAPTIME_OPT_CNT - 1) {
        log_e("Invalid GapTime!\r\n -> Valid GapTime: 0, 60\r\n");
        return;
      }
    }

    for (size_t j = 0; j < RUNTIME_OPT_CNT; j++) {
      if (t_runtime == c_RUNTIME[j]) {
        _configValue.runTime = t_runtime;
        break;
      } else if (j == RUNTIME_OPT_CNT - 1) {
        log_e("Invalid RunTime!\r\n -> Valid RunTime: 2, 5, 10, 20, 30\r\n");
        return;
      }
    }

    Serial.print(" -> Set GapTime: ");
    Serial.println(_configValue.gapTime);
    Serial.print(" -> Set RunTime: ");
    Serial.println(_configValue.runTime);
    Serial.println();
  }

  // To do : add preference save
}

void SerialCmd::cmdSetWiFi(char *ssid, char *pswd) {
  strcpy(_configValue.ssid, ssid);
  strcpy(_configValue.pswd, pswd);

  Serial.print(" -> Set WiFi SSID: ");
  Serial.println(_configValue.ssid);
  Serial.print(" -> Set WiFi Password: ");
  Serial.println(_configValue.pswd);
  Serial.println();
  // To do : add preference save
  // may need transform to String
}

void SerialCmd::cmdSetUDP(char *ipv4, char *port) {
  char *p_t_ipv4 = ipv4;
  char *p_t_port = port;

  uint8_t t_ipv4_len = strlen(p_t_ipv4);
  uint8_t t_point_cnt = 0;
  uint8_t t_number_cnt = 0;

  for (size_t i = 0; i < t_ipv4_len; i++) {
    if (p_t_ipv4[i] == '.') {
      t_point_cnt++;
      if (t_number_cnt > 3 || t_number_cnt < 1) {
        log_e("Invalid IP Format!\r\n -> Valid Format: xxx.xxx.xxx.xxx\r\n");
        return;
      }
      t_number_cnt = 0;
    } else {
      t_number_cnt++;
    }
  }
  if (t_point_cnt != 3 || t_number_cnt > 3 || t_number_cnt < 1) {
    log_e("Invalid IP Format!\r\n -> Valid Format: xxx.xxx.xxx.xxx\r\n");
    return;
  }

  if (strlen(p_t_port) > 5 || strlen(p_t_port) < 1) {
    log_e("Invalid Port Value!\r\n -> Valid Port Value: 1-65535\r\n");
    return;
  }

  uint32_t t_port = atoi(p_t_port);
  if (t_port > 65535 || t_port < 1) {
    log_e("Invalid Port Value!\r\n -> Valid Port Value: 1-65535\r\n");
    return;
  }

  strcpy(_configValue.ipv4, p_t_ipv4);
  _configValue.port = t_port;

  Serial.print(" -> Set Target IP: ");
  Serial.println(_configValue.ipv4);
  Serial.print(" -> Set Target Port: ");
  Serial.println(_configValue.port);
  Serial.println();
  // To do : add preference save
}

#ifdef ARCHIVE
void SerialCmd::cmdSplit() {
  String cmd = Serial.readString();
  int startIndex = 0;
  int endIndex = 0;
  if (cmd.isEmpty() == false) {
    while (endIndex != -1) {
      endIndex = cmd.indexOf(' ', startIndex);
      if (endIndex != -1) {
        String subStr = cmd.substring(startIndex, endIndex);
        Serial.println(subStr);
        startIndex = endIndex + 1;
      } else {
        String subStr = cmd.substring(startIndex);
        Serial.println(subStr);
      }
    }
  }
}
#endif