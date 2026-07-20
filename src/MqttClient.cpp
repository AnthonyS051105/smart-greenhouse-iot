#include "MqttClient.h"
#include <ArduinoJson.h>
#include <time.h>

MqttClient *MqttClient::instance_ = nullptr;

MqttClient::MqttClient(const char *host, int port, const char *username,
                        const char *password, const char *deviceId)
    : client_(secureClient_),
      host_(host),
      port_(port),
      username_(username),
      password_(password),
      deviceId_(deviceId),
      commandCallback_(nullptr),
      lastReconnectAttempt_(0) {
  instance_ = this;
  topicSensor_ = String("greenhouse/") + deviceId_ + "/sensor";
  topicCommand_ = String("greenhouse/") + deviceId_ + "/command";
  topicStatus_ = String("greenhouse/") + deviceId_ + "/status";
}

void MqttClient::begin() {
  // NOTE: Untuk skala demo bootcamp, verifikasi sertifikat TLS broker
  // dilonggarkan (insecure). Untuk produksi, pasang root CA HiveMQ yang benar.
  secureClient_.setInsecure();
  client_.setServer(host_, port_);
  client_.setCallback(staticMessageCallback);
}

bool MqttClient::isConnected() {
  return client_.connected();
}

void MqttClient::reconnect() {
  unsigned long now = millis();
  if (now - lastReconnectAttempt_ < 5000) {
    return;
  }
  lastReconnectAttempt_ = now;

  Serial.print("[MQTT] Menghubungkan ke broker sebagai ");
  Serial.println(deviceId_);

  String clientId = String(deviceId_) + "-" + String(random(0xffff), HEX);
  if (client_.connect(clientId.c_str(), username_, password_)) {
    Serial.println("[MQTT] Terhubung.");
    client_.subscribe(topicCommand_.c_str());
    Serial.print("[MQTT] Subscribe: ");
    Serial.println(topicCommand_);
  } else {
    Serial.print("[MQTT] Gagal, rc=");
    Serial.println(client_.state());
  }
}

void MqttClient::loop() {
  if (!isConnected()) {
    reconnect();
    return;
  }
  client_.loop();
}

void MqttClient::publishSensor(const String &jsonPayload) {
  if (!isConnected()) return;
  client_.publish(topicSensor_.c_str(), jsonPayload.c_str());
  Serial.print("[MQTT] Publish sensor: ");
  Serial.println(jsonPayload);
}

void MqttClient::publishStatus(const String &jsonPayload) {
  if (!isConnected()) return;
  client_.publish(topicStatus_.c_str(), jsonPayload.c_str());
  Serial.print("[MQTT] Publish status: ");
  Serial.println(jsonPayload);
}

void MqttClient::onCommand(CommandCallback callback) {
  commandCallback_ = callback;
}

void MqttClient::staticMessageCallback(char *topic, byte *payload, unsigned int length) {
  if (instance_) {
    instance_->handleMessage(topic, payload, length);
  }
}

// Bandingkan expiresAt (ISO 8601 UTC) terhadap waktu saat ini (NTP).
// Mengasumsikan configTime() sudah dipanggil di setup() firmware utama.
bool MqttClient::isExpired(const String &expiresAtIso) {
  struct tm expiresTm = {};
  if (!strptime(expiresAtIso.c_str(), "%Y-%m-%dT%H:%M:%SZ", &expiresTm)) {
    // Tidak bisa parse -> anggap kadaluarsa demi keamanan (abaikan command).
    return true;
  }
  // ESP32 Arduino core tidak menyediakan timegm(); mktime() dipakai sebagai
  // gantinya dan valid di sini karena configTime(0, 0, ...) di setup()
  // mengeset timezone board ke UTC (offset 0), jadi mktime() == UTC epoch.
  time_t expiresEpoch = mktime(&expiresTm);

  time_t now;
  time(&now);

  return now > expiresEpoch;
}

void MqttClient::handleMessage(char *topic, byte *payload, unsigned int length) {
  String topicStr(topic);
  if (topicStr != topicCommand_) {
    return;
  }

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, payload, length);
  if (err) {
    Serial.print("[MQTT] Command JSON invalid: ");
    Serial.println(err.c_str());
    return;
  }

  CommandPayload cmd;
  cmd.commandId = doc["command_id"].as<String>();
  cmd.actuator = doc["actuator"].as<String>();
  cmd.action = doc["action"].as<String>();
  cmd.durationSeconds = doc["duration_seconds"] | 0;
  cmd.expiresAt = doc["expires_at"].as<String>();
  cmd.source = doc["source"].as<String>();

  // Idempotensi (IOT-FR-13): abaikan command_id yang sudah pernah diproses.
  if (cmd.commandId.length() > 0 && cmd.commandId == lastCommandId_) {
    Serial.print("[MQTT] Command duplikat diabaikan: ");
    Serial.println(cmd.commandId);
    return;
  }

  // Validasi expires_at (IOT-FR-12).
  if (cmd.expiresAt.length() > 0 && isExpired(cmd.expiresAt)) {
    Serial.print("[MQTT] Command kadaluarsa diabaikan: ");
    Serial.println(cmd.commandId);
    return;
  }

  lastCommandId_ = cmd.commandId;

  if (commandCallback_) {
    commandCallback_(cmd);
  }
}
