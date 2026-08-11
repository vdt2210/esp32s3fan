#ifndef LED_RGB_H
#define LED_RGB_H

#include <Arduino.h>

#define RGB_PIN 48  // Thay đổi chân GPIO phù hợp với board ESP32-S3

inline void setup_rgb() {
  // Khởi tạo chân LED RGB (Sử dụng tính năng neopixel tích hợp của ESP32-S3)
  pinMode(RGB_PIN, OUTPUT);
  neopixelWrite(RGB_PIN, 0, 0, 0);  // Tắt LED lúc đầu
}

inline void set_rgb_color(uint8_t r, uint8_t g, uint8_t b) {
  neopixelWrite(RGB_PIN, r, g, b);
}

enum LedMode { MODE_STANDBY,
               MODE_BLINK };
inline LedMode current_mode = MODE_STANDBY;
inline uint32_t blink_end_time = 0;
inline uint8_t blink_count_remaining = 0;
inline uint16_t blink_interval = 0;
inline bool blink_led_on = false;
inline uint8_t cur_r = 0, cur_g = 0, cur_b = 0;

inline void update_standby_rgb() {
  static uint32_t last_time = 0;
  static int angle = 0;

  if (millis() - last_time >= 20) {  // Giảm tốc độ thở xuống 20ms cho dịu hơn
    last_time = millis();

    float rad = angle * PI / 180.0;
    float factor = (sin(rad) + 1.0f) / 2.0f;  // factor từ 0.0 -> 1.0

    // Dao động độ sáng R từ 5 -> 30, G từ 1 -> 8 (Tông cam ấm)
    uint8_t r = 5 + (uint8_t)(factor * 25);
    uint8_t g = 1 + (uint8_t)(factor * 7);

    set_rgb_color(r, g, 0);
    angle = (angle + 3) % 360;
  }
}

// Hàm nháy LED báo hiệu trạng thái
inline void blink_rgb(uint8_t r, uint8_t g, uint8_t b, uint16_t duration_ms, uint8_t count = 1) {
  if (count == 0) return;

  cur_r = r;
  cur_g = g;
  cur_b = b;
  blink_interval = duration_ms;
  blink_count_remaining = count * 2;

  set_rgb_color(cur_r, cur_g, cur_b);
  blink_led_on = true;
  blink_end_time = millis() + blink_interval;
  current_mode = MODE_BLINK;
}

enum BlinkState {
  BLINK_SUCCESS,
  BLINK_ERROR,
  BLINK_INFO,
  BLINK_WARNING
};
inline void blink_state(BlinkState state, uint8_t count = 0) {
  switch (state) {
    case BLINK_SUCCESS:
      blink_rgb(0, 80, 0, 150, count ? count : 1);  // Xanh lá nhẹ
      break;
    case BLINK_ERROR:
      blink_rgb(100, 0, 0, 250, count ? count : 3);  // Đỏ nhẹ
      break;
    case BLINK_INFO:
      blink_rgb(0, 0, 100, 200, count ? count : 1);  // Xanh dương
      break;
    case BLINK_WARNING:
      blink_rgb(80, 40, 0, 200, count ? count : 3);  // Cam/Vàng
      break;
  }
}

inline void update_led_system() {
  if (current_mode == MODE_BLINK) {
    if (millis() >= blink_end_time) {
      blink_count_remaining--;

      if (blink_count_remaining > 0) {
        blink_led_on = !blink_led_on;
        if (blink_led_on) {
          set_rgb_color(cur_r, cur_g, cur_b);
        } else {
          set_rgb_color(0, 0, 0);
        }
        blink_end_time = millis() + blink_interval;
        return;
      } else {
        current_mode = MODE_STANDBY;
      }
    } else {
      return;
    }
  }

  if (current_mode == MODE_STANDBY) {
    update_standby_rgb();
  }
}

#endif