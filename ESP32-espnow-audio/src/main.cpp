#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include "myI2S.h"

#define sample_buffer_size 1000
int16_t *soundata;
myI2S duplex;
esp_now_peer_info_t peerInfo;
uint8_t broadcastAddress[] = {0xA8, 0x42, 0xE3, 0x9E, 0xE0, 0x44};

#define DATAIN
// #define DATAOUT

#ifdef DATAIN
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  // Serial.print("\r\nLast Packet Send Status:\t");
  // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
#endif

#ifdef DATAOUT
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  for (uint32_t i = 0; i < len / 2; i++)
  {
    soundata[i] += incomingData[i * 4] << 8;
    // Serial.println(soundata[i]);
    soundata[i] += incomingData[i * 4 + 1];
    Serial.println(soundata[i]);
    // soundata[i] /= 2;
  }
  // for (uint32_t i = 0; i < len; i ++)
  // {
  //   Serial.println(incomingData[i]);
  // }
  duplex.Write(soundata, sample_buffer_size);
}
#endif

void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  Serial.println(WiFi.macAddress());

  soundata = (int16_t *)calloc(sample_buffer_size, sizeof(int16_t));

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  duplex.begin(0);
  // duplex.SetSampleRate(16000);
  duplex.SetChannelFormat(ONLY_RIGHT);
#ifdef DATAIN
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;     // 通道
  peerInfo.encrypt = false; // 是否加密为False

  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  duplex.SetDMABuffer(4, 64);
  duplex.SetInputMode(16, 17, 5);
#endif
#ifdef DATAOUT
  esp_now_register_recv_cb(OnDataRecv);
  duplex.SetOutputMode(13, 12, 14);
#endif
}

void loop()
{
#ifdef DATAIN
  // uint8_t trasnData[sample_buffer_size * sizeof(int16_t)];
  duplex.Read(soundata, sample_buffer_size);
  for (uint32_t i = 0; i < sample_buffer_size; i++)
  {
    // trasnData[i * 2] = (uint8_t)(soundata[i] >> 8);
    // trasnData[i * 2 + 1] = (uint8_t)(soundata[i]);
    Serial.printf("%d\n", soundata[i]);
  }
  // esp_err_t result = esp_now_send(broadcastAddress, trasnData, sample_buffer_size * sizeof(int16_t));
  // delay(1);

  // 判断是否发送成功
  // if (result == ESP_OK)
  // {
  //   Serial.println("Sent with success");
  // }
  // else
  // {
  //   Serial.println("Error sending the data");
  // }

#endif
}
