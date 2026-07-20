# CLAUDE.md — Panduan untuk Claude Code (Repo IoT)

> File ini memberi konteks kepada Claude Code saat mengerjakan repo **iot/**.
> Baca juga: `docs/SRS.md`, `docs/SDD.md`, `docs/Architecture.md`, `docs/data-contracts.md`.

---

## Tentang Proyek

**Smart Greenhouse + Marketplace** — Final Project Bootcamp TETI 2026 (AI × IoT × Mobile).
Repo ini adalah **subsistem IoT**: firmware untuk 2 board ESP32 yang membaca sensor greenhouse, menggerakkan aktuator servo secara closed-loop, dan berkomunikasi dengan backend via MQTT/HTTP.

**Tanaman fokus:** cabai (dapat berubah).

---

## Peran Repo Ini dalam Sistem

- Membaca sensor (DHT11, Capacitive Soil Moisture Sensor v1.2, BH1750) → kirim ke backend via MQTT.
- Menerima command dari backend → gerakkan servo (irigasi & ventilasi).
- ESP32-CAM ambil citra → kirim ke backend via HTTP.
- Fallback lokal saat offline.

**Backend, AI, dan Mobile ada di repo terpisah.** Repo ini HANYA firmware IoT.

---

## Status Hardware Saat Ini (sementara, akan berubah)

> **PENTING:** Hardware fisik yang tersedia SEKARANG lebih sedikit dari desain penuh di `docs/SRS.md`/`docs/SDD.md`. Kode firmware saat ini disesuaikan ke keterbatasan ini, tapi tetap modular agar mudah di-extend begitu hardware lengkap tersedia.

| Komponen | Status | Catatan |
|----------|--------|---------|
| DHT11 (pin **18**) | ✅ Tersedia | Terpasang & aktif di `SensorManager`. |
| Servo (pin **21**) | ✅ Tersedia, **hanya 1 unit** | Bukan 2 servo (irigasi+ventilasi terpisah) seperti desain awal. `ActuatorManager` memakai 1 servo generik yang merespons command `actuator` apapun (`irrigation` maupun `ventilation`). |
| ESP32-CAM | ✅ Tersedia | Firmware terpisah di `src/esp32cam/`, endpoint upload masih placeholder karena backend belum live. |
| Capacitive Soil Moisture Sensor v1.2 | ❌ Belum ada | Field `soil_moisture` dikirim sebagai **placeholder `-1`** di payload MQTT agar tetap sesuai `data-contracts.md §1.1`. |
| BH1750 (lux) | ❌ Belum ada | Field `light_intensity` juga placeholder `-1`. |
| OLED SSD1306 | ❌ Belum ada | Status ditampilkan via **Serial Monitor** saja (`DisplayManager` belum diimplementasikan). |
| Backend FastAPI (Railway) | ❌ Belum terhubung nyata | ESP32-CAM upload citra & command MQTT akan gagal/kosong sampai backend live — ini kondisi normal, bukan bug. |
| Mobile app ↔ Firebase | ❌ Belum terhubung nyata | Tidak memengaruhi firmware IoT secara langsung, tapi berarti belum ada validasi end-to-end penuh. |

**Saat sensor/hardware baru tersedia:** tambahkan pembacaan nyata di `SensorManager::readAll()` (ganti nilai placeholder), dan jika servo ke-2 tersedia, pisahkan kembali menjadi 2 instance `ActuatorManager` (irigasi & ventilasi terpisah) sesuai desain SDD asli.

---

## Stack Teknologi

- **Bahasa:** C++ (Arduino framework).
- **Board:** ESP32 DevKit (utama) + ESP32-CAM (terpisah).
- **Library utama:** PubSubClient (MQTT), ArduinoJson, DHT, BH1750 (I2C), Adafruit_SSD1306, ESP32Servo, esp_camera, HTTPClient. Capacitive Soil Moisture Sensor v1.2 dibaca langsung via `analogRead()`, tanpa library khusus.
- **Tools:** Arduino IDE / PlatformIO, Wokwi (wiring diagram).

---

## Konvensi Kode

- **Modularitas:** Pisahkan tiap tanggung jawab ke file `.h/.cpp` sendiri (SensorManager, ActuatorManager, MqttClient, dst — lihat `docs/SDD.md §2`).
- **Kredensial:** JANGAN hardcode WiFi/MQTT credentials. Gunakan `config.h` (di-.gitignore) dengan template `config.example.h`.
- **Penamaan:** camelCase untuk fungsi/variabel, PascalCase untuk class/struct, UPPER_CASE untuk konstanta.
- **JSON:** Selalu ikuti format di `docs/data-contracts.md` — jangan ubah nama field tanpa update kontrak.
- **Komentar:** Bahasa Indonesia, jelaskan "kenapa" bukan hanya "apa".

---

## Aturan Penting (JANGAN dilanggar)

1. **Format payload MQTT/HTTP harus PERSIS** sesuai `docs/data-contracts.md`. Perubahan format = koordinasi dengan tim backend dulu.
2. **Topik MQTT** mengikuti pola `greenhouse/{device_id}/{sensor|command|status}`.
3. **Validasi command:** selalu cek `expires_at` dan `command_id` sebelum menggerakkan servo.
4. **Fallback wajib ada** — jangan buat sistem yang mati total saat offline.
5. **Idempotensi:** command dengan `command_id` sama tidak dieksekusi dua kali.

---

## Cara Menjalankan / Menguji

> Proyek ini pakai **PlatformIO** (bukan Arduino IDE terpisah per board), dengan multi-environment di `platformio.ini`. Lihat struktur folder di bagian bawah.

```bash
# 1. Salin include/config.example.h -> include/config.h, isi kredensial WiFi & HiveMQ Cloud

# Firmware ESP32 utama (DHT11 + servo + MQTT closed-loop)
pio run -e main -t upload -t monitor

# Firmware ESP32-CAM (ambil citra + upload ke backend)
pio run -e esp32cam -t upload -t monitor
```

### Test per-komponen (debugging hardware)

Gunakan ini saat mencurigai satu sensor/aktuator tidak berfungsi — masing-masing env hanya mem-flash 1 komponen, tanpa dependensi ke modul lain:

```bash
pio run -e test-dht11 -t upload -t monitor       # DHT11 (pin 18) saja
pio run -e test-servo -t upload -t monitor        # Servo (pin 21) saja
pio run -e test-wifi-mqtt -t upload -t monitor    # WiFi + MQTT (HiveMQ Cloud) saja
pio run -e test-camera -t upload -t monitor        # ESP32-CAM: init + capture saja
```

**Uji cepat MQTT:** gunakan MQTT client (mis. MQTT Explorer) subscribe ke `greenhouse/#` untuk melihat data masuk.

---

## Prioritas Saat Ini

Lihat `docs/Task-Breakdown.md`. Fokus P0 dulu:
1. Sensor terbaca & terkirim (IOT-T01..10).
2. Command → servo bergerak (IOT-T11..16).
3. Fallback (IOT-T20..21).
4. ESP32-CAM kirim citra (IOT-T22..24).

---

## Yang TIDAK Perlu Dikerjakan di Repo Ini

- Logika AI (ada di repo `ai/`).
- Penyimpanan database (backend/Firestore).
- UI aplikasi (repo `mobile/`).
- Upload ke Cloudinary (dilakukan backend, bukan ESP32-CAM — ESP32-CAM hanya POST ke backend).

---

## Kontak Integrasi

- **Backend** menyediakan: broker MQTT URL, endpoint `POST /images`, format command.
- Jika butuh berubah kontrak data → update `docs/data-contracts.md` & kabari tim.

---

## Struktur Folder Repo (aktual, PlatformIO)

```
iot/
├── include/
│   ├── config.example.h      # Template kredensial (commit ini, bukan config.h)
│   ├── PinConfig.h            # Pemetaan pin fisik (DHT11=18, Servo=21)
│   ├── SensorManager.h
│   ├── ActuatorManager.h
│   ├── WifiManager.h
│   ├── MqttClient.h
│   ├── FallbackController.h
│   ├── CameraCapture.h
│   ├── CameraPins.h            # Pinout AI-Thinker ESP32-CAM
│   └── HttpUploader.h
├── src/
│   ├── SensorManager.cpp
│   ├── ActuatorManager.cpp
│   ├── WifiManager.cpp
│   ├── MqttClient.cpp
│   ├── FallbackController.cpp
│   ├── CameraCapture.cpp
│   ├── HttpUploader.cpp
│   ├── main/main.cpp           # Entry point firmware ESP32 utama (env: main)
│   ├── esp32cam/main.cpp       # Entry point firmware ESP32-CAM (env: esp32cam)
│   └── tests/                  # Firmware test per-komponen (lihat env test-*)
│       ├── test_dht11.cpp
│       ├── test_servo.cpp
│       ├── test_wifi_mqtt.cpp
│       └── test_camera.cpp
├── platformio.ini              # Multi-environment (main, esp32cam, test-*)
└── docs/                       # SRS, SDD, Task-Breakdown (symlink/copy shared)
```
