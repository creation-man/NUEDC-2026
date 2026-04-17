//
// Created by linji on 2026/4/16.
//
#include "usart.h"
#include "Callback.hpp"
#include "Vision.hpp"




// 中断回调
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    //if (huart->Instance == USART2) {
        vision.parseByte(vision.rx_data);
        HAL_UART_Receive_IT(&huart2, &vision.rx_data, 1);
   // }
}