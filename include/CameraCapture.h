#ifndef CAMERA_CAPTURE_H
#define CAMERA_CAPTURE_H

#include <esp_camera.h>

class CameraCapture {
 public:
  bool begin();

  // Ambil 1 frame JPEG. Caller WAJIB panggil release() setelah selesai pakai.
  camera_fb_t *capture();
  void release(camera_fb_t *fb);
};

#endif // CAMERA_CAPTURE_H
