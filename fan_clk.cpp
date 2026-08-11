#include "fan_clk.h"
#include "io_control.h"  // Nối với io_control để gọi LED và Còi

// Khai báo định nghĩa biến
int target_hz_clk = 0;   // Tần số mục tiêu (0 đến 450Hz)
int current_hz_clk = 0;  // Tần số hiện tại đang phát

static unsigned long last_update_time = 0;
static unsigned long last_ramp_time = 0;

// Khởi tạo chân xung CLK ban đầu
void setup_fan_clk() {
  ledcAttach(CLK_PIN, 450, 10);  // Giữ nguyên 10-bit độ phân giải cao
  ledcWrite(CLK_PIN, 0);         // Tắt quạt lúc mới bật
}

// Hàm gán tần số mục tiêu (Giới hạn từ 0 đến 450Hz)
void set_fan_hz(int hz) {
  if (hz < 0) hz = 0;
  if (hz > 450) hz = 450;

  // Nếu tần số có sự thay đổi thì kêu Bíp 1 tiếng
  if (target_hz_clk != hz) {
    beep(70);
  }

  target_hz_clk = hz;

  // Cập nhật 3 LED báo tốc độ
  update_led_by_speed(target_hz_clk);

  Serial.printf("[CLK-ONLY] Tần số mục tiêu -> %d Hz\n", target_hz_clk);
}

// Hàm Ramp tăng/giảm tần số mượt mà
void updateFanSpeed() {
  unsigned long now = millis();
  if (now - last_ramp_time < 20) return;  // 50Hz refresh rate

  if (last_update_time == 0) {
    last_update_time = now;
    last_ramp_time = now;
    return;
  }

  last_update_time = now;
  last_ramp_time = now;

  // Tăng/giảm tần số mượt mà
  if (current_hz_clk != target_hz_clk) {
    int step = 4;  // Mỗi 20ms tăng/giảm tối đa 4Hz
    if (current_hz_clk < target_hz_clk) {
      current_hz_clk += step;
      if (current_hz_clk > target_hz_clk) current_hz_clk = target_hz_clk;
    } else {
      current_hz_clk -= step;
      if (current_hz_clk < target_hz_clk) current_hz_clk = target_hz_clk;
    }

    if (current_hz_clk == 0) {
      ledcWrite(CLK_PIN, 0);  // Tắt quạt hoàn toàn
    } else {
      // Tắt báo lỗi rác Serial ở dải cực thấp < 40Hz
      esp_log_level_set("ledc", ESP_LOG_NONE);

      ledcDetach(CLK_PIN);
      delayMicroseconds(10);
      ledcAttach(CLK_PIN, current_hz_clk, 10);  // 10-BIT CHUẨN ĐÉT
      ledcWrite(CLK_PIN, 512);                  // Ghim cứng Duty 50% (512 / 1024)
    }
  }
}