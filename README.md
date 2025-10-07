# EC-Kiminyeop-2025
[README.md](https://github.com/user-attachments/files/22538286/README.md)
##

Embedded Controller HAL Library

Written by: Kim Inyeop

Program: C/C++

IDE/Compiler: Keil uVision 5

OS: WIn10

MCU: STM32F411RE, Nucleo-64

**Table of Contents**

- [GPIO Digital In/Out](https://ykkim.gitbook.io/ec/stm32-m4-programming/hal_documentation#gpio-digital-inout)
    
    - [Header File](https://ykkim.gitbook.io/ec/stm32-m4-programming/hal_documentation#header-file)
        
    - [GPIO_init()](https://ykkim.gitbook.io/ec/stm32-m4-programming/hal_documentation#gpio-init----)
        
    - [GPIO_mode()](https://ykkim.gitbook.io/ec/stm32-m4-programming/hal_documentation#gpio-mode----)
        
    - [GPIO_write()](https://ykkim.gitbook.io/ec/stm32-m4-programming/hal_documentation#gpio-write----)
        
    - [GPIO_read()](https://ykkim.gitbook.io/ec/stm32-m4-programming/hal_documentation#gpio-read----)
        
    - [GPIO_ospeed()](https://ykkim.gitbook.io/ec/stm32-m4-programming/hal_documentation#gpio-ospeed----)
        
    - [GPIO_otype()](https://ykkim.gitbook.io/ec/stm32-m4-programming/hal_documentation#gpio-otype----)
        
    - [GPIO_pupdr()](https://ykkim.gitbook.io/ec/stm32-m4-programming/hal_documentation#gpio-pupdr----)
        
    

## 

[](https://ykkim.gitbook.io/ec/stm32-m4-programming/hal_documentation#gpio-digital-inout)

GPIO Digital InOut

### 

[](https://ykkim.gitbook.io/ec/stm32-m4-programming/hal_documentation#header-file)

Header File

`#include "ecGPIO.h"`

Copy

```
#ifndef __ECGPIO2_H
#define __ECGPIO2_H

#include "stm32f411xe.h"
#include "ecRCC2.h"
#include "ecPinNames.h"

#define INPUT  0x00
#define OUTPUT 0x01
#define AF     0x02
#define ANALOG 0x03

#define HIGH 1
#define LOW  0

#define LED_PIN    //Find LED Port&Pin and Fill the blank	
#define BUTTON_PIN //Find BTN Port&Pin and Fill the blank

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */
	 
void GPIO_init(PinName_t pinName, uint32_t mode);     
void GPIO_write(PinName_t pinName, int Output);
int  GPIO_read(PinName_t pinName);
void GPIO_mode(PinName_t pinName, uint32_t mode);
void GPIO_ospeed(PinName_t pinName, int speed);
void GPIO_otype(PinName_t pinName, int type);
void GPIO_pupd(PinName_t pinName, int pupd);

#define EC_PU 01
 
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __ECGPIO2_H

```

### 

[](https://ykkim.gitbook.io/ec/stm32-m4-programming/hal_documentation#gpio_init)

GPIO_init()

Initializes GPIO pins with default setting and Enables GPIO Clock. Mode: In/Out/AF/Analog

Copy

```
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

```

**Parameters**

- **Port:** Port Number, GPIOA~GPIOH
    
- **pin**: pin number (int) 0~15
    
- **mode**: INPUT(0), OUTPUT(1), AF(02), ANALOG (03)
    


### 

[](https://ykkim.gitbook.io/ec/stm32-m4-programming/hal_documentation#gpio_mode)

GPIO_mode()

Configures GPIO pin modes: In/Out/AF/Analog

Copy

```
// GPIO MODER: Input(00), Output(01), AlterFunc(10), Analog(11)
void GPIO_mode(PinName_t pinName, uint32_t mode){
   GPIO_TypeDef * Port;
   unsigned int pin;
   ecPinmap(pinName,&Port,&pin);
   Port->MODER &= ~(3UL<<(2*pin)); //clear bits(00)    
   Port->MODER |= mode<<(2*pin); //save bits(00~11)
}
```

00: Input, 01: Output, 10: Alternate Function, 11: Analog
Port->MODER &= ~(3UL<<(2*pin)):  Initialize moder register to 00
Port->MODER |= mode<<(2*pin): Save moder value (mode: 00~11)


### 

[](https://ykkim.gitbook.io/ec/stm32-m4-programming/hal_documentation#gpio_write)

GPIO_write()

Write the data to GPIO pin: High, Low

Copy

```
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
```

HIGH: VCC, LOW: GND
port->ODR |= (1UL << pin): Mask 01 designated place.
port->ODR &= ~(1UL << pin): Mask 00 designated place.


### 

[](https://ykkim.gitbook.io/ec/stm32-m4-programming/hal_documentation#gpio_read)

GPIO_read()

Read the data from GPIO pin

Copy

```
// GPIO READ : return 0 or 1
int GPIO_read(PinName_t pinName){
    GPIO_TypeDef * Port;
    unsigned int pin;
    ecPinmap(pinName, &Port, &pin);

    return ( (Port->IDR & (1UL << pin)) ? 1 : 0 ); //read bits(00)
}
```

IDR: Input Date Register(Save pin input)
(Port->IDR & (1UL << pin): Mask designated place
? 1 : 0: True -> HIGH, False -> LOW

### 

[](https://ykkim.gitbook.io/ec/stm32-m4-programming/hal_documentation#gpio_ospeed)

GPIO_ospeed()

Configures output speed of GPIO pin : Low, Mid, Fast, High

Copy

```
// GPIO OSPEED : Low (00), Medium (01), Fast (10), High (11)
void GPIO_ospeed(PinName_t pinName, int speed){
    GPIO_TypeDef * Port;
    unsigned int pin;
    ecPinmap(pinName, &Port, &pin);
    Port->OSPEEDR &= ~(3UL << (2*pin)); //clear bits(00)
    Port->OSPEEDR |= (speed << (2*pin)); //save bits(00~11)
}
```

00: Low, 01: Medium, 10: Fast, 11: High
Port->OSPEEDR &= ~(3UL << (2*pin)): Initialize Ospeed register to 00
Port->OSPEEDR |= (speed << (2*pin)): Save moder value
    

### 

[](https://ykkim.gitbook.io/ec/stm32-m4-programming/hal_documentation#gpio_otype)

GPIO_otype()

Configures output type of GPIO pin: Push-Pull / Open-Drain

Copy

```
// GPIO OTYPER : Push-pull(0), Open-drain(1)
void GPIO_otype(PinName_t pinName, int type){
    GPIO_TypeDef * Port;
    unsigned int pin;
    ecPinmap(pinName, &Port, &pin);
    Port->OTYPER &= ~(1UL << pin); //clear bits(00)
    Port->OTYPER |= (type << pin); //save bits
}
```

00: Push-pull, 01: Open-drain
Port->OTYPER &= ~(1UL << pin): Initialize Ospeed register to 00
Port->OTYPER |= (type << pin): Save moder value
    

### 

[](https://ykkim.gitbook.io/ec/stm32-m4-programming/hal_documentation#gpio_pupdr)

GPIO_pupd()

Configures Pull-up/Pull-down mode of GPIO pin: No Pull-up, Pull-down/ Pull-up/ Pull-down/ Reserved

Copy

```
// GPIO Pull-up/Pull-down : None(00), Pull-up(01), Pull-down(10)
void GPIO_pupd(PinName_t pinName, int pupd){
    GPIO_TypeDef * Port;
    unsigned int pin;
    ecPinmap(pinName, &Port, &pin);
    Port->PUPDR &= ~(3UL << (2*pin)); //clear bits(00)
    Port->PUPDR |= (pupd << (2*pin)); //save bits(00~11)
}
```

00: None, 01: Pull-up, 10: Pull-down
Port->PUPDR &= ~(3UL << (2*pin)): Initialize Ospeed register to 00
Port->PUPDR |= (pupd << (2*pin)): Save moder value
    

## 

[](https://ykkim.gitbook.io/ec/stm32-m4-programming/hal_documentation#class-or-header-name)

`ecGPIO2.h`
`ecGPIO2.c`
### 



**Example code**This is your new *vault*.

Make a note of something, [[create a link]], or try [the Importer](https://help.obsidian.md/Plugins/Importer)!

```
uint8_t val[10] = {
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01101111  // 9
}; //define 1D-Array
```

Define 1D-Array: 0: LOW, 1: HIGH(Cathode)

```
// Main loop
int main(void){
    setup();

    uint8_t numDisplay = 0; //Initial Number:0
    int lastState = 1; //Because of Pull-up, default value is HIGH=1

    while(1){ //infinite loop
           int buttonState = GPIO_read(BUTTON_PIN); //read button
           if(lastState == 1 && buttonState == 0){ //no push button(1)->push button(0)
               numDisplay++; //increase +1
               if(numDisplay > 9) numDisplay =0; //if numDisplahy > 9, number is return to 0
           }
           seven_seg_FND_display(numDisplay); //Display 0~9 at 7-segment display 
           lastState = buttonState; //Update button state
    }
}
```

Initial state: 0
if(lastState == 1 && buttonState == 0): When, button is pressed (HIGH -> LOW) -> numDisplay++: increase +1
if(numDisplay > 9) numDisplay =0: If number is over 9, return to 0.
lastState = buttonState: Update button state.

`ecEXTI2.c`

```
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
```

This function initializes the GPIO pin to enable an external interrupt (EXTI).
First, it connects the GPIO port to the EXTI line by enabling the SYSCFG clock.
It then maps the EXTI line's port based on the pin number passed.
Next, it configures rising/falling edge triggering based on the trigger type (RISE, FALL, BOTH) passed as an argument.
Interrupts are enabled through the Interrupt Mask Register (IMR).
Finally, the NVIC prioritizes the EXTI line and allows interrupts.
This allows a specific GPIO pin (e.g., a button press) to generate an interrupt upon an external signal change.

```
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
```

These two functions control the enabling and disabling of external interrupts (EXTI) for a specified GPIO pin.
The EXTI_enable() function sets the interrupt mask bit for the pin to enable interrupts, while the EXTI_disable() function clears the bit to disable interrupts.
In other words, these functions allow software control of the interrupt operation of a specific EXTI line through the Interrupt Mask Register (IMR).

```
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
```

These two functions use the EXTI (External Interrupt) Pending Register to check the status of an external interrupt and clear the flag after processing.
The is_pending_EXTI() function checks whether the pending flag for a specified pin is set and returns whether an interrupt request has occurred.
The clear_pending_EXTI() function clears the flag by writing it to 1 after executing the interrupt service routine (ISR), thereby clearing the pending status so that the next interrupt can be detected properly.
In other words, these two functions are responsible for checking whether an interrupt request has occurred and for terminating the request.

`ecSysTick2.c`

```
void SysTick_init(void){	
	//  SysTick Control and Status Register
	SysTick->CTRL = 0;	// Disable SysTick IRQ and SysTick Counter

	// Select processor clock
	// 1 = processor clock;  0 = external clock
	SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;

	// uint32_t MCU_CLK=EC_SYSTEM_CLK
	// SysTick Reload Value Register
	SysTick->LOAD = MCU_CLK_HSI / 1000 - 1;	// 1ms, for HSI HSI = 16MHz.

	// SysTick Current Value Register
	SysTick->VAL = 0;

	// Enables SysTick exception request
	// 1 = counting down to zero asserts the SysTick exception request
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
	
	// Enable SysTick IRQ and SysTick Timer
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
		
	NVIC_SetPriority(SysTick_IRQn, 16);		// Set Priority to 1
	NVIC_EnableIRQ(SysTick_IRQn);			// Enable interrupt in NVIC
}
```

This function initializes the SysTick timer to generate periodic interrupts.
First, it disables the SysTick timer and selects the CPU clock (processor clock) as the clock source.
Next, it sets the LOAD register to a reload value to enable the timer to operate at a 1ms cycle (16 MHz / 1000).
It then resets the VAL register to 0.
It sets the TICKINT bit to generate a SysTick exception (interrupt) when the counter reaches 0.
It then starts the SysTick timer using the ENABLE bit.
Finally, it sets the priority of the SysTick interrupt in the NVIC and enables it.

```
void SysTick_Handler(void){
	SysTick_counter();	
}

void SysTick_counter(){
	msTicks++;
}	


void delay_ms (uint32_t mesc){
   uint32_t curTicks;

   curTicks = msTicks;
   while ((msTicks - curTicks) < mesc);
	
   msTicks = 0;
}
```

These three functions handle time delay and counting using the SysTick timer.
The SysTick_Handler() function is an interrupt service routine (ISR) that executes when the SysTick interrupt, which occurs every 1 millisecond, is called.
This function internally calls SysTick_counter(), which increments the global variable msTicks by 1.
This allows the system to track the passage of time in 1-ms increments.
The delay_ms() function uses this msTicks value to delay program execution for a specified amount of time (in milliseconds).
It stores the current tick value and repeatedly waits until the incremented msTicks value reaches the specified delay time (mesc).
At the end of the delay, msTicks is reset to 0.
In other words, these three functions work together to perform precise, SysTick-based delay and time management in milliseconds.

```
void SysTick_reset(void)
{
	// SysTick Current Value Register
	SysTick->VAL = 0;
}

uint32_t SysTick_val(void) {
	return SysTick->VAL;
}
```

These two functions control or read the current value (Current Value Register, VAL) of the SysTick timer.
The SysTick_reset() function initializes the SysTick->VAL register to 0,
immediately resetting the SysTick counter and restarting the count for the next cycle.
This function is used to restart time measurement or to initialize the counter before calling a delay function.
The SysTick_val() function returns the current value of the SysTick->VAL register,
allowing you to check the remaining count (current counter state).
This function can be used to check how much progress the SysTick timer has made or for precise time measurement.
In essence, these two functions initialize and read the value of the SysTick timer,
assisting in system time management and timing control.

`LAB_EXTI.c`

```
void LAB_EXTI_SysTick_Handler(void){
    static uint32_t tick = 0; //tick counter
    tick++;

    if(tick % 2 == 0){ 
        if(digit_flag == 0){ //If, digit_flag is 0
            seven_seg_FND_display(ones, 0); //ones return 0
            digit_flag = 1; //tens become 1
        } else { // digit_flag is 1
            seven_seg_FND_display(tens, 1); //display tens 1
            digit_flag = 0; //display ones 0
        }
    }
}
```

This function is called periodically by the SysTick timer,
and rapidly alternates (multiplexes) the numbers displayed on a two-digit 7-segment display (FND).
The tick variable is used as an internal counter that increments by 1 each time the function is called.
The conditional statement tck % 2 == 0 is used to cycle through the displayed digits at regular intervals,
and the value of digit_flag is checked to determine the current number of digits to display.
digit_flag == 0 → Displays the ones digit (ones).
digit_flag == 1 → Displays the tens digit (tens).
After each digit is displayed, digit_flag is inverted so that the opposite digit is displayed on the next call.
