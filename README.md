# ESP32-S3 Fan Controller (Nidec)

Đây là project điều khiển quạt Nidec bằng ESP32-S3 gồm web giao diện + điều khiển quạt qua tín hiệu xung + đọc/ghi IR + OTA.

## 1) Chuẩn bị nhanh

### Phần cứng
- ESP32-S3
- Mạch driver quạt có các đầu vào: `CLK` (GPIO9), `SWING`, nguồn 24V chính (GPIO16), buzzer và LED
- IR receiver (đã khai báo GPIO14)
- Nguồn nuôi ổn định + nút điều khiển

### Phần mềm
- Arduino IDE 1.8.13+ (legacy) hoặc Arduino IDE 2.x
- ESP32 core (đã cài)
- Thư viện: `IRremote`
- Plugin upload folder `data` (xem phần dưới theo từng IDE)

## 2) Kết nối chân GPIO trong code

- `PIN_BTN_1` = 4: ON/SPEED (active low)
- `PIN_BTN_2` = 5: SWING (active low)
- `PIN_BTN_3` = 6: OFF (active low)
- `PIN_LED_1` = 10, `PIN_LED_2` = 11, `PIN_LED_3` = 12, `PIN_LED_4` = 13
- `PIN_IR_RECV` = 14
- `CLK_PIN` = 9
- `PIN_SWING` = 15
- `PIN_FAN_POWER` = 16
- `PIN_BUZZER` = 19
- `RGB_PIN` = 48

## 3) Cài đặt và nạp firmware

1. Mở `esp32s3fan.ino` trong IDE.
2. Chọn đúng Board: `ESP32S3` (và cổng COM).
3. **Upload sketch**.
4. Mở Serial Monitor `115200` để xem IP sau khi connect Wi‑Fi.

## 4) Cấu hình Wi‑Fi

Mặc định Wi‑Fi đang đặt trực tiếp trong `wifi_config.h`:
- `WIFI_SSID_1`, `WIFI_PASSWORD_1`
- `WIFI_SSID_2`, `WIFI_PASSWORD_2`
- `WIFI_SSID_3`, `WIFI_PASSWORD_3`

Thay đổi các giá trị này theo mạng của bạn rồi upload lại sketch.

---

## 5) Upload folder `data` (các file web) lên LittleFS

### 5.1 Arduino IDE 1.8.x (legacy)

Sử dụng plugin **ESP32 Sketch Data Upload** (cơ chế cũ):
- Repository: [arduino-esp32fs-plugin](https://github.com/lorol/arduino-esp32fs-plugin)
- Releases: [arduino-esp32fs-plugin/releases](https://github.com/lorol/arduino-esp32fs-plugin/releases)

Các bước:
1. Tải `esp32fs.zip` mới nhất từ mục Releases.
2. Giải nén ra folder `Arduino/tools/ESP32FS/tool/` trong Sketchbook của bạn.
   - Ví dụ: `C:\Users\<user>\Documents\Arduino\tools\ESP32FS\tool\esp32fs.jar`
3. Khởi động lại IDE.
4. Mở project và chắc chắn có folder `data/` (đã có sẵn trong repo).
5. `Tools > ESP32 Sketch Data Upload`
6. Chọn `LittleFS`.
7. Đợi xong thông báo upload.

Lưu ý: đóng Serial Monitor trước khi upload dữ liệu FS.

### 5.2 Arduino IDE 2.x

Sử dụng plugin của Arduino IDE 2 (định dạng `.vsix`):
- Repository: [arduino-littlefs-upload](https://github.com/earlephilhower/arduino-littlefs-upload)
- Releases (tải file `.vsix`): [arduino-littlefs-upload/releases](https://github.com/earlephilhower/arduino-littlefs-upload/releases)

Các bước:
1. Tải file `.vsix` mới nhất từ Releases.
2. Copy `.vsix` vào:
   - Windows: `C:\Users\<user>\.arduinoIDE\plugins\`
   - Mac/Linux: `~/.arduinoIDE/plugins/`
3. Khởi động lại IDE 2.
4. Vào sketch đang làm việc, nhấn `Ctrl/Cmd + Shift + P`, tìm lệnh:
   - `Upload LittleFS to Pico/ESP8266/ESP32`
5. Chạy lệnh này để upload toàn bộ `data/`.

Nếu không thấy command trong palette, kiểm tra:
- Đã restart IDE chưa.
- File `.vsix` đúng version mới nhất và đúng folder.
- Đường dẫn sketch đúng và có folder `data/`.

---

## 6) Truy cập giao diện và kiểm tra nhanh

Sau khi nạp firmware + upload `data/`:
- Mở IP trên trình duyệt: `http://<ESP32_IP>`
- Giao diện cơ bản: `/`
- Nâng cao: `/advance`

## 7) API quan trọng

- `GET /status` → trạng thái hiện tại
- `GET /set_real_clk?hz=<0..450>` → đặt tốc độ
- `GET /set_swing?state=1|0` → bật/tắt swing
- `GET /start_learn?cmd=POWER|SPEED|SWING` → học lệnh IR
- `GET /get_ir_data` → lấy log IR
- `/ir-test` → test lặp lại IR và thống kê

## 8) Xử lý lỗi thường gặp

- Mở web lỗi: chưa upload `data/` lên đúng LittleFS.
- Không đổi tốc độ: kiểm tra nguồn quạt 24V và dây tín hiệu CLK.
- Không kết nối Wi‑Fi: xác nhận SSID/Password trong `wifi_config.h`.
- Không bíp/LED: kiểm tra nguồn/mạch kết nối buzzer/LED.
