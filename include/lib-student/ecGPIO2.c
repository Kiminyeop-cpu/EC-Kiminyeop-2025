/*----------------------------------------------------------------\
@ Embedded Controller by Young-Keun Kim - Handong Global University
Author           : [Kiminyeop]
Created          : 2025-09-19
Language/ver     : C++ in Keil uVision

Description      : Distributed to Students for LAB_GPIO
/----------------------------------------------------------------*/



#include "stm32f4xx.h"
#include "stm32f411xe.h"
#include "ecGPIO2.h"

void GPIO_init(PinName_t pinName, uint32_t mode){     
	GPIO_TypeDef * Port;
	unsigned int pin;
	ecPinmap(pinName, &Port, &pin);
	
	// mode  : Input(0), Output(1), AlterFunc(2), Analog(3)   
	if (Port == GPIOA)
		RCC_GPIOA_enable();
	if (Port == GPIOB)
        RCC_GPIOB_enable();
	if (Port == GPIOC)
		RCC_GPIOC_enable();
	if (Port == GPIOD)
        RCC_GPIOD_enable();
    if (Port == GPIOE) 
    	RCC_GPIOE_enable();
	if (Port == GPIOH)
    	RCC_GPIOH_enable();

	GPIO_mode(pinName, mode);
}


// GPIO MODER: Input(00), Output(01), AlterFunc(10), Analog(11)
void GPIO_mode(PinName_t pinName, uint32_t mode){
   GPIO_TypeDef * Port;
   unsigned int pin;
   ecPinmap(pinName,&Port,&pin);
   Port->MODER &= ~(3UL<<(2*pin)); //clear bits(00)    
   Port->MODER |= mode<<(2*pin); //save bits(00~11)
}


// GPIO OSPEED : Low (00), Medium (01), Fast (10), High (11)
void GPIO_ospeed(PinName_t pinName, int speed){
    GPIO_TypeDef * Port;
    unsigned int pin;
    ecPinmap(pinName, &Port, &pin);
    Port->OSPEEDR &= ~(3UL << (2*pin)); //clear bits(00)
    Port->OSPEEDR |= (speed << (2*pin)); //save bits(00~11)
}


// GPIO OTYPER : Push-pull(0), Open-drain(1)
void GPIO_otype(PinName_t pinName, int type){
    GPIO_TypeDef * Port;
    unsigned int pin;
    ecPinmap(pinName, &Port, &pin);
    Port->OTYPER &= ~(1UL << pin); //clear bits(00)
    Port->OTYPER |= (type << pin); //save bits(00~11)
}


// GPIO Pull-up/Pull-down : None(00), Pull-up(01), Pull-down(10)
void GPIO_pupd(PinName_t pinName, int pupd){
    GPIO_TypeDef * Port;
    unsigned int pin;
    ecPinmap(pinName, &Port, &pin);
    Port->PUPDR &= ~(3UL << (2*pin)); //clear bits(00)
    Port->PUPDR |= (pupd << (2*pin)); //save bits(00~11)
}


// GPIO READ : return 0 or 1
int GPIO_read(PinName_t pinName){
    GPIO_TypeDef * Port;
    unsigned int pin;
    ecPinmap(pinName, &Port, &pin);

    return ( (Port->IDR & (1UL << pin)) ? 1 : 0 ); //read bits(00)
}

// GPIO WRITE : Set the output
void GPIO_write(PinName_t pinName, int output){
    GPIO_TypeDef *port;
    unsigned int pin;
    ecPinmap(pinName, &port, &pin);

    if(output == HIGH) {
        // Open-drain: If using HIGH, set ODR '1' → maintain Pull-up resistor to HIGH
        port->ODR |= (1UL << pin);
    } 
    else {
        // Open-drain: If using LOW, set ODR '0' → Pull the pin to GND
        port->ODR &= ~(1UL << pin);
    }
}

// void LED_init(void){
//     void LED_toggle(void){
//         GPIOA->ODR^=GPIO_ODR_ODR_5;
//     }

//     void bit_toggle(unsigned int pinNum) {
//         GPIOA->ODR ^=1<<pinNum;
//     }
// }

void LED_toggle(void){
        GPIOA->ODR^=GPIO_ODR_ODR_5;
    }

void GPIO_AF_config(PinName_t pinName, uint8_t AFnum){
    GPIO_TypeDef *Port;
    unsigned int pin;
    ecPinmap(pinName, &Port, &pin);

    uint8_t AFR_idx = pin / 8;
    uint8_t AFR_shift = (pin % 8) * 4;

    Port->AFR[AFR_idx] &= ~(0xF << AFR_shift);
    Port->AFR[AFR_idx] |= (AFnum << AFR_shift);
}
