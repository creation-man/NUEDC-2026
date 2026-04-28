#include "uitrasonic.hpp"
#include "FreeRTOS.h"
#include "task.h"
#include "Delay.hpp"
namespace Sensor {

// ==========================================
// 基类实现
// ==========================================
float DistanceSensor::getDistanceAverage(uint8_t times) {
    float sum = 0.0f;
    uint8_t validSamples = 0;

    for (uint8_t i = 0; i < times; i++) {
        float d = this->measureOnce(); // 触发多态，调用具体传感器的测距方法
        if (d > 0.1f) { // 过滤掉 0.0f 等异常无效值
            sum += d;
            validSamples++;
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // 防止超声波余波干扰
    }

    // 严谨的除零保护（强制类型转换保证浮点运算）
    return (validSamples > 0) ? (sum / static_cast<float>(validSamples)) : 0.0f;
}

// ==========================================
// HCSR04 实现
// ==========================================

// 懒汉单例初始化：只有第一次调用 getInstance 时才会执行构造函数
    HCSR04& HCSR04::getInstance(GPIO_TypeDef* trigPort, uint16_t trigPin,
                                GPIO_TypeDef* echoPort, uint16_t echoPin,
                                TIM_HandleTypeDef* htim) {
    // 使用静态指针，确保对象在 RAM 的 .bss 或 .data 段
    static HCSR04* instancePtr = nullptr;

    if (instancePtr == nullptr) {
        // 第一次调用时，在 RAM 中分配空间（由于是单例，这样写很安全）
        static HCSR04 actualInstance(trigPort, trigPin, echoPort, echoPin, htim);
        instancePtr = &actualInstance;
    }

    // 每次调用都强制刷新硬件句柄，防止空指针
    if (trigPort != nullptr) {
        instancePtr->_trigPort = trigPort;
        instancePtr->_trigPin  = trigPin;
        instancePtr->_echoPort = echoPort;
        instancePtr->_echoPin  = echoPin;
        instancePtr->_htim     = htim;
    }

    return *instancePtr;
}

HCSR04::HCSR04(GPIO_TypeDef* trigPort, uint16_t trigPin,
               GPIO_TypeDef* echoPort, uint16_t echoPin,
               TIM_HandleTypeDef* htim)
    : _trigPort(trigPort), _trigPin(trigPin),
      _echoPort(echoPort), _echoPin(echoPin),
      _htim(htim), _overflowCount(0), _isRanging(false) {}

void HCSR04::onTimerOverflow() {

        _overflowCount++;

}
float HCSR04::measureOnce() {
    uint32_t timeout = 0;

    // 1. 发送触发信号 (10us 脉冲)
    HAL_GPIO_WritePin(_trigPort, _trigPin, GPIO_PIN_SET);
    for(volatile int i = 0; i < 720; i++); // 确保这个微秒延迟是准确的
    HAL_GPIO_WritePin(_trigPort, _trigPin, GPIO_PIN_RESET);

    // 2. 等待 Echo 变高（带超时）
    timeout = 0;
    while (HAL_GPIO_ReadPin(_echoPort, _echoPin) == GPIO_PIN_RESET) {
        if (++timeout > 100000) return 0.0f; // 没等到起始信号，直接退出
    }

    // 3. 开启定时器并清零计数
    __HAL_TIM_SET_COUNTER(_htim, 0);
    // 这里注意：如果你用的是外部全局变量，在这里清零它
    _overflowCount = 0;

    // 4. 等待 Echo 变低（带超时）
    timeout = 0;
    while (HAL_GPIO_ReadPin(_echoPort, _echoPin) == GPIO_PIN_SET) {
        if (++timeout > 1000000) break; // 没等到结束信号，防止卡死
    }

    // 5. 读取数据（进入临界区防止读取时被中断修改）
    vPortEnterCritical();
    uint32_t count = _overflowCount;
    uint32_t ticks = __HAL_TIM_GET_COUNTER(_htim);
    vPortExitCritical();

    // 6. 计算距离 (避开直接除法，先转为整数再处理)
    float totalUs = (float)ticks + (count * 65536.0f);
    return totalUs / 58.0f;
}

} // namespace Sensor