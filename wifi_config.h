#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

#include <cstddef>
#include <cstring>

#include <WiFi.h>
#include <WiFiMulti.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <WebServer.h>
#include "led_rgb.h"

struct WifiNetwork {
  const char* ssid;
  const char* password;
};

#ifndef WIFI_SECRETS_H
#include "wifi_secrets.h"
#endif

#ifndef WIFI_SECRETS_READY
#error "WIFI_SECRETS_H chua dung dinh dang. Hay sao chep wifi_secrets.h thanh wifi_secrets.h va dien thong tin."
#endif

WiFiMulti wifiMulti;
extern WebServer server;

inline bool add_wifi_if_valid(const char* ssid, const char* password) {
  if (ssid == nullptr || password == nullptr) {
    return false;
  }
  if (strlen(ssid) == 0 || strlen(password) == 0) {
    return false;
  }
  wifiMulti.addAP(ssid, password);
  return true;
}

inline void setup_wifi_ota() {
  Serial.println("\n--- KET NOI WIFI ---");
  WiFi.mode(WIFI_STA);

  int configured = 0;
  for (size_t i = 0; i < WIFI_NETWORK_COUNT; i++) {
    configured += static_cast<int>(add_wifi_if_valid(WIFI_NETWORKS[i].ssid, WIFI_NETWORKS[i].password));
  }

  if (configured == 0) {
    blink_state(BLINK_ERROR);
    Serial.println("\n[LOI] Khong co cau hinh Wi-Fi hop le.");
    Serial.println("[LOI] Vui long kiem tra wifi_secrets.h: WIFI_NETWORKS can phai co it nhat 1 dong hop le.");
    return;
  }

  Serial.print("Dang quet va ket noi toi cac Wi-Fi da cau hinh...");
  int timeout = 0;
  while (wifiMulti.run() != WL_CONNECTED && timeout < 30) {
    delay(500);
    Serial.print(".");
    timeout++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[OK] Ket noi Wifi thanh cong!");
    Serial.print("Mang dang ket noi: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP cua ESP32-S3: ");
    Serial.println(WiFi.localIP());
    blink_state(BLINK_SUCCESS, 3);

    if (!MDNS.begin("esp32s3fan")) {
      blink_state(BLINK_ERROR);
      Serial.println("[LOI] Loi cau hinh MDNS!");
    }
  } else {
    blink_state(BLINK_ERROR);
    Serial.println("\n[LOI] Ket noi Wifi that bai! Kiem tra SSID, mat khau va vung phu song.");
  }

  ArduinoOTA.onStart([]() {
    String type = (ArduinoOTA.getCommand() == U_FLASH) ? "phan mem" : "he thong tep";
    Serial.println("Bat dau nap code tu xa: " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\n[THANH CONG] Nap code hoan tat! Dang khoi dong lai...");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Dang tai: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Loi OTA [%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Sai mat khau OTA");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Loi bat dau");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Loi ket noi");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Loi nhan du lieu");
    } else if (error == OTA_END_ERROR) {
      Serial.println("Loi ket thuc");
    }
  });

  ArduinoOTA.begin();
}

inline void handle_ota() {
  ArduinoOTA.handle();

  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 10000) {
    lastCheck = millis();
    if (WiFi.status() != WL_CONNECTED) {
      blink_state(BLINK_INFO);
      Serial.println("Mat ket noi mang! Dang quet tim Wi-Fi thay the...");
      wifiMulti.run();
    }
  }
}

#endif
