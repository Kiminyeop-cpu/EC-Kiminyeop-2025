#ifndef __SSD1306_2_H__
#define __SSD1306_2_H__

#include "stm32f411xe.h"
#include "ecI2C2.h"
#include <stdint.h>

#define SSD1306_2_WIDTH   128
#define SSD1306_2_HEIGHT   64

// ======================================
// OLED2 구조체 정의
// ======================================
typedef struct {
    uint8_t buffer[SSD1306_2_WIDTH * SSD1306_2_HEIGHT / 8];
    uint8_t cursorX;
    uint8_t cursorY;
} SSD1306_2_t;

// ======================================
// 기본 제어 함수
// ======================================
void ssd1306_2_init(SSD1306_2_t *oled);       // ✅ I2C2_init() 사용 (PC5=SCL, PC4=SDA)
void ssd1306_2_clear(SSD1306_2_t *oled);
void ssd1306_2_update(SSD1306_2_t *oled);

// ======================================
// 텍스트 / 픽셀 출력
// ======================================
void ssd1306_2_setCursor(SSD1306_2_t *oled, uint8_t x, uint8_t y);
void ssd1306_2_drawChar(SSD1306_2_t *oled, char c);
void ssd1306_2_drawString(SSD1306_2_t *oled, char *str);
void ssd1306_2_drawPixel(SSD1306_2_t *oled, uint8_t x, uint8_t y, uint8_t color);

// ======================================
// 감정(비트) 출력 함수
// ======================================
void ssd1306_2_eye_happy(SSD1306_2_t *oled);
void ssd1306_2_eye_normal(SSD1306_2_t *oled);
void ssd1306_2_eye_sad(SSD1306_2_t *oled);

#endif
