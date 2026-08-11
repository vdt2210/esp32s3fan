#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

#include <WiFi.h>
#include <WiFiMulti.h> // Thêm thư viện quản lý nhiều Wi-Fi
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <WebServer.h>

// Wi-Fi 1 (Chính của bạn)
#define WIFI_SSID_1     "DOCTOR LAPTOP"
#define WIFI_PASSWORD_1 "91a32q10"

// Wi-Fi 2 (Dự phòng - bạn đổi lại tên và mật khẩu theo ý muốn)
#define WIFI_SSID_2     "Meo Con"
#define WIFI_PASSWORD_2 "huamynhung"

// Wi-Fi 3
#define WIFI_SSID_3     "VDT IoT"
#define WIFI_PASSWORD_3 "khongcoquyentruycap"

// Khai báo đối tượng WiFiMulti
WiFiMulti wifiMulti;

// Khai báo server dùng chung (sẽ định nghĩa ở file .ino)
extern WebServer server;

inline void setup_wifi_ota() {
    Serial.println("\n--- ĐANG KẾT NỐI WIFI ---");
    WiFi.mode(WIFI_STA);

    // Đăng ký danh sách các mạng Wi-Fi
    wifiMulti.addAP(WIFI_SSID_1, WIFI_PASSWORD_1);
    wifiMulti.addAP(WIFI_SSID_2, WIFI_PASSWORD_2);
    wifiMulti.addAP(WIFI_SSID_3, WIFI_PASSWORD_3);

    Serial.print("Đang quét và kết nối vào Wi-Fi phù hợp");
    int timeout = 0;
    // wifiMulti.run() sẽ tự chọn mạng có sóng tốt nhất để kết nối
    while (wifiMulti.run() != WL_CONNECTED && timeout < 30) {
        delay(500);
        Serial.print(".");
        timeout++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n[OK] Wifi đã kết nối thành công!");
        Serial.print("Mạng đang kết nối: ");
        Serial.println(WiFi.SSID());
        Serial.print("IP của ESP32-S3: ");
        Serial.println(WiFi.localIP());
        
        // Cấu hình tên MDNS để nạp code từ xa qua tên (ví dụ: esp32s3fan.local)
        if (!MDNS.begin("esp32s3fan")) {
            Serial.println("[Error] Lỗi cấu hình MDNS!");
        }
    } else {
        Serial.println("\n[Error] Kết nối Wifi thất bại! Đang không ở gần router nào cả.");
    }

    // --- CẤU HÌNH ARDUINO OTA (NẠP CODE QUA WIFI GIỮ NGUYÊN CỦA BẠN) ---
    ArduinoOTA.onStart([]() {
        String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
        Serial.println("Bắt đầu nạp code từ xa: " + type);
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\n[OK] Nạp code hoàn tất! Đang khởi động lại...");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Đang nạp: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Lỗi [%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Sai mật khẩu OTA");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Lỗi Begin");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Lỗi kết nối");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Lỗi nhận dữ liệu");
        else if (error == OTA_END_ERROR) Serial.println("Lỗi End");
    });

    ArduinoOTA.begin();
}

inline void handle_ota() {
    ArduinoOTA.handle();
    
    // Mẹo nhỏ: Kiểm tra rớt mạng trong lúc quạt đang chạy để tự động kết nối lại Wi-Fi phụ
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck > 10000) { // Mỗi 10 giây check 1 lần để tránh làm nghẽn chip
        lastCheck = millis();
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("Mất mạng! Đang quét tìm Wi-Fi dự phòng khả dụng...");
            wifiMulti.run();
        }
    }
}

#endif