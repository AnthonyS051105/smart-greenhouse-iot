#include "WifiManager.h"
#include <WiFi.h>

WifiManager::WifiManager(const char *ssid, const char *password)
    : ssid_(ssid), password_(password), lastAttemptMillis_(0) {}

void WifiManager::begin() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid_, password_);
  Serial.print("[WiFi] Menghubungkan ke ");
  Serial.println(ssid_);
}

bool WifiManager::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}

void WifiManager::ensureConnected() {
  if (isConnected()) {
    return;
  }
  unsigned long now = millis();
  if (now - lastAttemptMillis_ < RETRY_INTERVAL_MS) {
    return;
  }
  lastAttemptMillis_ = now;
  Serial.println("[WiFi] Terputus, mencoba reconnect...");
  WiFi.disconnect();
  WiFi.begin(ssid_, password_);
}
