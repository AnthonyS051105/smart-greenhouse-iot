#ifndef HTTP_UPLOADER_H
#define HTTP_UPLOADER_H

#include <Arduino.h>
#include <esp_camera.h>

class HttpUploader {
 public:
  // backendUrl: endpoint lengkap, mis. "https://xxx.up.railway.app/images".
  // Backend belum live saat ini -> upload akan gagal & di-retry, ini normal
  // (lihat memory proyek: AI/backend belum terhubung nyata).
  HttpUploader(const char *backendUrl, const char *deviceId, const char *plotId);

  // Upload 1 frame JPEG via HTTP POST multipart sesuai
  // shared/data-contracts.md §2. Retry hingga maxRetries kali jika gagal.
  bool upload(camera_fb_t *fb, int maxRetries = 3);

 private:
  const char *backendUrl_;
  const char *deviceId_;
  const char *plotId_;
};

#endif // HTTP_UPLOADER_H
