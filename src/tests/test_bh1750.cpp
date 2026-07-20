// Test debugging BH1750 (I2C, SDA=21 SCL=22) secara mandiri.
// Jalankan: pio run -e test-bh1750 -t upload -t monitor
//
// Ekspektasi Serial Monitor: "[OK] Lux=..." tercetak tiap 2 detik, nilai
// berubah saat ditutup/disorot cahaya. Jika "BH1750 tidak terdeteksi" terus
// muncul -> cek wiring SDA/SCL & alamat I2C (default 0x23, cek juga tidak
// bentrok dengan OLED bila sudah terpasang di alamat 0x3C).

#include <Arduino.h>
#include "LightManager.h"

LightManager light;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("=== TEST BH1750 (SDA=21, SCL=22) ===");

  if (!light.begin()) {
    Serial.println("[FAIL] BH1750 tidak terdeteksi di bus I2C. Cek wiring/alamat.");
  } else {
    Serial.println("[OK] BH1750 terdeteksi.");
  }
}

void loop() {
  float lux = light.readLux();
  if (isnan(lux)) {
    Serial.println("[FAIL] Gagal membaca BH1750.");
  } else {
    Serial.printf("[OK] Lux=%.1f\n", lux);
  }
  delay(2000);
}
