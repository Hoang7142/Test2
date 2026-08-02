#include "flow_sensor.h"
#include "delay.h"

/*
 * TIM3 chung: Motor TimeBase+CH4 PWM (PB1); Flow PartialRemap CH1 IC (PB4).
 * Chi cau hinh CH1/IT_CC1 — khong doi PSC/ARR/CH4. Motor_Init truoc FlowSensor_Init.
 */

volatile uint32_t pulse_count = 0;

static volatile uint16_t s_flow_lpm_x10 = 0;
static uint32_t s_last_calc_ms = 0;

#define FLOW_CALC_WINDOW_MS  500u

void FlowSensor_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_ICInitTypeDef TIM_ICInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStructure.TIM_ICFilter = 0x0F;
    TIM_ICInit(TIM3, &TIM_ICInitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_ITConfig(TIM3, TIM_IT_CC1, ENABLE);

    pulse_count = 0;
    s_flow_lpm_x10 = 0;
    s_last_calc_ms = millis();
}

void TIM3_IRQHandler(void) {
    if (TIM_GetITStatus(TIM3, TIM_IT_CC1) != RESET) {
        pulse_count++;
        TIM_ClearITPendingBit(TIM3, TIM_IT_CC1);
    }
}

uint16_t Flow_GetLpmX10(void) {
    uint32_t now = millis();
    uint32_t dt = now - s_last_calc_ms;

    if (dt >= FLOW_CALC_WINDOW_MS) {
        uint32_t pulses;
        __disable_irq();
        pulses = pulse_count;
        pulse_count = 0;
        __enable_irq();
        s_last_calc_ms = now;
        /* YF-S201: Q=F/7.5 -> Q_x10 = pulses*100000/(dt*75) */
        s_flow_lpm_x10 = (uint16_t)((pulses * 100000u) / (dt * 75u));
    }
    return s_flow_lpm_x10;
}
