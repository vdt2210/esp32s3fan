#ifndef IO_CONTROL_H
#define IO_CONTROL_H

#include <Arduino.h>

// --- ĐỊNH NGHĨA DÀN CHÂN QUY HOẠCH ---
#define PIN_BTN_1     4   // Nút ON/SPEED (Active Low)
#define PIN_BTN_2     5   // Nút SWING (Active Low)
#define PIN_BTN_3     6   // Nút OFF (Active Low)

#define PIN_LED_1     10  // LED 1 báo tốc độ Nhỏ
#define PIN_LED_2     11  // LED 2 báo tốc độ Vừa
#define PIN_LED_3     12  // LED 3 báo tốc độ Lớn
#define PIN_LED_4     13  // LED 4 báo trạng thái Đảo gió

#define PIN_IR_RECV   14  // Mắt thu IR (VS1838B)
#define PIN_SWING     15  // Đảo gió (Bo F5305S ngắt Dương)
#define PIN_FAN_POWER 16  // Ngắt nguồn 24V Động cơ chính (Bo F5305S ngắt Dương)
#define PIN_BUZZER    19  // Còi thụ động (Buzzer + Trở 100 Ohm)

// Biến trạng thái Đảo gió
extern bool swing_state;
extern bool fan_power_state; // <-- Biến lưu trạng thái Bật/Tắt nguồn 24V động cơ chính

// Khai báo các hàm
void setup_io();
void turn_fan_on(int target_hz);
void turn_fan_off();
void set_swing(bool enable);
void toggle_swing();

void set_fan_power(bool enable); // <-- Hàm đóng/ngắt nguồn 24V chính
void toggle_fan_power();         // <-- Hàm đảo trạng thái nguồn chính

void check_buttons();
void update_led_by_speed(int hz);
void update_io_tasks();

// Khai báo hàm phát tiếng còi
void beep(int duration_ms = 80);

#endif
