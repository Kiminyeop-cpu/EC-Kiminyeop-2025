#include "stm32f4xx.h"
#include "ecRCC2.h"

int EC_SYSCLK = 16000000;

void RCC_HSI_init(void) {
    RCC->CR |= RCC_CR_HSION;                      // Enable HSI
    while(!(RCC->CR & RCC_CR_HSIRDY));            // Wait for HSI ready

    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_HSI;                 // Select HSI as system clock

    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI); // Wait HSI used
    EC_SYSCLK = 16000000;
}

void RCC_PLL_init(void) {
    // 1. Enable HSI and wait
    RCC->CR |= RCC_CR_HSION;
    while(!(RCC->CR & RCC_CR_HSIRDY));

    // 2. Disable PLL for config
    RCC->CR &= ~RCC_CR_PLLON;

    // 3. Configure PLL
    RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLSRC;
    RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSI;       // Source = HSI

    RCC->PLLCFGR = (RCC->PLLCFGR & ~RCC_PLLCFGR_PLLN) | (84U << 6);
    RCC->PLLCFGR = (RCC->PLLCFGR & ~RCC_PLLCFGR_PLLM) | (8U);
    RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLP;            // PLLP = 2

    // 4. Enable PLL and wait
    RCC->CR |= RCC_CR_PLLON;
    while(!(RCC->CR & RCC_CR_PLLRDY));

    // 5. Flash latency
    FLASH->ACR &= ~FLASH_ACR_LATENCY;
    FLASH->ACR |= FLASH_ACR_LATENCY_2WS;

    // 6. AHB/APB prescalers
    RCC->CFGR &= ~RCC_CFGR_HPRE;
    RCC->CFGR &= ~RCC_CFGR_PPRE1;
    RCC->CFGR |= RCC_CFGR_PPRE1_2;                // APB1 = /2
    RCC->CFGR &= ~RCC_CFGR_PPRE2;

    // 7. Select PLL as SYSCLK
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL;

    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL); // ✅ 수정 완료

    EC_SYSCLK = 84000000;
}

// GPIO clock enables
void RCC_GPIOA_enable(void){ RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; }
void RCC_GPIOB_enable(void){ RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN; }
void RCC_GPIOC_enable(void){ RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN; }
void RCC_GPIOD_enable(void){ RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN; }
void RCC_GPIOE_enable(void){ RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN; }
void RCC_GPIOH_enable(void){ RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN; }
