#ifndef WEB_IR_LEARN_H
#define WEB_IR_LEARN_H

#include <WebServer.h>

extern WebServer server;

// HTML Giao diện Học IR riêng biệt
const char HTML_IR_LEARN[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Học Lệnh Remote IR</title>
    <style>
        body { font-family: Arial, sans-serif; background: #121212; color: #fff; text-align: center; margin: 0; padding: 20px; }
        .card { background: #1e1e1e; max-width: 500px; margin: auto; padding: 20px; border-radius: 12px; box-shadow: 0 4px 10px rgba(0,0,0,0.5); }
        h2 { color: #00e676; margin-bottom: 5px; }
        p { color: #aaa; font-size: 14px; }
        .log-box { background: #000; color: #00ff66; font-family: monospace; font-size: 14px; padding: 15px; border-radius: 8px; text-align: left; height: 180px; overflow-y: auto; border: 1px solid #333; margin-bottom: 15px; }
        .btn { background: #2196F3; color: white; border: none; padding: 10px 15px; font-size: 14px; border-radius: 6px; cursor: pointer; margin: 5px; }
        .btn:hover { background: #0b7dda; }
        .btn-clear { background: #f44336; }
        .btn-learn { background: #ff9800; }
        .learn-status { font-weight: bold; color: #ffeb3b; margin: 10px 0; }
    </style>
</head>
<body>
    <div class="card">
        <h2>📡 BỘ HỌC LỆNH REMOTE IR</h2>
        <p>Hướng Remote vào mắt nhận và bấm nút để xem mã</p>
        
        <div id="learn-status" class="learn-status">Trạng thái: Đang chờ tín hiệu IR...</div>

        <div id="log" class="log-box">Đang kết nối mắt đọc...<br></div>

        <button class="btn btn-clear" onclick="clearLog()">Xóa Lịch Sử</button>
        <button class="btn" onclick="copyLog()">Copy Mã Lửi Ní Gem</button>

        <hr style="border-color: #333; margin: 20px 0;">
        <h3>Gán Lệnh Tự Học</h3>
        <button class="btn btn-learn" onclick="startLearn('POWER')">Học Nút Bật/Tắt</button>
        <button class="btn btn-learn" onclick="startLearn('SPEED')">Học Nút Tốc Độ</button>
        <button class="btn btn-learn" onclick="startLearn('SWING')">Học Nút Đảo Gió</button>
    </div>

    <script>
        let fullLog = "";

        function fetchIR() {
            fetch('/get_ir_data')
                .then(response => response.json())
                .then(data => {
                    if (data.received) {
                        let logBox = document.getElementById('log');
                        let newEntry = `[${data.protocol}] Hex: ${data.hex} | Bits: ${data.bits}\n`;
                        fullLog += newEntry;
                        logBox.innerText += newEntry;
                        logBox.scrollTop = logBox.scrollHeight;
                        
                        if(data.learn_msg) {
                            document.getElementById('learn-status').innerText = data.learn_msg;
                        }
                    }
                });
        }

        function clearLog() {
            document.getElementById('log').innerText = "";
            fullLog = "";
        }

        function copyTextFallback(text) {
    var textArea = document.createElement("textarea");
    textArea.value = text;
    textArea.style.position = "fixed";  // Tránh cuộn trang
    document.body.appendChild(textArea);
    textArea.focus();
    textArea.select();
    try {
        document.execCommand('copy');
        alert('Đã copy thành công!');
    } catch (err) {
        alert('Không copy được, ní tự bôi đen giúp tui nha!');
    }
    document.body.removeChild(textArea);
}

        function startLearn(cmd) {
            document.getElementById('learn-status').innerText = "Đang chờ bấm remote cho nút: " + cmd + "...";
            fetch('/start_learn?cmd=' + cmd);
        }

        // Tự động kiểm tra dữ liệu IR mỗi 300ms
        setInterval(fetchIR, 300);
    </script>
</body>
</html>
)rawliteral";

void handle_ir_learn_page() {
    server.send(200, "text/html", HTML_IR_LEARN);
}

#endif