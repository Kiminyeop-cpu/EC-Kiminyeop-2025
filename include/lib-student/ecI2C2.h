#ifndef __EC_I2C2_H
#define __EC_I2C2_H

#include "stm32f411xe.h"
#include "ecGPIO2.h"
#include "ecRCC2.h"

#define SSD1306_ADDR   0x3C    // OLED1 (I2C1)
#define SSD1306_ADDR2  0x3C    // OLED2 (I2C2)

// ===============================
// I2C Initialization
// ===============================
void I2C1_init(void);   // PB6=SCL, PB7=SDA
void I2C2_init(void);   // PC5=SCL, PC4=SDA

// ===============================
// I2C1 (OLED1)
// ===============================
void I2C1_writeCmd(uint8_t cmd);
void I2C1_writeData(uint8_t data);
void I2C1_writeMulti(uint8_t control, uint8_t *data, uint16_t size);

// ===============================
// I2C2 (OLED2)
// ===============================
void I2C2_writeCmd(uint8_t cmd);
void I2C2_writeData(uint8_t data);
void I2C2_writeMulti(uint8_t control, uint8_t *data, uint16_t size);

#endif
