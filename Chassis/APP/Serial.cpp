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
#include "OLED.hpp"
unsigned char VofaUartSendArr[32] = {0};
float dist=0;
float dummy =0;
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
    //超声波
    if(imu.Init() == 0) {
        // 初始化成功后，立刻进行校准
        // 此时不要碰板子！
        imu.Calibrate_Gyro();
    }
    //陀螺仪

    char str_buf[32]; // 用于存放格式化后的字符串
    float current_dist = 0.0f;
    // 获取单例引用（建议在函数顶部缓存，避免重复调用）
    auto& oled = OLED::Driver::getInstance();
    oled.init(&hi2c1);

    // 【静态显示部分】：在进入循环前画好不需要动的内容
    oled.clear();
    // 画一个标题，最后两个参数为 false，表示先存入缓存不立即刷新屏幕
    oled.puts((char*)"--- MONITOR ---", &Font_7x10, OLED::Color::White);
    // 刷新一次，让标题显示出来
    oled.refresh();
    for (;;) {
        imu.Read_All();
        float current_accel_x = imu.getAccelX();
        imu.Update_Angles(0.01f);

        // 使用滑动平均滤波（去除最大最小值后求平均），测量5次
        // 可选方案：
        // - getDistanceAverage(5): 简单平均，速度快
        // - getDistanceMedian(5): 中值滤波，抗干扰最强但稍慢
        // - getDistanceSmoothed(5): 去除极值后平均，平衡性能和稳定性（推荐）
         dummy = myScanner.getDistanceSmoothed(5);

        // 调试输出
        printf("Distance: %.2f cm\r\n", dummy);

        //VofaUartSend();
       // IrRemote::GetInstance().ProcessAndPrint();




     // 第一步：清空缓冲区
     oled.clear();

     // 重新画上静态标题（使用小字体，高度10像素）
     oled.gotoXY(0, 0);
     oled.puts((char*)"--- MONITOR ---", &Font_7x10, OLED::Color::White);

     // 第二步：格式化并画上动态数值（手动转换浮点数，避免sprintf的%f问题）
     int dist_int  = (int)dummy;
     int dist_frac = (int)((dummy - dist_int) * 100);
     if (dist_frac < 0) dist_frac = -dist_frac; // 处理负数情况
     sprintf(str_buf, "Dist:%d.%02d cm", dist_int, dist_frac);

     // 在 y=12 的位置绘制，使用小字体（7x10），确保不超出32像素高度
     oled.gotoXY(0, 12);
     oled.puts(str_buf, &Font_7x10, OLED::Color::White);

     // 第三步：一次性将缓存推送到屏幕（避免闪烁的关键）
     oled.refresh();

       // printf("distance = %d.%d cm\r\n", (int)dist, (int)(dist * 100) % 100);
        osDelay((10));
    }
}