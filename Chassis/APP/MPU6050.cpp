#include "MPU6050.hpp"
#include "cmath"


// 构造函数
MPU6050::MPU6050(I2C_HandleTypeDef *hi2c, uint8_t devAddr) {
    _hi2c = hi2c;
    _devAddr = (devAddr << 1); // 自动处理 STM32 的 8 位地址对齐
}

// 辅助方法：写寄存器
HAL_StatusTypeDef MPU6050::writeReg(uint8_t reg, uint8_t data) {
    return HAL_I2C_Mem_Write(_hi2c, _devAddr, reg, 1, &data, 1, 100);
}

// 辅助方法：读多个寄存器
HAL_StatusTypeDef MPU6050::readRegs(uint8_t reg, uint8_t *data, uint16_t size) {
    return HAL_I2C_Mem_Read(_hi2c, _devAddr, reg, 1, data, size, 100);
}

uint8_t MPU6050::Init() {
    uint8_t check;

    // 1. 检查 WHO_AM_I
    if (readRegs(0x75, &check, 1) != HAL_OK || check != 0x68) {
        return 1; 
    }

    // 2. 唤醒、设置量程
    writeReg(0x6B, 0x00); // 唤醒
    writeReg(0x1B, 0x08); // 陀螺仪 ±500°/s
    writeReg(0x1C, 0x08); // 加速度 ±4g

    return 0; 
}

void MPU6050::Read_All() {
    uint8_t raw[14];
    if (readRegs(0x3B, raw, 14) == HAL_OK) {
        // 数据拼接
        int16_t ax = (int16_t)(raw[0] << 8 | raw[1]);
        int16_t ay = (int16_t)(raw[2] << 8 | raw[3]);
        int16_t az = (int16_t)(raw[4] << 8 | raw[5]);
        int16_t gx = (int16_t)(raw[8] << 8 | raw[9]);
        int16_t gy = (int16_t)(raw[10] << 8 | raw[11]);
        int16_t gz = (int16_t)(raw[12] << 8 | raw[13]);

        // 转换为物理单位 (±4g -> 8192, ±500deg/s -> 65.5)
        Accel_X = ax / 8192.0f;
        Accel_Y = ay / 8192.0f;
        Accel_Z = az / 8192.0f;
        Gyro_X = gx / 65.5f;
        Gyro_Y = gy / 65.5f;
        Gyro_Z = gz / 58.22f;
    }
}

void MPU6050::Update_Angles(float dt) {
    // 1. 先读取最新数据
    Read_All();

    // 2. 加速度计计算静态角度 (三角函数换算)
    // pitch: 绕 Y 轴旋转；roll: 绕 X 轴旋转
    float accel_pitch = atan2(Accel_X, sqrt(Accel_Y * Accel_Y + Accel_Z * Accel_Z)) * 57.29578f;
    float accel_roll = atan2(Accel_Y, Accel_Z) * 57.29578f;

    // 3. 互补滤波融合
    // 权重分配：0.98 给陀螺仪积分，0.02 给加速度计补偿偏移
    // 57.29578 是弧度转角度 (180/PI)
    pitch = 0.98f * (pitch + Gyro_Y * dt) + 0.02f * accel_pitch;
    roll = 0.98f * (roll + Gyro_X * dt) + 0.02f * accel_roll;

    // 4. 关于 Yaw (偏航角)
    // MPU6050 没有磁力计，Z 轴陀螺仪只能积分，无法靠加速度纠偏，会随时间漂移
    //每次积分前，剔除零误差

    yaw += (Gyro_Z - gyro_z_offset) * dt;
}

// 新增校准函数实现
void MPU6050::Calibrate_Gyro() {
    float sum_z = 0.0f;
    uint16_t sample_count = 1000; // 采样500次求平均

    // 假设系统滴答定时器已经启动，这里用 HAL_Delay 即可
    for(uint16_t i = 0; i < sample_count; i++) {
        Read_All(); // 读一次原始数据
        sum_z += Gyro_Z;
        HAL_Delay(2); // 等待 2ms 再读下一次
    }

    // 计算出静态零偏
    gyro_z_offset = sum_z / sample_count;
}