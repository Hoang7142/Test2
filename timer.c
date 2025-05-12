#include"timer.h"
void TIM2_Init(void) 
{
    TIM_TimeBaseInitTypeDef timerInit;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    
    timerInit.TIM_CounterMode = TIM_CounterMode_Up;
    timerInit.TIM_Period = 0xFFFF;
    timerInit.TIM_Prescaler = 72 - 1;
    TIM_TimeBaseInit(TIM2, &timerInit);
	  //TIM_ClearFlag(TIM2, TIM_FLAG_Update);
    TIM_Cmd(TIM2, ENABLE);
}


void TIM4_Init(void)
{
    TIM_TimeBaseInitTypeDef timerInit;
    NVIC_InitTypeDef nvicInit;

    // B?t clock cho TIM4
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    // C?u hình TIM4: d?m lên, 50ms ng?t 1 l?n
    timerInit.TIM_CounterMode = TIM_CounterMode_Up;
    timerInit.TIM_Prescaler = 7200 - 1;   // 72MHz / 7200 = 10kHz => 0.1ms/tick
    timerInit.TIM_Period = 10000 - 1;        // 500 tick = 50ms
    timerInit.TIM_ClockDivision = TIM_CKD_DIV1;
    timerInit.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM4, &timerInit);

    // Clear c? update
    TIM_ClearFlag(TIM4, TIM_FLAG_Update);

    // Enable ng?t update
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);

    // C?u hình NVIC cho TIM4
    nvicInit.NVIC_IRQChannel = TIM4_IRQn;
    nvicInit.NVIC_IRQChannelPreemptionPriority = 1;
    nvicInit.NVIC_IRQChannelSubPriority = 1;
    nvicInit.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvicInit);

    // B?t d?u Timer
    TIM_Cmd(TIM4, ENABLE);
}



