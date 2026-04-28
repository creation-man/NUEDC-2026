//
// Created by linji on 2026/4/16.
//
#include "usart.h"
#include "Callback.hpp"
#include "Vision.hpp"
#include "uitrasonic.hpp"
#include "tim.h"

uint8_t RxTemp = 0;


extern "C" {

// 1. 定义使用的串口句柄（如果是 UART1，就用 huart1）
#define USART_DEBUG huart1
extern UART_HandleTypeDef huart1; // 声明外部定义的句柄

// 2. 根据编译器环境选择原型
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

/**
 * @brief 重定向 C 标准库 printf 函数到串口
 */
PUTCHAR_PROTOTYPE
{
    // 使用阻塞模式发送，确保字符完整打印
    HAL_UART_Transmit(&USART_DEBUG, (uint8_t *)&ch, 1, 0xFFFF);
    return ch;
}
}

void USART1_UART_Init(void)
{
    //启动接收中断
    HAL_UART_Receive_IT(&huart1, (uint8_t *)&RxTemp, 1);
}

void USART1_IRQHandler(void)
{

    HAL_UART_IRQHandler(&huart1);

}
// 中断回调
 extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    // if (huart->Instance == USART2) {
    //     vision.parseByte(vision.rx_data);
    //     HAL_UART_Receive_IT(&huart2, &vision.rx_data, 1);
    // }
    // if (huart->Instance == USART1)
    // {
    HAL_UART_Transmit(&huart1, (uint8_t *)&RxTemp, 1, 0xFFFF);
    HAL_UART_Receive_IT(&huart1, (uint8_t *)&RxTemp, 1);


    // }
}
void Bsp_TIM7_Init(void)
{

    HAL_TIM_Base_Start_IT(&htim7);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    /* USER CODE BEGIN Callback 0 */

    /* USER CODE END Callback 0 */
    if (htim->Instance == TIM6)
    {
        HAL_IncTick();
    }

    /* USER CODE BEGIN Callback 1 */
    if (htim->Instance == TIM7) {
        Sensor::HCSR04::getInstance().onTimerOverflow();
    }
    /* USER CODE END Callback 1 */
}
