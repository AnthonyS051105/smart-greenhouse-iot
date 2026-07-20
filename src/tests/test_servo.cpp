// Test debugging Servo (pin 21) secara mandiri.
// Jalankan: pio run -e test-servo -t upload -t monitor
//
// Ekspektasi: servo bergerak bolak-balik antara posisi tutup (0deg) dan
// buka (90deg) tiap 2 detik. Jika servo diam/bergetar -> cek catu daya 5V
// terpisah (jangan ambil dari pin 5V ESP32 langsung jika servo banyak
// menyedot arus) & wiring sinyal ke pin 21.

#include <Arduino.h>
#include <ESP32Servo.h>
#include "PinConfig.h"
#include "ActuatorManager.h"

Servo testServo;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("=== TEST SERVO (pin 21) ===");

  testServo.setPeriodHertz(50);
  testServo.attach(PIN_SERVO, 500, 2400);
}

void loop() {
  Serial.println("[Servo] -> Posisi BUKA (90 deg)");
  testServo.write(SERVO_ANGLE_OPEN);
  delay(2000);

  Serial.println("[Servo] -> Posisi TUTUP (0 deg)");
  testServo.write(SERVO_ANGLE_CLOSED);
  delay(2000);
}
