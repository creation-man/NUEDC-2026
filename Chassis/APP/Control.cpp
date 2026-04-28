#include "Control.hpp"
#include "RobotHardware.hpp"
#include "cmsis_os.h"
#include "PIDcontrol.hpp"
#include "Vision.hpp"
#include "remote.hpp"
#include "MPU6050.hpp"

// --- PID 实例初始化 ---
// 既然你之前反馈给负数才正常，这里建议统一物理极性
// 如果发现电机反转，建议在 calculate 前面加负号，而不是给负的 Kp
static PID motor_pids[4] = {
    PID(1.0f, 0.6, 0, 3600.0f),
    PID(1.0f, 0.6, 0, 3600.0f),
    PID(1.0f, 0.6, 0, 3600.0f),
    PID(1.0f, 0.6, 0, 3600.0f)
};

const float RPM_FACTOR = 5.7692f;
// 最大合理转速阈值（根据你的电机规格设定，比如 500 RPM）
// 超过这个值的跳变会被认为是噪声直接滤除
const float MAX_REASONABLE_RPM = 1000.0f;

float current_rpm_0=0;
float current_rpm_1=0;
float current_rpm_2=0;
float current_rpm_3=0;
float output_pwm_0=0;
float output_pwm_1=0;
float output_pwm_2=0;
float output_pwm_3=0;
int vL, vR;
float target=0;
float target_yaw=0;
// 用于滤波的上一时刻转速
static float last_valid_rpm[4] = {0};

extern "C" void Control(void *argument) {
    Robot_Hardware_Init();


    for(;;) {
        uint8_t key = IrRemote::GetInstance().GetKey();
        // --- 第一步：获取反馈 ---
        for(int i = 0; i < 4; i++) {
            encoders[i].update();
        }

        // --- 第二步：获取并过滤转速 (解决“猛抽”的关键) ---

        // 原始读数获取
        float raw_rpm[4];
        raw_rpm[0] = (float)encoders[2].getSpeed() * RPM_FACTOR;
        raw_rpm[1] = (float)encoders[0].getSpeed() * RPM_FACTOR;
        raw_rpm[2] = -(float)encoders[3].getSpeed() * RPM_FACTOR;
        raw_rpm[3] = -(float)encoders[1].getSpeed() * RPM_FACTOR;

        // 异常值过滤：如果瞬间跳变太大，保留上一时刻的值
        for(int i = 0; i < 4; i++) {
            if (raw_rpm[i] > MAX_REASONABLE_RPM || raw_rpm[i] < -MAX_REASONABLE_RPM) {
                // 此时 raw_rpm[i] 维持 last_valid_rpm[i]，不更新，防止 PID 突跳
                raw_rpm[i] = last_valid_rpm[i];
            } else {
                last_valid_rpm[i] = raw_rpm[i];
            }
        }

        current_rpm_0 = raw_rpm[0];
        current_rpm_1 = raw_rpm[1];
        current_rpm_2 = raw_rpm[2];
        current_rpm_3 = raw_rpm[3];

        vision.getTargetSpeeds(vL, vR); // 获取解析好的速度，视觉的
        if (key != IrRemote::ERROR_CODE) {
            // 假设你通过串口测出右键的代码是 0x44
            if (key == 0x60) {

                // --- 陀螺仪转向逻辑 ---
                // 这里调用你之前校准过的 IMU 逻辑
                 target = 90.0f;

                // 使用你提到的 PID 或双环控制转向
                // chassis.setTargetAngle(target_yaw);
            }
        }
        target_yaw =(target-imu.yaw) * 2; // 向右转90度
        // --- 第三步：PID 计算 ---
        motor_pids[0].setTarget(-target_yaw);
        motor_pids[1].setTarget(-target_yaw);
        motor_pids[2].setTarget(target_yaw);
        motor_pids[3].setTarget(target_yaw);

        output_pwm_0 = motor_pids[0].calculate(current_rpm_0);
        output_pwm_1 = motor_pids[1].calculate(current_rpm_1);
        output_pwm_2 = motor_pids[2].calculate(current_rpm_2);
        output_pwm_3 = motor_pids[3].calculate(current_rpm_3);

        // --- 第四步：执行输出 ---
        motors[0].setSpeed(output_pwm_0);
        motors[1].setSpeed(output_pwm_1);
        motors[2].setSpeed(output_pwm_2);
        motors[3].setSpeed(output_pwm_3);

        osDelay(10);
    }
}