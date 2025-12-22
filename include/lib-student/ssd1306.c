#include "ssd1306.h"
#include <string.h>

// =====================================================
// Eye size & position (OLED2와 완전히 동일 개념)
// =====================================================
#define EYE_W  96
#define EYE_H  24

#define EYE_OX 16
#define EYE_OY 20


// =====================================================
// Low-level I2C (OLED1 = I2C1)
// =====================================================
static void oled_cmd(uint8_t cmd){ I2C1_writeCmd(cmd); }
static void oled_data(uint8_t data){ I2C1_writeData(data); }


// =====================================================
// OLED Init
// =====================================================
void ssd1306_init(SSD1306_t *oled){
    I2C1_init();   // PB6=SCL, PB7=SDA

    oled->cursorX = 0;
    oled->cursorY = 0;

    oled_cmd(0xAE);
    oled_cmd(0x20);
    oled_cmd(0x10);
    oled_cmd(0xB0);
    oled_cmd(0xC8);
    oled_cmd(0x00);
    oled_cmd(0x10);
    oled_cmd(0x40);
    oled_cmd(0x81);
    oled_cmd(0x7F);
    oled_cmd(0xA1);
    oled_cmd(0xA6);
    oled_cmd(0xA8);
    oled_cmd(0x3F);
    oled_cmd(0xA4);
    oled_cmd(0xD3);
    oled_cmd(0x00);
    oled_cmd(0xD5);
    oled_cmd(0x80);
    oled_cmd(0xD9);
    oled_cmd(0xF1);
    oled_cmd(0xDA);
    oled_cmd(0x12);
    oled_cmd(0xDB);
    oled_cmd(0x40);
    oled_cmd(0x8D);
    oled_cmd(0x14);
    oled_cmd(0xAF);

    ssd1306_clear(oled);
    ssd1306_update(oled);
}


// =====================================================
// Buffer Clear
// =====================================================
void ssd1306_clear(SSD1306_t *oled){
    memset(oled->buffer, 0x00, sizeof(oled->buffer));
}


// =====================================================
// Display Update
// =====================================================
void ssd1306_update(SSD1306_t *oled){
    for(uint8_t page = 0; page < 8; page++){
        oled_cmd(0xB0 + page);
        oled_cmd(0x00);
        oled_cmd(0x10);
        I2C1_writeMulti(0x40, &oled->buffer[page * 128], 128);
    }
}


// =====================================================
// Cursor
// =====================================================
void ssd1306_setCursor(SSD1306_t *oled, uint8_t x, uint8_t y){
    oled->cursorX = x;
    oled->cursorY = y;
}


// =====================================================
// Character Draw
// =====================================================
extern const uint8_t Font5x7[][5];

void ssd1306_drawChar(SSD1306_t *oled, char c){
    if(c < 32) c = 32;
    uint8_t idx = c - 32;

    for(int i=0; i<5; i++){
        oled->buffer[oled->cursorY * 128 + oled->cursorX + i] = Font5x7[idx][i];
    }

    oled->cursorX += 6;
}

void ssd1306_drawString(SSD1306_t *oled, char *str){
    while(*str){
        ssd1306_drawChar(oled, *str++);
    }
}


// =====================================================
// Pixel Draw
// =====================================================
void ssd1306_drawPixel(SSD1306_t *oled, uint8_t x, uint8_t y, uint8_t color){
    if(x >= 128 || y >= 64) return;

    uint16_t index = x + (y / 8) * 128;

    if(color)
        oled->buffer[index] |= (1 << (y % 8));
    else
        oled->buffer[index] &= ~(1 << (y % 8));
}


// =====================================================
// Eye — HAPPY (＾)
// =====================================================
void ssd1306_eye_happy(SSD1306_t *oled){
    ssd1306_2_clear(oled);

    int ox = EYE_OX;
    int oy = EYE_OY;

    for(int y = 0; y < EYE_H; y++){
        for(int x = 0; x < EYE_W; x++){
            uint8_t pixel = 0;

            // 🔥 y → (EYE2_H - y)
            if(x == (EYE_W/2 - (EYE_H - y)/2)) pixel = 1;
            if(x == (EYE_W/2 + (EYE_H - y)/2)) pixel = 1;

            if(pixel)
                ssd1306_2_drawPixel(oled, ox + x, oy + y, 1);
        }
    }

    ssd1306_2_update(oled);
}



// =====================================================
// Eye — NORMAL (—)
// =====================================================
void ssd1306_eye_normal(SSD1306_t *oled){
    ssd1306_clear(oled);

    int oy = 32;

    for(int x = 0; x < EYE_W; x++){
        ssd1306_drawPixel(oled, EYE_OX + x, oy,   1);
        ssd1306_drawPixel(oled, EYE_OX + x, oy+1, 1);
        ssd1306_drawPixel(oled, EYE_OX + x, oy+2, 1);
    }

    ssd1306_update(oled);
}


// =====================================================
// Eye — SAD (∨)
// =====================================================
void ssd1306_eye_sad(SSD1306_t *oled){
    ssd1306_2_clear(oled);

    int ox = EYE_OX;
    int oy = EYE_OY;

    for(int y = 0; y < EYE_H; y++){
        for(int x = 0; x < EYE_W; x++){
            uint8_t pixel = 0;

            // 🔥 (EYE2_H - y) → y
            if(x == (EYE_W/2 - y/2)) pixel = 1;
            if(x == (EYE_W/2 + y/2)) pixel = 1;

            if(pixel)
                ssd1306_2_drawPixel(oled, ox + x, oy + y, 1);
        }
    }

    ssd1306_2_update(oled);
}



