
#include "motor.h"
#include "delay.h"


/* === Bi?n debounce d? ki?m tra công t?c hành trình === */
static uint8_t roof_open_debounce = 0;    // Counter cho công t?c OPEN (PB7)
static uint8_t roof_close_debounce = 0;   // Counter cho công t?c CLOSE (PB8)

#define DEBOUNCE_THRESHOLD 3  // Ph?i d?c 3 l?n liên ti?p m?i xác nh?n

void Motor_Init(void) {
    GPIO_InitTypeDef  GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;

    /* 1. Cap clock cho GPIOA, GPIOB, TIM1, TIM3 va AFIO (de remap neu can) */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | 
                           RCC_APB2Periph_TIM1 | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    /* 2. Cau hinh chan PWM: PA8 (TIM1_CH1) va PB1 (TIM3_CH4) */
    /* Kieu AF_PP: Alternate Function Push-Pull de Timer dieu khien chan */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; 
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* 3. Cau hinh chan Huong: PB14, PB15 */
    /* Kieu Out_PP: Output Push-Pull de STM32 xuat muc 0/1 vao cau H */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* 4. Thiet lap thoi gian cho Timer (PWM 1kHz) */
    /* Tan so = 72MHz / (Prescaler * Period) = 72MHz / (72 * 1000) = 1000Hz */
    TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;
    TIM_TimeBaseStructure.TIM_Period = 1000 - 1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    /* 5. Cau hinh che do PWM Mode 1 */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

    TIM_OCInitStructure.TIM_Pulse = 0; /* Xuat xung 0% de bom nuoc */
    TIM_OC1Init(TIM1, &TIM_OCInitStructure); /* Kenh 1 cua TIM1 (PA8) */

    TIM_OCInitStructure.TIM_Pulse = 0; /* Ban dau toc do bang 0 */
    TIM_OC4Init(TIM3, &TIM_OCInitStructure); /* Kenh 4 cua TIM3 (PB1) */

    /* 6. Cho phep Timer hoat dong */
    TIM_Cmd(TIM1, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
		
		/* ==================================================================== */
    /* ?? [B? SUNG M?I] C?U HÌNH 2 CHÂN CÔNG T?C HÀNH TRÌNH PB7 VÀ PB8         */
    /* ==================================================================== */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;         // Dùng di?n tr? kéo lên n?i
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8; // C?u hình chân PB7 và PB8
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* Lenh bat buoc de TIM1 co the xuat xung PWM ra chan PA8 */
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
}

/* Cap nhat toc do Motor 1 (0-100%) */
void Motor1_SetSpeed(uint16_t speed) {
    if (speed > 100) speed = 100;
    TIM_SetCompare4(TIM3, speed * 10); /* Nhan 10 vi Period la 1000 */
}

/* Dieu khien huong Motor 1 (Bom) */
void Motor1_Dir(uint8_t dir) {
    if (dir == MOTOR_FORWARD) {
        GPIO_SetBits(GPIOB, GPIO_Pin_14); GPIO_ResetBits(GPIOB, GPIO_Pin_15);
    } else if (dir == MOTOR_BACKWARD) {
        GPIO_ResetBits(GPIOB, GPIO_Pin_14); GPIO_SetBits(GPIOB, GPIO_Pin_15);
    } else {
        GPIO_ResetBits(GPIOB, GPIO_Pin_14 | GPIO_Pin_15);
    }
}

/**
 * @brief Set t?c d? bom nu?c (0-100%)
 * ?? FIX: Hàm này nh?n PWM t? 0-100, không c? d?nh 50%
 */
void Pump_SetSpeed(uint16_t speed) {
    if (speed > 100) speed = 100;
    TIM_SetCompare1(TIM1, speed * 10); // Nhân 10 vì Period là 1000
}

/**
 * @brief B?t bom (100%)
 */
void Pump_On(void) {
    Pump_SetSpeed(100);  // B?t 100%
}

/**
 * @brief T?t bom (0%)
 */
void Pump_Off(void) {
    Pump_SetSpeed(0);    // T?t 0%
}

/**
 * @brief Giám sát hành trình mái che - Không ch?n lu?ng (Non-blocking debounce)
 * ?? FIX: Dùng software debounce thay vì Delay_Ms
 */
void Motor_Roof_Safety_Supervisor(uint8_t *roof_status, uint8_t *update_flag) {
    
    // === TRU?NG H?P 1: Mái dang M? (MOTOR_FORWARD) - Ki?m tra công t?c OPEN (PB7) ===
    if (*roof_status == MOTOR_FORWARD) {
        // N?u công t?c OPEN b? ch?m (m?c 0 = Bit_RESET)
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_7) == Bit_RESET) {
            roof_open_debounce++;  // Tang counter
            
            // N?u d?c du?c 3 l?n liên ti?p ? xác nh?n ch?m công t?c
            if (roof_open_debounce >= DEBOUNCE_THRESHOLD) {
                *roof_status = MOTOR_STOP;  // Ép tr?ng thái v? STOP
                *update_flag = 1;           // Báo c?p nh?t LCD + Web
                roof_open_debounce = 0;     // Reset counter
            }
        } else {
            // N?u công t?c th? ra (m?c 1) ? reset counter
            roof_open_debounce = 0;
        }
    } else {
        // N?u không ph?i MOTOR_FORWARD ? reset counter
        roof_open_debounce = 0;
    }

    // === TRU?NG H?P 2: Mái dang ÐÓNG (MOTOR_BACKWARD) - Ki?m tra công t?c CLOSE (PB8) ===
    if (*roof_status == MOTOR_BACKWARD) {
        // N?u công t?c CLOSE b? ch?m (m?c 0 = Bit_RESET)
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_8) == Bit_RESET) {
            roof_close_debounce++;  // Tang counter
            
            // N?u d?c du?c 3 l?n liên ti?p ? xác nh?n ch?m công t?c
            if (roof_close_debounce >= DEBOUNCE_THRESHOLD) {
                *roof_status = MOTOR_STOP;  // Ép tr?ng thái v? STOP
                *update_flag = 1;           // Báo c?p nh?t LCD + Web
                roof_close_debounce = 0;    // Reset counter
            }
        } else {
            // N?u công t?c th? ra (m?c 1) ? reset counter
            roof_close_debounce = 0;
        }
    } else {
        // N?u không ph?i MOTOR_BACKWARD ? reset counter
        roof_close_debounce = 0;
    }
}