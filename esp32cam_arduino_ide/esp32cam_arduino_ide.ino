// ============================================================================
// Firmware ESP32-CAM — Smart Greenhouse (Versi Arduino IDE, single file)
// ============================================================================
// Padanan dari iot/src/esp32cam/main.cpp + CameraCapture + HttpUploader +
// WifiManager (versi modular PlatformIO), digabung 1 file tanpa header
// terpisah untuk diupload lewat Arduino IDE.
//
// Mengambil citra tanaman berkala & mengirim ke backend via HTTP POST
// multipart (shared/data-contracts.md §2). Backend sudah live di Railway.
// Board terpisah dari ESP32 utama (Architecture.md ADR-05), tidak
// berkomunikasi langsung dengan MQTT/servo/DHT11.
//
// SEBELUM UPLOAD, wajib diisi manual sesuai iot/include/config.h kamu:
//   WIFI_SSID, WIFI_PASSWORD, CAM_DEVICE_ID, PLOT_ID, BACKEND_IMAGES_URL
//
// Board di Arduino IDE: Tools > Board > ESP32 Arduino > AI Thinker ESP32-CAM
// Partition Scheme: "Huge APP (3MB No OTA/1MB SPIFFS)"
// ============================================================================

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <esp_camera.h>

// ---------- ISI SESUAI config.h KAMU ----------
#define WIFI_SSID       "Adhyasta"
#define WIFI_PASSWORD   "juarasatu"
#define CAM_DEVICE_ID   "gh-esp32cam-01"
#define PLOT_ID         "plot-abc123"
#define BACKEND_IMAGES_URL "https://smart-greenhouse-backend-production.up.railway.app/images"
// ------------------------------------------------

static const unsigned long CAPTURE_INTERVAL_MS = 60UL * 60UL * 1000UL; // 1 jam
static const unsigned long WIFI_RETRY_INTERVAL_MS = 5000;

// Pinout standar board AI-Thinker ESP32-CAM.
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

unsigned long lastCaptureMillis = 0;
unsigned long lastWifiAttemptMillis = 0;

// ---------------------------------------------------------------------------
// WiFi
// ---------------------------------------------------------------------------

void wifiBegin() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("[WiFi] Menghubungkan ke ");
  Serial.println(WIFI_SSID);
}

bool wifiIsConnected() {
  return WiFi.status() == WL_CONNECTED;
}

void wifiEnsureConnected() {
  if (wifiIsConnected()) return;
  unsigned long now = millis();
  if (now - lastWifiAttemptMillis < WIFI_RETRY_INTERVAL_MS) return;
  lastWifiAttemptMillis = now;
  Serial.println("[WiFi] Terputus, mencoba reconnect...");
  WiFi.disconnect();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

// ---------------------------------------------------------------------------
// Kamera
// ---------------------------------------------------------------------------

bool cameraBegin() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  // XCLK 10MHz + CAMERA_GRAB_LATEST -- fix untuk crash "Stack canary
  // watchpoint triggered (cam_task)" (Guru Meditation Error) yang terjadi
  // di 20MHz/grab_mode default pada modul OV2640 board ini, diverifikasi
  // via diagnostik langsung sebelum firmware ini dibuat.
  config.xclk_freq_hz = 10000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_LATEST;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_SVGA; // 800x600, cukup untuk analisis vision
    config.jpeg_quality = 12;
    config.fb_count = 2;
    config.fb_location = CAMERA_FB_IN_PSRAM;
  } else {
    config.frame_size = FRAMESIZE_QVGA; // 320x240, aman tanpa PSRAM
    config.jpeg_quality = 15;
    config.fb_count = 1;
    config.fb_location = CAMERA_FB_IN_DRAM;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("[Camera] Init gagal, err=0x%x\n", err);
    return false;
  }
  Serial.println("[Camera] Berhasil diinisialisasi.");
  return true;
}

// ---------------------------------------------------------------------------
// Upload HTTP multipart ke backend (shared/data-contracts.md §2)
// ---------------------------------------------------------------------------

bool uploadImage(camera_fb_t *fb, int maxRetries = 3) {
  if (!fb) return false;

  const String boundary = "GreenhouseBoundary";

  String bodyStart;
  bodyStart += "--" + boundary + "\r\n";
  bodyStart += "Content-Disposition: form-data; name=\"device_id\"\r\n\r\n";
  bodyStart += String(CAM_DEVICE_ID) + "\r\n";
  bodyStart += "--" + boundary + "\r\n";
  bodyStart += "Content-Disposition: form-data; name=\"plot_id\"\r\n\r\n";
  bodyStart += String(PLOT_ID) + "\r\n";
  bodyStart += "--" + boundary + "\r\n";
  bodyStart += "Content-Disposition: form-data; name=\"image\"; filename=\"crop.jpg\"\r\n";
  bodyStart += "Content-Type: image/jpeg\r\n\r\n";

  String bodyEnd = "\r\n--" + boundary + "--\r\n";

  for (int attempt = 1; attempt <= maxRetries; attempt++) {
    WiFiClientSecure client;
    client.setInsecure(); // Demo bootcamp; ganti dgn root CA untuk produksi.

    HTTPClient http;
    if (!http.begin(client, BACKEND_IMAGES_URL)) {
      Serial.println("[Upload] Gagal begin() koneksi HTTP.");
      continue;
    }
    http.addHeader("Content-Type", "multipart/form-data; boundary=" + boundary);

    size_t totalLen = bodyStart.length() + fb->len + bodyEnd.length();
    uint8_t *buffer = (uint8_t *)malloc(totalLen);
    if (!buffer) {
      Serial.println("[Upload] Gagal alokasi buffer upload.");
      http.end();
      return false;
    }

    size_t offset = 0;
    memcpy(buffer + offset, bodyStart.c_str(), bodyStart.length());
    offset += bodyStart.length();
    memcpy(buffer + offset, fb->buf, fb->len);
    offset += fb->len;
    memcpy(buffer + offset, bodyEnd.c_str(), bodyEnd.length());

    int httpCode = http.POST(buffer, totalLen);
    free(buffer);

    if (httpCode == 200 || httpCode == 201) {
      Serial.printf("[Upload] Berhasil (percobaan %d), kode=%d\n", attempt, httpCode);
      Serial.println(http.getString());
      http.end();
      return true;
    }

    Serial.printf("[Upload] Gagal (percobaan %d/%d), kode=%d\n", attempt, maxRetries, httpCode);
    http.end();
    delay(1000);
  }

  Serial.println("[Upload] Gagal setelah semua percobaan, lewati siklus ini.");
  return false;
}

// ---------------------------------------------------------------------------
// setup() & loop()
// ---------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== Smart Greenhouse - ESP32-CAM ===");

  // Kamera WAJIB diinisialisasi SEBELUM WiFi -- keduanya berbagi resource
  // internal ESP32 (DMA channel, RF/clock timing).
  if (!cameraBegin()) {
    Serial.println("[Setup] Inisialisasi kamera gagal, cek wiring/power.");
  } else {
    // esp_camera menjalankan cam_task di BACKGROUND sejak esp_camera_init()
    // -- task ini terus mengisi buffer DMA sendiri terlepas dari kapan
    // esp_camera_fb_get() dipanggil pertama kali. Jika WiFi.begin() mulai
    // SEBELUM buffer pertama "dikosongkan", backlog DMA menumpuk terus dan
    // menyumbat CPU -> WiFi gagal connect berulang ("DMA overflow" loop,
    // diverifikasi via diagnostik langsung). Fix: ambil & lepas 1 frame
    // dummy di sini supaya cam_task settle dulu sebelum WiFi radio aktif.
    camera_fb_t *warmupFb = esp_camera_fb_get();
    if (warmupFb) {
      esp_camera_fb_return(warmupFb);
    }
    delay(200);
  }

  wifiBegin();
}

void loop() {
  wifiEnsureConnected();

  if (millis() - lastCaptureMillis > CAPTURE_INTERVAL_MS || lastCaptureMillis == 0) {
    if (wifiIsConnected()) {
      camera_fb_t *fb = esp_camera_fb_get();
      if (fb) {
        Serial.printf("[Capture] Frame diambil, ukuran=%u byte\n", fb->len);
        uploadImage(fb);
        esp_camera_fb_return(fb);
      } else {
        Serial.println("[Capture] Gagal mengambil frame.");
      }
    } else {
      Serial.println("[Capture] WiFi belum terhubung, lewati siklus ini.");
    }
    lastCaptureMillis = millis();
  }
}
