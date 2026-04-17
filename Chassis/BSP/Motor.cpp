#include "Motor.hpp"

Motor::Motor(TIM_HandleTypeDef* htim, uint32_t chA, uint32_t chB, bool reversed)
    : _htim(htim), _channelA(chA), _channelB(chB), _is_reversed(reversed) {}

void Motor::init() {
    HAL_TIM_PWM_Start(_htim, _channelA);
    HAL_TIM_PWM_Start(_htim, _channelB);
    if (_htim->Instance == TIM1 || _htim->Instance == TIM8) {
        __HAL_TIM_MOE_ENABLE(_htim);
    }
}

void Motor::setSpeed(int16_t speed) {
    if (speed == 0) {
        __HAL_TIM_SET_COMPARE(_htim, _channelA, 0);
        __HAL_TIM_SET_COMPARE(_htim, _channelB, 0);
        return;
    }
    int16_t val = _is_reversed ? -speed : speed;
    int16_t pulse = (val > 0) ? (val + DEAD_ZONE) : (val - DEAD_ZONE);

    // 限幅
    if (pulse > MAX_PULSE) pulse = MAX_PULSE;
    if (pulse < -MAX_PULSE) pulse = -MAX_PULSE;

    if (pulse > 0) {
        __HAL_TIM_SET_COMPARE(_htim, _channelA, (uint32_t)pulse);
        __HAL_TIM_SET_COMPARE(_htim, _channelB, 0);
    } else {
        __HAL_TIM_SET_COMPARE(_htim, _channelA, 0);
        __HAL_TIM_SET_COMPARE(_htim, _channelB, (uint32_t)-pulse);
    }
}

