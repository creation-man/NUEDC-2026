#ifndef OLED_HPP
#define OLED_HPP

#include "main.h"
#include "i2c.h"
#include <string.h>
#include "OLED_fonts.hpp"

namespace OLED {

    constexpr uint16_t WIDTH  = 128;
    constexpr uint16_t HEIGHT = 32;

    enum class Color : uint8_t {
        Black = 0x00,
        White = 0x01
    };

    class Driver {
    public:
        // ── 获取单例 ──────────────────────────────────────
        static Driver& getInstance() {
            static Driver instance;  // C++11 保证线程安全的懒汉式初始化
            return instance;
        }

        // ── 禁止拷贝和赋值 ────────────────────────────────
        Driver(const Driver&)            = delete;
        Driver& operator=(const Driver&) = delete;

        // ── 初始化（必须第一个调用，传入 I2C 句柄）────────
        void init(I2C_HandleTypeDef *i2cHandle);

        // ── 公开接口 ──────────────────────────────────────
        void clear();
        void refresh();
        void drawString(char *data, uint8_t x, uint8_t y,
                        bool clear = false, bool refresh = false);
        void drawLine(char *data, uint8_t line,
                      bool clear = false, bool refresh = false);

        void fill(Color color);
        void updateScreen();
        void drawPixel(uint16_t x, uint16_t y, Color color);
        void gotoXY(uint16_t x, uint16_t y);
        char putc(char ch, FontDef_t *font, Color color);
        char puts(char *str, FontDef_t *font, Color color);
        void drawLinePx(uint16_t x0, uint16_t y0,
                        uint16_t x1, uint16_t y1, Color c);

    private:
        // ── 私有构造，外部无法直接创建实例 ───────────────
        Driver() = default;

        // ── 内部状态（完全封装，无需 extern）──────────────
        I2C_HandleTypeDef *hi2c      = nullptr;
        uint8_t  buffer[WIDTH * HEIGHT / 8] = {};
        uint16_t currentX    = 0;
        uint16_t currentY    = 0;
        bool     inverted    = false;
        bool     initialized = false;

        static constexpr uint8_t ADDRESS = (0x3C << 1);

        // ── 底层私有方法 ──────────────────────────────────
        void writeCommand(uint8_t cmd);
        void writeData(uint8_t data);
        void writeByte(uint8_t addr, uint8_t data);
    };

} // namespace OLED

#endif // OLED_HPP