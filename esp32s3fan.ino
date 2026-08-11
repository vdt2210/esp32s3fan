#include <WebServer.h>
#include <LittleFS.h>

#include "wifi_config.h"   // Wifi
#include "led_rgb.h"       // LED
#include "fan_clk.h"       // tần
#include "io_control.h"    // Chân GPIO
#include "ir_processor.h"  // IR

//TODO tích hợp thành 1
#include "web_ir_learn.h"  // trang web hoc IR
#include "web_ir_test.h"   // duong dan /ir-test

WebServer server(80);

void serveFileOr404(const char* path, const char* contentType) {
  File file = LittleFS.open(path, FILE_READ);
  if (!file) {
    server.send(404, "text/plain", "Khong tim thay");
    return;
  }

  server.streamFile(file, contentType);
  file.close();
}

void handleRoot() {
  serveFileOr404("/basic.html", "text/html");
}

void handleAdvance() {
  serveFileOr404("/advance.html", "text/html");
}

void handleStatus() {
  static char json[64];

  int len = snprintf(
    json,
    sizeof(json),
    "{\"clk_hz\":%d,\"power\":%d,\"swing\":%d}",
    target_hz_clk,
    fan_power_state ? 1 : 0,
    swing_state ? 1 : 0);

  if (len > 0 && len < (int)sizeof(json)) {
    server.send(200, "application/json", json);
  } else {
    server.send(500, "text/plain", "JSON overflow");
  }
}

void handleSetRealClk() {
  if (!server.hasArg("hz")) {
    blink_state(BLINK_ERROR);
    server.send(400, "text/plain", "Thieu tham so hz!");
    return;
  }

  int real_hz = server.arg("hz").toInt();
  if (real_hz > 450) real_hz = 450;
  if (real_hz < 0) real_hz = 0;

  if (fan_power_state && target_hz_clk == real_hz) {
    blink_state(BLINK_SUCCESS);
    server.send(200, "text/plain", "OK");
    return;
  }

  if (real_hz > 0) {
    turn_fan_on(real_hz);
  } else {
    if (fan_power_state) turn_fan_off();
  }

  blink_state(BLINK_SUCCESS);
  server.send(200, "text/plain", "OK");
}

void handleSetSwing() {
  if (server.hasArg("state")) {
    bool state = server.arg("state") == "1";
    set_swing(state);

    if (swing_state == state) {
      blink_state(BLINK_SUCCESS);
      server.send(200, "text/plain", "OK");
    } else {
      blink_state(BLINK_ERROR);
      server.send(409, "text/plain", "Khong the bat dao gio khi quat dang tat!");
    }
  } else {
    blink_state(BLINK_ERROR);
    server.send(400, "text/plain", "Loi khong xac dinh!");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  setup_rgb();

  setup_fan_clk();
  setup_io();
  setup_ir();
  setup_wifi_ota();

  if (!LittleFS.begin(true)) {
    Serial.println("[LOI] Khoi tao LittleFS that bai");
  }

  // Routes web
  server.on("/", handleRoot);
  server.on("/advance", handleAdvance);
  server.on("/ir", handle_ir_learn_page);

  server.serveStatic("/main.js", LittleFS, "/main.js");
  server.serveStatic("/advance.js", LittleFS, "/advance.js");
  server.serveStatic("/style.css", LittleFS, "/style.css");

  // Routes API
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
      learn_result_msg = "Dang cho bam remote cho nut " + learning_cmd;
    }
    server.send(200, "text/plain", "OK");
  });

  server.on("/status", handleStatus);
  server.on("/set_real_clk", handleSetRealClk);
  server.on("/set_swing", handleSetSwing);
  setup_web_ir_test();

  server.begin();
  vTaskDelay(pdMS_TO_TICKS(100));
  Serial.println("[OK] He thong quat + IO + Web da san sang!");
}

void loop() {
  update_led_system();
  server.handleClient();
  updateFanSpeed();
  check_buttons();
  update_io_tasks();
  handle_ota();
  delay(2);
  loop_ir();
}
