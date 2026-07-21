// ============================================================================
// Test Konektivitas WiFi — ESP32-CAM (Versi Arduino IDE, single file)
// ============================================================================
// TUJUAN: mengisolasi masalah "ESP32-CAM tidak mau connect WiFi" dari
// esp32cam_arduino_ide.ino. Sketch ini SENGAJA tidak menginisialisasi kamera
// sama sekali, supaya hasilnya membuktikan apakah masalahnya ada di:
//   (a) WiFi/hardware/kredensial itu sendiri, ATAU
//   (b) konflik kamera+WiFi (DMA/power) seperti yang sudah pernah terjadi
//       di esp32cam_arduino_ide.ino (lihat komentar di cameraBegin() di sana).
//
// Jika sketch INI berhasil connect tapi esp32cam_arduino_ide.ino tidak
// -> masalahnya di kamera/power, bukan WiFi.
// Jika sketch INI JUGA gagal connect -> masalahnya murni WiFi (kredensial,
// sinyal, band 5GHz, atau power supply board).
//
// SEBELUM UPLOAD, isi WIFI_SSID & WIFI_PASSWORD di bawah.
//
// Board di Arduino IDE: Tools > Board > ESP32 Arduino > AI Thinker ESP32-CAM
// Upload: GPIO0 harus di-jumper ke GND dulu (mode flashing), lalu setelah
// upload SELESAI, lepas jumper GPIO0 dan tekan tombol RESET supaya sketch
// jalan normal (kalau GPIO0 masih ke GND, board akan stuck di boot mode
// dan WiFi TIDAK AKAN PERNAH connect walau kodenya benar).
// ============================================================================

#include <WiFi.h>
#include "esp_system.h"

// ---------- ISI SESUAI WiFi kamu ----------
#define WIFI_SSID       "Adhyasta"
#define WIFI_PASSWORD   "juarasatu"
// -------------------------------------------

static const unsigned long CONNECT_TIMEOUT_MS = 20000; // 20 detik per percobaan
static const unsigned long RETRY_INTERVAL_MS   = 5000;

unsigned long lastAttemptMillis = 0;
int attemptCount = 0;

// ---------------------------------------------------------------------------
// Diagnostik awal (power-on reason) -- brownout adalah penyebab paling umum
// ESP32-CAM "tidak mau connect WiFi": modul WiFi butuh lonjakan arus (~300+mA)
// saat transmit, dan kalau board disuplai dari pin 5V programmer FTDI/USB-TTL
// (yang cuma sanggup ~500mA total), tegangan drop dan radio WiFi gagal init
// diam-diam. Reset reason "Brownout" langsung mengonfirmasi ini.
// ---------------------------------------------------------------------------
void printResetReason() {
  esp_reset_reason_t reason = esp_reset_reason();
  Serial.print("[Diag] Reset reason: ");
  switch (reason) {
    case ESP_RST_POWERON:   Serial.println("Power-on normal"); break;
    case ESP_RST_BROWNOUT:
      Serial.println("BROWNOUT!");
      Serial.println("[Diag] -> Suplai daya TIDAK CUKUP (biasanya lewat pin 5V FTDI/USB-TTL).");
      Serial.println("[Diag] -> Coba beri daya ESP32-CAM dari adaptor 5V/2A terpisah, bukan dari USB-TTL programmer.");
      break;
    case ESP_RST_PANIC:     Serial.println("Software panic/crash"); break;
    case ESP_RST_INT_WDT:
    case ESP_RST_TASK_WDT:
    case ESP_RST_WDT:       Serial.println("Watchdog timeout (kemungkinan hang di suatu proses)"); break;
    case ESP_RST_DEEPSLEEP: Serial.println("Bangun dari deep sleep"); break;
    case ESP_RST_SW:        Serial.println("Restart via software (ESP.restart())"); break;
    default:
      Serial.printf("Lainnya (kode %d)\n", (int)reason);
      break;
  }
}

// ---------------------------------------------------------------------------
// Scan jaringan sekitar -- membuktikan radio WiFi ESP32-CAM hidup & bisa
// "mendengar" access point, terlepas dari berhasil/tidaknya nanti connect.
// ---------------------------------------------------------------------------
void scanNetworks() {
  Serial.println("\n[Scan] Mencari jaringan WiFi di sekitar...");
  int n = WiFi.scanNetworks();

  if (n <= 0) {
    Serial.println("[Scan] TIDAK ADA jaringan ditemukan sama sekali!");
    Serial.println("[Scan] -> Kemungkinan modul WiFi/antena ESP32-CAM rusak,");
    Serial.println("[Scan]    atau board sedang brownout (lihat Reset reason di atas).");
    return;
  }

  Serial.printf("[Scan] Ditemukan %d jaringan:\n", n);
  bool targetFound = false;
  for (int i = 0; i < n; i++) {
    String ssid = WiFi.SSID(i);
    Serial.printf("  %2d: %-32s RSSI=%4ld dBm  Ch=%2d  %s\n",
                  i + 1,
                  ssid.c_str(),
                  (long)WiFi.RSSI(i),
                  WiFi.channel(i),
                  (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "OPEN" : "SECURED");
    if (ssid == WIFI_SSID) targetFound = true;
  }

  if (targetFound) {
    Serial.printf("[Scan] SSID target \"%s\" DITEMUKAN di sekitar. Radio WiFi berfungsi normal.\n", WIFI_SSID);
  } else {
    Serial.printf("[Scan] SSID target \"%s\" TIDAK TERDETEKSI oleh scan!\n", WIFI_SSID);
    Serial.println("[Scan] -> Kemungkinan: (1) router pakai band 5GHz (ESP32 CUMA support 2.4GHz),");
    Serial.println("[Scan]    (2) salah ketik nama SSID, (3) SSID hidden, atau (4) sinyal terlalu lemah/jauh.");
  }
}

// ---------------------------------------------------------------------------
// Terjemahan wl_status_t ke pesan yang actionable.
// ---------------------------------------------------------------------------
void printStatusDiagnosis(wl_status_t status) {
  Serial.print("[WiFi] Status akhir: ");
  switch (status) {
    case WL_CONNECTED:
      Serial.println("TERHUBUNG");
      break;
    case WL_NO_SSID_AVAIL:
      Serial.println("SSID TIDAK DITEMUKAN");
      Serial.println("[WiFi] -> Cek nama SSID persis (huruf besar/kecil berpengaruh) & pastikan band 2.4GHz.");
      break;
    case WL_CONNECT_FAILED:
      Serial.println("GAGAL CONNECT (auth failure)");
      Serial.println("[WiFi] -> Kemungkinan besar PASSWORD salah, atau router menolak (MAC filter/limit device).");
      break;
    case WL_DISCONNECTED:
      Serial.println("DISCONNECTED (tidak ada progres sama sekali)");
      Serial.println("[WiFi] -> Router mungkin di luar jangkauan, atau board sedang brownout saat radio TX.");
      break;
    case WL_IDLE_STATUS:
      Serial.println("IDLE (WiFi.begin() belum benar-benar jalan)");
      break;
    default:
      Serial.printf("Kode %d (lihat referensi wl_status_t di dokumentasi ESP32 Arduino core)\n", (int)status);
      break;
  }
}

void attemptConnect() {
  attemptCount++;
  Serial.printf("\n[WiFi] === Percobaan koneksi ke-%d ===\n", attemptCount);
  Serial.print("[WiFi] Menghubungkan ke ");
  Serial.println(WIFI_SSID);

  WiFi.disconnect(true);
  delay(100);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < CONNECT_TIMEOUT_MS) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();

  wl_status_t finalStatus = WiFi.status();
  printStatusDiagnosis(finalStatus);

  if (finalStatus == WL_CONNECTED) {
    Serial.println("\n=== WiFi BERHASIL terhubung ===");
    Serial.print("[WiFi] IP Address : "); Serial.println(WiFi.localIP());
    Serial.print("[WiFi] Gateway    : "); Serial.println(WiFi.gatewayIP());
    Serial.print("[WiFi] Subnet     : "); Serial.println(WiFi.subnetMask());
    Serial.print("[WiFi] DNS        : "); Serial.println(WiFi.dnsIP());
    Serial.print("[WiFi] MAC        : "); Serial.println(WiFi.macAddress());
    Serial.print("[WiFi] RSSI       : "); Serial.print(WiFi.RSSI()); Serial.println(" dBm");
    Serial.println("\n=> Kesimpulan: hardware & kredensial WiFi board ini OK.");
    Serial.println("=> Kalau esp32cam_arduino_ide.ino tetap gagal connect, cek konflik kamera/power di sana.");
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== TEST KONEKTIVITAS WIFI — ESP32-CAM ===");
  Serial.println("(Sketch ini TIDAK menginisialisasi kamera sama sekali.)");

  printResetReason();
  scanNetworks();
  attemptConnect();

  lastAttemptMillis = millis();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    // Sudah connect: laporkan status tiap 5 detik supaya kelihatan kalau
    // koneksi drop/putus sendiri (indikasi power tidak stabil).
    static unsigned long lastReport = 0;
    if (millis() - lastReport > 5000) {
      Serial.printf("[WiFi] Masih terhubung. RSSI=%ld dBm, IP=%s\n",
                    (long)WiFi.RSSI(), WiFi.localIP().toString().c_str());
      lastReport = millis();
    }
    return;
  }

  // Kalau putus/gagal, coba lagi tiap RETRY_INTERVAL_MS.
  if (millis() - lastAttemptMillis > RETRY_INTERVAL_MS) {
    lastAttemptMillis = millis();
    attemptConnect();
  }
}
