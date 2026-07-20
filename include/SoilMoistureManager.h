#ifndef SOIL_MOISTURE_MANAGER_H
#define SOIL_MOISTURE_MANAGER_H

#include <Arduino.h>
#include "PinConfig.h"

class SoilMoistureManager {
 public:
  void begin();

  // Baca ADC mentah dan petakan ke persentase 0-100% memakai kalibrasi
  // SOIL_MOISTURE_DRY_RAW/SOIL_MOISTURE_WET_RAW (PinConfig.h).
  float readPercent();

  // Nilai ADC mentah tanpa pemetaan, untuk keperluan kalibrasi/debug.
  int readRaw();

 private:
  float mapToPercent(int rawAdc);
};

#endif // SOIL_MOISTURE_MANAGER_H
