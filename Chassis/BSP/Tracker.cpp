#include "Tracker.hpp"

namespace BSP {

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

    // 根据地址表设置 AD0/AD1/AD2（通道从 0 开始）
    void LineTracker::setAddress(uint8_t addr) {
        HAL_GPIO_WritePin(ad0Port_, ad0Pin_, (addr & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HAL_GPIO_WritePin(ad1Port_, ad1Pin_, (addr & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HAL_GPIO_WritePin(ad2Port_, ad2Pin_, (addr & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }

    uint8_t LineTracker::readChannel(uint8_t channel) {
        if (!initialized_ || channel < 1 || channel > 8) return 0;

        setAddress(channel - 1);        // 地址从 0 开始，通道从 1 开始
        HAL_Delay(1);                   // 等待信号稳定（可根据实际情况调整）

        // OUT 引脚：检测到黑线通常输出低电平
        return (HAL_GPIO_ReadPin(outPort_, outPin_) == GPIO_PIN_RESET) ? 1 : 0;
    }

    uint8_t LineTracker::readAll() {
        uint8_t result = 0;
        for (uint8_t i = 0; i < 8; i++) {
            if (readChannel(i + 1)) {
                result |= (1 << i);
            }
        }
        return result;
    }

    bool LineTracker::isOnLine() {
        return readAll() != 0;
    }

    int8_t LineTracker::getOffset() {
        // 通道位置权重：左边为负，右边为正
        // 通道排列：1(最左) ~ 8(最右)
        static const int8_t weights[8] = {-7, -5, -3, -1, 1, 3, 5, 7};

        int16_t weightedSum = 0;
        uint8_t count       = 0;

        for (uint8_t i = 0; i < 8; i++) {
            if (readChannel(i + 1)) {
                weightedSum += weights[i];
                count++;
            }
        }

        if (count == 0) return 0;       // 没检测到线，返回 0
        return (int8_t)(weightedSum / count);
    }

} // namespace BSP
