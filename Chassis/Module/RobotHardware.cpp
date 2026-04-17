#include "RobotHardware.hpp"
#include "tim.h"

// 1. 实例化电机：根据你的引脚和官方宏定义
Motor motors[4] = {
    { &htim8, TIM_CHANNEL_1, TIM_CHANNEL_2, true  }, // M1
    { &htim8, TIM_CHANNEL_3, TIM_CHANNEL_4, true  }, // M2
    { &htim1, TIM_CHANNEL_1, TIM_CHANNEL_2, false }, // M3
    { &htim1, TIM_CHANNEL_3, TIM_CHANNEL_4, false }  // M4
};

// 2. 实例化编码器：对应 TIM2, 3, 4, 5
Encoder encoders[4] = {
    { &htim2 }, // M1
    { &htim3 }, // M2
    { &htim4 }, // M3
    { &htim5 }  // M4
};

void Robot_Hardware_Init() {
    for (int i = 0; i < 4; i++) {
        motors[i].init();
        encoders[i].init();
    }
}
