#include "ecSTM32F4v2.h"
#include "ecGPIO2.h"
#include "ecUART2.h"
#include "ecSysTick2.h"
#include "ecDHT11.h"
#include "ssd1306.h"     // OLED1
#include "ssd13062.h"    // OLED2
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

// =======================================================
// ✅ 3-bit state input pins between MCUs (GPIO READ)
// =======================================================
// =======================================================
// ✅ UART6 Reception State Variables (Replaces GPIO)
// =======================================================
volatile uint8_t uart6_event = 4;   // default state
volatile uint8_t uart6_hold  = 0;   // HOLD state (0/1)

#ifndef LOW
#define LOW  0
#endif
#ifndef HIGH
#define HIGH 1
#endif

DHT11_t dht;

// =======================================================
// OLED Objects
// =======================================================
SSD1306_t oled1;       // Original OLED -> Left side
SSD1306_2_t oled2;     // New OLED -> Right side

// =======================================================
// EMOTION SYSTEM VARIABLES
// =======================================================
int emotion = 50;

volatile char bt_buf[3] = {0,0,0};
volatile uint8_t bt_idx = 0;
volatile uint8_t BT_code = 0;

volatile char pc_buf[2] = {0,0};
volatile uint8_t PC_code = 0;

extern volatile uint32_t msTicks;

int emo_state = 1;   // 0=Bad, 1=Normal, 2=Good

// ---------------------- DHT11 Variables --------------------
float temp = 0.0f;
float humi = 0.0f;

uint8_t env_bad = 0;
uint32_t env_bad_start_time = 0;
uint32_t last_env_print_time = 0;

// =======================================================
// ★ Automatic reduction variables for '00' Mode
// =======================================================
uint8_t mode_00_active = 0;
uint32_t mode_00_timer = 0;

// =======================================================
// OLED Alternating Display Control Variables
// =======================================================
uint8_t oled_toggle = 0;
uint32_t last_oled_swap = 0;

// =======================================================
// ✅ For GPIO State Detection
// =======================================================
uint8_t last_gpio_state = 4;   // default=001 -> treated as state 4

// =======================================================
// Function to limit Emotion value range
// =======================================================
void clamp_emotion(){
    if(emotion < 0) emotion = 0;
    if(emotion > 100) emotion = 100;
}

// =======================================================
// Bluetooth Transmission
// =======================================================
void BT_print(char *msg){
    USART1_write((uint8_t *)msg, strlen(msg));
}

void BT_printf(const char *fmt, ...)
{
    char buf[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    BT_print(buf);
}

// =======================================================
// USART1 RX INTERRUPT (Bluetooth)
// =======================================================
void USART1_IRQHandler(void){
    if(is_USART1_RXNE()){
        char c = USART1_read();

        if(c == '0' || c == '1'){
            bt_buf[bt_idx++] = c;

            if(bt_idx == 3){
                // === Only 5 specific states allowed ===
                if     (bt_buf[0]=='0' && bt_buf[1]=='0' && bt_buf[2]=='1') BT_code = 1; // 001 default
                else if(bt_buf[0]=='1' && bt_buf[1]=='0' && bt_buf[2]=='0') BT_code = 2; // 100 excessive
                else if(bt_buf[0]=='0' && bt_buf[1]=='1' && bt_buf[2]=='0') BT_code = 3; // 010 appropriate
                else if(bt_buf[0]=='0' && bt_buf[1]=='0' && bt_buf[2]=='0') BT_code = 4; // 000 lacking
                else if(bt_buf[0]=='1' && bt_buf[1]=='1' && bt_buf[2]=='0') BT_code = 5; // 110 petting
                else BT_code = 0;  // ❌ Input not allowed

                bt_idx = 0;
            }
        }
    }
}

// =======================================================
// USART2 RX INTERRUPT (PC Keyboard)
// =======================================================
void USART2_IRQHandler(void){
    if(is_USART2_RXNE()){
        char c = USART2_read();

        if(c=='0' || c=='1'){
            pc_buf[0] = pc_buf[1];
            pc_buf[1] = c;

            if(pc_buf[0] != 0 && pc_buf[1] != 0){
                if(pc_buf[0]=='0' && pc_buf[1]=='1') PC_code = 2;
                else if(pc_buf[0]=='1' && pc_buf[1]=='0') PC_code = 3;
                else if(pc_buf[0]=='1' && pc_buf[1]=='1') PC_code = 4;
                else if(pc_buf[0]=='0' && pc_buf[1]=='0') PC_code = 1;

                pc_buf[0] = pc_buf[1] = 0;
            }
        }
    }
}

void USART6_IRQHandler(void){
    if(is_USART_RXNE(USART6)){
        uint8_t rx = USART_read(USART6);

        if(rx == 0 || rx == 1){
            uart6_hold = rx;          // flag
        }
        else if(rx >= 2 && rx <= 5){
            uart6_event = rx;         // state 2/3/4
        }
    }
}

uint8_t get_event_from_uart6(void)
{
    return uart6_event;   // returns 2,3,4 or 4 (default)
}

uint8_t is_lack_from_uart6(void)
{
    return (uart6_hold == 1);
}

// =======================================================
// ✅ "Read" 3-bit GPIO state and convert to state number
// state=0:000, 1:100, 2:010, 3:110, 4:001(default)
// Other combinations are treated as 4 (default)
// =======================================================
uint8_t read_state_gpio(void)
{
    return uart6_event;
}

// =======================================================
// SETUP
// =======================================================
void setup(void){
    RCC_PLL_init();
    SysTick_init();

    UART2_init();
    UART2_baud(9600);
    UART1_init();
    UART1_baud(9600);

    USART1->CR1 |= USART_CR1_RXNEIE;
    USART2->CR1 |= USART_CR1_RXNEIE;

    NVIC_EnableIRQ(USART1_IRQn);
    NVIC_EnableIRQ(USART2_IRQn);

    // =====================================================
    // ✅ Initialize 3-bit GPIO "Input" pins (READ)
    // =====================================================
    UART6_init();
    UART6_baud(9600);

    USART6->CR1 |= USART_CR1_RXNEIE;
    NVIC_EnableIRQ(USART6_IRQn);

    // Initialize last_gpio_state by reading GPIO state immediately after boot
    last_gpio_state = read_state_gpio();
    DHT11_init(&dht, PB_5);

    printf("=== Emotion + DHT11 READY ===\r\n");
    BT_print("=== Emotion Score is Initialized by 50 ===\r\n");

    // =====================================================
    // Initialize two OLEDs
    // =====================================================
    ssd1306_init(&oled1);
    ssd1306_2_init(&oled2);
    ssd1306_clear(&oled1);
    ssd1306_2_clear(&oled2);
    ssd1306_update(&oled1);
    ssd1306_2_update(&oled2);
}

// =======================================================
// MAIN LOOP
// =======================================================
int main(void){
    setup();

    while(1){

        // ======================================================
        // (A) Output Temp/Humi once per second
        // ======================================================
        if(msTicks - last_env_print_time >= 1000){
            last_env_print_time = msTicks;

            int ret = DHT11_read(&dht, &temp, &humi);
            if(ret == 0){
                int temp_int = (int)temp;
                int temp_dec = (int)((temp - temp_int) * 100);
                int humi_int = (int)humi;
                int humi_dec = (int)((humi - humi_int) * 100);
                BT_printf("Temp=%d.%02dC, Humi=%d.%02d%%\r\n",
                       temp_int, temp_dec, humi_int, humi_dec);
            } else {
                BT_printf("DHT11 Error (%d)\r\n", ret);
            }

            uint8_t now_bad = (temp >= 30.0f || humi >= 70.0f);

            if(now_bad && env_bad == 0){
                env_bad = 1;
                env_bad_start_time = msTicks;
                emotion -= 1; clamp_emotion();
                BT_printf("Environment is worsen! Emotion=%d\r\n", emotion);
            }
            else if(now_bad && env_bad == 1){
                if(msTicks - env_bad_start_time >= 20000){
                    env_bad_start_time = msTicks;
                    emotion -= 1; clamp_emotion();
                    BT_printf("1 minute passed after environmental deterioration persists -> Emotion=%d\r\n", emotion);
                }
            }
            else if(!now_bad && env_bad == 1){
                env_bad = 0;
                BT_printf("Environment normalized. Deterioration timer reset.\r\n");
            }
        }

        // ======================================================
        // (B) Bluetooth Command Processing
        // ======================================================
        if(BT_code != 0){
            if(mode_00_active && BT_code == 5){
                BT_printf("[BT] 110 Ignored (Maintaining 000 mode)\r\n");
                BT_code = 0;
                continue;
            }
            if(BT_code != 4) mode_00_active = 0;

            // ✅ BT 3bit (5 states) -> Action
            int bt_state = 4;

            switch(BT_code){
                case 1:   // 001 default
                    bt_state = 4;
                    BT_print("[BT]001\r\n");
                    break;

                case 2:   // 100 excessive
                    bt_state = 1;
                    emotion -= 1; clamp_emotion(); BT_print("[BT]100\r\n");
                    break;

                case 3:   // 010 appropriate
                    bt_state = 2;
                    emotion += 5; clamp_emotion(); BT_print("[BT]010\r\n");
                    break;

                case 4:   // 000 lacking
                    bt_state = 0;
                    emotion -= 1; clamp_emotion(); BT_print("[BT]000\r\n");
                    mode_00_active = 1; mode_00_timer = msTicks;
                    break;

                case 5:   // 110 petting
                    bt_state = 3;
                    emotion += 1; clamp_emotion(); BT_print("[BT]110\r\n");
                    break;
            }

            BT_code = 0;
            BT_printf("Emotion = %d\r\n", emotion);
        }

        // ======================================================
        // ✅ (B-2) GPIO Input Processing — EDGE TRIGGER
        // ======================================================
        {
        uint8_t event = get_event_from_uart6();

        // ================================
        // 1️⃣ EVENT is always edge-triggered
        // ================================
        if(last_gpio_state == 4 && event != 4){

            BT_printf("[UART EVENT] %d\r\n", event);

            switch(event){
                case 2: // OVER
                    emotion -= 1;
                    clamp_emotion();
                    BT_printf("[EVENT 2: OVER: Joy or fullness is excessive. Rest is required.] Emotion=%d\r\n", emotion);
                    break;

                case 3: // CARD REFILL
                    emotion += 5;
                    clamp_emotion();
                    BT_printf("[EVENT 3: GOOD: If you continue to care about joy and fullness, your pet will stay happy.] Emotion=%d\r\n", emotion);
                    break;

                case 5: // PRESSURE / TOUCH
                    if(emotion < 25){
                        BT_printf("[EVENT 5: TOUCH IGNORED: A pet that has already lost interest cannot be soothed by touch alone. Please feed or play with it.] Emotion=%d\r\n", emotion);
                    }
                    else{
                        emotion += 1;
                        clamp_emotion();
                        BT_printf("[EVENT 5: TOUCH OK: When the pet is in a happy state, petting makes it even happier.] Emotion=%d\r\n", emotion);
                    }
                    break;
            }

            clamp_emotion();
            uart6_event = 4;   // Consume event
        }

        // Update last_state for EVENT detection
        last_gpio_state = event;

        }

        // ======================================================
        // (C) PC Command Processing
        // ======================================================
        if(PC_code != 0){
            switch(PC_code){
                case 1: emotion -= 1; break;
                case 2: emotion -= 1; break;
                case 3: emotion += 5; break;
                case 5: emotion += 1; break;
            }
            clamp_emotion();
            BT_printf("Emotion = %d\r\n", emotion);
            PC_code = 0;
        }

        // ======================================================
        // (LACK) UART6 HOLD -> Entry control for '00' Mode
        // ======================================================
        if(is_lack_from_uart6()){

            // Moment of entering 'lacking' state for the first time
            if(!mode_00_active){
                mode_00_active = 1;
                mode_00_timer = msTicks;

                emotion -= 1;
                clamp_emotion();

                BT_printf("[UART LACK ENTER: The pet is hungry or bored. Please feed it or play with it.] Emotion=%d\r\n", emotion);
            }
        }
        else{
            // Lack state released
            mode_00_active = 0;
        }

        // ======================================================
        // (F) Automatic reduction in '00' Mode
        // ======================================================
        if(mode_00_active){
            if(msTicks - mode_00_timer >= 20000){
                mode_00_timer = msTicks;
                emotion -= 1;
                clamp_emotion();
                BT_printf("The pet is hungry and bored. Please pay attention to it, master... -> Emotion=%d\r\n", emotion);
            }
        }

        // ======================================================
        // (E) State Transition Detection + OLED Output
        // ======================================================
        int new_state;
        if(emotion < 25) new_state = 0;
        else if(emotion < 70) new_state = 1;
        else new_state = 2;

        if(new_state != emo_state){
            emo_state = new_state;
            if(emo_state == 0){ BT_printf("Angry and Sad\r\n"); BT_print("Bad\r\n"); }
            if(emo_state == 1){ BT_printf("Normal\r\n"); BT_print("Normal\r\n"); }
            if(emo_state == 2){ BT_printf("Very Happy\r\n"); BT_print("Good\r\n"); }
        }

        // ======================================================
        // (G) Rapid alternating display on two OLEDs (Persistence of vision)
        // ======================================================
        if(msTicks - last_oled_swap >= 2){ // Approx 500Hz alternation
            last_oled_swap = msTicks;
            oled_toggle = !oled_toggle;

            if(oled_toggle == 0){
                ssd1306_clear(&oled1);
                if(emo_state == 0) ssd1306_eye_sad(&oled1);
                else if(emo_state == 1) ssd1306_eye_normal(&oled1);
                else ssd1306_eye_happy(&oled1);
                ssd1306_update(&oled1);
            } else {
                ssd1306_2_clear(&oled2);
                if(emo_state == 0) ssd1306_2_eye_sad(&oled2);
                else if(emo_state == 1) ssd1306_2_eye_normal(&oled2);
                else ssd1306_2_eye_happy(&oled2);
                ssd1306_2_update(&oled2);
            }
        }
        delay_ms(10);
    }
}