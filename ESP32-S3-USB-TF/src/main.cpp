#include <Arduino.h>
#include "cl_i2s_lib.h"

CL_I2S_LIB pcm4201(0, pcm4201.MASTER, pcm4201.RX, pcm4201.PCM);
uint32_t raw_samples_invt[1024];

void setup() {
  Serial.begin(115200);
  pcm4201.begin(1000, 32);
  pcm4201.setformat(pcm4201.RIGHT_LEFT, pcm4201.I2S);
  pcm4201.setDMABuffer(4, 1024);
  pcm4201.install(5, 6, 7);
}

void loop() {
  pcm4201.Read(raw_samples_invt, 1024);
  for(int i = 0; i < 1024; i++) {
    Serial.println(raw_samples_invt[i]);
  }
}