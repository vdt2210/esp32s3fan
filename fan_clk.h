#ifndef FAN_CLK_H
#define FAN_CLK_H

#include <Arduino.h>

#define CLK_PIN 9  // Chân phát xung kết nối vào đầu IN của bo PC817

// Biến toàn cục để các file khác (Web, IR, Nút bấm) có thể đọc/ghi
extern int target_hz_clk;
extern int current_hz_clk;

// Khai báo các hàm điều khiển
void setup_fan_clk();
void set_fan_hz(int hz);
void updateFanSpeed();

#endif