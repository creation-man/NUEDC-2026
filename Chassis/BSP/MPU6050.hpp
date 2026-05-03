#ifndef MPU6050_HPP
#define MPU6050_HPP

#include "main.h"
#include "i2c.h"
class MPU6050 {
public:
    // 构造函数：初始化时绑定硬件 I2C 句柄
    MPU6050(I2C_HandleTypeDef *hi2c, uint8_t devAddr = 0x68);

    // 核心操作方法
    uint8_t Init();
    void Read_All();
    void Calibrate_Gyro(); // 新增：陀螺仪校准函数
    // 数据获取接口（封装属性）
    float getAccelX() const { return Accel_X; }
    float getAccelY() const { return Accel_Y; }
    float getAccelZ() const { return Accel_Z; }
    float getGyroX() const { return Gyro_X; }
    float getGyroY() const { return Gyro_Y; }
    float getGyroZ() const { return Gyro_Z; }
    void Update_Angles(float dt); // dt 为两次计算的时间间隔，单位秒

    float pitch, roll, yaw; // 解算后的欧拉角
private:
    I2C_HandleTypeDef *_hi2c; // 硬件句柄指针
    uint16_t _devAddr;        // 移位后的 8 位地址

    // 原始数据转换后的物理量
    float Accel_X, Accel_Y, Accel_Z;
    float Gyro_X, Gyro_Y, Gyro_Z;
    float gyro_z_offset;  // 新增：Z轴零偏值
    // 内部私有方法：简化寄存器读写
    HAL_StatusTypeDef writeReg(uint8_t reg, uint8_t data);
    HAL_StatusTypeDef readRegs(uint8_t reg, uint8_t *data, uint16_t size);
};
extern MPU6050 imu;
#endif