#include "OLED.hpp"

namespace OLED {

// ── 底层 I2C 写入（不再依赖任何全局变量）─────────────
void Driver::writeByte(uint8_t addr, uint8_t data) {
    HAL_I2C_Mem_Write(hi2c, ADDRESS, addr, I2C_MEMADD_SIZE_8BIT,
                      &data, 1, HAL_MAX_DELAY);
}

void Driver::writeCommand(uint8_t cmd)  { writeByte(0x00, cmd); }
void Driver::writeData   (uint8_t data) { writeByte(0x40, data); }

// ── 初始化 ────────────────────────────────────────────
void Driver::init(I2C_HandleTypeDef *i2cHandle) {
    hi2c = i2cHandle;  // 在这里注入句柄，替代 extern

    HAL_Delay(100);

    writeCommand(0xAE); writeCommand(0xA6);
    writeCommand(0xD5); writeCommand(0x80);
    writeCommand(0xA8); writeCommand(0x1F);
    writeCommand(0xD3); writeCommand(0x00);
    writeCommand(0x40);
    writeCommand(0x8D); writeCommand(0x14);
    writeCommand(0x20); writeCommand(0x02);
    writeCommand(0xA1); writeCommand(0xC8);
    writeCommand(0xDA); writeCommand(0x02);
    writeCommand(0x81); writeCommand(0xCF);
    writeCommand(0xD9); writeCommand(0xF1);
    writeCommand(0xDB); writeCommand(0x40);
    writeCommand(0x2E); writeCommand(0xA4);
    writeCommand(0xA6); writeCommand(0xAF);

    fill(Color::Black);
    updateScreen();

    currentX    = 0;
    currentY    = 0;
    initialized = true;
}

// ── 屏幕刷新 ──────────────────────────────────────────
void Driver::updateScreen() {
    for (uint8_t page = 0; page < 8; ++page) {
        writeCommand(0xB0 + page);
        writeCommand(0x00);
        writeCommand(0x10);
        HAL_I2C_Mem_Write(hi2c, ADDRESS, 0x40, I2C_MEMADD_SIZE_8BIT,
                          &buffer[WIDTH * page], WIDTH, HAL_MAX_DELAY);
    }
}

void Driver::fill(Color color) {
    memset(buffer, (color == Color::Black) ? 0x00 : 0xFF, sizeof(buffer));
}

void Driver::drawPixel(uint16_t x, uint16_t y, Color color) {
    if (x >= WIDTH || y >= HEIGHT) return;

    if (inverted)
        color = (color == Color::White) ? Color::Black : Color::White;

    if (color == Color::White)
        buffer[x + (y / 8) * WIDTH] |=  (1 << (y % 8));
    else
        buffer[x + (y / 8) * WIDTH] &= ~(1 << (y % 8));
}

void Driver::gotoXY(uint16_t x, uint16_t y) {
    currentX = x;
    currentY = y;
}

char Driver::putc(char ch, FontDef_t *font, Color color) {
    if (WIDTH  <= (currentX + font->FontWidth) ||
        HEIGHT <= (currentY + font->FontHeight))
        return 0;

    for (uint32_t i = 0; i < font->FontHeight; ++i) {
        uint32_t b = font->data[(ch - 32) * font->FontHeight + i];
        for (uint32_t j = 0; j < font->FontWidth; ++j) {
            drawPixel(currentX + j, currentY + i,
                      (b << j) & 0x8000 ? color
                                        : (color == Color::White ? Color::Black : Color::White));
        }
    }

    currentX += font->FontWidth;
    return ch;
}

char Driver::puts(char *str, FontDef_t *font, Color color) {
    while (*str) {
        if (putc(*str, font, color) != *str)
            return *str;
        ++str;
    }
    return *str;
}

void Driver::drawLinePx(uint16_t x0, uint16_t y0,
                        uint16_t x1, uint16_t y1, Color c) {
    if (x0 >= WIDTH)  x0 = WIDTH  - 1;
    if (x1 >= WIDTH)  x1 = WIDTH  - 1;
    if (y0 >= HEIGHT) y0 = HEIGHT - 1;
    if (y1 >= HEIGHT) y1 = HEIGHT - 1;

    int16_t dx = (x0 < x1) ? (x1 - x0) : (x0 - x1);
    int16_t dy = (y0 < y1) ? (y1 - y0) : (y0 - y1);
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = ((dx > dy) ? dx : -dy) / 2;

    if (dx == 0) {
        if (y1 < y0) { uint16_t t = y0; y0 = y1; y1 = t; }
        for (uint16_t i = y0; i <= y1; ++i) drawPixel(x0, i, c);
        return;
    }
    if (dy == 0) {
        if (x1 < x0) { uint16_t t = x0; x0 = x1; x1 = t; }
        for (uint16_t i = x0; i <= x1; ++i) drawPixel(i, y0, c);
        return;
    }
    while (true) {
        drawPixel(x0, y0, c);
        if (x0 == x1 && y0 == y1) break;
        int16_t e2 = err;
        if (e2 > -dx) { err -= dy; x0 += sx; }
        if (e2 <  dy) { err += dx; y0 += sy; }
    }
}

// ── 高层接口 ──────────────────────────────────────────
void Driver::clear()   { fill(Color::Black); }
void Driver::refresh() { updateScreen(); }

void Driver::drawString(char *data, uint8_t x, uint8_t y,
                        bool doClear, bool doRefresh) {
    if (doClear)   clear();
    gotoXY(x, y);
    puts(data, &Font_7x10, Color::White);
    if (doRefresh) refresh();
}

void Driver::drawLine(char *data, uint8_t line,
                      bool doClear, bool doRefresh) {
    if (line >= 1 && line <= 3)
        drawString(data, 0, static_cast<uint8_t>(10 * (line - 1)),
                   doClear, doRefresh);
}

} // namespace OLED