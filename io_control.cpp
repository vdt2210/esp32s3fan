#include "io_control.h"
#include "fan_clk.h" // Cần để điều khiển tần số quạt

// Khai báo biến trạng thái
bool swing_state = false;
bool fan_power_state = false;

// Trạng thái các nút bấm (để chống dội / debounce cơ bản)
static bool btn1_prev = HIGH;
static bool btn2_prev = HIGH;
static bool btn3_prev = HIGH;
static unsigned long btn_guard_until = 0;

// Buzzer không chặn (tắt pin sau thời điểm đã đặt)
static bool beep_active = false;
static unsigned long beep_end_time = 0;

// Tắt quạt theo dạng async: đợi tần số về 0 rồi cắt nguồn
static bool fan_power_down_pending = false;

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

    // Mặc định ban đầu tắt quạt, tắt dừng
    digitalWrite(PIN_SWING, LOW);
    digitalWrite(PIN_FAN_POWER, LOW);
    digitalWrite(PIN_BUZZER, LOW);

    swing_state = false;
    fan_power_state = false;
}

// -------------------------------------------------------------
// BẬT QUẠT VÀO TRẠNG THÁI AN TOÀN (OFF -> ON)
// -------------------------------------------------------------
void turn_fan_on(int target_hz) {
    if (fan_power_down_pending) {
        fan_power_down_pending = false;
    }

    if (!fan_power_state) {
        digitalWrite(PIN_FAN_POWER, HIGH);
        fan_power_state = true;
    }

    set_fan_hz(target_hz);
    update_led_by_speed(target_hz);
}

// -------------------------------------------------------------
// TẮT QUẠT MẠNH MẠI (ON -> OFF)
// -------------------------------------------------------------
void turn_fan_off() {
    if (!fan_power_state || fan_power_down_pending) return;

    // 1. Tắt dậy gió trước
    set_swing(false);

    // 2. Bắt đầu chuỗi dừng: hạ về 0 rồi tắt nguồn sau khi dừng rung
    fan_power_down_pending = true;
    set_fan_hz(0);
}

// Hàm đồng bộ hóa giai đoạn tắt quạt (gọi mỗi vòng loop)
static void process_fan_power_down() {
    if (!fan_power_down_pending) return;

    if (current_hz_clk == 0 && target_hz_clk == 0) {
        delayMicroseconds(50);
        digitalWrite(PIN_FAN_POWER, LOW);
        fan_power_state = false;
        fan_power_down_pending = false;
        update_led_by_speed(0);
    }
}

// -------------------------------------------------------------
// ĐIỀU KHIỂN NGUỒN 24V ĐỒNG CẤP
// -------------------------------------------------------------
void set_fan_power(bool enable) {
    if (enable) {
        // Nếu bật nguồn mà chưa có tốc độ, mặc định chạy ở 100Hz
        turn_fan_on(100); 
    } else {
        turn_fan_off();
    }
}

void toggle_fan_power() {
    set_fan_power(!fan_power_state);
}

// -------------------------------------------------------------
// ĐIỀU KHIỂN DẠO GIÓ (SWING) - KHÔNG THỂ BẬT KHI QUẠT TẮT
// -------------------------------------------------------------
void set_swing(bool enable) {
    // Nếu quạt đang TẮT mà muốn BẬT dậy gió -> Bỏ qua ngay
    if (enable && !fan_power_state) {
        return; 
    }
    
    swing_state = enable;
    digitalWrite(PIN_SWING, swing_state ? HIGH : LOW);
    digitalWrite(PIN_LED_4, swing_state ? HIGH : LOW);
}

void toggle_swing() {
    // Chỉ cho phép đổi trạng thái khi quạt đang BẬT
    if (fan_power_state) {
        set_swing(!swing_state);
    }
}

// -------------------------------------------------------------
// CẬP NHẬT LED BÁO TỐC ĐỘ QUẠT
// -------------------------------------------------------------
void update_led_by_speed(int hz) {
    // Ví dụ mức tần số tương ứng với Tốc Độ 1, 2, 3
    digitalWrite(PIN_LED_1, (hz > 0 && hz <= 100) ? HIGH : LOW);
    digitalWrite(PIN_LED_2, (hz > 100 && hz <= 200) ? HIGH : LOW);
    digitalWrite(PIN_LED_3, (hz > 200) ? HIGH : LOW);
}

// -------------------------------------------------------------
// PHÁT TIẾNG BUZZER (BUZZER)
// -------------------------------------------------------------
void beep(int duration_ms) {
    if (duration_ms <= 0) {
        digitalWrite(PIN_BUZZER, LOW);
        beep_active = false;
        return;
    }

    digitalWrite(PIN_BUZZER, HIGH);
    beep_active = true;
    beep_end_time = millis() + (unsigned long)duration_ms;
}

static void update_beep() {
    if (!beep_active) return;
    if ((long)(millis() - beep_end_time) >= 0) {
        digitalWrite(PIN_BUZZER, LOW);
        beep_active = false;
    }
}

// -------------------------------------------------------------
// QUẢN LÝ NÚT BẤM (BUTTONS)
// -------------------------------------------------------------
void check_buttons() {
    unsigned long now = millis();
    if (now < btn_guard_until) return;

    bool btn1 = digitalRead(PIN_BTN_1); // ON / SPEED
    bool btn2 = digitalRead(PIN_BTN_2); // SWING
    bool btn3 = digitalRead(PIN_BTN_3); // OFF

    // Nút 1: ON / SPEED
    if (btn1 == LOW && btn1_prev == HIGH) {
        beep(50);
        if (!fan_power_state) {
            turn_fan_on(100);
        } else {
            int current_hz = target_hz_clk; 
            int next_hz = (current_hz >= 300) ? 100 : (current_hz + 100);
            set_fan_hz(next_hz);
            update_led_by_speed(next_hz);
        }
        btn_guard_until = now + 70;
    }
    btn1_prev = btn1;

    // Nút 2: SWING
    if (btn2 == LOW && btn2_prev == HIGH) {
        beep(50);
        toggle_swing();
        btn_guard_until = now + 70;
    }
    btn2_prev = btn2;

    // Nút 3: OFF
    if (btn3 == LOW && btn3_prev == HIGH) {
        beep(50);
        turn_fan_off();
        btn_guard_until = now + 70;
    }
    btn3_prev = btn3;
}

// -------------------------------------------------------------
// Cập nhật nhiệm vụ nền của IO (buzzer + power down)
// -------------------------------------------------------------
void update_io_tasks() {
    update_beep();
    process_fan_power_down();
}
