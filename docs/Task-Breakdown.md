# Task Breakdown — IoT

> Repo: **iot/**. Penanggung jawab utama: **Anggota 1 (IoT — Hardware & Firmware)**.
> Timeline IoT: Coaching 11, 13/14, 17 Juli; Final Presentation 22 Juli.

---

## Legenda Prioritas
- 🔴 **P0** — Wajib (blocker demo).
- 🟡 **P1** — Penting (nilai tambah signifikan).
- 🟢 **P2** — Opsional (jika waktu tersisa).

---

## Fase 1: Setup & Sensor Dasar (Hari 1–3)

| ID | Task | Prioritas | Est. | Dependensi |
|----|------|-----------|------|------------|
| IOT-T01 | Setup Arduino IDE / PlatformIO + board ESP32 & ESP32-CAM | 🔴 P0 | 2j | - |
| IOT-T02 | Rangkai & uji baca DHT11 (suhu, kelembapan) | 🔴 P0 | 2j | IOT-T01 |
| IOT-T03 | Rangkai & uji baca BME280 via I2C | 🔴 P0 | 2j | IOT-T01 |
| IOT-T04 | Rangkai & uji OLED SSD1306 (tampil dummy) | 🔴 P0 | 2j | IOT-T01 |
| IOT-T05 | (Opsional) Rangkai & baca MQ gas | 🟢 P2 | 1j | IOT-T01 |
| IOT-T06 | Implement `SensorManager` (baca semua + toJson) | 🔴 P0 | 3j | IOT-T02,03 |

## Fase 2: Konektivitas (Hari 3–5)

| ID | Task | Prioritas | Est. | Dependensi |
|----|------|-----------|------|------------|
| IOT-T07 | Setup akun HiveMQ Cloud / Mosquitto + kredensial | 🔴 P0 | 1j | - |
| IOT-T08 | Implement `WifiManager` (connect + reconnect) | 🔴 P0 | 2j | IOT-T01 |
| IOT-T09 | Implement `MqttClient`: publish sensor ke topik | 🔴 P0 | 3j | IOT-T07,08 |
| IOT-T10 | Verifikasi payload sesuai `data-contracts.md §1.1` | 🔴 P0 | 1j | IOT-T09 |
| IOT-T11 | Implement subscribe command + parse JSON | 🔴 P0 | 3j | IOT-T09 |

## Fase 3: Aktuator Closed-Loop (Hari 5–7)

| ID | Task | Prioritas | Est. | Dependensi |
|----|------|-----------|------|------------|
| IOT-T12 | Rangkai 2 servo + catu daya | 🔴 P0 | 2j | IOT-T01 |
| IOT-T13 | Implement `ActuatorManager` (open/close + durasi) | 🔴 P0 | 3j | IOT-T12 |
| IOT-T14 | Rakit mekanisme pinch-valve (servo + selang) | 🔴 P0 | 3j | IOT-T12 |
| IOT-T15 | Rakit mekanisme louver ventilasi | 🟡 P1 | 2j | IOT-T12 |
| IOT-T16 | Hubungkan command MQTT → gerak servo (end-to-end) | 🔴 P0 | 3j | IOT-T11,13 |
| IOT-T17 | Validasi `expires_at` & idempotensi `command_id` | 🟡 P1 | 2j | IOT-T16 |
| IOT-T18 | Publish status aktuator ke `greenhouse/{id}/status` | 🟡 P1 | 2j | IOT-T16 |

## Fase 4: OLED, Fallback & Kamera (Hari 7–9)

| ID | Task | Prioritas | Est. | Dependensi |
|----|------|-----------|------|------------|
| IOT-T19 | Implement `DisplayManager` (status real-time di OLED) | 🔴 P0 | 3j | IOT-T04,06 |
| IOT-T20 | Implement `FallbackController` (aturan threshold lokal) | 🔴 P0 | 3j | IOT-T06,13 |
| IOT-T21 | Uji fallback: putus WiFi → servo tetap merespons lokal | 🔴 P0 | 2j | IOT-T20 |
| IOT-T22 | Setup ESP32-CAM: init kamera + ambil JPEG | 🔴 P0 | 3j | IOT-T01 |
| IOT-T23 | Implement `HttpUploader` → POST /images ke backend | 🔴 P0 | 3j | IOT-T22 |
| IOT-T24 | Uji end-to-end: citra sampai ke backend & Cloudinary | 🔴 P0 | 2j | IOT-T23 |

## Fase 5: Dashboard, Data Logging & Finalisasi (Hari 9–11)

| ID | Task | Prioritas | Est. | Dependensi |
|----|------|-----------|------|------------|
| IOT-T25 | Setup Node-RED dashboard (subscribe & visualisasi) | 🔴 P0 | 3j | IOT-T09 |
| IOT-T26 | Implement ekspor CSV di Node-RED / backend | 🔴 P0 | 2j | IOT-T25 |
| IOT-T27 | Buat wiring diagram lengkap di Wokwi | 🔴 P0 | 3j | semua rakit |
| IOT-T28 | Kalibrasi final servo & pinch-valve dengan air nyata | 🔴 P0 | 2j | IOT-T14 |
| IOT-T29 | Uji sistem penuh end-to-end (closed-loop + fallback) | 🔴 P0 | 3j | semua |
| IOT-T30 | Rekam video demo + siapkan materi presentasi IoT | 🔴 P0 | 2j | IOT-T29 |
| IOT-T31 | Rapikan repo + README + config.example.h | 🟡 P1 | 1j | semua |

---

## Milestone

| Milestone | Target | Kriteria |
|-----------|--------|----------|
| M1 | Akhir Hari 3 | Semua sensor terbaca & tampil di OLED. |
| M2 | Akhir Hari 5 | Data sensor sampai ke broker/backend. |
| M3 | Akhir Hari 7 | Servo bergerak merespons command (closed-loop dasar). |
| M4 | Akhir Hari 9 | Fallback berfungsi + citra sampai backend. |
| M5 | Akhir Hari 11 | Sistem penuh + dashboard + CSV + Wokwi + video demo. |

---

## Risiko & Mitigasi (IoT-spesifik)

| Risiko | Mitigasi |
|--------|----------|
| Servo kurang torsi menjepit selang | Pakai selang silikon tipis; kalibrasi awal (IOT-T28) jauh sebelum demo. |
| ESP32-CAM sulit dikonfigurasi | Alokasikan waktu ekstra; siapkan contoh kode referensi. |
| Konflik pin I2C (BME280 + OLED) | Gunakan bus I2C bersama dengan alamat berbeda; verifikasi alamat. |
| Integrasi command lambat | Prioritaskan 1 aktuator berfungsi penuh dulu (irigasi) sebelum ventilasi. |
