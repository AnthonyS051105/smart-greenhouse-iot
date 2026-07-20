// Firmware ESP32 UTAMA — Smart Greenhouse
//
// Hardware yang TERSEDIA saat ini: DHT11 (pin 18), Servo generik (pin 21).
// Soil Moisture Sensor v1.2, BH1750, dan OLED BELUM terpasang -> status
// ditampilkan via Serial Monitor, dan field soil_moisture/light_intensity
// dikirim sebagai placeholder (-1) agar payload tetap sesuai
// shared/data-contracts.md §1.1.
//
// AI/backend belum terhubung nyata & Mobile app belum terhubung Firebase
// (lihat memory proyek) -- firmware ini hanya menjalankan sisi IoT:
// publish sensor, subscribe command, gerakkan servo, fallback lokal.

#include <Arduino.h>
#include <time.h>
#include "config.h"
#include "PinConfig.h"
#include "SensorManager.h"
#include "ActuatorManager.h"
#include "WifiManager.h"
#include "MqttClient.h"
#include "FallbackController.h"

static const unsigned long READ_INTERVAL_MS = 5UL * 60UL * 1000UL; // 5 menit

SensorManager sensorManager;
ActuatorManager actuatorManager;
WifiManager wifiManager(WIFI_SSID, WIFI_PASSWORD);
MqttClient mqttClient(MQTT_HOST, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD, DEVICE_ID);
FallbackController fallbackController(actuatorManager);

unsigned long lastReadMillis = 0;

void publishStatus(const char *mode) {
  String status = String("{\"device_id\":\"") + DEVICE_ID +
                   "\",\"irrigation_state\":\"" + (actuatorManager.isOpen() ? "open" : "closed") +
                   "\",\"ventilation_state\":\"" + (actuatorManager.isOpen() ? "open" : "closed") +
                   "\",\"wifi_rssi\":" + String(WiFi.RSSI()) +
                   ",\"mode\":\"" + mode + "\"}";
  mqttClient.publishStatus(status);
}

// Callback dipanggil MqttClient saat command valid (belum expired, belum
// pernah diproses) diterima dari topik greenhouse/{device_id}/command.
void onCommandReceived(const CommandPayload &cmd) {
  Serial.print("[Command] actuator=");
  Serial.print(cmd.actuator);
  Serial.print(" action=");
  Serial.print(cmd.action);
  Serial.print(" duration=");
  Serial.println(cmd.durationSeconds);

  // Hanya 1 servo fisik tersedia -> merespons command actuator apapun
  // (irrigation ATAU ventilation) dengan servo yang sama.
  if (cmd.action == "open") {
    actuatorManager.open(cmd.durationSeconds, cmd.actuator.c_str());
  } else if (cmd.action == "close") {
    actuatorManager.close();
  }

  publishStatus(fallbackController.isInFallbackMode() ? "fallback" : "online");
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== Smart Greenhouse - ESP32 Utama ===");

  sensorManager.begin();
  actuatorManager.begin();
  wifiManager.begin();
  mqttClient.onCommand(onCommandReceived);
  mqttClient.begin();

  // NTP diperlukan untuk validasi expires_at command (MqttClient::isExpired).
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
}

void loop() {
  wifiManager.ensureConnected();
  mqttClient.loop();

  bool connected = wifiManager.isConnected() && mqttClient.isConnected();
  fallbackController.updateConnectionState(connected);

  if (millis() - lastReadMillis > READ_INTERVAL_MS || lastReadMillis == 0) {
    SensorData data = sensorManager.readAll();

    if (!data.valid) {
      Serial.println("[Sensor] Gagal membaca DHT11 (NaN), lewati siklus ini.");
    } else {
      Serial.printf("[Sensor] T=%.1fC H=%.1f%% soil=%.1f light=%.1f\n",
                    data.temperature, data.humidity,
                    data.soil_moisture, data.light_intensity);

      if (connected) {
        String payload = sensorManager.toJson(data, DEVICE_ID);
        mqttClient.publishSensor(payload);
        publishStatus("online");
      } else {
        Serial.println("[Mode] Offline -> jalankan fallback lokal.");
        fallbackController.evaluate(data);
      }
    }
    lastReadMillis = millis();
  }

  actuatorManager.update();
}
