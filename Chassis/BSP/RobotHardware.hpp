#ifndef ROBOT_HARDWARE_HPP
#define ROBOT_HARDWARE_HPP

#include "Motor.hpp"
#include "Encode.hpp"

// 声明外部调用的对象数组
extern Motor motors[4];
extern Encoder encoders[4];

void Robot_Hardware_Init();

#endif
