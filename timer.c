#include "timer.h"

/**
 * TIM2 one-shot timebase for delay.c only (TIM_SetCounter resets it).
 * At SYSCLK 72 MHz, APB1 prescaler /2 => TIM2 kernel clock 72 MHz.
 * PSC 72-1 => 1 MHz counter (1 tick = 1 us); ARR 0xFFFF.
 */
void TIM2_Init(void)
{
    TIM_TimeBaseInitTypeDef timerInit;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    timerInit.TIM_Prescaler = 72 - 1;
    timerInit.TIM_CounterMode = TIM_CounterMode_Up;
    timerInit.TIM_Period = 0xFFFF;
    timerInit.TIM_ClockDivision = TIM_CKD_DIV1;
    timerInit.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &timerInit);

    TIM_ClearFlag(TIM2, TIM_FLAG_Update);
    TIM_SetCounter(TIM2, 0);
    TIM_Cmd(TIM2, ENABLE);
}

/**
 * TIM4 free-running us counter for debug heartbeat (never reset by delay.c).
 * Same 1 MHz tick rate as TIM2; ARR 0xFFFF => ~65.5 ms wrap.
 */
void TIM4_HeartbeatInit(void)
{
    TIM_TimeBaseInitTypeDef timerInit;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    timerInit.TIM_Prescaler = 72 - 1;
    timerInit.TIM_CounterMode = TIM_CounterMode_Up;
    timerInit.TIM_Period = 0xFFFF;
    timerInit.TIM_ClockDivision = TIM_CKD_DIV1;
    timerInit.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM4, &timerInit);

    TIM_ClearFlag(TIM4, TIM_FLAG_Update);
    TIM_SetCounter(TIM4, 0);
    TIM_Cmd(TIM4, ENABLE);
}
