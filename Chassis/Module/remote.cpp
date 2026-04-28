#include "remote.hpp"
#include <cstdio> // 用于 printf
uint8_t data[4] = {0};
// 计算低电平的持续时间
uint16_t IrRemote::MeasureLowTime() {
    uint16_t time = 0;
    while (HAL_GPIO_ReadPin(Irremote_GPIO_Port, Irremote_Pin) == GPIO_PIN_RESET) {
        if (time >= 500) break;
        time++;
        Delay::US(17);;
    }
    return time;
}

// 计算高电平的持续时间
uint16_t IrRemote::MeasureHighTime() {
    uint16_t time = 0;
    while (HAL_GPIO_ReadPin(Irremote_GPIO_Port, Irremote_Pin) == GPIO_PIN_SET) {
        if (time >= 250) break;
        time++;
        Delay::US(17);
    }
    return time;
}

// 获取红外遥控数据
uint8_t IrRemote::DecodeData() {
    uint16_t time;


    // 获取引导码
    time = MeasureLowTime();
    if (time < 400 || time >= 500) return ERROR_CODE;
    
    time = MeasureHighTime();
    if (time < 150 || time > 250) return ERROR_CODE;

    // 获取数据
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            time = MeasureLowTime();
            if (time < 20 || time > 35) return ERROR_CODE;

            time = MeasureHighTime(); 
            data[i] <<= 1;
            if (time >= 35) {
                data[i] |= 0x1; // 判定为数据 1
            }
        }
    }
    return data[2]; // 返回数据码
}

// 中断处理逻辑
void IrRemote::DecodeInterruptHandler() {
    uint8_t data = DecodeData();
    if (data != ERROR_CODE) {
        currentKey = data;
        // 如果需要长按连发功能，可以解除下面代码的注释
        // oldKey = data; 
    }
}

// 安全获取键值
uint8_t IrRemote::GetKey() {
    if (currentKey != ERROR_CODE) {
        uint8_t key = currentKey;
        currentKey = ERROR_CODE; // 读后清空
        return key;
    }
    return ERROR_CODE;
}

// 打印处理
void IrRemote::ProcessAndPrint() {
    uint8_t key = GetKey();
    if (key != ERROR_CODE) {
        printf("IrRemote key is : %X \r\n", key);
    }
}


// IrRemote.cpp 文件的末尾

extern "C" void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    // 这里的 Irremote_Pin 必须是在 main.h 或类中定义的引脚号
    if (GPIO_Pin == Irremote_Pin) {
        IrRemote::GetInstance().DecodeInterruptHandler();
    }
}