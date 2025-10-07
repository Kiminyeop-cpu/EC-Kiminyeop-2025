#include "ecGPIO2.h"
#include "ecSysTick2.h"
#include "ecEXTI2.h"


void EXTI_init(PinName_t pinName, int trig_type,int priority){

	GPIO_TypeDef *port;
	unsigned int pin;
	ecPinmap(pinName,&port,&pin);
	// SYSCFG peripheral clock enable	
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;		
	
	// Connect External Line to the GPIO
	int EXTICR_port;
	if			(port == GPIOA) EXTICR_port = 0;
	else if	(port == GPIOB) EXTICR_port = 1;
	else if	(port == GPIOC) EXTICR_port = 2;
	else if	(port == GPIOD) EXTICR_port = 3;
	else 										EXTICR_port = 4;
	
	int exti_num = pin / 4;
	int exti_shift = (pin % 4) * 4;

	SYSCFG->EXTICR[exti_num] &= ~(0xF << exti_shift);			// clear 4 bits
	SYSCFG->EXTICR[exti_num] |= (EXTICR_port << exti_shift);			// set 4 bits
	
// Configure Trigger edge
	if (trig_type == FALL) {
		EXTI->FTSR |=  (1UL << pin);   // Falling trigger enable
		EXTI->RTSR &= ~(1UL << pin);   // Rising trigger disable
	}
	else if (trig_type == RISE) {
		EXTI->RTSR |=  (1UL << pin);   // Rising trigger enable
		EXTI->FTSR &= ~(1UL << pin);   // Falling trigger disable
	}
	else if (trig_type == BOTH) {
		EXTI->RTSR |=  (1UL << pin); 
		EXTI->FTSR |=  (1UL << pin);
	}

	
	// Configure Interrupt Mask (Interrupt enabled)
	EXTI->IMR  |= (1UL << pin);     // not masked
	
	
	// NVIC(IRQ) Setting
	int EXTI_IRQn = 0;
	
	if (pin < 5) 	EXTI_IRQn = EXTI0_IRQn + pin;
	else if	(pin < 10) 	EXTI_IRQn = EXTI9_5_IRQn;
	else 			EXTI_IRQn = EXTI15_10_IRQn;
								
	NVIC_SetPriority(EXTI_IRQn, priority);	// EXTI priority
	NVIC_EnableIRQ(EXTI_IRQn); 	// EXTI IRQ enable
}


void EXTI_enable(PinName_t pinName) {
	GPIO_TypeDef *port;
	unsigned int pin;
	ecPinmap(pinName,&port,&pin);
	EXTI->IMR |= (1UL << pin);     // not masked (i.e., Interrupt enabled)
}
void EXTI_disable(PinName_t pinName) {
	GPIO_TypeDef *port;
	unsigned int pin;
	ecPinmap(pinName,&port,&pin);
	EXTI->IMR &= ~(1UL << pin);     // masked (i.e., Interrupt disabled)
}

uint32_t is_pending_EXTI(PinName_t pinName) {
	GPIO_TypeDef *port;
	unsigned int pin;
	ecPinmap(pinName,&port,&pin); 
	uint32_t EXTI_PRx = (EXTI->PR & (1UL << pin));     	// check  EXTI pending 	
	return ((EXTI->PR & (1UL << pin)) == (1UL << pin));
}


void clear_pending_EXTI(PinName_t pinName) {
	GPIO_TypeDef *port;
	unsigned int pin;
	ecPinmap(pinName,&port,&pin); 
	EXTI->PR |= (1UL << pin);     // clear EXTI pending 
}
