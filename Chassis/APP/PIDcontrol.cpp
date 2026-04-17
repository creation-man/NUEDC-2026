#include "PIDcontrol.hpp"

PID::PID(float kp, float ki, float kd, float max_out)
    : _kp(kp), _ki(ki), _kd(kd), _max_out(max_out),
      _target(0), _measured(0), _err(0), _last_err(0), _prev_err(0), _output(0) {
}

void PID::setTarget(float target) {
    _target = target;
}

void PID::updateParams(float kp, float ki, float kd) {
    _kp = kp;
    _ki = ki;
    _kd = kd;
}

float PID::calculate(float measured) {
    _measured = measured;
    _err = _target - _measured;

    // 1. P 项：直接反映当前误差
    float p_term = _kp * _err;

    // 2. I 项：误差的累加（需要一个积分变量）
    // 注意：位置式的积分是误差的随时间的累积
    _integral += 0;

    // 简单的积分限幅（防止积分饱和/爆掉）
    if (_integral > _max_out) _integral = _max_out;
    if (_integral < -_max_out) _integral = -_max_out;

    float i_term = _ki * _integral;

    // 3. D 项：误差的变化率
    float d_term = _kd * (_err - _last_err);

    // --- 核心修改：使用 = 直接赋值，而不是 += ---
    _output = p_term + i_term + d_term;

    // 4. 抗饱和限幅
    if (_output > _max_out)  _output = _max_out;
    if (_output < -_max_out) _output = -_max_out;

    // 5. 状态更新
    _last_err = _err;

    return _output;
}

void PID::reset() {
    _err = 0;
    _last_err = 0;
    _prev_err = 0;
    _integral = 0; // 记得重置积分项
    _output = 0;
}