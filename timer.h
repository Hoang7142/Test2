#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
void TIM2_Init(void);           /* delay.c — do not use for other timing */
void TIM4_HeartbeatInit(void); /* free-running 1 us/tick, debug heartbeat */
