#ifndef __ECGPIO2_H
#define __ECGPIO2_H

#include "stm32f411xe.h"
#include "ecRCC2.h"
#include "ecPinNames.h"

// ================= MODE =================
#define INPUT   0x00
#define OUTPUT  0x01
#define AF      0x02
#define ANALOG  0x03

// #define LED_PIN
// #define BUTTON_PIN

// ================= SPEED =================
#define EC_LOW      0
#define EC_MEDIUM   1
#define EC_FAST     2
#define EC_HIGH     3

// ================= PUPD =================
#define EC_NONE 0   // No Pull-up / Pull-down
#define EC_PU   1   // Pull-up
#define EC_PD   2   // Pull-down

// ================= LOGIC =================
#define HIGH 1
#define LOW  0

// ================= GPIO Function Prototypes =================
#ifdef __cplusplus
 extern "C" {
#endif

void GPIO_init(PinName_t pinName, uint32_t mode);
void GPIO_write(PinName_t pinName, int output);
int  GPIO_read(PinName_t pinName);
void GPIO_mode(PinName_t pinName, uint32_t mode);
void GPIO_ospeed(PinName_t pinName, int speed);
void GPIO_otype(PinName_t pinName, int type);
void GPIO_pupd(PinName_t pinName, int pupd);
void GPIO_AF_config(PinName_t pinName, uint8_t AFnum);
void LED_toggle(void);

#ifdef __cplusplus
}
#endif

#endif
