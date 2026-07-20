#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <DHT.h>
#include "PinConfig.h"

// Nilai placeholder untuk field sensor yang belum tersedia secara fisik
// (Capacitive Soil Moisture Sensor v1.2 & BH1750). Payload tetap mengikuti
// shared/data-contracts.md §1.1 (semua field wajib ada), tapi diisi -1
// sebagai penanda "belum terpasang" sampai sensor fisik tersedia.
#define SOIL_MOISTURE_PLACEHOLDER   -1.0f
#define LIGHT_INTENSITY_PLACEHOLDER -1.0f

struct SensorData {
  float temperature;     // °C, dari DHT11
  float humidity;        // %, dari DHT11
  float soil_moisture;   // %, placeholder (-1) sampai sensor terpasang
  float light_intensity; // lux, placeholder (-1) sampai sensor terpasang
  bool valid;            // false jika DHT11 gagal baca (NaN)
};

class SensorManager {
 public:
  SensorManager();

  // Inisialisasi sensor yang tersedia (DHT11).
  void begin();

  // Baca seluruh sensor yang tersedia; sensor yang belum ada hardware-nya
  // diisi nilai placeholder. valid=false jika DHT11 gagal (NaN).
  SensorData readAll();

  // Susun payload JSON sesuai shared/data-contracts.md §1.1.
  String toJson(const SensorData &data, const char *deviceId);

 private:
  DHT dht;
};

#endif // SENSOR_MANAGER_H
