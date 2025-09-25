# EC-Kiminyeop-2025
[README.md](https://github.com/user-attachments/files/22538286/README.md)
##

Embedded Controller HAL Library

Written by: Your Name

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
