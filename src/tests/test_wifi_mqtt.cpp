// Test debugging koneksi WiFi + MQTT (HiveMQ Cloud) secara mandiri.
// Jalankan: pio run -e test-wifi-mqtt -t upload -t monitor
//
// Ekspektasi: "[WiFi] Terhubung" lalu "[MQTT] Terhubung", kemudian tiap
// 5 detik publish payload dummy ke greenhouse/{DEVICE_ID}/sensor.
// Verifikasi penerimaan dengan MQTT Explorer / mosquitto_sub, subscribe
// ke "greenhouse/#".
//
// Jika WiFi gagal -> cek SSID/password di config.h.
// Jika MQTT gagal (rc != 0) -> cek host/port/username/password HiveMQ Cloud
// & pastikan port 8883 (TLS) tidak diblokir jaringan.

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "config.h"

WiFiClientSecure secureClient;
PubSubClient mqtt(secureClient);

void connectWifi() {
  Serial.print("[WiFi] Menghubungkan ke ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[WiFi] Terhubung. IP: " + WiFi.localIP().toString());
}

void connectMqtt() {
  secureClient.setInsecure();
  mqtt.setServer(MQTT_HOST, MQTT_PORT);

  String clientId = String(DEVICE_ID) + "-test-" + String(random(0xffff), HEX);
  Serial.print("[MQTT] Menghubungkan sebagai ");
  Serial.println(clientId);

  if (mqtt.connect(clientId.c_str(), MQTT_USERNAME, MQTT_PASSWORD)) {
    Serial.println("[MQTT] Terhubung.");
  } else {
    Serial.print("[MQTT] Gagal, rc=");
    Serial.println(mqtt.state());
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("=== TEST WIFI + MQTT (HiveMQ Cloud) ===");
  connectWifi();
  connectMqtt();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWifi();
  }
  if (!mqtt.connected()) {
    connectMqtt();
  }
  mqtt.loop();

  static unsigned long lastPublish = 0;
  if (millis() - lastPublish > 5000) {
    String topic = String("greenhouse/") + DEVICE_ID + "/sensor";
    String payload = "{\"device_id\":\"" + String(DEVICE_ID) +
                      "\",\"plot_id\":\"" + String(PLOT_ID) +
                      "\",\"timestamp\":\"1970-01-01T00:00:00Z\"," +
                      "\"temperature\":25.0,\"humidity\":60.0," +
                      "\"soil_moisture\":45.0,\"light_intensity\":850.0}";
    bool ok = mqtt.publish(topic.c_str(), payload.c_str());
    Serial.printf("[MQTT] Publish dummy ke %s -> %s\n", topic.c_str(), ok ? "OK" : "GAGAL");
    lastPublish = millis();
  }
}
