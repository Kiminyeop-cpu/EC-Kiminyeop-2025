#include "ssd13062.h"
#include <string.h>
// 공통 위치 / 크기
#define EYE2_W  96
#define EYE2_H  24

// 중앙 정렬
#define EYE2_OX 16
#define EYE2_OY 20


// ==============================
// Low-level (I2C2 기반: PC5=SCL, PC4=SDA)
// ==============================
static void oled2_cmd(uint8_t cmd){ I2C2_writeCmd(cmd); }
static void oled2_data(uint8_t data){ I2C2_writeData(data); }

// ==============================
// OLED2 Init
// ==============================
void ssd1306_2_init(SSD1306_2_t *oled){
    I2C2_init();   // ✅ PC5=SCL, PC4=SDA

    oled->cursorX = 0;
    oled->cursorY = 0;

    oled2_cmd(0xAE);
    oled2_cmd(0x20);
    oled2_cmd(0x10);
    oled2_cmd(0xB0);
    oled2_cmd(0xC8);
    oled2_cmd(0x00);
    oled2_cmd(0x10);
    oled2_cmd(0x40);
    oled2_cmd(0x81);
    oled2_cmd(0x7F);
    oled2_cmd(0xA1);
    oled2_cmd(0xA6);
    oled2_cmd(0xA8);
    oled2_cmd(0x3F);
    oled2_cmd(0xA4);
    oled2_cmd(0xD3);
    oled2_cmd(0x00);
    oled2_cmd(0xD5);
    oled2_cmd(0x80);
    oled2_cmd(0xD9);
    oled2_cmd(0xF1);
    oled2_cmd(0xDA);
    oled2_cmd(0x12);
    oled2_cmd(0xDB);
    oled2_cmd(0x40);
    oled2_cmd(0x8D);
    oled2_cmd(0x14);
    oled2_cmd(0xAF);

    ssd1306_2_clear(oled);
    ssd1306_2_update(oled);
}

// ==============================
// Buffer Clear
// ==============================
void ssd1306_2_clear(SSD1306_2_t *oled){
    memset(oled->buffer, 0x00, sizeof(oled->buffer));
}

// ==============================
// Display Update
// ==============================
void ssd1306_2_update(SSD1306_2_t *oled){
    for(uint8_t page = 0; page < 8; page++){
        oled2_cmd(0xB0 + page);
        oled2_cmd(0x00);
        oled2_cmd(0x10);

        I2C2_writeMulti(0x40, &oled->buffer[page * 128], 128);
    }
}

// ==============================
// Cursor
// ==============================
void ssd1306_2_setCursor(SSD1306_2_t *oled, uint8_t x, uint8_t y){
    oled->cursorX = x;
    oled->cursorY = y;
}

// ==============================
// Char Draw
// ==============================
extern const uint8_t Font5x7[][5];

void ssd1306_2_drawChar(SSD1306_2_t *oled, char c){
    if(c < 32) c = 32;
    uint8_t idx = c - 32;

    for(int i=0; i<5; i++){
        oled->buffer[oled->cursorY * 128 + oled->cursorX + i] = Font5x7[idx][i];
    }

    oled->cursorX += 6;
}

// ==============================
// String Draw
// ==============================
void ssd1306_2_drawString(SSD1306_2_t *oled, char *str){
    while(*str){
        ssd1306_2_drawChar(oled, *str++);
    }
}

// ==============================
// Pixel Draw
// ==============================
void ssd1306_2_drawPixel(SSD1306_2_t *oled, uint8_t x, uint8_t y, uint8_t color){
    if(x >= 128 || y >= 64) return;

    uint16_t index = x + (y / 8) * 128;

    if(color)
        oled->buffer[index] |= (1 << (y % 8));
    else
        oled->buffer[index] &= ~(1 << (y % 8));
}

// ==============================
// 비트식 눈 — happy
// ==============================
void ssd1306_2_eye_happy(SSD1306_2_t *oled){
    ssd1306_2_clear(oled);

    int ox = EYE2_OX;
    int oy = EYE2_OY;

    for(int y = 0; y < EYE2_H; y++){
        for(int x = 0; x < EYE2_W; x++){
            uint8_t pixel = 0;

            // 🔥 y → (EYE2_H - y)
            if(x == (EYE2_W/2 - (EYE2_H - y)/2)) pixel = 1;
            if(x == (EYE2_W/2 + (EYE2_H - y)/2)) pixel = 1;

            if(pixel)
                ssd1306_2_drawPixel(oled, ox + x, oy + y, 1);
        }
    }

    ssd1306_2_update(oled);
}



// ==============================
// 비트식 눈 — normal
// ==============================
void ssd1306_2_eye_normal(SSD1306_2_t *oled){
    ssd1306_2_clear(oled);

    int ox = EYE2_OX;
    int oy = 32;   // 정확히 중앙 수평

    for(int x = 0; x < EYE2_W; x++){
        // 두께 3픽셀
        ssd1306_2_drawPixel(oled, ox + x, oy,   1);
        ssd1306_2_drawPixel(oled, ox + x, oy+1, 1);
        ssd1306_2_drawPixel(oled, ox + x, oy+2, 1);
    }

    ssd1306_2_update(oled);
}


// ==============================
// 비트식 눈 — sad
// ==============================
void ssd1306_2_eye_sad(SSD1306_2_t *oled){
    ssd1306_2_clear(oled);

    int ox = EYE2_OX;
    int oy = EYE2_OY;

    for(int y = 0; y < EYE2_H; y++){
        for(int x = 0; x < EYE2_W; x++){
            uint8_t pixel = 0;

            // 🔥 (EYE2_H - y) → y
            if(x == (EYE2_W/2 - y/2)) pixel = 1;
            if(x == (EYE2_W/2 + y/2)) pixel = 1;

            if(pixel)
                ssd1306_2_drawPixel(oled, ox + x, oy + y, 1);
        }
    }

    ssd1306_2_update(oled);
}
