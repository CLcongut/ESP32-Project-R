#include <Arduino.h>
#include <driver/i2s.h>
#include <WIFI.h>

#define wifi_SSID "CCongut"
#define wifi_PSWD "88888888"
// #define wifi_SSID "CLcongut"
// #define wifi_PSWD "88888888"

#define SAMPLE_BUFFER_SIZE 4000
#define SAMPLE_RATE 40000
#define I2S_MIC_SERIAL_CLOCK GPIO_NUM_27
#define I2S_MIC_LEFT_RIGHT_CLOCK GPIO_NUM_26
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

i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 32,
    .dma_buf_len = 1024,
    .use_apll = false};

i2s_pin_config_t i2s_mic_pins = {
    .bck_io_num = I2S_MIC_SERIAL_CLOCK,
    .ws_io_num = I2S_MIC_LEFT_RIGHT_CLOCK,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_MIC_SERIAL_DATA};

int32_t *raw_samples;

void Tim0Interrupt()
{
  restart_flag = true;
}

void UDPTask(void *param)
{
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
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
    {

      Serial.printf("UDP Transmit Start:%d\r\n", millis());

      udp.beginPacket(remote_IP, remoteUdpPort);
      for (uint32_t i = 0; i < SAMPLE_BUFFER_SIZE; i++)
      {
        udp.write(raw_samples[i] >> 24);
        udp.write(raw_samples[i] >> 16);
        udp.write(raw_samples[i] >> 8);
        udp.write(raw_samples[i]);
      }
      vTaskDelay(2);
      udp.endPacket();

      Serial.printf("UDP Transmit END:%d\r\n", millis());
    }
    vTaskDelay(5);
  }
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

  raw_samples = (int32_t *)calloc(SAMPLE_BUFFER_SIZE, sizeof(int32_t));

  delay(1000);
  timerAlarmEnable(tim0_once);
}

void loop()
{
  if (restart_flag && touchRead(32) < 20)
  {
    size_t bytes_read = 0;

    Serial.printf("I2S Collect Start:%d\r\n", millis());

    i2s_read(I2S_NUM_0, raw_samples, sizeof(int32_t) * SAMPLE_BUFFER_SIZE, &bytes_read, portMAX_DELAY);

    Serial.printf("I2S Collect End:%d\r\n", millis());

    xTaskNotifyGive(xUDPTrasn);

#if 0
    for (uint32_t i = 0; i < samples_read * 3; i++)
    {
      Serial.printf("%ld\n", raw_samples[i] >> 8);
    }

#endif
  }
}
