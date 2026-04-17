#ifndef MOTOR_HPP
#define MOTOR_HPP

#include "main.h"

class Motor {
public:
    Motor(TIM_HandleTypeDef* htim, uint32_t chA, uint32_t chB, bool reversed = false);
    void init();
    void setSpeed(int16_t speed); // 输入范围建议 -2000 到 2000

private:
    TIM_HandleTypeDef* _htim;
    uint32_t _channelA;
    uint32_t _channelB;
    bool _is_reversed;
    static constexpr int16_t DEAD_ZONE = 0; // 对应你之前的 MOTOR_IGNORE_PULSE
    static constexpr int16_t MAX_PULSE = 3600;
};

#endif