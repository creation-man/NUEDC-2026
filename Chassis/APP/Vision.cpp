#include "Vision.hpp"
VisionReceiver vision; // 全局对象
// 构造函数初始化
VisionReceiver::VisionReceiver()
    : target_L(0)
    , target_R(0)
    , last_tick(0)
{}

void VisionReceiver::parseByte(uint8_t byte) {
    static char buf[BUF_SIZE];
    static uint8_t index = 0;
    static bool is_capturing = false;

    if (byte == '$') {  // 帧头识别
        is_capturing = true;
        index = 0;
        return;
    }

    if (is_capturing) {
        if (byte == '#') {  // 帧尾识别
            buf[index] = '\0';
            // 解析字符串，sscanf 返回成功解析的变量个数
            if (sscanf(buf, "%d,%d", &target_L, &target_R) == 2) {
                last_tick = HAL_GetTick(); // 成功解析后刷新时间戳
            }
            is_capturing = false;
        }
        else {
            // 将字符存入缓冲区，并防止溢出
            buf[index++] = byte;
            if (index >= BUF_SIZE - 1) {
                index = 0;
                is_capturing = false;
            }
        }
    }
}

void VisionReceiver::getTargetSpeeds(int &outL, int &outR) {
    // 保护机制：如果 500ms 内没有收到有效帧，说明视觉系统或连接异常，强制停车
    if (HAL_GetTick() - last_tick > 500) {
        target_L = 0;
        target_R = 0;
    }

    outL = target_L;
    outR = target_R;
}
