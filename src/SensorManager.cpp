#include "SensorManager.h"
#include <ArduinoJson.h>
#include <time.h>

SensorManager::SensorManager() : dht(PIN_DHT11, DHT_TYPE) {}

void SensorManager::begin() {
  dht.begin();
}

SensorData SensorManager::readAll() {
  SensorData data;
  data.temperature = dht.readTemperature();
  data.humidity = dht.readHumidity();
  data.soil_moisture = SOIL_MOISTURE_PLACEHOLDER;
  data.light_intensity = LIGHT_INTENSITY_PLACEHOLDER;

  data.valid = !(isnan(data.temperature) || isnan(data.humidity));
  return data;
}

String SensorManager::toJson(const SensorData &data, const char *deviceId) {
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
  doc["light_intensity"] = data.light_intensity;

  String output;
  serializeJson(doc, output);
  return output;
}
