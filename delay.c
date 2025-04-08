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