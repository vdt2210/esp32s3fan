#ifndef LED_RGB_H
#define LED_RGB_H

#include <Arduino.h>

#define RGB_PIN  48  // Thay đổi chân GPIO phù hợp với board ESP32-S3 của ní

inline void setup_rgb() {
    // Khởi tạo chân LED RGB (Sử dụng tính năng neopixel tích hợp của ESP32-S3)
    pinMode(RGB_PIN, OUTPUT);
    neopixelWrite(RGB_PIN, 0, 0, 0); // Tắt LED lúc đầu
}

inline void set_rgb_color(uint8_t r, uint8_t g, uint8_t b) {
    neopixelWrite(RGB_PIN, r, g, b);
}

// Hàm nháy LED báo hiệu trạng thái
inline void blink_rgb(uint8_t r, uint8_t g, uint8_t b, int delay_ms) {
    set_rgb_color(r, g, b);
    delay(delay_ms);
    set_rgb_color(0, 0, 0);
}

#endif