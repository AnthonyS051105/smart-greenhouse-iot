#include "LightManager.h"
#include <Wire.h>

bool LightManager::begin() {
  Wire.begin(PIN_BH1750_SDA, PIN_BH1750_SCL);
  return lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
}

float LightManager::readLux() {
  float lux = lightMeter.readLightLevel();
  if (lux < 0) {
    return NAN; // claws/BH1750 mengembalikan -1 saat pembacaan gagal
  }
  return lux;
}
