#ifndef ACTUATOR_MANAGER_H
#define ACTUATOR_MANAGER_H

#include <Arduino.h>
#include <ESP32Servo.h>
#include "PinConfig.h"

// Sudut servo (dikalibrasi ulang saat pemasangan fisik, lihat SDD §2.4).
#define SERVO_ANGLE_CLOSED 0
#define SERVO_ANGLE_OPEN   90

// Hanya 1 servo fisik tersedia saat ini (bukan 2 seperti desain awal).
// Servo ini digerakkan oleh command APAPUN yang diterima (baik
// actuator="irrigation" maupun actuator="ventilation") -- lihat
// shared/Architecture.md ADR-04 & catatan revisi hardware di CLAUDE.md.
class ActuatorManager {
 public:
  ActuatorManager();

  void begin();

  // Buka servo selama durationSec detik, lalu otomatis tutup di update().
  // sourceActuator dipakai hanya untuk logging/status ("irrigation"/"ventilation").
  void open(unsigned long durationSec, const char *sourceActuator);

  // Tutup servo segera.
  void close();

  // Dipanggil tiap loop() untuk menutup otomatis setelah durasi habis.
  void update();

  bool isOpen() const;
  const char *lastSource() const;

 private:
  Servo servo;
  bool open_;
  unsigned long closeAtMillis_;
  char lastSource_[16];
};

#endif // ACTUATOR_MANAGER_H
