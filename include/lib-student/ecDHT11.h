#ifndef __EC_DHT11_H
#define __EC_DHT11_H

#include "ecSTM32F4v2.h"   // Includes entire EC_HAL such as GPIO, TIM, SysTick, UART, etc.

typedef struct{
    PinName_t pin;        // DHT11 Data Pin
} DHT11_t;

// Initialize DHT11 (Assign pin)
void DHT11_init(DHT11_t *dht, PinName_t pinName);

// Read temperature and humidity from DHT11
// Returns 0 on success, negative value on failure
int DHT11_read(DHT11_t *dht, float *temperature, float *humidity);

#endif