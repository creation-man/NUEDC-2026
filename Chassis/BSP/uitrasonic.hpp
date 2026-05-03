#ifndef ULTRASONIC_HPP
#define ULTRASONIC_HPP

#include "main.h"

namespace Sensor {

    // ==========================================
    // 1. 抽象基类：所有距离传感器的通用接口
    // ==========================================
    class DistanceSensor {
    public:
        virtual ~DistanceSensor() = default;

        // 纯虚函数：子类必须实现单次测量
        virtual float measureOnce() = 0;

        // 虚函数：通用的均值滤波算法（子类自动继承，也可重写）
        virtual float getDistanceAverage(uint8_t times = 5);

        // 中值滤波：去除异常值
        virtual float getDistanceMedian(uint8_t times = 5);

        // 滑动平均滤波：平滑数据
        virtual float getDistanceSmoothed(uint8_t times = 5);
    };

    // ==========================================
    // 2. HC-SR04 具体实现（Meyers 单例模式）
    // ==========================================
    class HCSR04 : public DistanceSensor {
    public:
        // 获取唯一实例的静态方法（干掉 extern 的核心）
        static HCSR04& getInstance(GPIO_TypeDef* trigPort = nullptr, uint16_t trigPin = 0,
                                   GPIO_TypeDef* echoPort = nullptr, uint16_t echoPin = 0,
                                   TIM_HandleTypeDef* htim = nullptr);

        // 禁用拷贝和赋值，确保实例的唯一性
        HCSR04(const HCSR04&) = delete;
        HCSR04& operator=(const HCSR04&) = delete;

        // 实现基类接口
        float measureOnce() override;

        // 供定时器中断调用的回调
        void onTimerOverflow();

    private:
        // 构造函数私有化
        HCSR04(GPIO_TypeDef* trigPort, uint16_t trigPin,
               GPIO_TypeDef* echoPort, uint16_t echoPin,
               TIM_HandleTypeDef* htim);

        GPIO_TypeDef* _trigPort;
        uint16_t      _trigPin;
        GPIO_TypeDef* _echoPort;
        uint16_t      _echoPin;
        TIM_HandleTypeDef* _htim;

        volatile uint32_t _overflowCount;
        volatile bool     _isRanging;

        // --- 核心优化：消灭魔法数字 ---
        static constexpr float    SPEED_OF_SOUND_DIVISOR = 58.0f; // 声速换算常数
        static constexpr uint32_t MAX_ECHO_TIMEOUT       = 100000; // 等待高电平超时
        static constexpr uint32_t MAX_RANGING_TICKS      = 3000;  // 最大测距溢出次数(防卡死)
    };

} // namespace Sensor

#endif