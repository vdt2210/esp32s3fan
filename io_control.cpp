#include "io_control.h"
#include "fan_clk.h" // Cần bao hàm file này để điều khiển tần số quạt

// Khai báo biến trạng thái
bool swing_state = false;
bool fan_power_state = false;

// Trạng thái các nút bấm (để chống dội / debounce)
static bool btn1_prev = HIGH;
static bool btn2_prev = HIGH;
static bool btn3_prev = HIGH;

void setup_io() {
    // Cấu hình Nút bấm (Active Low - kéo lên nội bộ)
    pinMode(PIN_BTN_1, INPUT_PULLUP);
    pinMode(PIN_BTN_2, INPUT_PULLUP);
    pinMode(PIN_BTN_3, INPUT_PULLUP);

    // Cấu hình LED báo trạng thái
    pinMode(PIN_LED_1, OUTPUT);
    pinMode(PIN_LED_2, OUTPUT);
    pinMode(PIN_LED_3, OUTPUT);
    pinMode(PIN_LED_4, OUTPUT);

    // Tắt tất cả LED ban đầu
    digitalWrite(PIN_LED_1, LOW);
    digitalWrite(PIN_LED_2, LOW);
    digitalWrite(PIN_LED_3, LOW);
    digitalWrite(PIN_LED_4, LOW);

    // Cấu hình Chân xuất tín hiệu điều khiển
    pinMode(PIN_SWING, OUTPUT);
    pinMode(PIN_FAN_POWER, OUTPUT);
    pinMode(PIN_BUZZER, OUTPUT);

    // Mặc định ban đầu tắt ngắt dương 24V và đảo gió
    digitalWrite(PIN_SWING, LOW);
    digitalWrite(PIN_FAN_POWER, LOW);
    digitalWrite(PIN_BUZZER, LOW);

    swing_state = false;
    fan_power_state = false;
}

// -------------------------------------------------------------
// HÀM BẬT QUẠT VỚI TRÌNH TỰ AN TOÀN (OFF -> ON)
// -------------------------------------------------------------
void turn_fan_on(int target_hz) {
    if (!fan_power_state) {
        digitalWrite(PIN_FAN_POWER, HIGH);
        fan_power_state = true;
        delay(100); // Chờ nguồn 24V ổn định
    }

    set_fan_hz(target_hz);
    update_led_by_speed(target_hz);
}

// -------------------------------------------------------------
// HÀM TẮT QUẠT MỀM MẠI (ON -> OFF)
// -------------------------------------------------------------
void turn_fan_off() {
    if (!fan_power_state) return;

    // 1. Tắt đảo gió trước
    set_swing(false);

    // 2. Lấy tần số hiện tại trực tiếp từ biến target_hz_clk
    int current_hz = target_hz_clk;

    // 3. Hạ xung mềm mại về 0Hz
    if (current_hz > 0) {
        int steps = 10;
        int step_hz = current_hz / steps;
        int delay_per_step = 300 / steps;

        for (int i = steps - 1; i >= 0; i--) {
            set_fan_hz(i * step_hz);
            delay(delay_per_step);
        }
    }

    // 4. Đưa tần số về 0Hz hẳn
    set_fan_hz(0);

    // 5. Ngắt nguồn 24V
    delay(50);
    digitalWrite(PIN_FAN_POWER, LOW);
    fan_power_state = false;

    update_led_by_speed(0);
}

// -------------------------------------------------------------
// ĐIỀU KHIỂN NGUỒN 24V ĐỘNG CƠ CHÍNH
// -------------------------------------------------------------
void set_fan_power(bool enable) {
    if (enable) {
        // Nếu bật nguồn mà chưa có tốc độ, mặc định chạy ở Tốc độ 1 (ví dụ 100Hz)
        turn_fan_on(100); 
    } else {
        turn_fan_off();
    }
}

void toggle_fan_power() {
    set_fan_power(!fan_power_state);
}

// -------------------------------------------------------------
// ĐIỀU KHIỂN ĐẢO GIÓ (SWING) - CÓ KHÓA THEO NGUỒN QUẠT
// -------------------------------------------------------------
void set_swing(bool enable) {
    // Nếu quạt đang TẮT mà cố tình BẬT đảo gió -> Bỏ qua ngay
    if (enable && !fan_power_state) {
        return; 
    }
    
    swing_state = enable;
    digitalWrite(PIN_SWING, swing_state ? HIGH : LOW);
    digitalWrite(PIN_LED_4, swing_state ? HIGH : LOW);
}

void toggle_swing() {
    // Chỉ cho phép đảo trạng thái khi quạt đang BẬT
    if (fan_power_state) {
        set_swing(!swing_state);
    }
}

// -------------------------------------------------------------
// CẬP NHẬT LED BÁO TỐC ĐỘ QUẠT
// -------------------------------------------------------------
void update_led_by_speed(int hz) {
    // Ví dụ các mức tần số tương ứng với Tốc độ 1, 2, 3
    digitalWrite(PIN_LED_1, (hz > 0 && hz <= 100) ? HIGH : LOW);
    digitalWrite(PIN_LED_2, (hz > 100 && hz <= 200) ? HIGH : LOW);
    digitalWrite(PIN_LED_3, (hz > 200) ? HIGH : LOW);
}

// -------------------------------------------------------------
// PHÁT TIẾNG CÒI BÍP (BUZZER)
// -------------------------------------------------------------
void beep(int duration_ms) {
    digitalWrite(PIN_BUZZER, HIGH);
    delay(duration_ms);
    digitalWrite(PIN_BUZZER, LOW);
}

// -------------------------------------------------------------
// QUẢN LÝ NÚT BẤM (BUTTONS)
// -------------------------------------------------------------
void check_buttons() {
    bool btn1 = digitalRead(PIN_BTN_1); // ON / SPEED
    bool btn2 = digitalRead(PIN_BTN_2); // SWING
    bool btn3 = digitalRead(PIN_BTN_3); // OFF

    // Nút 1: ON / SPEED
    if (btn1 == LOW && btn1_prev == HIGH) {
        beep(50);
        if (!fan_power_state) {
            turn_fan_on(100);
        } else {
            // Đọc trực tiếp từ biến target_hz_clk
            int current_hz = target_hz_clk; 
            int next_hz = (current_hz >= 300) ? 100 : (current_hz + 100);
            set_fan_hz(next_hz);
            update_led_by_speed(next_hz);
        }
        delay(50);
    }
    btn1_prev = btn1;

    // Nút 2: SWING
    if (btn2 == LOW && btn2_prev == HIGH) {
        beep(50);
        toggle_swing();
        delay(50);
    }
    btn2_prev = btn2;

    // Nút 3: OFF
    if (btn3 == LOW && btn3_prev == HIGH) {
        beep(50);
        turn_fan_off();
        delay(50);
    }
    btn3_prev = btn3;
}