// Test debugging ESP32-CAM secara mandiri (init kamera + ambil frame).
// Jalankan (upload ke board ESP32-CAM, bukan ESP32 utama):
//   pio run -e test-camera -t upload -t monitor
//
// Ekspektasi: "[Camera] Berhasil diinisialisasi." lalu tiap 3 detik
// tercetak ukuran frame JPEG yang berhasil diambil.
// Jika init gagal -> cek wiring pin kamera (bawaan board AI-Thinker) &
// pastikan catu daya 5V stabil (ESP32-CAM rawan brownout via USB-TTL lemah).

#include <Arduino.h>
#include "CameraCapture.h"

CameraCapture cameraCapture;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("=== TEST ESP32-CAM ===");

  if (!cameraCapture.begin()) {
    Serial.println("[FAIL] Inisialisasi kamera gagal.");
  }
}

void loop() {
  camera_fb_t *fb = cameraCapture.capture();
  if (fb) {
    Serial.printf("[OK] Frame diambil, ukuran=%u byte, %dx%d\n",
                  fb->len, fb->width, fb->height);
    cameraCapture.release(fb);
  } else {
    Serial.println("[FAIL] Gagal mengambil frame.");
  }
  delay(3000);
}
