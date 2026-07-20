// Firmware ESP32-CAM — Smart Greenhouse
//
// Mengambil citra tanaman berkala & mengirim ke backend via HTTP POST
// multipart (shared/data-contracts.md §2). Backend sudah live di Railway.
//
// Board terpisah dari ESP32 utama (Architecture.md ADR-05), tidak
// berkomunikasi langsung dengan MQTT/servo/DHT11.

#include <Arduino.h>
#include "config.h"
#include "WifiManager.h"
#include "CameraCapture.h"
#include "HttpUploader.h"

#define BACKEND_IMAGES_URL "https://smart-greenhouse-backend-production.up.railway.app/images"
#define PLOT_ID_FOR_CAM     PLOT_ID

static const unsigned long CAPTURE_INTERVAL_MS = 60UL * 60UL * 1000UL; // 1 jam

WifiManager wifiManager(WIFI_SSID, WIFI_PASSWORD);
CameraCapture cameraCapture;
HttpUploader httpUploader(BACKEND_IMAGES_URL, CAM_DEVICE_ID, PLOT_ID_FOR_CAM);

unsigned long lastCaptureMillis = 0;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== Smart Greenhouse - ESP32-CAM ===");

  wifiManager.begin();

  if (!cameraCapture.begin()) {
    Serial.println("[Setup] Inisialisasi kamera gagal, cek wiring/power.");
  }
}

void loop() {
  wifiManager.ensureConnected();

  if (millis() - lastCaptureMillis > CAPTURE_INTERVAL_MS || lastCaptureMillis == 0) {
    if (wifiManager.isConnected()) {
      camera_fb_t *fb = cameraCapture.capture();
      if (fb) {
        Serial.printf("[Capture] Frame diambil, ukuran=%u byte\n", fb->len);
        httpUploader.upload(fb);
        cameraCapture.release(fb);
      }
    } else {
      Serial.println("[Capture] WiFi belum terhubung, lewati siklus ini.");
    }
    lastCaptureMillis = millis();
  }
}
