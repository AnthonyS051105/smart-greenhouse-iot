#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>

class WifiManager {
 public:
  WifiManager(const char *ssid, const char *password);

  void begin();
  bool isConnected();

  // Coba reconnect non-blocking (dipanggil tiap loop()).
  void ensureConnected();

 private:
  const char *ssid_;
  const char *password_;
  unsigned long lastAttemptMillis_;
  static const unsigned long RETRY_INTERVAL_MS = 5000;
};

#endif // WIFI_MANAGER_H
