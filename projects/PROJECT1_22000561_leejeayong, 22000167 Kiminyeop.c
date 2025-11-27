/******************************************************
 * FINAL VERSION - Backward FIX + STOP FIX + Manual State Print
 ******************************************************/

#include "ecSTM32F4v2.h"
#include "ecADC2.h"
#include "ecPWM2.h"
#include "ecICAP2.h"
#include <stdio.h>

// =====================================================
// ==== IR Sensors
// =====================================================
#define IR_RIGHT_PIN PB_0
#define IR_LEFT_PIN  PB_1

// =====================================================
// ==== Motors (AUTO mode uses set_speed / Manual uses drive_speed)
// =====================================================
#define LEFT_MOTOR_PIN  PA_0
#define RIGHT_MOTOR_PIN PA_1

// =====================================================
// ==== STOP LED
// =====================================================
#define STOP_LED PA_5

// =====================================================
// ==== Servo (Side ultrasonic)
// =====================================================
#define SERVO_PIN PA_8
#define SERVO_PERIOD_MS 20
#define SERVO_MIN_US 500
#define SERVO_MAX_US 2500

// =====================================================
// ==== Ultrasonic FRONT / SIDE
// =====================================================
#define TRIG_PIN   PA_6
#define ECHO_PIN   PB_6

#define TRIG2_PIN  PA_7
#define ECHO2_PIN  PB_7

#define WHITE_THRESHOLD 400U
#define BLACK_THRESHOLD 3500U

extern volatile uint32_t msTicks;

// ---------------- FRONT US capture
volatile uint8_t us1_new = 0;
volatile uint32_t us1_t1 = 0, us1_t2 = 0;
volatile uint32_t us1_ovf = 0, us1_interval = 0;

// ---------------- SIDE US capture
volatile uint8_t us2_new = 0;
volatile uint32_t us2_t1 = 0, us2_t2 = 0;
volatile uint32_t us2_ovf = 0, us2_interval = 0;

// ---------------- IR values
volatile uint32_t ir_right_value = 0;
volatile uint32_t ir_left_value  = 0;

uint32_t last_front = 999;
uint32_t last_side  = 999;


// =====================================================
// ==== Servo Control
// =====================================================
void servo_init(){
    PWM_init(SERVO_PIN);
    PWM_period_ms(SERVO_PIN, SERVO_PERIOD_MS);
    PWM_pulsewidth_us(SERVO_PIN, SERVO_MIN_US);
}

void servo_set_deg(int deg){
    if(deg<0) deg=0;
    if(deg>180) deg=180;

    float pulse = SERVO_MIN_US + (deg/180.0f)*(SERVO_MAX_US-SERVO_MIN_US);
    PWM_pulsewidth_us(SERVO_PIN,(uint32_t)pulse);
}

void servo_turn(){ servo_set_deg(250); }
void servo_reset(){ servo_set_deg(0); }


// =====================================================
// ==== AUTO MODE MOTOR
// =====================================================
static inline void set_speed(float L, float R){
    if(L<0) L=0; if(L>1) L=1;
    if(R<0) R=0; if(R>1) R=1;
    PWM_duty(LEFT_MOTOR_PIN,  L);
    PWM_duty(RIGHT_MOTOR_PIN, R);
}


// =====================================================
// ==== ADC IRQ
// =====================================================
void ADC_IRQHandler(void){
    static int idx = 0;
    if(is_ADC_EOC()){
        uint32_t v = ADC_read();
        if(idx==0) ir_right_value=v;
        else       ir_left_value=v;
        idx ^= 1;
    }
}


// =====================================================
// ==== TIM4 IRQ - Ultrasonic
// =====================================================
void TIM4_IRQHandler(void){
    if(is_UIF(TIM4)){
        clear_UIF(TIM4);
        us1_ovf++;
        us2_ovf++;
    }

    // -------- FRONT CH1 ----------
    if(is_CCIF(TIM4,1)){
        clear_CCIF(TIM4,1);
        uint32_t now = TIM4->CCR1;

        if((TIM4->CCER & TIM_CCER_CC1P)==0){
            us1_t1=now; us1_ovf=0;
            TIM4->CCER |= TIM_CCER_CC1P;
        } else {
            us1_t2=now;
            us1_interval = (us1_ovf*65536)+(us1_t2-us1_t1);
            us1_new = 1;
            TIM4->CCER &= ~TIM_CCER_CC1P;
        }
    }

    // -------- SIDE CH2 ----------
    if(is_CCIF(TIM4,2)){
        clear_CCIF(TIM4,2);
        uint32_t now2 = TIM4->CCR2;

        if((TIM4->CCER & TIM_CCER_CC2P)==0){
            us2_t1=now2; us2_ovf=0;
            TIM4->CCER |= TIM_CCER_CC2P;
        } else {
            us2_t2=now2;
            us2_interval = (us2_ovf*65536)+(us2_t2-us2_t1);
            us2_new = 1;
            TIM4->CCER &= ~TIM_CCER_CC2P;
        }
    }
}


// =====================================================
// ==== AUTO MODE LOGIC
// =====================================================
void auto_mode_loop(uint8_t *stopped, uint8_t *servo_state,
                    uint32_t *lastTrig1, uint32_t *lastTrig2)
{
    uint32_t now = msTicks;

    if(now - *lastTrig1 >= 60){
        *lastTrig1 = now;
        GPIO_write(TRIG_PIN,1);
        for(volatile int i=0;i<84;i++);
        GPIO_write(TRIG_PIN,0);
    }

    if(now - *lastTrig2 >= 60){
        *lastTrig2 = now;
        GPIO_write(TRIG2_PIN,1);
        for(volatile int i=0;i<84;i++);
        GPIO_write(TRIG2_PIN,0);
    }

    int rightBlack = (ir_right_value >= BLACK_THRESHOLD);
    int leftBlack  = (ir_left_value  >= BLACK_THRESHOLD);

    if(*stopped==0){
        if(rightBlack && !leftBlack) set_speed(0.0,1);
        else if(leftBlack && !rightBlack) set_speed(1,0.0);
        else set_speed(0.0,0.0);
    }

    if(us1_new){
        us1_new = 0;
        uint32_t d=(uint32_t)(((uint64_t)(us1_interval*10)*343ULL)/20000ULL);
        if(d>0 && d<400) last_front=d;

        if(last_front <= 10){
            *stopped=1;
            set_speed(1,1);
            GPIO_write(STOP_LED,1);
        }
        else if(*stopped){
            *stopped=0;
            GPIO_write(STOP_LED,0);
        }
    }


    // =====================================================
    // ========== 여기가 새로 교체되는 SIDE 로직 ===========
    // =====================================================
    static uint8_t side_trigger_ready = 1;

    if(us2_new){
        us2_new = 0;
        uint32_t d2=(uint32_t)(((uint64_t)(us2_interval*10)*343ULL)/20000ULL);
        if(d2>0 && d2<400) last_side=d2;
    }

    // --- 장애물 새 감지(엣지) ---
    if(last_side <= 10 && last_front <= 10 && side_trigger_ready == 1){
        side_trigger_ready = 0;        
        servo_turn();
        delay_ms(400);
        servo_reset();
    }

    // --- 감지 해제되면 다시 Ready ---
    if(last_side > 10 && last_front > 5){
        side_trigger_ready = 1;
    }
}



// =====================================================
// ==== MANUAL FSM
// =====================================================
volatile char BT_key = 0;
volatile uint8_t esc_stage = 0;

#define STATE_LED PA_5
#define LEFT_MOTOR PA_0
#define RIGHT_MOTOR PA_1
#define DIR_PIN PA_4

// ---- Manual drive_speed (원본) ----
static inline void drive_speed(float left, float right, char direction){
    if(left<0) left=0;
    if(right<0) right=0;
    if(left>1) left=1;
    if(right>1) right=1;

    float L,R;

    if(direction=='F'){
        L = 1.0f - left;
        R = 1.0f - right;
    } else {
        L = left;
        R = right;
    }

    PWM_duty(LEFT_MOTOR,L);
    PWM_duty(RIGHT_MOTOR,R);
}


// =====================================================
// ==== BT INTERRUPT
// =====================================================
void USART1_IRQHandler(){
    if(is_USART1_RXNE()){
        char c = USART1_read();

        if(c>='a'&&c<='z') c-=32;

        if(esc_stage==0){
            if(c==0x1B){ esc_stage=1; return; }
        }
        else if(esc_stage==1){
            if(c=='['){ esc_stage=2; return; }
            esc_stage=0;
        }
        else if(esc_stage==2){
            switch(c){
                case 'A': BT_key='U'; break;
                case 'B': BT_key='D'; break;
                case 'C': BT_key='R'; break;
                case 'D': BT_key='L'; break;
            }
            esc_stage=0;
            return;
        }

        if(c=='S'||c=='M'||c=='A'||c=='F'||c=='B')
            BT_key=c;
    }
}


// =====================================================
// ==== SETUP
// =====================================================
void setup(void){
    RCC_PLL_init();
    SysTick_init();

    UART2_init();
    UART2_baud(9600);

    UART1_init();
    UART1_baud(9600);

    GPIO_init(STATE_LED, OUTPUT);

    GPIO_init(DIR_PIN, OUTPUT);
    GPIO_write(DIR_PIN,1);

    PWM_init(LEFT_MOTOR);
    PWM_init(RIGHT_MOTOR);
    PWM_period_us(LEFT_MOTOR,100);
    PWM_period_us(RIGHT_MOTOR,100);

    drive_speed(0,0,'F');

    servo_init();
    /* Diagnostic: move servo to mid (90deg) briefly and print timer registers */
    servo_set_deg(90);
    delay_ms(500);
    /* Print TIM1 registers (PA_8 -> TIM1_CH1) to help debug servo PWM */
    printf("[SERVO TEST] TIM1->PSC=%lu ARR=%lu CCR1=%lu BDTR=0x%08lX\n", (unsigned long)TIM1->PSC, (unsigned long)TIM1->ARR, (unsigned long)TIM1->CCR1, (unsigned long)TIM1->BDTR);
    servo_set_deg(0);

    ADC_init(IR_RIGHT_PIN);
    ADC_init(IR_LEFT_PIN);
    PinName_t seqCHn[2]={IR_RIGHT_PIN,IR_LEFT_PIN};
    ADC_sequence(seqCHn,2);

    GPIO_init(ECHO_PIN,AF);
    GPIO_pupd(ECHO_PIN,EC_PU);
    ICAP_init(ECHO_PIN);
    ICAP_counter_us(ECHO_PIN,10);
    ICAP_setup(ECHO_PIN,1,IC_BOTH);

    GPIO_init(ECHO2_PIN,AF);
    GPIO_pupd(ECHO2_PIN,EC_PU);
    ICAP_init(ECHO2_PIN);
    ICAP_counter_us(ECHO2_PIN,10);
    ICAP_setup(ECHO2_PIN,2,IC_BOTH);

    NVIC_EnableIRQ(TIM4_IRQn);

    GPIO_init(TRIG_PIN,OUTPUT);
    GPIO_write(TRIG_PIN,0);

    GPIO_init(TRIG2_PIN,OUTPUT);
    GPIO_write(TRIG2_PIN,0);

    GPIO_init(STOP_LED,OUTPUT);
    GPIO_write(STOP_LED,0);

    printf("\n=== SYSTEM READY (Manual + Auto merged + FIXED) ===\n");

    NVIC_SetPriority(USART1_IRQn, 0);   // 가장 높음 (BT 안정)
    NVIC_SetPriority(ADC_IRQn,    1);   // IR 안정
    NVIC_SetPriority(SysTick_IRQn, 2);  // 기본 타이머
    NVIC_SetPriority(TIM4_IRQn,   3);   // 가장 낮음 (초음파 폭주 방지)
}


// =====================================================
// ==== MAIN
// =====================================================
int main(void){
    setup();

    uint32_t STATE = 0;
    uint32_t prev_state = 999;

    int speed_level = 0;
    int steer_level = 0;
    char direction = 'F';

    float speed_table[4]={0.0,0.60,0.80,1.00};

    uint32_t last_blink = 0;
    uint32_t last_status = 0;

    uint32_t lastTrig1=0, lastTrig2=0;
    uint8_t stopped=0;
    uint8_t servo_state=0;

    while(1){
        char key=0;
        if(BT_key!=0){
            key=BT_key;
            BT_key=0;
            printf("[BT]=%c\n",key);
        }

        /* 1-second periodic status: mode, dir, steer, vel, front & side distances */
        if(msTicks - last_status >= 1000){
            last_status = msTicks;
            char mod = (STATE==11)?'A':(STATE==1)?'M':'S';
            char steer_buf[6];
            if(steer_level < 0) sprintf(steer_buf, "-%02d", -steer_level);
            else sprintf(steer_buf, "%02d", steer_level);
            const char *frs = (last_front>400)?"--":NULL;
            const char *sds = (last_side>400)?"--":NULL;
            if(frs==NULL && sds==NULL)
                printf("MOD: %c DIR: %c STR: %s VEL: %02d FR:%03lu SD:%03lu\n", mod, direction, steer_buf, speed_level, (unsigned long)last_front, (unsigned long)last_side);
            else if(frs==NULL)
                printf("MOD: %c DIR: %c STR: %s VEL: %02d FR:%03lu SD:--\n", mod, direction, steer_buf, speed_level, (unsigned long)last_front);
            else if(sds==NULL)
                printf("MOD: %c DIR: %c STR: %s VEL: %02d FR:-- SD:%03lu\n", mod, direction, steer_buf, speed_level, (unsigned long)last_side);
            else
                printf("MOD: %c DIR: %c STR: %s VEL: %02d FR:-- SD:--\n", mod, direction, steer_buf, speed_level);
        }

        switch(STATE){

        case 0: // STOP
            if(prev_state!=STATE){
                printf(">> STOP MODE\n");
                prev_state=STATE;
            }

            GPIO_write(STATE_LED,0);

            // ===== FIX 02: STOP 시 무조건 Forward 방향 설정 =====
            drive_speed(0,0,'F');
            GPIO_write(DIR_PIN,1);
            direction='F';

            if(key=='M'){ STATE=1; speed_level=0; steer_level=0; direction='F'; GPIO_write(DIR_PIN,1); }
            if(key=='A'){ STATE=11; }            
            break;


        case 1: // MANUAL
            if(prev_state!=STATE){
                printf(">> MANUAL MODE\n");
                prev_state=STATE;
            }

            GPIO_write(STATE_LED,1);

            if(key=='S'){ 
                STATE=0; 
                drive_speed(0,0,'F');  
                break; 
            }
            if(key=='A'){ 
                STATE=11;  
                drive_speed(0,0,'F'); 
                break; 
            }

            if(key=='F'){ 
                direction='F'; 
                GPIO_write(DIR_PIN,1); 
            }
            if(key=='B'){ 
                direction='B'; 
                GPIO_write(DIR_PIN,0); 
            }

            if(key=='U' && speed_level<3) speed_level++;
            if(key=='D' && speed_level>0) speed_level--;

            if(key=='L' && steer_level>-3) steer_level--;
            if(key=='R' && steer_level<3) steer_level++;

            {
                float base  = speed_table[speed_level];
                float steer = steer_level * 0.25f;

                float L = base*(1-steer);
                float R = base*(1+steer);

                if(speed_level>0){
                    if(L<0.60) L=0.60;
                    if(R<0.60) R=0.60;
                }

                drive_speed(L,R,direction);
            }

            /* Print manual state only when it changed to avoid flooding */
            {
                static int prev_speed_level = -1;
                static int prev_steer_level = 999;
                static char prev_direction = '\0';

                if(prev_speed_level != speed_level || prev_steer_level != steer_level || prev_direction != direction || prev_state != STATE){
                    printf("[STATE] Direction=%s, SpeedLevel=V%d (%.2f), Steer=%d\n",
                        (direction=='F'?"FORWARD":"BACKWARD"),
                        speed_level,
                        speed_table[speed_level],
                        steer_level);
                    prev_speed_level = speed_level;
                    prev_steer_level = steer_level;
                    prev_direction = direction;
                }
            }

            break;


        case 11: // AUTO MODE
            if(prev_state!=STATE){
                printf(">> AUTO MODE\n");
                prev_state=STATE;
            }

            if(msTicks - last_blink >= 1000){
                last_blink = msTicks;
                LED_toggle();
            }

            // ===== FIX 01: backward 로 들어와도 forward로 AUTO 실행 =====
            direction = 'F';
            GPIO_write(DIR_PIN,1);   
            drive_speed(0,0,'F');    

            if(key=='S'){ STATE=0; drive_speed(0,0,'F'); break; }
            if(key=='M'){ STATE=1; drive_speed(0,0,'F'); speed_level=0; steer_level=0; direction='F'; GPIO_write(DIR_PIN,1); break; }

            auto_mode_loop(&stopped, &servo_state, &lastTrig1, &lastTrig2);

            break;
        }

        delay_ms(10);
    }
}

