#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <ArduinoEigen.h>
#include <SPI.h>
#include <TFT_eSPI.h>

using namespace Eigen;

struct Line
{
  uint8_t start;
  uint8_t end;
};

MatrixXd pt(8, 4);  // 立边体顶点
MatrixXi tp(12, 2); // 立方体边

MatrixXd tm0(4, 4); // 放大60位
MatrixXd tm1(4, 4); // 平移
Matrix4d txr;
Matrix4d tyr;
Matrix4d tzr;

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
float xout, yout, zout;
// float roll, pitch;
// float rollF, pitchF;
float xangle, yangle, zangle;
float xangleF, yangleF, zangleF;

TFT_eSPI tft = TFT_eSPI();

xSemaphoreHandle xMutex = xSemaphoreCreateMutex();

void adxlTask(void *param)
{
  while (true)
  {
    if (xSemaphoreTake(xMutex, portMAX_DELAY))
    {
      // 临界资源处理
      sensors_event_t event;
      accel.getEvent(&event);

      xout = event.acceleration.x;
      yout = event.acceleration.y;
      zout = event.acceleration.z;

      xangle = atan(xout / sqrt(pow(yout, 2) + pow(zout, 2))) * 180 / PI;
      yangle = atan(yout / sqrt(pow(xout, 2) + pow(zout, 2))) * 180 / PI;
      zangle = atan(zout / sqrt(pow(xout, 2) + pow(yout, 2))) * 180 / PI;

      xangleF = 0.94 * xangleF + 0.06 * xangle;
      yangleF = 0.94 * yangleF + 0.06 * yangle;
      zangleF = 0.94 * zangleF + 0.06 * zangle;
      xSemaphoreGive(xMutex);
    }
    // delay(5);
    vTaskDelay(5);
  };
}

void drawTube(uint16_t color)
{
  int nr = tp.rows(); // 行数
  for (int i = 0; i < nr; i++)
  {
    auto start = pt.row(tp(i, 0));
    auto end = pt.row(tp(i, 1));
    tft.drawLine(start(0), start(1), end(0), end(1), color);
  }
}

void drawTube(const MatrixXd &m, uint16_t color)
{
  int nr = tp.rows(); // 行数
  for (int i = 0; i < nr; i++)
  {
    auto start = m.row(tp(i, 0));
    auto end = m.row(tp(i, 1));
    tft.drawLine(start(0), start(1), end(0), end(1), color);
  }
}

void SetXRotationTranslation(double angle)
{
  double tmp = 3.1415926 * angle / 180.0;
  double c = cos(tmp);
  double s = sin(tmp);
  txr(1, 1) = c;
  txr(1, 2) = s;
  txr(2, 1) = -s;
  txr(2, 2) = c;
}

void SetYRotationTranslation(double angle)
{
  double tmp = 3.1415926 * angle / 180.0;
  double c = cos(tmp);
  double s = sin(tmp);
  tyr(0, 0) = c;
  tyr(0, 2) = -s;
  tyr(2, 0) = s;
  tyr(2, 2) = c;
}

void SetZRotationTranslation(double angle)
{
  double tmp = 3.1415926 * angle / 180.0;
  double c = cos(tmp);
  double s = sin(tmp);
  tzr(0, 0) = c;
  tzr(0, 1) = s;
  tzr(1, 0) = -s;
  tzr(1, 1) = c;
}

void setup(void)
{
  Serial.begin(115200);

  xTaskCreatePinnedToCore(
    adxlTask, 
    "adxlTask", 
    4096, 
    NULL,
    5, 
    NULL, 
    0);

  accel.begin();

  accel.setRange(ADXL345_RANGE_16_G);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  pt << -1, -1, 1, 1, // A 0
      1, -1, 1, 1,    // B 1
      1, 1, 1, 1,     // c 2
      -1, 1, 1, 1,    // d 3
      -1, -1, -1, 1,  // e 4
      1, -1, -1, 1,   // f 5
      1, 1, -1, 1,    // g 6
      -1, 1, -1, 1;   // h 7

  tp << 0, 1, // A-B
      1, 2,   // b-c
      2, 3,   // c-d
      3, 0,   // d-a
      0, 4,   // a-e
      1, 5,   // b-f
      2, 6,   // c-g
      3, 7,   // d-h
      4, 5,   // e-f
      5, 6,   // f-g
      6, 7,   // g-h
      7, 4;   // h-e

  tm0 << 20, 0, 0, 0,
      0, 20, 0, 0,
      0, 0, 20, 0,
      0, 0, 0, 1;

  tm1 << 1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      60, 60, 120, 1;

  txr.setIdentity(4, 4);
  tyr.setIdentity(4, 4);
  tzr.setIdentity(4, 4);

  pt *= tm0; // 这个就是我们最初的大个立方体
}

void loop(void)
{

  // Serial.println(rollF);
  // Serial.println(pitchF);

  MatrixXd tmp(8, 4);
  double xxa, yya, zza;
  // 临界资源处理

  if (xSemaphoreTake(xMutex, portMAX_DELAY))
  {
    // 临界资源处理
    xxa = xangleF;
    yya = yangleF;
    zza = zangleF;
    xSemaphoreGive(xMutex);
  }

  SetXRotationTranslation(xxa);
  SetYRotationTranslation(yya);
  SetZRotationTranslation(zza);

  tmp = pt;
  tmp *= txr;
  tmp *= tyr;
  tmp *= tzr;
  tmp *= tm1;

  drawTube(tmp, TFT_WHITE);
  delay(5);
  drawTube(tmp, TFT_BLACK);
  // delay(10);
}
