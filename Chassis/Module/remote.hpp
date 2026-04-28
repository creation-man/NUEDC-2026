#ifndef __IRREMOTE_HPP_
#define __IRREMOTE_HPP_

#include "main.h"    // 包含 HAL 库和引脚定义
#include "Delay.hpp"   // 你的微秒延时函数

class IrRemote {
public:
    // 定义错误码，避免使用魔术数字
    static constexpr uint8_t ERROR_CODE = 0xFF;

    // 单例模式获取实例 (推荐嵌入式硬件驱动使用单例)
    static IrRemote& GetInstance() {
        static IrRemote instance;
        return instance;
    }

    // 在中断回调中被调用
    void DecodeInterruptHandler();

    // 轮询获取当前按下的按键值，获取后自动清除
    uint8_t GetKey();

    // 检查是否有按键按下并打印 (等同于原来的 Print_Irrmote)
    void ProcessAndPrint();

private:
    // 私有化构造函数，防止外部实例化
    IrRemote() : currentKey(ERROR_CODE), oldKey(ERROR_CODE) {}

    // 核心解码变量
    volatile uint8_t currentKey;
    volatile uint8_t oldKey;

    // 底层测量与解码函数，不对外暴露
    uint16_t MeasureLowTime();
    uint16_t MeasureHighTime();
    uint8_t  DecodeData();
};

#endif // __IRREMOTE_HPP_