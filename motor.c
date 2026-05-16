#include "motor.h"

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

    /* 3. Cau hinh chan Huong: PA9, PB6, PB14, PB15 */
    /* Kieu Out_PP: Output Push-Pull de STM32 xuat muc 0/1 vao cau H */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_14 | GPIO_Pin_15;
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
    TIM_OCInitStructure.TIM_Pulse = 0; /* Ban dau toc do bang 0 */
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

    TIM_OC1Init(TIM1, &TIM_OCInitStructure); /* Kenh 1 cua TIM1 (PA8) */
    TIM_OC4Init(TIM3, &TIM_OCInitStructure); /* Kenh 4 cua TIM3 (PB1) */

    /* 6. Cho phep Timer hoat dong */
    TIM_Cmd(TIM1, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
    
    /* Lenh bat buoc de TIM1 co the xuat xung PWM ra chan PA8 */
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
}

/* Cap nhat toc do Motor 1 (0-100%) */
void Motor1_SetSpeed(uint16_t speed) {
    if (speed > 100) speed = 100;
    TIM_SetCompare4(TIM3, speed * 10); /* Nhan 10 vi Period la 1000 */
}

/* Cap nhat toc do Motor 2 (0-100%) */
void Motor2_SetSpeed(uint16_t speed) {
    if (speed > 100) speed = 100;
    TIM_SetCompare1(TIM1, speed * 10);
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

/* Dieu khien huong Motor 2 (Mai che) */
void Motor2_Dir(uint8_t dir) {
    if (dir == MOTOR_FORWARD) {
        GPIO_SetBits(GPIOA, GPIO_Pin_9); GPIO_ResetBits(GPIOB, GPIO_Pin_6);
    } else if (dir == MOTOR_BACKWARD) {
        GPIO_ResetBits(GPIOA, GPIO_Pin_9); GPIO_SetBits(GPIOB, GPIO_Pin_6);
    } else {
        GPIO_ResetBits(GPIOA, GPIO_Pin_9); GPIO_ResetBits(GPIOB, GPIO_Pin_6);
    }
}
