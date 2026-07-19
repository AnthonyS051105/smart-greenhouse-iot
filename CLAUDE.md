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

```bash
# ESP32 utama
# 1. Salin config.example.h → config.h, isi kredensial
# 2. Buka esp32-main/esp32-main.ino di Arduino IDE
# 3. Pilih board "ESP32 Dev Module", upload
# 4. Buka Serial Monitor (115200) untuk log

# ESP32-CAM
# 1. Buka esp32-cam/esp32-cam.ino
# 2. Pilih board "AI Thinker ESP32-CAM", upload (perlu FTDI/USB-TTL)
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
