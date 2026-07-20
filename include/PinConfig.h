#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

// Pemetaan pin fisik untuk hardware yang TERSEDIA saat ini.
// Catatan revisi hardware (lihat iot/CLAUDE.md & docs/Task-Breakdown.md):
// - Hanya 1 servo tersedia (bukan 2 seperti desain awal SDD) -> servo generik,
//   digerakkan oleh command apapun (actuator=irrigation ATAU ventilation).
// - Soil Moisture Sensor v1.2 & BH1750 & OLED BELUM tersedia secara fisik.

#define PIN_DHT11        18   // DHT11 data pin
#define PIN_SERVO        21   // Servo generik (irigasi/ventilasi)

#define DHT_TYPE         DHT11

#endif // PIN_CONFIG_H
