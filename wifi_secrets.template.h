//!!! KHÔNG UP FILE CÓ MẬT KHẨU WIFI LÊN GIT !!!
// Để sử dụng hãy copy file này ra wifi_secrets.h, rồi điền Wifi & password ở file đó
#ifndef WIFI_SECRETS_H
#define WIFI_SECRETS_H
#define WIFI_SECRETS_READY 1

// Danh sách Wi-Fi dạng {"SSID", "PASSWORD"}
const WifiNetwork WIFI_NETWORKS[] = {
  { "WIFI_1", "PASS_1" },
  { "WIFI_2", "PASS_2" },
};

const size_t WIFI_NETWORK_COUNT = sizeof(WIFI_NETWORKS) / sizeof(WIFI_NETWORKS[0]);

#endif
