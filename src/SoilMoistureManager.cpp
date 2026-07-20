#include "SoilMoistureManager.h"

void SoilMoistureManager::begin() {
  pinMode(PIN_SOIL_MOISTURE, INPUT);
}

int SoilMoistureManager::readRaw() {
  return analogRead(PIN_SOIL_MOISTURE);
}

float SoilMoistureManager::mapToPercent(int rawAdc) {
  // Sensor kapasitif: nilai ADC makin RENDAH saat makin BASAH.
  // map() dari rentang [DRY..WET] (raw) ke [0..100] (persen).
  long percent = map(rawAdc, SOIL_MOISTURE_DRY_RAW, SOIL_MOISTURE_WET_RAW, 0, 100);
  return (float)constrain(percent, 0, 100);
}

float SoilMoistureManager::readPercent() {
  return mapToPercent(readRaw());
}
