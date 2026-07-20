#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <DHT.h>
#include "PinConfig.h"
#include "SoilMoistureManager.h"
#include "LightManager.h"

struct SensorData {
  float temperature;     // °C, dari DHT11
  float humidity;        // %, dari DHT11
  float soil_moisture;   // %, dari Capacitive Soil Moisture Sensor v1.2
  float light_intensity; // lux, dari BH1750
  bool valid;            // false jika salah satu pembacaan wajib gagal (NaN)
};

class SensorManager {
 public:
  SensorManager();

  // Inisialisasi seluruh sensor (DHT11, soil moisture, BH1750).
  // Mengembalikan false jika BH1750 gagal terdeteksi di bus I2C (dicatat
  // via Serial, pembacaan lux selanjutnya akan NaN tapi sistem tetap jalan).
  bool begin();

  // Baca seluruh sensor. valid=false jika DHT11 gagal (NaN) -- soil moisture
  // & light tidak memblokir validitas karena punya fallback (lihat SDD).
  SensorData readAll();

  // Susun payload JSON MQTT. Mengandung SKEMA GANDA secara sengaja:
  // - Field kontrak resmi (shared/data-contracts.md §1.1): device_id,
  //   timestamp, temperature, humidity, soil_moisture, light_intensity.
  // - Field tambahan agar backend NYATA yang sudah live (skema lama
  //   app/schemas.py: plot_id, water_level) juga bisa menyimpan &
  //   memakai data ini untuk model AI irigasi. water_level dipetakan
  //   langsung dari soil_moisture (representasi yang sama: % kelembapan
  //   tanah). Lihat catatan integrasi di iot/CLAUDE.md.
  String toJson(const SensorData &data, const char *deviceId, const char *plotId);

 private:
  DHT dht;
  SoilMoistureManager soilMoisture;
  LightManager light;
};

#endif // SENSOR_MANAGER_H
