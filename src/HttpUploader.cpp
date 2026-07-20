#include "HttpUploader.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

HttpUploader::HttpUploader(const char *backendUrl, const char *deviceId, const char *plotId)
    : backendUrl_(backendUrl), deviceId_(deviceId), plotId_(plotId) {}

bool HttpUploader::upload(camera_fb_t *fb, int maxRetries) {
  if (!fb) return false;

  const String boundary = "GreenhouseBoundary";

  String bodyStart;
  bodyStart += "--" + boundary + "\r\n";
  bodyStart += "Content-Disposition: form-data; name=\"device_id\"\r\n\r\n";
  bodyStart += String(deviceId_) + "\r\n";
  bodyStart += "--" + boundary + "\r\n";
  bodyStart += "Content-Disposition: form-data; name=\"plot_id\"\r\n\r\n";
  bodyStart += String(plotId_) + "\r\n";
  bodyStart += "--" + boundary + "\r\n";
  bodyStart += "Content-Disposition: form-data; name=\"image\"; filename=\"crop.jpg\"\r\n";
  bodyStart += "Content-Type: image/jpeg\r\n\r\n";

  String bodyEnd = "\r\n--" + boundary + "--\r\n";

  for (int attempt = 1; attempt <= maxRetries; attempt++) {
    WiFiClientSecure client;
    client.setInsecure(); // Demo bootcamp; ganti dgn root CA untuk produksi.

    HTTPClient http;
    if (!http.begin(client, backendUrl_)) {
      Serial.println("[HttpUploader] Gagal begin() koneksi HTTP.");
      continue;
    }
    http.addHeader("Content-Type", "multipart/form-data; boundary=" + boundary);

    size_t totalLen = bodyStart.length() + fb->len + bodyEnd.length();
    uint8_t *buffer = (uint8_t *)malloc(totalLen);
    if (!buffer) {
      Serial.println("[HttpUploader] Gagal alokasi buffer upload.");
      http.end();
      return false;
    }

    size_t offset = 0;
    memcpy(buffer + offset, bodyStart.c_str(), bodyStart.length());
    offset += bodyStart.length();
    memcpy(buffer + offset, fb->buf, fb->len);
    offset += fb->len;
    memcpy(buffer + offset, bodyEnd.c_str(), bodyEnd.length());

    int httpCode = http.POST(buffer, totalLen);
    free(buffer);

    if (httpCode == 200 || httpCode == 201) {
      Serial.printf("[HttpUploader] Upload berhasil (percobaan %d), kode=%d\n", attempt, httpCode);
      Serial.println(http.getString());
      http.end();
      return true;
    }

    Serial.printf("[HttpUploader] Upload gagal (percobaan %d/%d), kode=%d\n",
                  attempt, maxRetries, httpCode);
    http.end();
    delay(1000);
  }

  Serial.println("[HttpUploader] Upload gagal setelah semua percobaan, lewati siklus ini.");
  return false;
}
