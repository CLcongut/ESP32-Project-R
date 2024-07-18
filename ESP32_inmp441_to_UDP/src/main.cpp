#include <Arduino.h>
#include <driver/i2s.h>
#include <WIFI.h>

#define wifi_SSID "CCongut"
#define wifi_PSWD "88888888"
// #define wifi_SSID "CLcongut"
// #define wifi_PSWD "88888888"

#define SAMPLE_BUFFER_SIZE 4000
#define SAMPLE_RATE 40000
#define I2S_MIC_SERIAL_CLOCK GPIO_NUM_17
#define I2S_MIC_LEFT_RIGHT_CLOCK GPIO_NUM_21
#define I2S_MIC_SERIAL_DATA GPIO_NUM_4

#define UDP_PACKAGE_SIZE 1000

#define sign_LED 2

static TaskHandle_t xUDPTrasn = NULL;
static bool restart_flag = false;

hw_timer_t *tim0_once = NULL;

WiFiUDP udp;
IPAddress remote_IP(192, 168, 31, 199);
// IPAddress remote_IP(192, 168, 22, 172);
uint32_t remoteUdpPort = 6060;

uint32_t *data_inventory;
uint8_t *data_transmit_inventory;
const uint16_t sample_memory_size = 4000;

i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
    // I2S_COMM_FORMAT_I2S is deprecated, instead of I2S_COMM_FORMAT_STAND_I2S
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 16,
    .dma_buf_len = 64,
    .use_apll = false};

i2s_pin_config_t i2s_mic_pins = {
    .bck_io_num = I2S_MIC_SERIAL_CLOCK,
    .ws_io_num = I2S_MIC_LEFT_RIGHT_CLOCK,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_MIC_SERIAL_DATA};

int32_t raw_samples[SAMPLE_BUFFER_SIZE];

void Tim0Interrupt()
{
  restart_flag = true;
}

void UDPTask(void *param)
{
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false); // 关闭STA模式下wifi休眠，提高响应速度
  WiFi.begin(wifi_SSID, wifi_PSWD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(200);
  }
  Serial.print("Connected, IP Address: ");
  Serial.println(WiFi.localIP());

  for (;;)
  {
    if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) == pdTRUE)
    // if (xTaskNotifyWait(0x00, 0x00, NULL, portMAX_DELAY) == pdTRUE)
    {

      Serial.printf("UDP Transmit Start:%d\r\n", millis());
#if 0
      for (uint32_t i = 0; i < 4380; i++)
      {
        udp.write((uint8_t)data_transmit_inventory[i]);
      }
      vTaskDelay(5);

      for (uint32_t i = 4380; i < 8760; i++)
      {
        udp.write((uint8_t)data_transmit_inventory[i]);
      }
      vTaskDelay(5);

      for (uint32_t i = 8760; i < 12000; i++)
      {
        udp.write((uint8_t)data_transmit_inventory[i]);
      }
#endif
#if 1
      udp.beginPacket(remote_IP, remoteUdpPort);
      for (uint32_t i = 0; i < 12000; i++)
      {
        udp.write((uint8_t)data_transmit_inventory[i]);
      }
      vTaskDelay(2);
      udp.endPacket();
#endif
#if 0
      static uint32_t package_num;
      static uint32_t package_byte;
      for (package_num = 0; package_num < (SAMPLE_BUFFER_SIZE * 3) / UDP_PACKAGE_SIZE; package_num++)
      {
        udp.beginPacket(remote_IP, remoteUdpPort);
        udp.write((uint8_t)(package_num >> 8));
        udp.write((uint8_t)package_num);
        for (package_byte = 0; package_byte < UDP_PACKAGE_SIZE; package_byte++)
        {
          udp.write((uint8_t)data_transmit_inventory[(package_num * UDP_PACKAGE_SIZE) + package_byte]);
        }
        udp.endPacket();
        vTaskDelay(1);
        // Serial.printf("UDP Package %d Sent!\r\n", package_num);
      }
#endif
      Serial.printf("UDP Transmit END:%d\r\n", millis());
    }
    vTaskDelay(5);
  }
}

void setup()
{
  pinMode(sign_LED, OUTPUT);
  Serial.begin(115200);
  tim0_once = timerBegin(0, 240, true); // 定时器分频根据主频更改
  timerAttachInterrupt(tim0_once, Tim0Interrupt, true);
  timerAlarmWrite(tim0_once, 100000, true); // 定时器单次或者循环

  xTaskCreatePinnedToCore(
      UDPTask,
      "UDPTask",
      4096,
      NULL,
      5,
      &xUDPTrasn,
      0);

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &i2s_mic_pins);

  // data_inventory = (uint32_t *)calloc(sample_memory_size, sizeof(uint32_t));
  data_transmit_inventory = (uint8_t *)calloc(sample_memory_size * 3, sizeof(uint8_t));
  timerAlarmEnable(tim0_once);
#if 0
  for (uint32_t i = 0; i < samples_read; i++)
  {
    udp.write((uint8_t)(data_inventory[i] >> 16));
  }

  udp.endPacket();
  // delay(50);

  for (uint32_t i = 0; i < samples_read; i++)
  {
    udp.write((uint8_t)(data_inventory[i] >> 8));
  }

  udp.endPacket();
  // delay(50);

  for (uint32_t i = 0; i < samples_read; i++)
  {
    udp.write((uint8_t)(data_inventory[i]));
  }

  udp.endPacket();
#endif
#if 0
  Serial.println("INTERVAL!");
  for (uint32_t i = 0; i < samples_read / 2; i++)
  {
    udp.write((uint8_t)(data_inventory[i] >> 16));
    udp.write((uint8_t)(data_inventory[i] >> 8));
    udp.write((uint8_t)(data_inventory[i]));
  }
  Serial.println("INTERVAL!");
  udp.endPacket();
  Serial.printf("UDP Transmit END 1:%d\r\n", millis());
  // delay(50);

  for (uint32_t i = samples_read / 2; i < samples_read; i++)
  {
    udp.write((uint8_t)(data_inventory[i] >> 16));
    udp.write((uint8_t)(data_inventory[i] >> 8));
    udp.write((uint8_t)(data_inventory[i]));
  }

  udp.endPacket();
  Serial.printf("UDP Transmit END 2:%d\r\n", millis());

#endif
#if 0
  for (uint32_t i = 0; i < 12000; i++)
  {
    udp.write(0xff);
    // Serial.printf("INTERVAL:%d\r\n", millis());
  }
  // for (uint32_t i = 4380; i < 8760; i++)
  // {
  //   udp.write(data_transmit_inventory[i]);
  // }
  // for (uint32_t i = 8760; i < 13140; i++)
  // {
  //   udp.write(data_transmit_inventory[i]);
  // }
  udp.endPacket();
  Serial.printf("UDP Transmit END:%d\r\n", millis());

#endif

  // Serial.printf("UDP Transmit END Last:%d\r\n", millis());
}

void loop()
{
  // vTaskDelay(1000);
  if ( touchRead(32) < 20)
  {
    size_t bytes_read = 0;

    Serial.printf("I2S Collect Start:%d\r\n", millis());

    i2s_read(I2S_NUM_0, raw_samples, sizeof(int32_t) * SAMPLE_BUFFER_SIZE, &bytes_read, portMAX_DELAY);

    Serial.printf("I2S Collect End:%d\r\n", millis());

    uint32_t samples_read = bytes_read / sizeof(int32_t);
    Serial.printf("Sound Data Process Start:%d\r\n", millis());
#if 0
  for (uint32_t i = 0; i < samples_read; i++)
  {
    data_inventory[i] = raw_samples[i] >> 8;
  }
#endif
#if 1
    for (uint32_t i = 0; i < samples_read; i++)
    {
      data_transmit_inventory[i * 3] = (uint8_t)(raw_samples[i] >> 24);
      data_transmit_inventory[i * 3 + 1] = (uint8_t)(raw_samples[i] >> 16);
      data_transmit_inventory[i * 3 + 2] = (uint8_t)(raw_samples[i] >> 8);
    }
    // i2s_stop(I2S_NUM_0);

    Serial.printf("Sound Data Process End:%d\r\n", millis());

    xTaskNotifyGive(xUDPTrasn);
    // // xTaskNotify(xUDPTrasn, 0, eNoAction);
    restart_flag = false;

#endif
#if 0
    for (uint32_t i = 0; i < samples_read * 3; i++)
    {
      Serial.printf("%ld\n", raw_samples[i] >> 8);
    }

#endif
  }
}
