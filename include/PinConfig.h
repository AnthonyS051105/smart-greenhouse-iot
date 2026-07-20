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
// Soil Moisture WAJIB di pin ADC1 (bukan ADC2) -- ADC2 tidak bisa dipakai
// bersamaan WiFi aktif (keterbatasan hardware ESP32, lihat esp-idf ADC
// limitations). GPIO25 (pin lama) adalah ADC2 -> pindah ke GPIO32 (ADC1).
#define PIN_SOIL_MOISTURE      32   // Capacitive Soil Moisture Sensor v1.2 (ADC1)
#define PIN_BH1750_SDA         21   // BH1750 I2C SDA
#define PIN_BH1750_SCL         22   // BH1750 I2C SCL

#define DHT_TYPE         DHT11

// Kalibrasi Soil Moisture Sensor v1.2: nilai ADC mentah di kondisi kering
// (di udara) dan basah penuh (dicelup air), dipetakan linear ke 0-100%.
// Nilai berikut dikalibrasi di pin lama (GPIO25/ADC2) -- WAJIB dikalibrasi
// ULANG dengan pio run -e test-soil-moisture setelah kabel sensor dipindah
// ke GPIO32, karena karakteristik ADC1 vs ADC2 bisa sedikit berbeda.
#define SOIL_MOISTURE_DRY_RAW   2533
#define SOIL_MOISTURE_WET_RAW   980

#endif // PIN_CONFIG_H
