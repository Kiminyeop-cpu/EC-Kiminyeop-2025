/**
******************************************************************************
* @author  SSSLAB
* @Mod	   2021-8-12 by YKKIM  	
* @brief   Embedded Controller:  EC_HAL_for_student_exercise 
* 
******************************************************************************
*/

#include "ecTIM2.h"
#include "ecGPIO2.h"

/* Timer Configuration */

// Default Setting:  1 msec of TimerUEV with Counter_Clk 100kHz / PSC=840-1, ARR=100-1
void TIM_init(TIM_TypeDef* TIMx){     
    // 1. Enable Timer CLOCK
	if(TIMx == TIM1)      RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
	else if(TIMx == TIM2) RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;   // <-- 채움
	else if(TIMx == TIM3) RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;   // <-- 채움
	// repeat for TIM4, TIM5, TIM9, TIM11
    // YOUR CODE GOES HERE
	// YOUR CODE GOES HERE
	
    // 2. Set CNT period
	uint32_t msec = 1;
	TIM_period_ms(TIMx, msec); 
	
    // 3. CNT Direction
	TIMx->CR1 &= ~TIM_CR1_DIR;					// <-- 채움 (Upcounter)	
	
    // 4. Enable Timer Counter
	TIMx->CR1 |= TIM_CR1_CEN;		
}

/*
 * Timer Update Event Period
 * 1 ~ 60000 usec with 1MHz Counter (1 tick = 1us), ARR = usec - 1
 * (TIM2/5: 32-bit ARR, Others: 16-bit ARR)
 */
void TIM_period_us(TIM_TypeDef *TIMx, uint32_t usec){   
	uint16_t PSCval;
	uint32_t Sys_CLK;

	if((RCC->CFGR & RCC_CFGR_SW_PLL) == RCC_CFGR_SW_PLL)
		Sys_CLK = 84000000;
	else if((RCC->CFGR & RCC_CFGR_SW_HSI) == RCC_CFGR_SW_HSI) 
		Sys_CLK = 16000000;
	
	// 1MHz counter clock -> 1 tick = 1us
	PSCval = Sys_CLK / 1000000;  // 84 or 16

	if (TIMx == TIM2 || TIMx == TIM5){
		uint32_t ARRval;
		// f_cnt = Sys_CLK / PSC = 1MHz, ARR = usec - 1
		ARRval = usec;								
		TIMx->PSC = PSCval - 1;                    // <-- 채움
		TIMx->ARR = ARRval - 1;				
	}
	else{
		uint16_t ARRval;
		ARRval = (uint16_t)usec;								
		TIMx->PSC = PSCval - 1;                    // <-- 채움
		TIMx->ARR = ARRval - 1;
	}			
}

/*
 * Timer Update Event Period
 * 1 ~ 6000 msec with 100kHz Counter (1 tick = 10us), ARR = 100 * msec - 1
 */
void TIM_period_ms(TIM_TypeDef* TIMx, uint32_t msec){ 
	uint16_t PSCval;
	uint32_t Sys_CLK;
	
	if((RCC->CFGR & RCC_CFGR_SW_PLL) == RCC_CFGR_SW_PLL )
		 Sys_CLK = 84000000;
	else if((RCC->CFGR & RCC_CFGR_SW_HSI) == RCC_CFGR_SW_HSI) 
		Sys_CLK = 16000000;
	
	// f_cnt = 100kHz (1 tick = 10us)
	PSCval = Sys_CLK/100000;  // 840 or 160

	if (TIMx == TIM2 || TIMx == TIM5){
		uint32_t ARRval;		
		ARRval = (Sys_CLK/PSCval/1000) * msec;     // <-- 채움 (100kHz * msec)
		TIMx->PSC = PSCval - 1;
		TIMx->ARR = ARRval - 1;                    // <-- 채움
	}
	else{
		uint16_t ARRval;
		ARRval = (uint16_t)((Sys_CLK/PSCval/1000) * msec);   // <-- 채움
		TIMx->PSC = PSCval - 1;
		TIMx->ARR = ARRval - 1;                              // <-- 채움
	}
}

// msec = 1 to 655
void TIM_period(TIM_TypeDef* TIMx, uint32_t msec){
	TIM_period_ms(TIMx, msec);
}

/* Update Event Interrupt */
void TIM_UI_init(TIM_TypeDef* TIMx, uint32_t msec){
    // 1. Initialize Timer	
	TIM_init(TIMx);                 // ✅ 함수 시그니처에 맞게 수정 (원 주석 혼동 방지)

	TIM_period_ms(TIMx, msec);
	
    // 2. Enable Update Interrupt
	TIM_UI_enable(TIMx);
	
    // 3. NVIC Setting
	uint32_t IRQn_reg = 0;
	if      (TIMx == TIM1)  IRQn_reg = TIM1_UP_TIM10_IRQn;
	else if (TIMx == TIM2)  IRQn_reg = TIM2_IRQn;            // <-- 채움
	// repeat for TIM3, TIM4, TIM5, TIM9, TIM10, TIM11
    // YOUR CODE GOES HERE
	// YOUR CODE GOES HERE
	
	NVIC_EnableIRQ(IRQn_reg);				
	NVIC_SetPriority(IRQn_reg, 2);
}

void TIM_UI_enable(TIM_TypeDef* TIMx){
	TIMx->DIER |= (1UL << 0);			// <-- 채움 (UIE: Update Interrupt enable)	
}

void TIM_UI_disable(TIM_TypeDef* TIMx){
	TIMx->DIER &= ~(1UL << 0);			// <-- 채움 (disable)
}

uint32_t is_UIF(TIM_TypeDef *TIMx){
	return (TIMx->SR & TIM_SR_UIF_Msk); // <-- 채움
}

void clear_UIF(TIM_TypeDef *TIMx){
	TIMx->SR &= ~TIM_SR_UIF_Msk;        // <-- 채움
}
