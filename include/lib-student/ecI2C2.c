#include "ecI2C2.h"
#include "ecSysTick2.h"

//
// ==============================
//  I2C1 (PB6=SCL, PB7=SDA) → OLED1
// ==============================
void I2C1_init(void){
    RCC_GPIOB_enable();
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    // PB6 = SCL
    GPIO_init(PB_6, AF);
    GPIO_otype(PB_6, 1);       // open-drain
    GPIO_pupd(PB_6, EC_PU);
    GPIO_ospeed(PB_6, EC_HIGH);
    GPIO_AF_config(PB_6, 4);   // AF4 = I2C1_SCL

    // PB7 = SDA
    GPIO_init(PB_7, AF);
    GPIO_otype(PB_7, 1);
    GPIO_pupd(PB_7, EC_PU);
    GPIO_ospeed(PB_7, EC_HIGH);
    GPIO_AF_config(PB_7, 4);   // AF4 = I2C1_SDA

    // I2C1 configuration
    I2C1->CR1 = 0;
    I2C1->CR2 = 42;
    I2C1->CCR = 210;
    I2C1->TRISE = 43;
    I2C1->CR1 |= I2C_CR1_PE;
}

//
// ==============================
//  I2C2 (PB10=SCL, PB3=SDA) → OLED2
// ==============================
void I2C2_init(void){
    RCC_GPIOB_enable();
    RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;

    // PB10 = SCL
    GPIO_init(PB_10, AF);
    GPIO_otype(PB_10, 1);
    GPIO_pupd(PB_10, EC_PU);
    GPIO_ospeed(PB_10, EC_HIGH);
    GPIO_AF_config(PB_10, 4);   // AF4 = I2C2_SCL

    // PB3 = SDA
    GPIO_init(PB_3, AF);
    GPIO_otype(PB_3, 1);
    GPIO_pupd(PB_3, EC_PU);
    GPIO_ospeed(PB_3, EC_HIGH);
    GPIO_AF_config(PB_3, 9);    // PB3 alternate AF9 = I2C2_SDA

    // Peripheral reset
    I2C2->CR1 |= I2C_CR1_SWRST;
    delay_ms(1);
    I2C2->CR1 &= ~I2C_CR1_SWRST;

    // I2C2 configuration
    I2C2->CR1 = 0;
    I2C2->CR2 = 42;
    I2C2->CCR = 210;
    I2C2->TRISE = 43;
    I2C2->CR1 |= I2C_CR1_PE;
}

//
// ==============================
// I2C1 Low-level (OLED1)
// ==============================
static void I2C1_start(uint8_t addr){
    while(I2C1->SR2 & I2C_SR2_BUSY);
    I2C1->CR1 |= I2C_CR1_START;
    while(!(I2C1->SR1 & I2C_SR1_SB));

    I2C1->DR = addr << 1;
    while(!(I2C1->SR1 & I2C_SR1_ADDR));
    volatile int tmp = I2C1->SR2;
}

static void I2C1_stop(void){
    I2C1->CR1 |= I2C_CR1_STOP;
    delay_us(5);
}

void I2C1_writeCmd(uint8_t cmd){
    I2C1_start(SSD1306_ADDR);
    while(!(I2C1->SR1 & I2C_SR1_TXE));
    I2C1->DR = 0x00;
    while(!(I2C1->SR1 & I2C_SR1_TXE));
    I2C1->DR = cmd;
    while(!(I2C1->SR1 & I2C_SR1_BTF));
    I2C1_stop();
}

void I2C1_writeData(uint8_t data){
    I2C1_start(SSD1306_ADDR);
    while(!(I2C1->SR1 & I2C_SR1_TXE));
    I2C1->DR = 0x40;
    while(!(I2C1->SR1 & I2C_SR1_TXE));
    I2C1->DR = data;
    while(!(I2C1->SR1 & I2C_SR1_BTF));
    I2C1_stop();
}

void I2C1_writeMulti(uint8_t control, uint8_t *data, uint16_t size){
    I2C1_start(SSD1306_ADDR);
    while(!(I2C1->SR1 & I2C_SR1_TXE));
    I2C1->DR = control;

    for(int i=0; i<size; i++){
        while(!(I2C1->SR1 & I2C_SR1_TXE));
        I2C1->DR = data[i];
    }
    while(!(I2C1->SR1 & I2C_SR1_BTF));
    I2C1_stop();
}

//
// ==============================
// I2C2 Low-level (OLED2)
// ==============================
static void I2C2_start(uint8_t addr){
    while(I2C2->SR2 & I2C_SR2_BUSY);
    I2C2->CR1 |= I2C_CR1_START;
    while(!(I2C2->SR1 & I2C_SR1_SB));

    I2C2->DR = addr << 1;
    while(!(I2C2->SR1 & I2C_SR1_ADDR));
    volatile int tmp = I2C2->SR2;
}

static void I2C2_stop(void){
    I2C2->CR1 |= I2C_CR1_STOP;
    delay_us(5);
}

void I2C2_writeCmd(uint8_t cmd){
    I2C2_start(SSD1306_ADDR2);
    while(!(I2C2->SR1 & I2C_SR1_TXE));
    I2C2->DR = 0x00;
    while(!(I2C2->SR1 & I2C_SR1_TXE));
    I2C2->DR = cmd;
    while(!(I2C2->SR1 & I2C_SR1_BTF));
    I2C2_stop();
}

void I2C2_writeData(uint8_t data){
    I2C2_start(SSD1306_ADDR2);
    while(!(I2C2->SR1 & I2C_SR1_TXE));
    I2C2->DR = 0x40;
    while(!(I2C2->SR1 & I2C_SR1_TXE));
    I2C2->DR = data;
    while(!(I2C2->SR1 & I2C_SR1_BTF));
    I2C2_stop();
}

void I2C2_writeMulti(uint8_t control, uint8_t *data, uint16_t size){
    I2C2_start(SSD1306_ADDR2);
    while(!(I2C2->SR1 & I2C_SR1_TXE));
    I2C2->DR = control;

    for(int i=0; i<size; i++){
        while(!(I2C2->SR1 & I2C_SR1_TXE));
        I2C2->DR = data[i];
    }
    while(!(I2C2->SR1 & I2C_SR1_BTF));
    I2C2_stop();
}
