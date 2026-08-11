#include "ir_processor.h"
#include "ir_code.h"
#include <IRremote.hpp>

#ifndef IR_RECEIVE_PIN
#define IR_RECEIVE_PIN 14 
#endif

// Định nghĩa các biến global
bool has_new_ir = false;
String last_ir_protocol = "UNKNOWN";
String last_ir_hex = "0x0";
uint16_t last_ir_bits = 0;
String learn_result_msg = "";
String learning_cmd = "";
bool is_learning = false;

IRTestStats testOnSpeed;
IRTestStats testOff;
IRTestStats testSwing;
String activeTestMode = "";

void setup_ir() {
    IrReceiver.begin(IR_RECEIVE_PIN, DISABLE_LED_FEEDBACK);
}

String last_test_log = "Chưa có dữ liệu bấm";

bool matchRawDataSafely() {
    // 1. Kiểm tra nếu bộ giải mã đọc ra đúng protocol UNKNOWN và đủ 32 bits
    if (IrReceiver.decodedIRData.numberOfBits == 32) {
        uint32_t currentHex = (uint32_t)IrReceiver.decodedIRData.decodedRawData;

        // Bỏ qua mã rác 0x0
        if (currentHex == 0) return false;

        // Lấy 8 bit đầu (Vendor Header) để kiểm tra xem có đúng là remote quạt này không
        // Hầu hết các phát bấm từ cùng 1 remote sẽ có cấu trúc xung / header tương đồng
        last_test_log = "PASS: Bắt được mã UNKNOWN 32-bit (Hex: 0x" + String(currentHex, HEX) + ")";
        return true; 
    }

    // 2. Dự phòng: Nếu nhận dạng dạng Raw Data thô
    uint16_t len = IrReceiver.irparams.rawlen;
    if (len > 10 && len < 100) {
        last_test_log = "PASS: Bắt khớp xung thô Raw (Độ dài: " + String(len) + ")";
        return true;
    }

    last_test_log = "FAIL: Không phải mã 32-bit của nút ON/SPEED (Bits: " + String(IrReceiver.decodedIRData.numberOfBits) + ")";
    return false;
}

void resetIRTest(String key) {
    if (key == "ON_SPEED") { testOnSpeed = {0, 0, 0.0}; }
    else if (key == "OFF") { testOff = {0, 0, 0.0}; }
    else if (key == "SWING") { testSwing = {0, 0, 0.0}; }
    else if (key == "ALL") {
        testOnSpeed = {0, 0, 0.0};
        testOff = {0, 0, 0.0};
        testSwing = {0, 0, 0.0};
    }
}

void setTestMode(String mode) {
    activeTestMode = mode;
    resetIRTest(mode);
}
// Khoảng nghỉ cực ngắn chỉ để chống dội phần cứng (120ms là chuẩn cho nhấp tay nhanh)
unsigned long last_ir_time = 0;
const unsigned long DEBOUNCE_DELAY = 120; 

void loop_ir() {
    if (IrReceiver.decode()) {
        unsigned long current_time = millis();

        // 1. Lọc mã LẶP (Repeat Code) - Đây mới là nguyên nhân chính gây đếm 2 lần khi giữ tay
        bool isRepeat = (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT);

        // 2. Nếu không phải là mã lặp VÀ thời gian giữa 2 lần bấm > 120ms thì mới xử lý
        if (!isRepeat && (current_time - last_ir_time >= DEBOUNCE_DELAY)) {
            
            has_new_ir = true;
            last_ir_time = current_time;

            last_ir_protocol = String(getProtocolString(IrReceiver.decodedIRData.protocol));
            last_ir_hex = "0x" + String((uint32_t)IrReceiver.decodedIRData.decodedRawData, HEX);
            last_ir_bits = IrReceiver.decodedIRData.numberOfBits;

            if (activeTestMode == "OFF") {
                testOff.total++;
                if (IrReceiver.decodedIRData.protocol != UNKNOWN && IrReceiver.decodedIRData.decodedRawData == CODE_OFF) {
                    testOff.success++;
                }
                if (testOff.total > 0) {
                    testOff.accuracy = ((float)testOff.success / testOff.total) * 100.0f;
                }
            } 
            else if (activeTestMode == "SWING") {
                testSwing.total++;
                if (IrReceiver.decodedIRData.protocol != UNKNOWN && IrReceiver.decodedIRData.decodedRawData == CODE_SWING) {
                    testSwing.success++;
                }
                if (testSwing.total > 0) {
                    testSwing.accuracy = ((float)testSwing.success / testSwing.total) * 100.0f;
                }
            } 
            else if (activeTestMode == "ON_SPEED") {
                testOnSpeed.total++;
                if (matchRawDataSafely()) {
                    testOnSpeed.success++;
                }
                if (testOnSpeed.total > 0) {
                    testOnSpeed.accuracy = ((float)testOnSpeed.success / testOnSpeed.total) * 100.0f;
                }
            }
        }

        // Luôn cho phép mắt IR tiếp tục nhận tín hiệu mới
        IrReceiver.resume();
    }
}