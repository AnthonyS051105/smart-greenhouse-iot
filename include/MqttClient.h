#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

// Payload command hasil parsing, diteruskan ke callback pemanggil.
struct CommandPayload {
  String commandId;
  String actuator;   // "irrigation" | "ventilation"
  String action;      // "open" | "close"
  int durationSeconds;
  String expiresAt;   // ISO 8601
  String source;       // "ai_auto" | "manual"
};

typedef void (*CommandCallback)(const CommandPayload &cmd);

class MqttClient {
 public:
  MqttClient(const char *host, int port, const char *username,
             const char *password, const char *deviceId);

  void begin();

  // Harus dipanggil tiap loop() untuk memproses pesan masuk & menjaga koneksi.
  void loop();

  bool isConnected();
  void reconnect();

  void publishSensor(const String &jsonPayload);
  void publishStatus(const String &jsonPayload);

  void onCommand(CommandCallback callback);

 private:
  WiFiClientSecure secureClient_;
  PubSubClient client_;
  const char *host_;
  int port_;
  const char *username_;
  const char *password_;
  const char *deviceId_;
  CommandCallback commandCallback_;
  unsigned long lastReconnectAttempt_;

  String topicSensor_;
  String topicCommand_;
  String topicStatus_;

  // Idempotensi: simpan command_id terakhir yang sudah dieksekusi
  // (shared/data-contracts.md §1.2, IOT-FR-13).
  String lastCommandId_;

  static MqttClient *instance_;
  static void staticMessageCallback(char *topic, byte *payload, unsigned int length);
  void handleMessage(char *topic, byte *payload, unsigned int length);
  bool isExpired(const String &expiresAtIso);
};

#endif // MQTT_CLIENT_H
