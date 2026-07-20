#ifndef CONFIG_H
#define CONFIG_H

// Salin file ini menjadi "config.h" (sudah di-.gitignore) lalu isi nilai asli.
// JANGAN commit config.h ke repo.

// ---------- WiFi ----------
#define WIFI_SSID       "NAMA_WIFI_ANDA"
#define WIFI_PASSWORD   "PASSWORD_WIFI_ANDA"

// ---------- MQTT (HiveMQ Cloud) ----------
// Host didapat dari dashboard HiveMQ Cloud, format: xxxxxxxx.s1.eu.hivemq.cloud
#define MQTT_HOST       "xxxxxxxx.s1.eu.hivemq.cloud"
#define MQTT_PORT       8883
#define MQTT_USERNAME   "MQTT_USERNAME_ANDA"
#define MQTT_PASSWORD   "MQTT_PASSWORD_ANDA"

// ---------- Identitas Device ----------
// Sesuai shared/data-contracts.md: topik = greenhouse/{device_id}/...
#define DEVICE_ID       "gh-esp32-01"
#define PLOT_ID         "plot-abc123"

// ESP32-CAM memakai device_id terpisah (lihat Architecture.md ADR-05)
#define CAM_DEVICE_ID   "gh-esp32cam-01"

#endif // CONFIG_H
