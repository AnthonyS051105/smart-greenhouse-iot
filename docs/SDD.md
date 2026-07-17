# SDD — System Design Document (IoT)

> Repo: **iot/**. Referensi: `shared/Architecture.md`, `shared/data-contracts.md`, `iot/SRS.md`.

---

## 1. Gambaran Desain

Subsistem IoT terdiri dari 2 firmware terpisah pada 2 board. Keduanya berkomunikasi dengan backend (via broker MQTT / HTTP) namun tidak saling terhubung langsung.

```
┌─────────────────────────────┐     ┌──────────────────────────┐
│      ESP32 UTAMA            │     │      ESP32-CAM           │
│  ┌──────────────────────┐  │     │  ┌────────────────────┐  │
│  │ SensorManager        │  │     │  │ CameraCapture      │  │
│  │ (DHT11, BME280, MQ)  │  │     │  │ (ambil JPEG)       │  │
│  ├──────────────────────┤  │     │  ├────────────────────┤  │
│  │ ActuatorManager      │  │     │  │ HttpUploader       │  │
│  │ (2×Servo)            │  │     │  │ (POST /images)     │  │
│  ├──────────────────────┤  │     │  └────────────────────┘  │
│  │ MqttClient           │  │     │  ┌────────────────────┐  │
│  │ (pub/sub)            │  │     │  │ WifiManager        │  │
│  ├──────────────────────┤  │     │  └────────────────────┘  │
│  │ DisplayManager (OLED)│  │     └──────────────────────────┘
│  ├──────────────────────┤  │
│  │ FallbackController   │  │
│  ├──────────────────────┤  │
│  │ WifiManager          │  │
│  └──────────────────────┘  │
└─────────────────────────────┘
```

---

## 2. Desain Modul — ESP32 Utama

### 2.1 `WifiManager`
- **Tanggung jawab:** koneksi & reconnect WiFi.
- **Fungsi kunci:** `begin()`, `isConnected()`, `reconnect()`.

### 2.2 `MqttClient`
- **Tanggung jawab:** koneksi broker, publish/subscribe, parsing command JSON.
- **Library:** PubSubClient + ArduinoJson.
- **Fungsi kunci:**
  - `connect()` — koneksi dengan kredensial.
  - `publishSensor(payload)` — ke `greenhouse/{id}/sensor`.
  - `publishStatus(payload)` — ke `greenhouse/{id}/status`.
  - `onCommand(callback)` — subscribe & parse command.
- **Callback command** memvalidasi `expires_at` & `command_id` (idempotensi) sebelum meneruskan ke `ActuatorManager`.

### 2.3 `SensorManager`
- **Tanggung jawab:** baca semua sensor, susun payload.
- **Library:** DHT, Adafruit_BME280.
- **Fungsi kunci:** `readAll() → SensorData`, `toJson(SensorData) → String`.
- **Struktur data:**
  ```cpp
  struct SensorData {
    float temperature;
    float humidity;
    float pressure;
    int gas_level;   // -1 jika tidak ada MQ
    unsigned long timestamp;
  };
  ```

### 2.4 `ActuatorManager`
- **Tanggung jawab:** kendali 2 servo.
- **Library:** ESP32Servo.
- **Fungsi kunci:**
  - `openIrrigation(durationSec)` — putar servo pinch-valve ke posisi buka, jadwalkan tutup.
  - `closeIrrigation()`.
  - `openVentilation()` / `closeVentilation()` — servo louver.
  - `update()` — dipanggil di loop untuk menutup otomatis setelah durasi.
- **Kalibrasi sudut:** 0° = tutup, 90° = buka penuh (disesuaikan saat kalibrasi fisik).

### 2.5 `DisplayManager` (OLED)
- **Tanggung jawab:** render status ke OLED.
- **Library:** Adafruit_SSD1306, Adafruit_GFX.
- **Tampilan:** suhu, kelembapan, status WiFi/MQTT, status 2 aktuator, mode (online/fallback).

### 2.6 `FallbackController`
- **Tanggung jawab:** logika mandiri saat offline.
- **Aturan:**
  - Jika `!mqtt.isConnected()` selama > 60 dtk → `mode = fallback`.
  - Suhu > 32°C → `openVentilation()`.
  - Kelembapan < ambang & tidak ada update → `openIrrigation(short)`.
- **Kembali online:** saat MQTT reconnect, `mode = online`, kirim status.

### 2.7 Loop Utama (pseudocode)
```cpp
void loop() {
  wifi.ensureConnected();
  mqtt.loop();                 // proses pesan masuk
  if (millis() - lastRead > READ_INTERVAL) {
    SensorData d = sensor.readAll();
    display.render(d, state);
    if (mqtt.isConnected()) {
      mqtt.publishSensor(sensor.toJson(d));
      mode = ONLINE;
    } else {
      fallback.evaluate(d);    // jalankan aturan lokal
      mode = FALLBACK;
    }
    lastRead = millis();
  }
  actuator.update();           // tutup servo jika durasi habis
}
```

---

## 3. Desain Modul — ESP32-CAM

### 3.1 `WifiManager`
Sama seperti ESP32 utama.

### 3.2 `CameraCapture`
- **Tanggung jawab:** inisialisasi kamera & ambil frame JPEG.
- **Library:** esp_camera.
- **Fungsi kunci:** `init()`, `capture() → framebuffer`.

### 3.3 `HttpUploader`
- **Tanggung jawab:** kirim JPEG via HTTP POST multipart ke `POST /images`.
- **Library:** HTTPClient.
- **Payload:** `device_id`, `plot_id`, `image` (binary).
- **Interval:** setiap 1–2 jam (timer).

---

## 4. Desain Wiring (Ringkas)

### ESP32 Utama
| Pin ESP32 | Terhubung ke |
|-----------|--------------|
| GPIO (digital) | DHT11 data |
| GPIO21 (SDA), GPIO22 (SCL) | BME280 & OLED (I2C bus bersama) |
| GPIO (analog/ADC) | MQ AO (opsional) |
| GPIO (PWM) | Servo 1 (irigasi) signal |
| GPIO (PWM) | Servo 2 (ventilasi) signal |
| 5V, GND | Catu daya servo (eksternal jika perlu) |

### ESP32-CAM
- Modul kamera sudah terpasang di board; hanya butuh WiFi + catu daya stabil 5V.

> Wiring diagram lengkap dibuat di **Wokwi** (wajib dilampirkan saat submission).

---

## 5. Format Data

Mengacu penuh pada `shared/data-contracts.md`:
- Sensor payload → §1.1
- Command → §1.2
- Status → §1.3
- Citra upload → §2

---

## 6. Penanganan Error

| Kondisi | Penanganan |
|---------|------------|
| WiFi putus | Reconnect otomatis; sementara masuk fallback. |
| MQTT putus | Reconnect; fallback lokal aktif. |
| Sensor gagal baca (NaN) | Lewati publish siklus itu, log ke serial, pakai nilai terakhir untuk fallback. |
| Command tidak valid/kadaluarsa | Diabaikan. |
| Upload citra gagal | Retry beberapa kali; jika tetap gagal, lewati siklus. |

---

## 7. Strategi Pengujian

| Level | Uji |
|-------|-----|
| Unit | Parsing JSON command, kalkulasi kalibrasi servo. |
| Integrasi | Publish sensor → cek diterima backend; kirim command → cek servo bergerak. |
| Sistem | Skenario closed-loop penuh + fallback (putus WiFi saat demo). |
| Fisik | Kalibrasi sudut servo & pinch-valve dengan selang nyata (H-3 sebelum demo). |

---

## 8. Struktur Folder Repo `iot/`

```
iot/
├── esp32-main/
│   ├── esp32-main.ino          # entry point
│   ├── WifiManager.h/.cpp
│   ├── MqttClient.h/.cpp
│   ├── SensorManager.h/.cpp
│   ├── ActuatorManager.h/.cpp
│   ├── DisplayManager.h/.cpp
│   ├── FallbackController.h/.cpp
│   └── config.example.h        # template kredensial (bukan yang asli)
├── esp32-cam/
│   ├── esp32-cam.ino
│   ├── CameraCapture.h/.cpp
│   └── HttpUploader.h/.cpp
├── wokwi/                      # diagram wiring
├── docs/                       # SRS, SDD, Task, CLAUDE, Architecture (symlink/copy shared)
└── README.md
```
