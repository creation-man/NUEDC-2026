#include "Encode.hpp"

Encoder::Encoder(TIM_HandleTypeDef* htim) : _htim(htim), _last_count(0), _speed(0) {}
int16_t current_count=0;
void Encoder::init() {
    HAL_TIM_Encoder_Start(_htim, TIM_CHANNEL_ALL);
    __HAL_TIM_SET_COUNTER(_htim, 0);
}

void Encoder::update() {
    // 核心逻辑：利用 int16_t 的溢出特性自动处理回环
     current_count = (int16_t)__HAL_TIM_GET_COUNTER(_htim);
    _speed = current_count - _last_count; 
    _last_count = current_count;
}

int32_t Encoder::getSpeed() {
    return _speed;
}