#include "pwm.h"

// GPIO cấu hình PWM tại PB4 (TIM3_CH1)
void PWM_GPIO_Config(void) {
    GPIO_InitTypeDef gpioInit;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    gpioInit.GPIO_Mode = GPIO_Mode_AF_PP;
    gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
    gpioInit.GPIO_Pin = GPIO_Pin_4; // PB4 - TIM3_CH1
    GPIO_Init(GPIOB, &gpioInit);
}

void PWM_Init(void) {
    TIM_TimeBaseInitTypeDef timInit;
    TIM_OCInitTypeDef pwmInit;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    timInit.TIM_Prescaler = 72 - 1;          // 72MHz / 72 = 1MHz timer clock
    timInit.TIM_CounterMode = TIM_CounterMode_Up;
    timInit.TIM_Period = 100 - 1;            // 1ms period => 1kHz PWM
    timInit.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM3, &timInit);

    pwmInit.TIM_OCMode = TIM_OCMode_PWM1;
    pwmInit.TIM_OutputState = TIM_OutputState_Enable;
    pwmInit.TIM_Pulse = 0; // Mặc định 0%
    pwmInit.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(TIM3, &pwmInit);
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);

    TIM_ARRPreloadConfig(TIM3, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
}

// Đặt duty cycle PWM theo phần trăm (0–100%)
void PWM_SetDutyCycle(uint16_t duty) {
    if (duty > 100) duty = 100;
    uint16_t ccr = (duty * (100 - 1)) / 100; // ARR = 99
    TIM_SetCompare1(TIM3, ccr);
}

// ====== Điều khiển Cầu H L298N ======
// PA4 -> IN1, PA5 -> IN2

void HBridge_GPIO_Config(void) {
    GPIO_InitTypeDef gpioInit;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    gpioInit.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
    gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpioInit);
}

void Motor_Forward(void) {
    GPIO_SetBits(GPIOA, GPIO_Pin_4);
    GPIO_ResetBits(GPIOA, GPIO_Pin_5);
}

void Motor_Backward(void) {
    GPIO_ResetBits(GPIOA, GPIO_Pin_4);
    GPIO_SetBits(GPIOA, GPIO_Pin_5);
}

void Motor_Stop(void) {
    GPIO_ResetBits(GPIOA, GPIO_Pin_4 | GPIO_Pin_5);
}
