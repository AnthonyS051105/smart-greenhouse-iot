#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

// Pemetaan pin fisik untuk hardware yang TERSEDIA saat ini.
// Catatan revisi hardware (lihat iot/CLAUDE.md & docs/Task-Breakdown.md):
// - Hanya 1 servo tersedia (bukan 2 seperti desain awal SDD) -> servo generik,
//   digerakkan oleh command apapun (actuator=irrigation ATAU ventilation).
// - Sensor set lengkap sekarang: DHT11, Capacitive Soil Moisture Sensor v1.2,
//   BH1750. OLED masih BELUM tersedia secara fisik.

#define PIN_DHT11              18   // DHT11 data pin
#define PIN_SERVO              19   // Servo generik (irigasi/ventilasi)
#define PIN_SOIL_MOISTURE      25   // Capacitive Soil Moisture Sensor v1.2 (ADC)
#define PIN_BH1750_SDA         21   // BH1750 I2C SDA
#define PIN_BH1750_SCL         22   // BH1750 I2C SCL

#define DHT_TYPE         DHT11

// Kalibrasi Soil Moisture Sensor v1.2: nilai ADC mentah di kondisi kering
// (di udara) dan basah penuh (dicelup air), dipetakan linear ke 0-100%.
// Nilai default berikut adalah PERKIRAAN AWAL untuk ESP32 ADC 12-bit
// (0-4095) -- WAJIB dikalibrasi ulang dengan sensor fisik sebelum demo
// (lihat src/tests/test_soil_moisture.cpp untuk membaca nilai raw).
#define SOIL_MOISTURE_DRY_RAW   3000
#define SOIL_MOISTURE_WET_RAW   1200

#endif // PIN_CONFIG_H
