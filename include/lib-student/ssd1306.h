#ifndef __SSD1306_H__
#define __SSD1306_H__

#include "stm32f411xe.h"
#include "ecI2C2.h"
#include <stdint.h>

#define SSD1306_WIDTH   128
#define SSD1306_HEIGHT   64

typedef struct {
    uint8_t buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];
    uint8_t cursorX;
    uint8_t cursorY;
} SSD1306_t;

void ssd1306_init(SSD1306_t *oled);
void ssd1306_clear(SSD1306_t *oled);
void ssd1306_update(SSD1306_t *oled);

void ssd1306_setCursor(SSD1306_t *oled, uint8_t x, uint8_t y);
void ssd1306_drawChar(SSD1306_t *oled, char c);
void ssd1306_drawString(SSD1306_t *oled, char *str);

void ssd1306_face_happy(SSD1306_t *oled);
void ssd1306_face_normal(SSD1306_t *oled);
void ssd1306_face_angry(SSD1306_t *oled);
void ssd1306_drawPixel(SSD1306_t *oled, uint8_t x, uint8_t y, uint8_t color);

void ssd1306_eye_happy(SSD1306_t *oled);    // 웃는눈
void ssd1306_eye_normal(SSD1306_t *oled);   // 보통눈
void ssd1306_eye_sad(SSD1306_t *oled);      // 나쁜눈


#endif
