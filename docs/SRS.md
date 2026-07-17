# SRS — Software Requirements Specification (IoT)

> Repo: **iot/** — Smart Greenhouse + Marketplace.
> Referensi induk: `shared/PRD.md`, `shared/data-contracts.md`, `shared/Architecture.md`.

---

## 1. Pendahuluan

### 1.1 Tujuan
Mendefinisikan kebutuhan perangkat lunak (firmware) dan perangkat keras untuk subsistem IoT: akuisisi data sensor, kontrol aktuator servo closed-loop, komunikasi MQTT, tampilan OLED, dan fallback lokal.

### 1.2 Ruang Lingkup
Subsistem IoT mencakup 2 board:
- **ESP32 utama:** membaca DHT11, BME280, MQ (opsional); menampilkan status di OLED; menggerakkan 2 servo (irigasi & ventilasi); komunikasi MQTT.
- **ESP32-CAM:** mengambil citra tanaman berkala; mengirim via HTTP POST ke backend.

### 1.3 Definisi
Lihat `shared/glossary.md`.

---

## 2. Deskripsi Umum

### 2.1 Perspektif Produk
Subsistem IoT adalah "indera & tangan" sistem: membaca kondisi greenhouse dan mengeksekusi aksi fisik. Terhubung ke backend via MQTT broker.

### 2.2 Antarmuka Sistem
- **Ke broker MQTT:** publish sensor & status, subscribe command.
- **Ke backend (ESP32-CAM):** HTTP POST citra.
- **Ke pengguna (lokal):** OLED menampilkan status.

### 2.3 Batasan Perangkat Keras
- ESP32-CAM memiliki GPIO terbatas (banyak dipakai modul kamera) → tidak dibebani sensor/servo.
- Servo SG90: torsi ~1.8 kg/cm, ditenagai 5V; >2 servo perlu catu daya eksternal.

---

## 3. Kebutuhan Fungsional (Functional Requirements)

### 3.1 Akuisisi Data Sensor
| ID | Kebutuhan | Prioritas |
|----|-----------|-----------|
| IOT-FR-01 | ESP32 membaca suhu & kelembapan dari DHT11 setiap 5–10 menit. | Wajib |
| IOT-FR-02 | ESP32 membaca suhu, kelembapan, tekanan dari BME280 setiap 5–10 menit. | Wajib |
| IOT-FR-03 | ESP32 membaca kadar gas dari MQ setiap 10–15 menit (opsional). | Opsional |
| IOT-FR-04 | ESP32 menyusun payload JSON sesuai `data-contracts.md` §1.1. | Wajib |

### 3.2 Komunikasi
| ID | Kebutuhan | Prioritas |
|----|-----------|-----------|
| IOT-FR-05 | ESP32 terhubung WiFi & MQTT broker dengan kredensial. | Wajib |
| IOT-FR-06 | ESP32 publish data sensor ke `greenhouse/{id}/sensor`. | Wajib |
| IOT-FR-07 | ESP32 subscribe & memproses command dari `greenhouse/{id}/command`. | Wajib |
| IOT-FR-08 | ESP32 publish status aktuator ke `greenhouse/{id}/status`. | Wajib |
| IOT-FR-09 | ESP32-CAM mengirim citra via HTTP POST ke `POST /images` setiap 1–2 jam. | Wajib |

### 3.3 Kontrol Aktuator
| ID | Kebutuhan | Prioritas |
|----|-----------|-----------|
| IOT-FR-10 | ESP32 menggerakkan servo pinch-valve (irigasi) sesuai command (open/close + durasi). | Wajib |
| IOT-FR-11 | ESP32 menggerakkan servo louver (ventilasi) sesuai command. | Wajib |
| IOT-FR-12 | ESP32 mengabaikan command yang sudah melewati `expires_at`. | Wajib |
| IOT-FR-13 | ESP32 tidak mengeksekusi command dengan `command_id` yang sudah pernah diproses (idempotensi). | Sebaiknya |

### 3.4 Tampilan & Fallback
| ID | Kebutuhan | Prioritas |
|----|-----------|-----------|
| IOT-FR-14 | OLED menampilkan suhu, kelembapan, status koneksi, & status aktuator terkini. | Wajib |
| IOT-FR-15 | Saat koneksi broker terputus > 60 detik, ESP32 masuk mode fallback & jalankan threshold lokal. | Wajib |
| IOT-FR-16 | Threshold fallback: suhu > 32°C → buka ventilasi; kelembapan tidak update & di bawah ambang → buka irigasi singkat. | Wajib |
| IOT-FR-17 | Saat koneksi pulih, ESP32 kembali ke mode online & lapor status. | Wajib |
| IOT-FR-18 | OLED/serial menampilkan pairing code / device_id untuk proses pairing di app. | Sebaiknya |

### 3.5 Data Logging
| ID | Kebutuhan | Prioritas |
|----|-----------|-----------|
| IOT-FR-19 | (via backend/Node-RED) Seluruh data sensor tersimpan & dapat diekspor ke CSV. | Wajib |

---

## 4. Kebutuhan Non-Fungsional

| ID | Kebutuhan |
|----|-----------|
| IOT-NFR-01 | **Reliability:** Fallback lokal menjaga tanaman saat offline. |
| IOT-NFR-02 | **Robustness:** Data yang gagal terkirim di-buffer & dikirim ulang saat koneksi pulih (jika ada penyimpanan lokal). |
| IOT-NFR-03 | **Keamanan:** Kredensial WiFi/MQTT tidak di-hardcode di repo publik (gunakan file config terpisah / secrets). |
| IOT-NFR-04 | **Maintainability:** Kode modular (sensor, aktuator, komunikasi, fallback dalam file/modul terpisah). |
| IOT-NFR-05 | **Power:** Interval baca disesuaikan agar hemat daya & bandwidth. |
| IOT-NFR-06 | **Latensi:** Command dari backend dieksekusi < 2 detik setelah diterima. |

---

## 5. Kebutuhan Perangkat Keras

| Komponen | Spesifikasi | Jumlah |
|----------|-------------|--------|
| ESP32 DevKit | Mikrokontroler utama, WiFi | 1 |
| ESP32-CAM | Board kamera terpisah | 1 |
| DHT11 | Sensor suhu & kelembapan | 1 |
| BME280 | Sensor suhu, kelembapan, tekanan (I2C) | 1 |
| MQ series | Sensor gas (opsional) | 0–1 |
| OLED SSD1306 | Display 128×64 (I2C) | 1 |
| Servo SG90 | Aktuator (pinch-valve & louver) | 2 |
| Selang silikon lunak | Untuk mekanisme pinch-valve | 1 |
| Catu daya 5V | Untuk servo (eksternal jika >2) | 1 |
| Breadboard, kabel jumper | Prototyping | secukupnya |

---

## 6. Skenario Penggunaan (Use Case Ringkas)

- **UC-IOT-01:** Sistem membaca sensor & mengirim ke backend secara periodik.
- **UC-IOT-02:** Backend mengirim command irigasi → servo membuka pinch-valve selama durasi tertentu → menutup.
- **UC-IOT-03:** Suhu tinggi → backend/AI kirim command ventilasi → servo louver terbuka.
- **UC-IOT-04:** Koneksi terputus → sistem masuk fallback → jalankan aturan lokal.
- **UC-IOT-05:** ESP32-CAM memotret tanaman → kirim ke backend untuk dianalisis AI.

---

## 7. Kriteria Penerimaan (Acceptance Criteria)

- [ ] Data sensor tampil di dashboard/backend secara periodik & benar formatnya.
- [ ] Servo irigasi & ventilasi bergerak otomatis merespons command dari backend.
- [ ] OLED menampilkan status real-time.
- [ ] Fallback lokal terbukti berfungsi saat WiFi diputus.
- [ ] Citra dari ESP32-CAM berhasil sampai ke backend & tersimpan di Cloudinary.
- [ ] Data dapat diekspor ke CSV.
- [ ] Wiring diagram tersedia di Wokwi.

---

## 8. Ketertelusuran ke Rubrik IoT

| Kebutuhan | Kriteria Rubrik |
|-----------|-----------------|
| Closed-loop (IOT-FR-10..12) + fallback (IOT-FR-15..17) | Desain & Arsitektur Sistem (25%) |
| Servo actuation | Kreativitas tools & metodologi (20%) |
| Demo aktuator bergerak | Cara kerja & demonstrasi (20%) |
| Data logging & CSV (IOT-FR-19) | Kesesuaian output (15%) |
