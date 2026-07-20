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

> **PENTING:** Hardware fisik yang tersedia SEKARANG lebih sedikit dari desain penuh di `docs/SRS.md`/`docs/SDD.md` (masih 1 servo, bukan 2; OLED belum ada). Kode firmware tetap modular agar mudah di-extend begitu hardware lengkap tersedia.

| Komponen | Status | Catatan |
|----------|--------|---------|
| DHT11 (pin **18**) | ✅ Tersedia | Aktif di `SensorManager`. |
| Servo (pin **19**) | ✅ Tersedia, **hanya 1 unit** | Dipindah dari pin 21 (sekarang dipakai BH1750 SDA) ke pin 19 untuk menghindari konflik. Bukan 2 servo (irigasi+ventilasi terpisah) seperti desain awal — `ActuatorManager` memakai 1 servo generik yang merespons command `actuator` apapun (`irrigation` maupun `ventilation`). |
| Capacitive Soil Moisture Sensor v1.2 (pin **25**, ADC) | ✅ Tersedia | Aktif di `SoilMoistureManager`. **Kalibrasi `SOIL_MOISTURE_DRY_RAW`/`SOIL_MOISTURE_WET_RAW` di `include/PinConfig.h` masih nilai perkiraan** — wajib dikalibrasi ulang dengan sensor fisik sebelum demo (pakai `pio run -e test-soil-moisture`, lihat IOT-T28). |
| BH1750 (I2C, SDA=**21** SCL=**22**) | ✅ Tersedia | Aktif di `LightManager`. Alamat default `0x23`. |
| ESP32-CAM | ✅ Tersedia | Firmware di `src/esp32cam/`, upload ke backend Railway live. |
| OLED SSD1306 | ❌ Belum ada | Status ditampilkan via **Serial Monitor** saja (`DisplayManager` belum diimplementasikan). |
| Backend FastAPI (Railway), MQTT (HiveMQ Cloud), Firebase, Cloudinary | ✅ Live & terkonfigurasi | Closed-loop penuh sudah bisa diuji end-to-end. |

**Jika servo ke-2 tersedia:** pisahkan kembali menjadi 2 instance `ActuatorManager` (irigasi & ventilasi terpisah) sesuai desain SDD asli.

### Catatan Integrasi Backend (penting)

Backend FastAPI (`backend/app/`) awalnya dibangun terhadap skema sensor **berbeda** dari `shared/data-contracts.md` (memakai `plot_id`+`water_level`+`N`/`P`/`K`, hasil model AI yang sudah dilatih terhadap dataset lama). Ini sudah **diselaraskan** ke kontrak resmi:

- `backend/app/schemas.py` (`SensorReadingIn`) sekarang menerima `device_id`, `plot_id`, `temperature`, `humidity`, `soil_moisture`, `light_intensity` — persis field payload MQTT firmware ini.
- `backend/app/services/sensor_service.py` menyimpan `water_level` sebagai **alias** dari `soil_moisture` (representasi fisik yang sama) khusus agar model AI irigasi (`actuator_model.py`, sudah dilatih dengan nama fitur `water_level`) tetap berfungsi tanpa perlu di-retrain.
- `POST /images` sekarang menerima field `device_id`+`plot_id`+`image` (bukan `plot_id`+`file` versi lama) — cocok dengan `HttpUploader` di `src/esp32cam/`.
- `POST /irrigation/trigger` sekarang mem-publish command MQTT lengkap sesuai `data-contracts.md §1.2` (`command_id`, `actuator`, `expires_at`, `source`) — sebelumnya hanya `{action, duration_seconds}` yang akan DITOLAK oleh `MqttClient::handleMessage` firmware ini (validasi `expires_at`/idempotensi `command_id` wajib ada).
- **Firmware TIDAK dilonggarkan** untuk menerima command tanpa `command_id`/`expires_at` — kontrak `data-contracts.md` adalah sumber kebenaran, backend yang menyesuaikan.

Payload sensor firmware ini sekarang menyertakan `plot_id` (dari `config.h`) di luar skema resmi §1.1, semata agar backend bisa langsung mengaitkan `sensor_readings` ke plot tanpa lookup `device_id`→`plot_id` terpisah.

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

# Firmware ESP32 utama (DHT11 + Soil Moisture v1.2 + BH1750 + servo + MQTT closed-loop)
pio run -e main -t upload -t monitor

# Firmware ESP32-CAM (ambil citra + upload ke backend)
pio run -e esp32cam -t upload -t monitor
```

### Test per-komponen (debugging hardware)

Gunakan ini saat mencurigai satu sensor/aktuator tidak berfungsi — masing-masing env hanya mem-flash 1 komponen, tanpa dependensi ke modul lain:

```bash
pio run -e test-dht11 -t upload -t monitor          # DHT11 (pin 18) saja
pio run -e test-servo -t upload -t monitor           # Servo (pin 19) saja
pio run -e test-soil-moisture -t upload -t monitor   # Soil Moisture v1.2 (pin 25) saja -- lihat nilai raw utk kalibrasi
pio run -e test-bh1750 -t upload -t monitor          # BH1750 (I2C SDA=21 SCL=22) saja
pio run -e test-wifi-mqtt -t upload -t monitor       # WiFi + MQTT (HiveMQ Cloud) saja
pio run -e test-camera -t upload -t monitor           # ESP32-CAM: init + capture saja
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
│   ├── PinConfig.h            # Pemetaan pin fisik (DHT11=18, Servo=19, Soil=25, BH1750 SDA=21/SCL=22)
│   ├── SensorManager.h
│   ├── SoilMoistureManager.h
│   ├── LightManager.h
│   ├── ActuatorManager.h
│   ├── WifiManager.h
│   ├── MqttClient.h
│   ├── FallbackController.h
│   ├── CameraCapture.h
│   ├── CameraPins.h            # Pinout AI-Thinker ESP32-CAM
│   └── HttpUploader.h
├── src/
│   ├── SensorManager.cpp
│   ├── SoilMoistureManager.cpp
│   ├── LightManager.cpp
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
│       ├── test_soil_moisture.cpp
│       ├── test_bh1750.cpp
│       ├── test_wifi_mqtt.cpp
│       └── test_camera.cpp
├── platformio.ini              # Multi-environment (main, esp32cam, test-*)
└── docs/                       # SRS, SDD, Task-Breakdown (symlink/copy shared)
```
