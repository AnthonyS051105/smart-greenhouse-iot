// ============================================================================
// DIAGNOSTIK ESP32-CAM — Versi Arduino IDE (single file, TANPA modular)
// ============================================================================
// Tujuan: membuktikan kamera & board AI-Thinker ESP32-CAM berfungsi secara
// fisik, dipakai saat Serial Monitor board ini terus-menerus menampilkan
// karakter acak (sudah dicoba: baud 115200 & 9600, ganti kabel USB, tekan
// RST -- semua hasilnya sama, WiFi juga tidak pernah berhasil connect).
//
// File ini punya 2 lapis diagnostik yang berjalan BERURUTAN, supaya kita
// tahu persis board berhenti di tahap mana:
//
//   TAHAP 0 (paling awal, TIDAK butuh kamera/WiFi sama sekali):
//     LED flash on-board (GPIO 4) berkedip terus tiap 500ms sejak baris
//     PERTAMA di loop(). Kalau LED ini TIDAK PERNAH kedip sama sekali,
//     board bahkan tidak sanggup menjalankan kode paling sederhana --
//     ini konfirmasi PASTI brownout/board rusak, di luar soal kamera.
//
//   TAHAP 1: Inisialisasi kamera (esp_camera_init). Kalau board mati/reset
//     tepat di titik ini (LED yang tadinya kedip jadi berhenti/board
//     restart), itu konfirmasi brownout SAAT KAMERA DINYALAKAN.
//
//   TAHAP 2: Connect WiFi + jalankan web server MJPEG stream. Kalau lolos
//     sampai sini, buka http://<IP-yang-tercetak>/ di browser HP/laptop
//     yang satu jaringan WiFi.
//
// CARA UJI PALING PENTING (tanpa perlu Serial Monitor sama sekali):
//   1. Upload sketch ini via Arduino IDE seperti biasa (lewat laptop).
//   2. Setelah upload selesai, CABUT dari laptop, colokkan ke CHARGER HP
//      terpisah (5V, idealnya 2A) -- BUKAN ke port USB laptop.
//   3. Amati LED flash board:
//        - LED kedip teratur terus-menerus            -> board hidup stabil,
//          lanjut cek kamera (LED akan mati/redup sebentar saat kamera
//          coba dinyalakan, sekitar 1-2 detik setelah boot).
//        - LED TIDAK PERNAH menyala/kedip sama sekali  -> board benar-benar
//          tidak dapat daya cukup / rusak, ganti sumber daya atau board.
//        - LED kedip lalu MATI TOTAL setelah beberapa detik (tidak kedip
//          lagi) -> board reset-loop, kemungkinan besar brownout saat
//          esp_camera_init() atau WiFi.begin().
//
// SEBELUM UPLOAD, wajib diisi manual (baris ~40-42 di bawah):
//   WIFI_SSID, WIFI_PASSWORD -- sama seperti di iot/include/config.h
//
// Board di Arduino IDE: pilih "AI Thinker ESP32-CAM"
// (Tools > Board > ESP32 Arduino > AI Thinker ESP32-CAM)
// Partition Scheme: "Huge APP (3MB No OTA/1MB SPIFFS)" (kamera butuh flash
// besar), Upload Speed bisa dicoba 115200 dulu kalau upload gagal di speed
// tinggi (ini juga bisa jadi indikasi masalah kabel/driver USB-Serial).
// ============================================================================

#include <WiFi.h>
#include <esp_camera.h>
#include <esp_http_server.h>

// ---------- ISI SESUAI CONFIG.H KAMU ----------
#define WIFI_SSID     "Adhyasta"
#define WIFI_PASSWORD "juarasatu"
// ------------------------------------------------

#define LED_FLASH_PIN 4  // LED flash on-board AI-Thinker ESP32-CAM

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

#define PART_BOUNDARY "greenhousecamboundary"
static const char *STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t streamServer = NULL;
bool cameraOk = false;
bool wifiOk = false;

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
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
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

    esp_camera_fb_return(fb);

    if (res != ESP_OK) break;
    delay(100); // batasi ~10 fps
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
  }
}

bool initCamera() {
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
  // XCLK diturunkan ke 10MHz -- 20MHz diketahui menyebabkan cam_task stack
  // overflow/Guru Meditation Error di sebagian modul OV2640 board AI-Thinker
  // murah (clock generator internal sensor tidak stabil di 20MHz).
  config.xclk_freq_hz = 10000000;
  config.pixel_format = PIXFORMAT_JPEG;
  // CAMERA_GRAB_LATEST (bukan default CAMERA_GRAB_WHEN_EMPTY) mencegah
  // driver menunggu/mem-block di task internal saat buffer penuh --
  // salah satu penyebab umum "Stack canary watchpoint triggered (cam_task)".
  config.grab_mode = CAMERA_GRAB_LATEST;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 2;
    config.fb_location = CAMERA_FB_IN_PSRAM;
  } else {
    config.frame_size = FRAMESIZE_QVGA; // paling ringan, khusus diagnostik
    config.jpeg_quality = 15;
    config.fb_count = 1;
    config.fb_location = CAMERA_FB_IN_DRAM;
  }

  return esp_camera_init(&config) == ESP_OK;
}

void setup() {
  // TAHAP 0: LED indikator dulu, sebelum apapun yang berat (kamera/WiFi).
  pinMode(LED_FLASH_PIN, OUTPUT);
  digitalWrite(LED_FLASH_PIN, LOW);

  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== ESP32-CAM Diagnostik (Arduino IDE, single file) ===");

  // Kedip 3x cepat di awal -> bukti board hidup & sampai baris ini.
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_FLASH_PIN, HIGH);
    delay(150);
    digitalWrite(LED_FLASH_PIN, LOW);
    delay(150);
  }

  // TAHAP 1: kamera.
  Serial.println("[Tahap 1] Inisialisasi kamera...");
  cameraOk = initCamera();
  Serial.println(cameraOk ? "[Tahap 1] Kamera OK." : "[Tahap 1] Kamera GAGAL.");

  // Kedip 2x -> bukti berhasil lewati inisialisasi kamera tanpa crash.
  for (int i = 0; i < 2; i++) {
    digitalWrite(LED_FLASH_PIN, HIGH);
    delay(300);
    digitalWrite(LED_FLASH_PIN, LOW);
    delay(300);
  }

  // TAHAP 2: WiFi + web server.
  Serial.println("[Tahap 2] Menghubungkan WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  wifiOk = (WiFi.status() == WL_CONNECTED);
  if (wifiOk) {
    Serial.println("\n[Tahap 2] WiFi terhubung!");
    Serial.print("[Tahap 2] Buka di browser: http://");
    Serial.println(WiFi.localIP());
    if (cameraOk) {
      startStreamServer();
      Serial.println("[Tahap 2] Server stream siap.");
    }
  } else {
    Serial.println("\n[Tahap 2] WiFi GAGAL terhubung.");
  }
}

void loop() {
  // LED berkedip PELAN terus-menerus selama board hidup normal (heartbeat).
  // Kalau LED ini berhenti berkedip kapan saja setelah setup() selesai,
  // board sudah reset/crash lagi -- amati pola kedipnya untuk diagnosis.
  digitalWrite(LED_FLASH_PIN, HIGH);
  delay(500);
  digitalWrite(LED_FLASH_PIN, LOW);
  delay(500);
}
