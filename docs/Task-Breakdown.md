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
| IOT-T02 | Rangkai & uji baca DHT11 (suhu, kelembapan udara) | 🔴 P0 | 2j | IOT-T01 |
| IOT-T03 | Rangkai & uji baca Capacitive Soil Moisture Sensor v1.2 (analog) + kalibrasi kering/basah | 🔴 P0 | 2j | IOT-T01 |
| IOT-T04 | Rangkai & uji OLED SSD1306 (tampil dummy) | 🔴 P0 | 2j | IOT-T01 |
| IOT-T05 | Rangkai & uji baca BH1750 via I2C (lux) | 🔴 P0 | 2j | IOT-T01 |
| IOT-T06 | Implement `SensorManager` (baca semua + toJson) | 🔴 P0 | 3j | IOT-T02,03,05 |

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
| IOT-T25 | Setup Node-RED dashboard (subscribe & visualisasi) — **4 widget gauge/chart: Suhu, Kelembapan Udara, Kelembapan Tanah, Intensitas Cahaya** (lihat catatan revisi sensor di bawah) | 🔴 P0 | 3j | IOT-T09 |
| IOT-T26 | Implement ekspor CSV di Node-RED / backend — kolom CSV: `timestamp, temperature, humidity, soil_moisture, light_intensity` | 🔴 P0 | 2j | IOT-T25 |
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
| Konflik pin I2C (BH1750 + OLED) | Gunakan bus I2C bersama dengan alamat berbeda; verifikasi alamat (BH1750 `0x23`, OLED `0x3C`). |
| Integrasi command lambat | Prioritaskan 1 aktuator berfungsi penuh dulu (irigasi) sebelum ventilasi. |
| Soil Moisture Sensor v1.2 drift/tidak stabil (korosi ringan, noise ADC) | Kalibrasi ulang nilai kering/basah H-3 sebelum demo (sejalan IOT-T28); pertimbangkan rata-rata beberapa pembacaan (smoothing) untuk kurangi noise. |

---

## Catatan Revisi Sensor — Wajib Diterapkan ke Dashboard Node-RED (IOT-T25/T26)

> Set sensor proyek berubah dari (DHT11 + BME280 + MQ opsional) menjadi **DHT11 + Capacitive Soil
> Moisture Sensor v1.2 + BH1750**. Jika flow Node-RED sudah pernah dibuat/di-import dari versi
> lama (mis. dari template lain atau sesi sebelumnya), sesuaikan sebagai berikut sebelum demo:

1. **Node MQTT-in**: subscribe tetap ke topik `greenhouse/{device_id}/sensor` (tidak berubah),
   tapi payload JSON kini berisi `temperature`, `humidity`, `soil_moisture`, `light_intensity` —
   **field `pressure` dan `gas_level` sudah tidak dikirim ESP32 sama sekali**. Hapus node
   gauge/chart yang membaca `msg.payload.pressure` / `msg.payload.gas_level`, ganti jadi
   `msg.payload.soil_moisture` / `msg.payload.light_intensity`.
2. **Dashboard UI (4 widget)**: susun ulang jadi 4 gauge/chart sesuai `docs/data-contracts.md
   §1.1` & §3.4:
   - **Suhu** (`temperature`, °C) — gauge, rentang wajar 15–40°C.
   - **Kelembapan Udara** (`humidity`, %) — gauge, rentang 0–100%.
   - **Kelembapan Tanah** (`soil_moisture`, %) — gauge, rentang 0–100% (sudah hasil pemetaan
     ADC→persen di firmware, node Node-RED tidak perlu kalkulasi ulang).
   - **Intensitas Cahaya** (`light_intensity`, lux) — gauge/chart, rentang bisa cukup lebar
     (0–2000+ lux tergantung kondisi greenhouse) — gunakan skala non-linear atau chart garis biasa
     bila gauge linear kurang terbaca.
3. **Node CSV export (IOT-T26)**: update `function`/`csv` node agar header & urutan kolom jadi
   `timestamp,temperature,humidity,soil_moisture,light_intensity` — hapus kolom `pressure`/
   `gas_level` dari template lama.
4. **Alarm/threshold node (jika ada)**: aturan lama yang memicu warning berbasis `gas_level`
   (mis. "gas > ambang → alert") harus dihapus atau diganti — tidak ada penggantinya karena sensor
   gas memang tidak dipakai lagi (bukan penyederhanaan sepihak, ini keputusan tim).
5. **Verifikasi akhir**: kirim payload dummy (`mosquitto_pub` atau MQTT Explorer) dengan skema
   baru ke topik sensor, pastikan seluruh 4 gauge/chart di dashboard Node-RED terisi benar dan
   ekspor CSV menghasilkan 4 kolom data yang sesuai — lakukan ini sebagai bagian pengujian
   IOT-T29 (uji sistem penuh end-to-end).
