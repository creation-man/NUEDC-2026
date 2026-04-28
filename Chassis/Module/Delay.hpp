#ifndef __DELAY_HPP_
#define __DELAY_HPP_

#include "main.h"

class Delay {
public:
    // 初始化延时倍乘数
    static void Init(uint32_t sysclk_mhz = 72);

    // 微秒延时
    static void US(uint32_t nus);

    // 毫秒延时
    static void MS(uint16_t nms);

private:
    // 静态变量，记录每微秒的时钟节拍数
    // inline static 需要 C++17，如果是老版本，请在 .cpp 中初始化它
    static inline uint32_t g_fac_us = 0; 
};

#endif