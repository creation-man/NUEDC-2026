#include "Blink.hpp"

// 必须包含底层 C 头文件，并用 extern "C" 包裹
extern "C" {
#include "main.h" // 包含 HAL 库和引脚定义
}

void CPP_Main_Entry(void) {
    // 经典的翻转电平点灯
    HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
    HAL_Delay(500); // 500ms 闪烁一次
}//
// Created by linji on 2026/4/10.
//
