#ifndef ENCODER_HPP
#define ENCODER_HPP

#include "main.h"

class Encoder {
public:
    Encoder(TIM_HandleTypeDef* htim);
    void init();
    void update();      // 建议每 10ms 调用一次
    int32_t getSpeed(); // 返回当前周期的增量（脉冲数）

private:
    TIM_HandleTypeDef* _htim;
    int16_t _last_count;
    int32_t _speed;
};

#endif