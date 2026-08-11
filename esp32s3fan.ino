#include <WebServer.h>
// Đã bỏ web_page.h cũ, thay bằng 3 tab chia nhỏ giao diện mới:
#include "web_basic.h"    // Tab giao diện Cơ bản (basic.html)
#include "web_advance.h"  // Tab giao diện Cho Hoàng (advance.html)
#include "web_script.h"   // Tab chứa Javascript chung (main.js)

#include "wifi_config.h"
#include "led_rgb.h"
#include "fan_clk.h"       // Tab điều tốc quạt (GPIO 9)
#include "io_control.h"    // Tab dàn chân mới (GPIO 4, 5, 6, 10..15)
#include "ir_processor.h"  // <--- DÒNG NÀY ĐỂ MÁY BIẾT HÀM setup_ir() VÀ loop_ir()
#include "web_ir_learn.h"  // <--- DÒNG NÀY ĐỂ MÁY BIẾT TRANG WEB HỌC IR
#include "web_ir_test.h"   // <--- Thêm dòng này nếu chưa có
WebServer server(80);

// 1. Trả về trang Cơ bản (Cho vợ)
void handleRoot() {
  server.send(200, "text/html", HTML_BASIC);
}

// 2. Trả về trang Nâng cao (Cho Hoàng)
void handleAdvance() {
  server.send(200, "text/html", HTML_ADVANCE);
}

// 3. Trả về file Script JS dùng chung cho cả 2 trang
void handleScript() {
  server.send(200, "application/javascript", JS_MAIN);
}

void handleStatus() {
  // Trả về cả tần số quạt và trạng thái đảo gió lên Web
  String json = "{\"clk_hz\":" + String(target_hz_clk) + ",\"swing\":" + String(swing_state ? 1 : 0) + "}";
  server.send(200, "application/json", json);
}

void handleSetRealClk() {
  if (server.hasArg("hz")) {
    int real_hz = server.arg("hz").toInt();
    set_fan_hz(real_hz);

    blink_rgb(0, 255, 255, 100);
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Thiếu tham số hz!");
  }
}

// API điều khiển Đảo gió từ Web
void handleSetSwing() {
  if (server.hasArg("state")) {
    bool state = server.arg("state") == "1";
    set_swing(state);
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Thiếu tham số state!");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  setup_rgb();
  blink_rgb(255, 0, 0, 300);

  setup_fan_clk();  // Khởi tạo xung quạt (GPIO 9)
  setup_io();       // Khởi tạo toàn bộ dàn chân GPIO mới (4,5,6, 10,11,12,13, 15)
  setup_ir();       // <--- THÊM DÒNG NÀY VÀO ĐÂY
  setup_wifi_ota();

  if (WiFi.status() == WL_CONNECTED) {
    set_rgb_color(0, 255, 0);
  }

  // Map các Route Web Server
  server.on("/", handleRoot);            // Trang chính (Cơ bản)
  server.on("/advance", handleAdvance);  // Trang Nâng cao (Cho Hoàng)
  server.on("/main.js", handleScript);   // File JS chung

  // --- THÊM CHỖ NÀY (TRƯỚC DÒNG SERVER.BEGIN) ---
  server.on("/ir", handle_ir_learn_page);

  server.on("/get_ir_data", []() {
    String json = "{";
    json += "\"received\":" + String(has_new_ir ? "true" : "false") + ",";
    json += "\"protocol\":\"" + last_ir_protocol + "\",";
    json += "\"hex\":\"" + last_ir_hex + "\",";
    json += "\"bits\":" + String(last_ir_bits) + ",";
    json += "\"learn_msg\":\"" + learn_result_msg + "\"";
    json += "}";

    has_new_ir = false;
    server.send(200, "application/json", json);
  });

  server.on("/start_learn", []() {
    if (server.hasArg("cmd")) {
      learning_cmd = server.arg("cmd");
      is_learning = true;
      learn_result_msg = "Đang chờ bấm remote cho nút " + learning_cmd;
    }
    server.send(200, "text/plain", "OK");
  });
  // ----------------------------------------------
  // Map các Route API xử lý
  server.on("/status", handleStatus);
  server.on("/set_real_clk", handleSetRealClk);
  server.on("/set_swing", handleSetSwing);
  setup_web_ir_test();
  server.begin();
  // Thêm dòng này cuối hàm setup() để reset lại watchdog / cấp thêm nhịp cho CPU
  vTaskDelay(pdMS_TO_TICKS(100));
  Serial.println("[OK] Hệ thống quạt + Dàn chân IO + Web đa giao diện đã sẵn sàng!");
}

void loop() {
  server.handleClient();
  updateFanSpeed();  // Ramp tần số mượt mà
  check_buttons();   // Quét nút bấm cứng
  handle_ota();
  delay(2);
  loop_ir();  // <--- THÊM DÒNG NÀY ĐỂ ĐỌC IR LIÊN TỤC
}