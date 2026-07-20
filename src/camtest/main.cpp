// DIAGNOSTIK ESP32-CAM — Web MJPEG Stream Server
//
// Tujuan: membuktikan kamera & board berfungsi TANPA bergantung pada Serial
// Monitor sama sekali (Serial Monitor board ini terus menampilkan karakter
// acak meski sudah dicoba baud 9600 & ganti kabel USB -- mengindikasikan
// board reboot-loop, bukan soal komunikasi serial). Jika halaman web ini
// bisa diakses & menampilkan stream kamera live, maka kamera+board FISIK
// berfungsi normal; kalau board tetap crash sebelum WiFi connect, itu
// konfirmasi kuat brownout saat esp_camera_init() (butuh catu daya
// eksternal terpisah dari USB, bukan bug kode).
//
// Cara pakai:
//   pio run -e camtest -t upload
//   (tunggu ~10 detik agar board boot & connect WiFi tanpa perlu monitor)
//   Buka router/aplikasi WiFi untuk cari IP board, ATAU sambungkan monitor
//   SETELAHNYA (board yang sudah lolos WiFi connect biasanya sudah stabil)
//   dan lihat IP yang dicetak, lalu buka http://<IP>/ di browser HP/laptop
//   yang satu jaringan WiFi dengan board.

#include <Arduino.h>
#include <WiFi.h>
#include <esp_http_server.h>
#include "config.h"
#include "CameraCapture.h"

CameraCapture cameraCapture;
httpd_handle_t streamServer = NULL;

#define PART_BOUNDARY "greenhousecamboundary"
static const char *STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

// Halaman HTML sederhana, langsung menampilkan <img> yang menunjuk ke /stream.
static const char INDEX_HTML[] PROGMEM = R"HTML(
<!DOCTYPE html><html><head><title>ESP32-CAM Diagnostik</title></head>
<body style="background:#111;text-align:center;font-family:sans-serif;color:#eee;">
<h2>ESP32-CAM Diagnostik - Live Stream</h2>
<img src="/stream" style="max-width:100%%;border:2px solid #4caf50;" />
</body></html>
)HTML";

static esp_err_t indexHandler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, INDEX_HTML, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t streamHandler(httpd_req_t *req) {
  esp_err_t res = httpd_resp_set_type(req, STREAM_CONTENT_TYPE);
  if (res != ESP_OK) return res;

  char partBuffer[64];

  while (true) {
    camera_fb_t *fb = cameraCapture.capture();
    if (!fb) {
      Serial.println("[Stream] Gagal ambil frame, hentikan stream.");
      res = ESP_FAIL;
      break;
    }

    res = httpd_resp_send_chunk(req, STREAM_BOUNDARY, strlen(STREAM_BOUNDARY));
    if (res == ESP_OK) {
      size_t hlen = snprintf(partBuffer, sizeof(partBuffer), STREAM_PART, fb->len);
      res = httpd_resp_send_chunk(req, partBuffer, hlen);
    }
    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len);
    }

    cameraCapture.release(fb);

    if (res != ESP_OK) break;
    delay(100); // batasi ~10 fps, cukup untuk verifikasi visual
  }

  return res;
}

void startStreamServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;

  httpd_uri_t indexUri = {"/", HTTP_GET, indexHandler, NULL};
  httpd_uri_t streamUri = {"/stream", HTTP_GET, streamHandler, NULL};

  if (httpd_start(&streamServer, &config) == ESP_OK) {
    httpd_register_uri_handler(streamServer, &indexUri);
    httpd_register_uri_handler(streamServer, &streamUri);
    Serial.println("[HTTP] Server stream siap.");
  } else {
    Serial.println("[HTTP] Gagal memulai server.");
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== ESP32-CAM Diagnostik: Web Stream ===");

  if (!cameraCapture.begin()) {
    Serial.println("[Setup] Inisialisasi kamera GAGAL. Cek wiring/daya.");
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("[WiFi] Menghubungkan");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WiFi] Terhubung!");
    Serial.print("[WiFi] Buka di browser: http://");
    Serial.println(WiFi.localIP());
    startStreamServer();
  } else {
    Serial.println("\n[WiFi] GAGAL terhubung setelah 20 detik.");
  }
}

void loop() {
  delay(1000);
}
