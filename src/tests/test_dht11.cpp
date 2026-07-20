// Test debugging DHT11 (pin 18) secara mandiri.
// Jalankan: pio run -e test-dht11 -t upload -t monitor
//
// Ekspektasi Serial Monitor: baris suhu/kelembapan tercetak tiap 2 detik.
// Jika terus "Gagal membaca DHT11!" -> cek wiring (data/VCC/GND) & pull-up
// resistor 10k pada pin data.

#include <Arduino.h>
#include <DHT.h>
#include "PinConfig.h"

DHT dht(PIN_DHT11, DHT_TYPE);

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("=== TEST DHT11 (pin 18) ===");
  dht.begin();
}

void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("[FAIL] Gagal membaca DHT11! Cek wiring/pull-up resistor.");
  } else {
    Serial.printf("[OK] Suhu=%.1f C  Kelembapan=%.1f %%\n", temperature, humidity);
  }

  delay(2000);
}
