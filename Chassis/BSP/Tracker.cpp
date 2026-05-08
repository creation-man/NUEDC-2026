#include "Tracker.hpp"

namespace BSP {
    uint8_t g_tracker_mask = 0;
    int8_t  g_tracker_offset = 0;

    void LineTracker::init(GPIO_TypeDef* ad0Port, uint16_t ad0Pin,
                           GPIO_TypeDef* ad1Port, uint16_t ad1Pin,
                           GPIO_TypeDef* ad2Port, uint16_t ad2Pin,
                           GPIO_TypeDef* outPort, uint16_t outPin) {
        ad0Port_ = ad0Port; ad0Pin_ = ad0Pin;
        ad1Port_ = ad1Port; ad1Pin_ = ad1Pin;
        ad2Port_ = ad2Port; ad2Pin_ = ad2Pin;
        outPort_ = outPort; outPin_ = outPin;
        initialized_ = true;
    }

    void LineTracker::setAddress(uint8_t addr) {
        HAL_GPIO_WritePin(ad0Port_, ad0Pin_, (addr & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HAL_GPIO_WritePin(ad1Port_, ad1Pin_, (addr & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HAL_GPIO_WritePin(ad2Port_, ad2Pin_, (addr & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }

    uint8_t LineTracker::readChannel(uint8_t channel) {
       // if (!initialized_ || channel < 1 || channel > 8) return 0;

        setAddress(channel - 1);

        // --- 优化 1: 将 2ms 延迟改为微秒级 ---
        // 如果你的循迹模块响应够快，甚至可以只用几个 __NOP();
        for(volatile int i=0; i<100; i++);

        // --- 优化 2: 注意极性 ---
        // 你注释说检测到黑线通常是低电平，但代码里写 SET 为 1。
        // 如果发现读反了，把 GPIO_PIN_SET 改成 GPIO_PIN_RESET
        return (HAL_GPIO_ReadPin(outPort_, outPin_) == GPIO_PIN_SET) ? 1 : 0;
    }

    uint8_t LineTracker::readAll() {
        uint8_t temp_mask = 0; // 使用局部变量存储本次扫描结果
        for (uint8_t i = 0; i < 8; i++) {
            if (readChannel(i + 1)) {
                temp_mask |= (1 << i);
            }
        }
        g_tracker_mask = temp_mask; // 一次性更新全局变量
        return g_tracker_mask;
    }

    int8_t LineTracker::getOffset() {
        // --- 优化 3: 直接利用已经读好的 g_tracker_mask，不再重复扫描硬件 ---
        static const int8_t weights[8] = {-10, -7, -4, -1, 1, 4, 7, 10};
        int16_t weightedSum = 0;
        uint8_t count = 0;

        for (uint8_t i = 0; i < 8; i++) {
            // 检查 g_tracker_mask 的第 i 位
            if (g_tracker_mask & (1 << i)) {
                weightedSum += weights[i];
                count++;
            }
        }

        if (count == 0) {
            g_tracker_offset = 0;
        } else {
            g_tracker_offset = (int8_t)(weightedSum / count);
        }
        return g_tracker_offset;
    }

} // namespace BSP