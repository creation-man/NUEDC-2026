#ifndef LINE_TRACKER_HPP
#define LINE_TRACKER_HPP

#include "main.h"
#include <stdint.h>

namespace BSP {

    class LineTracker {
    public:
        static LineTracker& getInstance() {
            static LineTracker instance;
            return instance;
        }
        LineTracker(const LineTracker&)            = delete;
        LineTracker& operator=(const LineTracker&) = delete;

        // 初始化，传入四个引脚的 GPIO 配置
        void init(GPIO_TypeDef* ad0Port, uint16_t ad0Pin,
                  GPIO_TypeDef* ad1Port, uint16_t ad1Pin,
                  GPIO_TypeDef* ad2Port, uint16_t ad2Pin,
                  GPIO_TypeDef* outPort, uint16_t outPin);

        // 读取单个通道（channel: 1~8）
        // 返回 1 = 检测到黑线，0 = 未检测到
        uint8_t readChannel(uint8_t channel);

        // 读取全部 8 个通道，返回 8 位掩码
        // bit0 = 通道1, bit7 = 通道8
        uint8_t readAll();

        // 判断是否在线上（任意通道检测到黑线）
        bool isOnLine();

        // 获取线的偏移量（-7 ~ +7，0 = 居中）
        // 用于 PID 循迹控制
        int8_t getOffset();

    private:
        LineTracker() = default;

        void setAddress(uint8_t addr);

        GPIO_TypeDef* ad0Port_ = nullptr; uint16_t ad0Pin_ = 0;
        GPIO_TypeDef* ad1Port_ = nullptr; uint16_t ad1Pin_ = 0;
        GPIO_TypeDef* ad2Port_ = nullptr; uint16_t ad2Pin_ = 0;
        GPIO_TypeDef* outPort_ = nullptr; uint16_t outPin_ = 0;

        bool initialized_ = false;
    };

} // namespace BSP

#endif