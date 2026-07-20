#include "FallbackController.h"
#include "SensorManager.h"

FallbackController::FallbackController(ActuatorManager &actuator)
    : actuator_(actuator),
      wasConnected_(true),
      disconnectedSinceMillis_(0),
      inFallback_(false) {}

void FallbackController::updateConnectionState(bool connected) {
  if (connected) {
    if (!wasConnected_ && inFallback_) {
      Serial.println("[Fallback] Koneksi pulih, kembali ke mode online.");
    }
    inFallback_ = false;
    disconnectedSinceMillis_ = 0;
  } else {
    if (wasConnected_) {
      // Baru saja putus, mulai hitung waktu.
      disconnectedSinceMillis_ = millis();
    } else if (millis() - disconnectedSinceMillis_ > FALLBACK_TIMEOUT_MS) {
      if (!inFallback_) {
        Serial.println("[Fallback] Koneksi terputus > 60 detik, masuk mode fallback.");
      }
      inFallback_ = true;
    }
  }
  wasConnected_ = connected;
}

bool FallbackController::isInFallbackMode() {
  return inFallback_;
}

void FallbackController::evaluate(const SensorData &data) {
  if (!inFallback_ || !data.valid) {
    return;
  }

  if (data.temperature > FALLBACK_TEMP_THRESHOLD_C) {
    Serial.println("[Fallback] Suhu tinggi -> buka aktuator (ventilasi).");
    actuator_.open(FALLBACK_IRRIGATION_SEC, "ventilation");
  }

  // soil_moisture placeholder (-1) tidak akan pernah < ambang secara valid;
  // aturan ini otomatis aktif kembali begitu sensor fisik terpasang.
  if (data.soil_moisture >= 0 && data.soil_moisture < FALLBACK_SOIL_THRESHOLD_PCT) {
    Serial.println("[Fallback] Tanah kering -> buka aktuator (irigasi singkat).");
    actuator_.open(FALLBACK_IRRIGATION_SEC, "irrigation");
  }
}
