/*----------------------------------------------------------------\
@ Embedded Controller Lab - Handong Global University
Author           : [Your Name]
Created          : 2025-09-14
Language/ver     : C in PlatformIO (CMSIS)

Description      : LAB_GPIO_DIO_multiLED
                 - Control 4 LEDs sequentially with Button B1
                 - Button: PA4 (Input, Pull-up)
                 - LEDs  : PB12, PB13, PB14, PB15
                 - Push-Pull, Pull-up, Medium Speed
----------------------------------------------------------------*/

#include "ecRCC2.h"
#include "ecGPIO2.h"

#define Push_Button  PA_4
#define LED0        PB_12
#define LED1        PB_13
#define LED2        PB_14
#define LED3        PB_15

void setup(void);

int main(void) {
    setup();

    int buttonState, lastState = 1;   // 버튼 상태 (1=안눌림, 0=눌림)
    int ledIndex = 0;                 // 현재 켜질 LED 인덱스
    PinName_t leds[4] = {LED0, LED1, LED2, LED3};

    while(1){
        buttonState = GPIO_read(Push_Button);

        // 버튼 눌림 감지 (눌렀다가 뗐을 때 동작)
        if(lastState == 1 && buttonState == 0){
            // 모든 LED OFF
            for(int i=0; i<4; i++) GPIO_write(leds[i], LOW);

            // 현재 LED ON
            GPIO_write(leds[ledIndex], HIGH);

            // 다음 LED 준비
            ledIndex = (ledIndex + 1) % 4;

            // 간단한 소프트웨어 디바운싱 (대략 200ms)
            for(int d=0; d<800000; d++) __NOP();
        }

        lastState = buttonState;
    }
}

void setup(void){
    RCC_HSI_init();

    // 버튼 초기화: 입력 + Pull-up
    GPIO_init(Push_Button, INPUT);
    GPIO_pupd(Push_Button, 1);   // Pull-up

    // LED 초기화: 출력 + Push-pull + Medium speed + Pull-up
    GPIO_init(LED0, OUTPUT);
    GPIO_otype(LED0, 0);
    GPIO_ospeed(LED0, 1);
    GPIO_pupd(LED0, 1);

    GPIO_init(LED1, OUTPUT);
    GPIO_otype(LED1, 0);
    GPIO_ospeed(LED1, 1);
    GPIO_pupd(LED1, 1);

    GPIO_init(LED2, OUTPUT);
    GPIO_otype(LED2, 0);
    GPIO_ospeed(LED2, 1);
    GPIO_pupd(LED2, 1);

    GPIO_init(LED3, OUTPUT);
    GPIO_otype(LED3, 0);
    GPIO_ospeed(LED3, 1);
    GPIO_pupd(LED3, 1);

    GPIO_write(LED0, HIGH);
}
