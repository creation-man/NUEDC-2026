//
// Created by linji on 2026/4/13.
//
#include "Serial.hpp"
#include "cmsis_os.h"
#include "usart.h"
#include "cstring"
#include "RobotHardware.hpp"
#include "Vision.hpp"
#include "MPU6050.hpp"
#include "i2c.h"
#include "Delay.hpp"
#include "Callback.hpp"
#include "remote.hpp"
#include "uitrasonic.hpp"
#include "tim.h"
unsigned char VofaUartSendArr[32] = {0};
float dist=0;
/* 实例化对象 */

// 绑定硬件 I2C2，默认地址 0x68
 MPU6050 imu(&hi2c2);
void VofaUartSend()
{

    float Ch1 = (float)encoders[0].getSpeed()*5.7692f; // 通道1：M1速度
    float Ch2 = (float)encoders[1].getSpeed()*5.7692f; // 通道2：M2速度
    float Ch3 = -(float)encoders[2].getSpeed()*5.7692f; // 通道2：M2速度
    float Ch4 = -(float)encoders[3].getSpeed()*5.7692f; // 通道2：M2速度
    float Ch5 = imu.yaw;
    float Ch6 = imu.pitch;
    float Ch7 = imu.roll;
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
   // HAL_UART_Transmit_DMA(&huart1, VofaUartSendArr, sizeof(float) * 8);

}

extern "C" void SerailTask(void *argument)
{    HAL_UART_Receive_IT(&huart2, &vision.rx_data, 1);
     Delay::Init(72);
     USART1_UART_Init();
     Bsp_TIM7_Init();

    auto & myScanner = Sensor::HCSR04::getInstance(GPIOF, GPIO_PIN_11, GPIOF, GPIO_PIN_12, &htim7);
    if(imu.Init() == 0) {
        // 初始化成功后，立刻进行校准
        // 此时不要碰板子！
        imu.Calibrate_Gyro();
    }
    for (;;) {
        imu.Read_All();
        float current_accel_x = imu.getAccelX();
        imu.Update_Angles(0.01f);
        volatile int dummy = myScanner.getDistanceAverage(1);
        //VofaUartSend();
       // IrRemote::GetInstance().ProcessAndPrint();


       // printf("distance = %d.%d cm\r\n", (int)dist, (int)(dist * 100) % 100);
        osDelay((10));
    }
}