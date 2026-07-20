#include "CameraCapture.h"
#include "CameraPins.h"
#include <Arduino.h>

bool CameraCapture::begin() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  // XCLK diturunkan ke 10MHz -- 20MHz terbukti menyebabkan crash
  // "Stack canary watchpoint triggered (cam_task)" (Guru Meditation Error)
  // di modul OV2640 board AI-Thinker yang dipakai (clock generator internal
  // sensor tidak stabil di 20MHz), diverifikasi via diagnostik langsung.
  config.xclk_freq_hz = 10000000;
  config.pixel_format = PIXFORMAT_JPEG;
  // CAMERA_GRAB_LATEST mencegah task internal driver kamera mem-block saat
  // buffer penuh -- bagian dari fix crash cam_task yang sama di atas.
  config.grab_mode = CAMERA_GRAB_LATEST;

  // Board AI-Thinker murah sering TIDAK punya PSRAM terpasang -- frame
  // besar (mis. SVGA) + fb_count>1 tanpa PSRAM bisa kehabisan RAM internal
  // dan menyebabkan brownout/reboot loop saat esp_camera_init(). Deteksi
  // otomatis & turunkan resolusi/kualitas jika PSRAM tidak ditemukan.
  bool hasPsram = psramFound();
  Serial.printf("[Camera] PSRAM %s.\n", hasPsram ? "terdeteksi" : "TIDAK terdeteksi");
  if (hasPsram) {
    config.frame_size = FRAMESIZE_SVGA; // 800x600
    config.jpeg_quality = 12;
    config.fb_count = 2;
    config.fb_location = CAMERA_FB_IN_PSRAM;
  } else {
    config.frame_size = FRAMESIZE_QVGA; // 320x240, aman tanpa PSRAM
    config.jpeg_quality = 15;
    config.fb_count = 1;
    config.fb_location = CAMERA_FB_IN_DRAM;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("[Camera] Init gagal, err=0x%x\n", err);
    return false;
  }
  Serial.println("[Camera] Berhasil diinisialisasi.");

  // esp_camera menjalankan cam_task di BACKGROUND sejak esp_camera_init()
  // -- task ini terus mengisi buffer DMA sendiri terlepas dari kapan
  // capture() pertama dipanggil. Jika WiFi mulai SEBELUM buffer pertama
  // "dikosongkan", backlog DMA menumpuk terus & menyumbat CPU -> WiFi gagal
  // connect berulang ("DMA overflow" loop, diverifikasi via diagnostik
  // langsung). Ambil & lepas 1 frame dummy di sini supaya cam_task settle
  // dulu sebelum caller melanjutkan ke inisialisasi WiFi.
  camera_fb_t *warmupFb = esp_camera_fb_get();
  if (warmupFb) {
    esp_camera_fb_return(warmupFb);
  }
  delay(200);

  return true;
}

camera_fb_t *CameraCapture::capture() {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("[Camera] Gagal mengambil frame.");
  }
  return fb;
}

void CameraCapture::release(camera_fb_t *fb) {
  if (fb) {
    esp_camera_fb_return(fb);
  }
}
