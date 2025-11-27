#include "ecICAP2.h"

#include <math.h>

#include "ecGPIO2.h"
#include "ecTIM2.h"


#include "ecICAP2.h"
#include "ecGPIO2.h"
#include "ecTIM2.h"

void ICAP_init(PinName_t pinName) {
    GPIO_TypeDef *port;
    unsigned int pin;
    ecPinmap(pinName, &port, &pin);
    TIM_TypeDef *TIMx;
    int TIn;

    ICAP_pinmap(pinName, &TIMx, &TIn);
    int ICn = TIn;

    // ✅ Enable TIMx clock
    if (TIMx == TIM1)       RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    else if (TIMx == TIM2)  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    else if (TIMx == TIM3)  RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    else if (TIMx == TIM4)  RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
    else if (TIMx == TIM5)  RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;

    // ✅ GPIO 설정
    GPIO_init(pinName, AF);
    GPIO_ospeed(pinName, EC_HIGH);
    uint8_t afr_index = pin >> 3;
    uint8_t afr_shift = (pin % 8) * 4;
    port->AFR[afr_index] &= ~(0xF << afr_shift);

    if (TIMx == TIM1 || TIMx == TIM2)
        port->AFR[afr_index] |= (0x1 << afr_shift);
    else if (TIMx == TIM3 || TIMx == TIM4 || TIMx == TIM5)
        port->AFR[afr_index] |= (0x2 << afr_shift);

    // ✅ Timer 기본 설정 (10us per tick)
    TIMx->PSC = 839;        // 84MHz / (839+1) = 100kHz
    TIMx->ARR = 0xFFFF;
    TIMx->CR1 &= ~TIM_CR1_DIR;

    // ✅ Input Capture
    TIMx->CCMR1 &= ~(TIM_CCMR1_CC1S | TIM_CCMR1_CC2S);
    TIMx->CCMR1 |= TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_0;
    TIMx->CCER &= ~(0b1111 << 4 * (ICn - 1));
    TIMx->CCER |= (1 << (4 * (ICn - 1)));

    // ✅ 인터럽트 활성화
    TIMx->DIER |= TIM_DIER_CC1IE | TIM_DIER_CC2IE | TIM_DIER_UIE;

    // ✅ 카운터 시작
    TIMx->CR1 |= TIM_CR1_CEN;
}



// Configure Selecting TIx-ICy and Edge Type
void ICAP_setup(PinName_t pinName, int ICn, int edge_type) {
    // 0. Match Input Capture Port and Pin for TIMx
    GPIO_TypeDef *port;
    unsigned int pin;
    ecPinmap(pinName, &port, &pin);
    TIM_TypeDef *TIMx;
    int CHn;
    ICAP_pinmap(pinName, &TIMx, &CHn);

    // 1. Disable  CC. Disable CCInterrupt for ICn.
    TIMx->CCER &= ~(1 << (4 * (ICn - 1)));  // Capture Enable
    TIMx->DIER &= ~(1 << ICn);              // CCn Interrupt enabled

    // 2. Configure  IC number(user selected) with given IC pin(TIMx_CHn)
    switch (ICn) {
        case 1:
            TIMx->CCMR1 &= ~TIM_CCMR1_CC1S;  // reset   CC1S
            if (ICn == CHn)
                TIMx->CCMR1 |= TIM_CCMR1_CC1S_0;  // 01<<0   CC1S    Tx_Ch1=IC1
            else
                TIMx->CCMR1 |= TIM_CCMR1_CC1S_1;  // 10<<0   CC1S    Tx_Ch2=IC1
            break;
        case 2:
            TIMx->CCMR1 &= ~TIM_CCMR1_CC2S;  // reset   CC2S
            if (ICn == CHn)
                TIMx->CCMR1 |= TIM_CCMR1_CC2S_0;  // 01<<0   CC2S    Tx_Ch2=IC2
            else
                TIMx->CCMR1 |= TIM_CCMR1_CC2S_1;  // 10<<0   CC2S    Tx_Ch1=IC2
            break;
        case 3:
            TIMx->CCMR2 &= ~TIM_CCMR2_CC3S;  // reset   CC3S
            if (ICn == CHn)
                TIMx->CCMR2 |= TIM_CCMR2_CC3S_0;  // 01<<0   CC3S    Tx_Ch3=IC3
            else
                TIMx->CCMR2 |= TIM_CCMR2_CC3S_1;  // 10<<0   CC3S    Tx_Ch4=IC3
            break;
        case 4:
            TIMx->CCMR2 &= ~TIM_CCMR2_CC4S;  // reset   CC4S
            if (ICn == CHn)
                TIMx->CCMR2 |= TIM_CCMR2_CC4S_0;  // 01<<0   CC4S    Tx_Ch4=IC4
            else
                TIMx->CCMR2 |= TIM_CCMR2_CC4S_1;  // 10<<0   CC4S    Tx_Ch3=IC4
            break;
        default:
            break;
    }

    // 3. Configure Activation Edge direction
    TIMx->CCER &= ~(0b1010 << 4 * (ICn - 1));  // Clear CCnNP/CCnP bits
    switch (edge_type) {
        case IC_RISE:
            TIMx->CCER &= ~(0b1010 << 4 * (ICn - 1));
            break;  // rising:  00
        case IC_FALL:
            TIMx->CCER |= 0b0010 << 4 * (ICn - 1);
            break;  // falling: 01
        case IC_BOTH:
            TIMx->CCER |= 0b1010 << 4 * (ICn - 1);
            break;  // both:    11
    }

    // 4. Enable CC. Enable CC Interrupt.
    TIMx->CCER |= 1 << (4 * (ICn - 1));  // Capture Enable
    TIMx->DIER |= 1 << ICn;              // CCn Interrupt enabled
}

// Time span for one counter step
void ICAP_counter_us(PinName_t pinName, int usec) {
    // 0. Match Input Capture Port and Pin for TIMx
    GPIO_TypeDef *port;
    unsigned int pin;
    ecPinmap(pinName, &port, &pin);
    TIM_TypeDef *TIMx;
    int CHn;
    ICAP_pinmap(pinName, &TIMx, &CHn);

    TIMx->PSC = 84 * usec - 1;  // Timer counter clock: 1M`Hz(1us)
    TIMx->ARR = 0xFFFF;         // Set auto reload register to maximum (count up to 65535)
}

uint32_t ICAP_capture(TIM_TypeDef *TIMx, uint32_t ICn) {
    uint32_t capture_Value;

    if (ICn == 1)
        capture_Value = TIMx->CCR1;
    else if (ICn == 2)
        capture_Value = TIMx->CCR2;
    else if (ICn == 3)
        capture_Value = TIMx->CCR3;
    else
        capture_Value = TIMx->CCR4;

    return capture_Value;
}

uint32_t is_CCIF(TIM_TypeDef *TIMx, uint32_t ccNum) {
    return (TIMx->SR & (0x1UL << ccNum)) != 0;
}

void clear_CCIF(TIM_TypeDef *TIMx, uint32_t ccNum) {
    TIMx->SR &= ~(1 << ccNum);
}

// Needed for Input Capture
void ICAP_pinmap(PinName_t pinName, TIM_TypeDef **TIMx, int *chN) {
    GPIO_TypeDef *port;
    unsigned int pin;
    ecPinmap(pinName, &port, &pin);

    if (port == GPIOA) {
        switch (pin) {
            case 0:
                *TIMx = TIM2;
                *chN = 1;
                break;
            case 1:
                *TIMx = TIM2;
                *chN = 2;
                break;
            case 5:
                *TIMx = TIM2;
                *chN = 1;
                break;
            case 6:
                *TIMx = TIM3;
                *chN = 1;
                break;
            // case 7: *TIMx = TIM1; *chN = 1N; break;
            case 8:
                *TIMx = TIM1;
                *chN = 1;
                break;
            case 9:
                *TIMx = TIM1;
                *chN = 2;
                break;
            case 10:
                *TIMx = TIM1;
                *chN = 3;
                break;
            case 15:
                *TIMx = TIM2;
                *chN = 1;
                break;
            default:
                break;
        }
    } else if (port == GPIOB) {
        switch (pin) {
            // case 0: *TIMx = TIM1; *chN = 2N; break;
            // case 1: *TIMx = TIM1; *chN = 3N; break;
            case 3:
                *TIMx = TIM2;
                *chN = 2;
                break;
            case 4:
                *TIMx = TIM3;
                *chN = 1;
                break;
            case 5:
                *TIMx = TIM3;
                *chN = 2;
                break;
            case 6:
                *TIMx = TIM4;
                *chN = 1;
                break;
            case 7:
                *TIMx = TIM4;
                *chN = 2;
                break;
            case 8:
                *TIMx = TIM4;
                *chN = 3;
                break;
            case 9:
                *TIMx = TIM4;
                *chN = 4;
                break;
            case 10:
                *TIMx = TIM2;
                *chN = 3;
                break;

            default:
                break;
        }
    } else if (port == GPIOC) {
        switch (pin) {
            case 6:
                *TIMx = TIM3;
                *chN = 1;
                break;
            case 7:
                *TIMx = TIM3;
                *chN = 2;
                break;
            case 8:
                *TIMx = TIM3;
                *chN = 3;
                break;
            case 9:
                *TIMx = TIM3;
                *chN = 4;
                break;

            default:
                break;
        }
    }
}
