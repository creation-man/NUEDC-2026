//
// Created by linji on 2026/4/13.
//
#include "Serial.hpp"
#include "cmsis_os.h"
#include "usart.h"
#include "cstring"
#include "RobotHardware.hpp"
#include "Vision.hpp"
unsigned char VofaUartSendArr[32] = {0};

void VofaUartSend()
{

    float Ch1 = (float)encoders[0].getSpeed()*5.7692f; // 通道1：M1速度
    float Ch2 = (float)encoders[1].getSpeed()*5.7692f; // 通道2：M2速度
    float Ch3 = -(float)encoders[2].getSpeed()*5.7692f; // 通道2：M2速度
    float Ch4 = -(float)encoders[3].getSpeed()*5.7692f; // 通道2：M2速度
    float Ch5 = 0;
    float Ch6 = 0;
    float Ch7 = 0;
    uint32_t special_value = 0x7f800000;
    memcpy(&VofaUartSendArr[0], &Ch1, sizeof(float));
    memcpy(&VofaUartSendArr[4], &Ch2, sizeof(float));
    memcpy(&VofaUartSendArr[8], &Ch3, sizeof(float));
    memcpy(&VofaUartSendArr[12], &Ch4, sizeof(float));
    memcpy(&VofaUartSendArr[16], &Ch5, sizeof(float));
    memcpy(&VofaUartSendArr[20], &Ch6, sizeof(float));
    memcpy(&VofaUartSendArr[24], &Ch7, sizeof(float));
    memcpy(&VofaUartSendArr[sizeof(float) * 7], &special_value, sizeof(uint32_t));
    //if (huart1.gState == HAL_UART_STATE_READY)
    HAL_UART_Transmit_DMA(&huart1, VofaUartSendArr, sizeof(float) * 8);

}

extern "C" void SerailTask(void *argument)
{    HAL_UART_Receive_IT(&huart2, &vision.rx_data, 1);
    for (;;) {
        VofaUartSend();
        osDelay((1));
    }
}