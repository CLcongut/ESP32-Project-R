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

int32_t *sample_inventory_0;
int32_t *sample_inventory_1;

size_t bytes_read_cnt_0 = 0;
size_t bytes_read_cnt_1 = 0;

i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
    // I2S_COMM_FORMAT_I2S is deprecated, instead of I2S_COMM_FORMAT_STAND_I2S
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = 0,
    .dma_buf_count = 32,
    .dma_buf_len = 1024,
    .use_apll = false};

i2s_pin_config_t i2s_mic_pins = {
    .bck_io_num = I2S_MIC_SERIAL_CLOCK,
    .ws_io_num = I2S_MIC_LEFT_RIGHT_CLOCK,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_MIC_SERIAL_DATA};

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
  uint32_t ulNotificationValue;

  for (;;)
  {
    // ulNotificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    xTaskNotifyWaitIndexed(0,                    /* 等待第 0 个通知。*/
                           0x00,                 /* 进入时不清除任何通知位。*/
                           ULONG_MAX,            /* 退出时将通知值重置为 0。*/
                           &ulNotificationValue, /* 通知值在ulNotifiedValue 中传递。*/
                           portMAX_DELAY);       /* 无限期阻塞。*/

    if (ulNotificationValue & 0x01)
    // if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) == pdTRUE)
    // if (xTaskNotifyWait(0x00, 0x00, NULL, portMAX_DELAY) == pdTRUE)
    {
      uint32_t all_byte;
      Serial.printf("UDP Transmit Start in 0:%d\r\n", millis());
      udp.beginPacket(remote_IP, remoteUdpPort);
      for (all_byte = 0; all_byte < bytes_read_cnt_0 / 4; all_byte++)
      {
        // udp.write(sample_inventory_0[all_byte]);
        udp.printf("%ld", sample_inventory_0[all_byte]);
      }
      vTaskDelay(2);
      udp.endPacket();
      Serial.printf("UDP Transmit   END in 0:%d\r\n", millis());
    }
    if (ulNotificationValue & 0x02)
    {
      uint32_t all_byte;
      Serial.printf("UDP Transmit Start in 1:%d\r\n", millis());
      udp.beginPacket(remote_IP, remoteUdpPort);
      for (all_byte = 0; all_byte < bytes_read_cnt_1 / 4; all_byte++)
      {
        // udp.write(sample_inventory_0[all_byte]);
        udp.printf("%ld", sample_inventory_1[all_byte]);
      }
      vTaskDelay(2);
      udp.endPacket();
      Serial.printf("UDP Transmit   END in 1:%d\r\n", millis());
    }
  }
  vTaskDelay(5);
}

void setup()
{
  pinMode(sign_LED, OUTPUT);
  Serial.begin(115200);
  tim0_once = timerBegin(0, 240, true);
  timerAttachInterrupt(tim0_once, Tim0Interrupt, true);
  timerAlarmWrite(tim0_once, 100000, false);

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

  sample_inventory_0 = (int32_t *)calloc(SAMPLE_BUFFER_SIZE, sizeof(int32_t));
  sample_inventory_1 = (int32_t *)calloc(SAMPLE_BUFFER_SIZE, sizeof(int32_t));

  delay(1000);
  timerAlarmEnable(tim0_once);
}

void loop()
{
  if (restart_flag)
  {
    Serial.printf("I2S Collect Start in 0:%d\r\n", millis());

    i2s_read(I2S_NUM_0, sample_inventory_0, sizeof(int32_t) * SAMPLE_BUFFER_SIZE, &bytes_read_cnt_0, portMAX_DELAY);

    Serial.printf("I2S Collect   End in 0:%d\r\n", millis());

    // xTaskNotify(xUDPTrasn, 0x01, eSetValueWithOverwrite);

    Serial.printf("I2S Collect Start in 1:%d\r\n", millis());

    i2s_read(I2S_NUM_0, sample_inventory_1, sizeof(int32_t) * SAMPLE_BUFFER_SIZE, &bytes_read_cnt_1, portMAX_DELAY);

    Serial.printf("I2S Collect   End in 1:%d\r\n", millis());

    // xTaskNotify(xUDPTrasn, 0x02, eSetValueWithOverwrite);
  }
}
