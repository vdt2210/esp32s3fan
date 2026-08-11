#ifndef WEB_IR_TEST_H
#define WEB_IR_TEST_H

#include <WebServer.h>
#include "ir_processor.h"

extern WebServer server;

const char HTML_IR_TEST[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32-S3 IR Test</title>
    <style>
        body { font-family: Arial, sans-serif; text-align: center; background: #121212; color: #fff; padding: 20px; }
        .card { background: #1e1e1e; padding: 15px; margin: 10px auto; max-width: 450px; border-radius: 10px; box-shadow: 0 4px 6px rgba(0,0,0,0.3); }
        button { padding: 10px 20px; margin: 5px; border: none; border-radius: 5px; background: #007bff; color: white; font-weight: bold; cursor: pointer; }
        button:hover { background: #0056b3; }
        .acc { font-size: 1.5em; color: #28a745; font-weight: bold; }
        .btn-reset { background: #dc3545; }
        .btn-copy { background: #17a2b8; }
        .log-box { background: #000; color: #00ff00; font-family: monospace; padding: 10px; text-align: left; border-radius: 5px; max-height: 150px; overflow-y: auto; font-size: 0.9em; word-break: break-all; }
    </style>
</head>
<body>
    <h2>📡 IR Accuracy Tester</h2>
    <p>Chế độ đang chọn: <b id="mode-text" style="color:#ffc107;">Chưa chọn</b></p>
    
    <div>
        <button onclick="setMode('ON_SPEED')">Test ON / SPEED (Raw)</button>
        <button onclick="setMode('OFF')">Test OFF</button>
        <button onclick="setMode('SWING')">Test SWING</button>
    </div>

    <div class="card">
        <h3>ON / SPEED</h3>
        <p>Thành công / Tổng: <span id="on-count">0/0</span></p>
        <p class="acc"><span id="on-acc">0</span>%</p>
        <button class="btn-reset" onclick="resetCount('ON_SPEED')">Reset</button>
    </div>

    <div class="card">
        <h3>OFF</h3>
        <p>Thành công / Tổng: <span id="off-count">0/0</span></p>
        <p class="acc"><span id="off-acc">0</span>%</p>
        <button class="btn-reset" onclick="resetCount('OFF')">Reset</button>
    </div>

    <div class="card">
        <h3>SWING</h3>
        <p>Thành công / Tổng: <span id="swing-count">0/0</span></p>
        <p class="acc"><span id="swing-acc">0</span>%</p>
        <button class="btn-reset" onclick="resetCount('SWING')">Reset</button>
    </div>

    <div class="card">
        <h3>📋 Log Chi Tiết Nhận Tín Hiệu</h3>
        <div id="log-text" class="log-box">Đang chờ tín hiệu...</div>
        <br>
        <button class="btn-copy" onclick="copyLog()">📋 Copy Log Gửi Gemini</button>
    </div>

    <script>
        function setMode(mode) { fetch('/api/ir/test-mode?mode=' + mode); }
        function resetCount(key) { fetch('/api/ir/reset?key=' + key); }
        function copyLog() {
            let txt = document.getElementById('log-text').innerText;
            navigator.clipboard.writeText(txt).then(() => alert('Đã copy Log vào bộ nhớ tạm!'));
        }
        function updateStats() {
            fetch('/api/ir/status')
                .then(r => r.json())
                .then(d => {
                    document.getElementById('mode-text').innerText = d.mode || 'Chưa chọn';
                    document.getElementById('on-count').innerText = d.on_speed.success + '/' + d.on_speed.total;
                    document.getElementById('on-acc').innerText = d.on_speed.accuracy;
                    document.getElementById('off-count').innerText = d.off.success + '/' + d.off.total;
                    document.getElementById('off-acc').innerText = d.off.accuracy;
                    document.getElementById('swing-count').innerText = d.swing.success + '/' + d.swing.total;
                    document.getElementById('swing-acc').innerText = d.swing.accuracy;
                    if(d.last_log) {
                        document.getElementById('log-text').innerText = d.last_log;
                    }
                });
        }
        setInterval(updateStats, 800);
    </script>
</body>
</html>
)rawliteral";

inline void setup_web_ir_test() {
    server.on("/ir-test", HTTP_GET, []() {
        server.send(200, "text/html", HTML_IR_TEST);
    });

    server.on("/api/ir/test-mode", HTTP_GET, []() {
        if (server.hasArg("mode")) {
            setTestMode(server.arg("mode"));
            server.send(200, "application/json", "{\"status\":\"ok\"}");
        } else {
            server.send(400, "application/json", "{\"status\":\"missing mode\"}");
        }
    });

    server.on("/api/ir/status", HTTP_GET, []() {
        String json = "{";
        json += "\"mode\":\"" + activeTestMode + "\",";
        json += "\"on_speed\":{\"total\":" + String(testOnSpeed.total) + ",\"success\":" + String(testOnSpeed.success) + ",\"accuracy\":" + String(testOnSpeed.accuracy, 1) + "},";
        json += "\"off\":{\"total\":" + String(testOff.total) + ",\"success\":" + String(testOff.success) + ",\"accuracy\":" + String(testOff.accuracy, 1) + "},";
        json += "\"swing\":{\"total\":" + String(testSwing.total) + ",\"success\":" + String(testSwing.success) + ",\"accuracy\":" + String(testSwing.accuracy, 1) + "},";
        json += "\"last_log\":\"" + last_test_log + "\"";
        json += "}";
        server.send(200, "application/json", json);
    });

    server.on("/api/ir/reset", HTTP_GET, []() {
        if (server.hasArg("key")) {
            resetIRTest(server.arg("key"));
            server.send(200, "application/json", "{\"status\":\"ok\"}");
        } else {
            server.send(400, "application/json", "{\"status\":\"missing key\"}");
        }
    });
}

#endif