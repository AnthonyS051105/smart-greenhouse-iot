// Test debugging Capacitive Soil Moisture Sensor v1.2 (pin 25) secara mandiri.
// Jalankan: pio run -e test-soil-moisture -t upload -t monitor
//
// Ekspektasi: nilai raw ADC & persentase tercetak tiap 2 detik. Cabut sensor
// dari tanah (di udara) untuk lihat nilai "kering", celup ke air untuk lihat
// nilai "basah" -- pakai kedua pembacaan untuk update SOIL_MOISTURE_DRY_RAW/
// SOIL_MOISTURE_WET_RAW di include/PinConfig.h (kalibrasi wajib sebelum demo,
// lihat IOT-T28 di docs/Task-Breakdown.md).

#include <Arduino.h>
#include "SoilMoistureManager.h"

SoilMoistureManager soilMoisture;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("=== TEST Capacitive Soil Moisture Sensor v1.2 (pin 25) ===");
  Serial.println("Kalibrasi: catat nilai raw saat KERING (di udara) & BASAH (dicelup air).");
  soilMoisture.begin();
}

void loop() {
  int raw = soilMoisture.readRaw();
  float percent = soilMoisture.readPercent();
  Serial.printf("[Soil] raw=%d  -> %.1f%%\n", raw, percent);
  delay(2000);
}
