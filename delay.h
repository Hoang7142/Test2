#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_gpio.h"

void Delay1MS(void);
void Delay_Ms(uint32_t u32DelayInMs);
void Delay_Us(uint32_t Delay);