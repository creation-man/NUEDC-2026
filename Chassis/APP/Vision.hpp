//
// Created by linji on 2026/4/16.
//

#ifndef VISION_RECEIVER_HPP
#define VISION_RECEIVER_HPP

#include "main.h"
#include <cstdio>

class VisionReceiver {
public:
    uint8_t rx_data;
    VisionReceiver();

    /**
     * @brief 解析串口接收到的单个字节
     * @param byte 接收到的 8 位数据
     */
    void parseByte(uint8_t byte);

    /**
     * @brief 获取当前的目标速度
     * @param outL 输出左轮目标速度
     * @param outR 输出右轮目标速度
     */
    void getTargetSpeeds(int &outL, int &outR);

private:
    int target_L;
    int target_R;
    uint32_t last_tick;

    // 内部缓冲区大小定义
    static const uint8_t BUF_SIZE = 32;
};
extern VisionReceiver vision;
#endif // VISION_RECEIVER_HPP
