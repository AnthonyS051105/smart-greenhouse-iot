#include "SensorManager.h"
#include <ArduinoJson.h>
#include <time.h>

SensorManager::SensorManager() : dht(PIN_DHT11, DHT_TYPE) {}

bool SensorManager::begin() {
  dht.begin();
  soilMoisture.begin();
  bool lightOk = light.begin();
  if (!lightOk) {
    Serial.println("[SensorManager] BH1750 tidak terdeteksi di bus I2C.");
  }
  return lightOk;
}

SensorData SensorManager::readAll() {
  SensorData data;
  data.temperature = dht.readTemperature();
  data.humidity = dht.readHumidity();
  data.soil_moisture = soilMoisture.readPercent();
  data.light_intensity = light.readLux();

  data.valid = !(isnan(data.temperature) || isnan(data.humidity));
  return data;
}

String SensorManager::toJson(const SensorData &data, const char *deviceId, const char *plotId) {
  JsonDocument doc;

  doc["device_id"] = deviceId;

  // ISO 8601 UTC (shared/data-contracts.md §6 aturan konsistensi).
  time_t now;
  time(&now);
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  char isoTime[25];
  strftime(isoTime, sizeof(isoTime), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
  doc["timestamp"] = isoTime;

  doc["temperature"] = data.temperature;
  doc["humidity"] = data.humidity;
  doc["soil_moisture"] = data.soil_moisture;
  doc["light_intensity"] = isnan(data.light_intensity) ? -1.0 : data.light_intensity;

  // plot_id disertakan agar backend (yang mengaitkan sensor_readings ke
  // plot, bukan device) bisa langsung memakai payload MQTT ini tanpa
  // pemetaan tambahan. Lihat app/services/sensor_service.py.
  doc["plot_id"] = plotId;

  String output;
  serializeJson(doc, output);
  return output;
}
