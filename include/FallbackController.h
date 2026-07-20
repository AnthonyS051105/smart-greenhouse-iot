#ifndef FALLBACK_CONTROLLER_H
#define FALLBACK_CONTROLLER_H

#include <Arduino.h>
#include "SensorManager.h"
#include "ActuatorManager.h"

// Threshold fallback (IOT-FR-16).
#define FALLBACK_TEMP_THRESHOLD_C     32.0f
#define FALLBACK_SOIL_THRESHOLD_PCT   30.0f
#define FALLBACK_IRRIGATION_SEC       10UL
#define FALLBACK_TIMEOUT_MS           60000UL // 60 detik (IOT-FR-15)

class FallbackController {
 public:
  FallbackController(ActuatorManager &actuator);

  // Dipanggil setiap kali status koneksi (WiFi+MQTT) diperbarui, dari loop utama.
  void updateConnectionState(bool connected);

  // true jika sudah > FALLBACK_TIMEOUT_MS sejak terakhir online.
  bool isInFallbackMode();

  // Jalankan aturan threshold lokal terhadap data sensor terbaru.
  void evaluate(const SensorData &data);

 private:
  ActuatorManager &actuator_;
  bool wasConnected_;
  unsigned long disconnectedSinceMillis_;
  bool inFallback_;
};

#endif // FALLBACK_CONTROLLER_H
