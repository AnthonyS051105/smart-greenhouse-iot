#ifndef LIGHT_MANAGER_H
#define LIGHT_MANAGER_H

#include <Arduino.h>
#include <BH1750.h>
#include "PinConfig.h"

class LightManager {
 public:
  // Inisialisasi bus I2C (Wire) pada pin SDA/SCL BH1750 & sensor itu sendiri.
  // return false jika sensor tidak terdeteksi di bus I2C.
  bool begin();

  // Baca intensitas cahaya dalam lux. Mengembalikan NAN jika pembacaan gagal.
  float readLux();

 private:
  BH1750 lightMeter;
};

#endif // LIGHT_MANAGER_H
