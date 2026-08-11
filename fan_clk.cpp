#include "fan_clk.h"
#include "io_control.h"  // Cần để điều khiển LED báo tốc độ khi set tần số

// Khai báo biến
int target_hz_clk = 0;   // Tần số mục tiêu (0 đến 450Hz)
int current_hz_clk = 0;  // Tần số hiện tại đang phát

static unsigned long last_update_time = 0;

// Khởi tạo chân xung CLK ban đầu
void setup_fan_clk() {
  ledcAttach(CLK_PIN, 450, 10);  // Gửi tín hiệu 10-bit độ phân giải cao
  ledcWrite(CLK_PIN, 0);         // Tắt quạt lúc mới bật
  // Tắt log ledc trong runtime update để tránh overhead
  esp_log_level_set("ledc", ESP_LOG_NONE);
}

// Hàm gán tần số mục tiêu (Giới hạn từ 0 đến 450Hz)
void set_fan_hz(int hz) {
  if (hz < 0) hz = 0;
  if (hz > 450) hz = 450;

  if (target_hz_clk == hz) return;
  target_hz_clk = hz;

  // Cập nhật 3 LED báo tốc độ
  update_led_by_speed(target_hz_clk);
  Serial.printf("[CLK-ONLY] Tần số mục tiêu -> %d Hz\n", target_hz_clk);
}

// Hàm Ramp tăng/giảm tần số một mạch
void updateFanSpeed() {
  unsigned long now = millis();
  if (now - last_update_time < 20) return;  // 50Hz refresh rate
  last_update_time = now;

  // Tăng/giảm tần số mỗi mạch
  if (current_hz_clk != target_hz_clk) {
    int step = 4;  // Mỗi 20ms tăng/giảm tới đây 4Hz
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
      ledcDetach(CLK_PIN);
      delayMicroseconds(10);
      ledcAttach(CLK_PIN, current_hz_clk, 10);  // 10-BIT CHUẨN ĐẶT
      ledcWrite(CLK_PIN, 512);                  // Ghim cứng Duty 50% (512 / 1024)
    }
  }
}
