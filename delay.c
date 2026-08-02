#include "delay.h"

void Delay1MS(void) 
{
	TIM_SetCounter(TIM2,0);
	while (TIM_GetCounter(TIM2)<1000) {
	}
}
void Delay_Ms(uint32_t u32DelayInMs)
{
	while (u32DelayInMs){
	    Delay1MS();
		  --u32DelayInMs;
	}
}

void Delay_Us(uint32_t Delay)
{
	TIM_SetCounter(TIM2,0);
	while (TIM_GetCounter(TIM2)<Delay) {
	}
}

/**
 * @brief L?y th?i gian hi?n t?i (ms) d?a trên TIM4 free-running counter
 * ?? FIX BUG #7: Dùng cho debounce timestamp-based trong menu_control.c
 */
uint32_t millis(void)
{
    static uint16_t last_counter = 0;
    static uint32_t millis_counter = 0;
    uint16_t now_counter = TIM_GetCounter(TIM4); // TIM4 ch?y ? 1 MHz (1 tick = 1 us)
    
    // Ki?m tra wrap-around (TIM4 16-bit => wrap m?i 65536 us = ~65.5 ms)
    if (now_counter < last_counter) {
        millis_counter += 65; // C?ng thêm ~65ms khi wrap
    }
    
    last_counter = now_counter;
    
    // T?ng ms = (s? l?n wrap × 65) + (counter hi?n t?i / 1000)
    return millis_counter + (now_counter / 1000);
}
