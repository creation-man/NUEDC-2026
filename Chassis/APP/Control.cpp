#include "Control.hpp"
#include "RobotHardware.hpp"
#include "cmsis_os.h"
#include "../Module/PIDcontrol.hpp"
#include "Vision.hpp"
#include "remote.hpp"
#include "../BSP/MPU6050.hpp"
#include "Tracker.hpp"
// --- PID 实例初始化 ---
// 既然你之前反馈给负数才正常，这里建议统一物理极性
// 如果发现电机反转，建议在 calculate 前面加负号，而不是给负的 Kp
static PID motor_pids[4] = {
    PID(10.0f, 0.6, 0, 3600.0f),
    PID(10.0f, 0.6, 0, 3600.0f),
    PID(10.0f, 0.6, 0, 3600.0f),
    PID(10.0f, 0.6, 0, 3600.0f)
};
static PID line_pid(70.0f, 0.0f, 2.0f, 300.0f); // Kp, Ki, Kd, max_out
// 基础直走速度（单位 RPM）
static int junction_count = 0;        // 路口计数
static bool junction_handled = false; // 防止同一条横线重复计数
static constexpr float BASE_SPEED = 70.0f;
const float RPM_FACTOR = 2.8846f;
// 在 last_valid_rpm 那行下面加
static float last_left_target  = 0.0f;
static float last_right_target = 0.0f;
// 最大合理转速阈值（根据你的电机规格设定，比如 500 RPM）
// 超过这个值的跳变会被认为是噪声直接滤除
const float MAX_REASONABLE_RPM = 1000.0f;
float raw_rpm[4];
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
uint8_t mask =0;
int8_t  offset=0;
float turn=0;


// 顶部添加
volatile bool car_started = false;
volatile bool key_flag = false;      // 中断置位
static uint32_t key_tick = 0;        // 消抖计时


enum class TrackState {
    STRAIGHT,    // 直线
    TURN_LEFT,   // 左转
    TURN_RIGHT,  // 右转
    SHARP_LEFT,   // ← 新增
    SHARP_RIGHT,  // ← 新增
    LOST,        // 丢线
    JUNCTION,    // 十字路口（全部触发）
};

TrackState detectState(uint8_t mask) {

    // 直角弯：实测值
    if (mask == 0xFF) return TrackState::JUNCTION;
    // 右直角：0xF8 = 1111 1000 左边5个亮
    // 左直角：0x1F = 0001 1111 右边5个亮
    if ((mask & 0xF8) == 0xF8) return TrackState::SHARP_RIGHT;
    if ((mask & 0x1F) == 0x1F) return TrackState::SHARP_LEFT;
    // 全部触发 = 十字路口


    // 没有触发 = 丢线
    if (mask == 0x00) return TrackState::LOST;



    // 只有左边触发（bit0~2）= 左转弯
    if ((mask & 0x07) && !(mask & 0xE0)) return TrackState::TURN_LEFT;

    // 只有右边触发（bit5~7）= 右转弯
    if ((mask & 0xE0) && !(mask & 0x07)) return TrackState::TURN_RIGHT;

    return TrackState::STRAIGHT;
}
extern "C" void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    // 这里的 Irremote_Pin 必须是在 main.h 或类中定义的引脚号
    if (GPIO_Pin == Irremote_Pin) {
        IrRemote::GetInstance().DecodeInterruptHandler();
    }
    if (GPIO_Pin == GPIO_PIN_3)
    {
        car_started = !car_started;  // 只切换标志，不做任何延时
    }

}
static TrackState last_sharp_dir = TrackState::STRAIGHT; // 正确写法
static bool is_sharp_turning = false;
extern "C" void Control(void *argument) {
    Robot_Hardware_Init();
    BSP::LineTracker::getInstance().init(
          GPIOF, GPIO_PIN_15,   // AD0
          GPIOF, GPIO_PIN_14,   // AD1
          GPIOF, GPIO_PIN_13,   // AD2
          GPIOG, GPIO_PIN_0    // OUT
      );
    auto& tracker = BSP::LineTracker::getInstance();
    for(;;) {
        // ★ 按键消抖处理（不阻塞）
        // if (key_flag)
        // {
        //     key_flag = false;
        //     uint32_t now = HAL_GetTick();
        //     if (now - key_tick > 200)  // 200ms 内只响应一次
        //     {
        //         key_tick = now;
        //         car_started = !car_started;
        //     }
        // }
        //
        // // ★ 启停守卫
        // if (!car_started) {
        //     for (int i = 0; i < 4; i++) {
        //         motors[i].setSpeed(0);
        //         motor_pids[i].setTarget(0.0f);
        //     }
        //     osDelay(20);
        //     continue;
        // }




         mask    = tracker.readAll();
         offset  = tracker.getOffset();
        TrackState state = detectState(mask);

        // --- 逻辑拦截器 ---
        // 如果正在强行转向，且中间传感器还没抓到新线，则保持转向状态
        if (is_sharp_turning) {
            // ✅ 新增：横线/十字路口时强制解锁，不能被上一次弯道状态覆盖
            if (mask == 0xFF) {
                is_sharp_turning = false;
                // state 已经在 detectState 里被判断为 JUNCTION，直接放行
            } else {
                bool middle_detect = (mask & 0x18) != 0;
                bool side_clear = false;

                if (last_sharp_dir == TrackState::SHARP_RIGHT) {
                    side_clear = (mask & 0xE0) == 0; // 右边侧重灯已熄灭
                } else {
                    side_clear = (mask & 0x07) == 0; // 左边侧重灯已熄灭
                }
                if (middle_detect && side_clear) {
                    is_sharp_turning = false;
                } else {
                    state = last_sharp_dir; // 强制保持转向状态
                }
            }
        }
        switch (state) {

            case TrackState::STRAIGHT:
            case TrackState::TURN_LEFT:
            case TrackState::TURN_RIGHT: {
                // 统一用 PID 处理，offset 自然反映转弯程度
                junction_handled = false;
                line_pid.setTarget(0.0f);
                 turn = line_pid.calculate((float)offset);

                motor_pids[0].setTarget(BASE_SPEED - turn); // 前左
                motor_pids[1].setTarget(BASE_SPEED - turn); // 后左
                motor_pids[2].setTarget(BASE_SPEED + turn); // 前右
                motor_pids[3].setTarget(BASE_SPEED + turn); // 后右
                break;
            }
            case TrackState::SHARP_LEFT: {
                //junction_handled = false;
                is_sharp_turning = true; // 必须在这里显式开启锁定！
                last_left_target  = 0.0f;
                last_sharp_dir = TrackState::SHARP_LEFT;

                motor_pids[0].setTarget(-450.0f);
                motor_pids[1].setTarget(-450.0f);
                motor_pids[2].setTarget(450);
                motor_pids[3].setTarget(450);
                break;
            }

            case TrackState::SHARP_RIGHT: {
                //junction_handled = false;
                is_sharp_turning = true; // 必须在这里显式开启锁定！

                last_right_target = 0.0f;
                last_sharp_dir = TrackState::SHARP_RIGHT;
                motor_pids[0].setTarget(450);
                motor_pids[1].setTarget(450);
                motor_pids[2].setTarget(-450.0f);
                motor_pids[3].setTarget(-450.0f);
                break;
            }
            case TrackState::JUNCTION: {

                // 防抖：只在第一次进入横线时计数
                if (!junction_handled) {
                    junction_count++;
                    junction_handled = true;
                }
                if (junction_count == 1) {
                motor_pids[0].setTarget(450);
                motor_pids[1].setTarget(450);
                motor_pids[2].setTarget(-450);
                motor_pids[3].setTarget(-450);
                } else if (junction_count >= 2) {
                    // 第二次横线：停止
                    // 第二次横线：彻底停死
                    for (int i = 0; i < 4; i++) {
                        motor_pids[i].setTarget(0.0f);
                        motor_pids[i].reset(); // ← 需要在 PID 类里加这个方法
                    }
                }
                break;
            }

            case TrackState::LOST: {
                //junction_handled = false;
                // 丢线：原地停止
                for (int i = 0; i < 4; i++)
                    motor_pids[i].setTarget(0.0f);
                break;
            }
        }
        uint8_t key = IrRemote::GetInstance().GetKey();
        // --- 第一步：获取反馈 ---
        for(int i = 0; i < 4; i++) {
            encoders[i].update();
        }

        // --- 第二步：获取并过滤转速 (解决“猛抽”的关键) ---

        // 原始读数获取

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
        // target_yaw =100; // 向右转90度
        // // --- 第三步：PID 计算 ---
        // motor_pids[0].setTarget(-target_yaw);
        // motor_pids[1].setTarget(-target_yaw);
        // motor_pids[2].setTarget(target_yaw);
        // motor_pids[3].setTarget(target_yaw);

        output_pwm_0 = motor_pids[0].calculate(current_rpm_0);
        output_pwm_1 = motor_pids[1].calculate(current_rpm_1);
        output_pwm_2 = motor_pids[2].calculate(current_rpm_2);
        output_pwm_3 = motor_pids[3].calculate(current_rpm_3);

        // --- 第四步：执行输出 ---
        motors[0].setSpeed(output_pwm_0);
        motors[1].setSpeed(output_pwm_1);
        motors[2].setSpeed(output_pwm_2);
        motors[3].setSpeed(output_pwm_3);

        osDelay(20);
    }
}