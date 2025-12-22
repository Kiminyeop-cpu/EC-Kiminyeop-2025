#include "ecDHT11.h"

// Internal use: Delay in 1.00 μs units using TIM5
static void DHT11_delay_us(uint32_t us){
    // Enable TIM5 clock
    RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;                // FIXED: Enable TIM5 clock for DHT11 μs delay

    // Timer setting: 84.00 MHz / 84 = 1.00 MHz → 1.00 μs resolution
    TIM5->PSC = 84 - 1;
    TIM5->ARR = 0xFFFF;
    TIM5->CR1 |= TIM_CR1_CEN;

    TIM5->CNT = 0;
    while (TIM5->CNT < us);
}

// Internal use: Wait until the desired level is reached (timeout in μs)
static int DHT11_wait_level(DHT11_t *dht, int level, uint32_t timeout_us){
    uint32_t cnt = 0;
    while ( (GPIO_read(dht->pin) ? 1 : 0) != level ){
        if (cnt++ >= timeout_us){
            return -1;   // Timeout
        }
        DHT11_delay_us(1);
    }
    return 0;            // Success
}

void DHT11_init(DHT11_t *dht, PinName_t pinName){
    dht->pin = pinName;

    // Default to Input Pull-up (or use external pull-up resistor)
    GPIO_init(dht->pin, INPUT);
    GPIO_pupd(dht->pin, EC_PU);
}

int DHT11_read(DHT11_t *dht, float *temperature, float *humidity){
    uint8_t data[5] = {0,0,0,0,0};

    // 1. MCU → DHT11 Start Signal
    GPIO_mode(dht->pin, OUTPUT);          // Output mode
    GPIO_otype(dht->pin, 0);
    GPIO_pupd(dht->pin, EC_NONE);          // Recommended to use external pull-up resistor

    GPIO_write(dht->pin, HIGH);
    delay_ms(1);                          // Wait 1.00 ms (Stabilization)

    GPIO_write(dht->pin, LOW);            // Start: Keep LOW for ≥ 18.00 ms
    delay_ms(20);

    GPIO_write(dht->pin, HIGH);           // Switch to HIGH and wait briefly
    DHT11_delay_us(30);

    // 2. Switch line to Input mode, enable Pull-up
    GPIO_mode(dht->pin, INPUT);
    GPIO_pupd(dht->pin, EC_PU);

    // 3. DHT11 Response Sequence:
    //    - 80.00 μs LOW
    //    - 80.00 μs HIGH
    if (DHT11_wait_level(dht, 0, 100) < 0) return -1;    // First LOW (Response start)
    if (DHT11_wait_level(dht, 1, 100) < 0) return -2;    // Followed by HIGH
    if (DHT11_wait_level(dht, 0, 100) < 0) return -3;    // LOW before data transmission starts

    // 4. Receive 40 bits (5 bytes)
    for (int i = 0; i < 40; i++){
        // Each Bit:
        //   Determine 0/1 by the length of HIGH time after 50.00 μs LOW
        // (Currently starting directly from LOW state)

        // Wait until HIGH starts
        if (DHT11_wait_level(dht, 1, 70) < 0) return -4;

        // Sample at the middle of HIGH duration (approx. 40.00 μs)
        DHT11_delay_us(40);

        // If HIGH, consider '1'; if LOW, consider '0'
        uint8_t bit = (GPIO_read(dht->pin) ? 1 : 0);

        data[i/8] <<= 1;
        data[i/8] |= bit;

        // Wait until it goes LOW for the next bit start
        if (DHT11_wait_level(dht, 0, 80) < 0) return -5;
    }

    // 5. Verify Checksum
    uint8_t sum = (uint8_t)(data[0] + data[1] + data[2] + data[3]);
    if (sum != data[4]){
        return -6;   // Checksum Error
    }

    // 6. DHT11 Data Format
    // data[0] : Humidity Integer part
    // data[1] : Humidity Decimal part (usually 0 for basic modules)
    // data[2] : Temperature Integer part
    // data[3] : Temperature Decimal part (usually 0 for basic modules)
    if (humidity != 0){
        *humidity = (float)data[0] + (float)data[1] / 100.0f;
    }
    if (temperature != 0){
        *temperature = (float)data[2] + (float)data[3] / 100.0f;
    }

    return 0;   // Success
}