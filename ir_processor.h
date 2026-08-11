#ifndef IR_PROCESSOR_H
#define IR_PROCESSOR_H

#include <Arduino.h>

// Định nghĩa struct thống kê
struct IRTestStats {
    uint16_t total = 0;
    uint16_t success = 0;
    float accuracy = 0.0;
};

// Khai báo extern các biến dùng chung
extern bool has_new_ir;
extern String last_ir_protocol;
extern String last_ir_hex;
extern uint16_t last_ir_bits;
extern String learn_result_msg;
extern String learning_cmd;
extern bool is_learning;

extern String activeTestMode;
extern IRTestStats testOnSpeed;
extern IRTestStats testOff;
extern IRTestStats testSwing;
extern String last_test_log; // <--- Dòng mới lưu log chi tiết

// Khai báo các hàm
void setup_ir();
void loop_ir();
void setTestMode(String mode);
void resetIRTest(String key);

#endif