#include "Delay.hpp"

/**
 * @brief 初始化延时倍乘数
 * @param sysclk_mhz 系统主频，STM32F103通常为 72
 */
void Delay::Init(uint32_t sysclk_mhz) {
    g_fac_us = sysclk_mhz;
}

/**
 * @brief 微秒级延时 (时钟摘取法)
 * @param nus 延时的微秒数
 */
void Delay::US(uint32_t nus) {
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = SysTick->LOAD;    /* LOAD的值 */
    
    ticks = nus * g_fac_us;             /* 需要的节拍数 */
    told = SysTick->VAL;                /* 刚进入时的计数器值 */
    
    while (true) {
        tnow = SysTick->VAL;
        if (tnow != told) {
            // SysTick是一个递减计数器
            if (tnow < told) {
                tcnt += told - tnow;
            } else {
                tcnt += reload - tnow + told;
            }
            told = tnow;
            if (tcnt >= ticks) {
                break;
            }
        }
    }
}

/**
 * @brief 毫秒级延时
 */
void Delay::MS(uint16_t nms) {
    US(static_cast<uint32_t>(nms) * 1000);
}