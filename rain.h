#ifndef RAIN_H
#define RAIN_H

#include "stm32f10x.h"
#include <stdint.h>
#include "stm32f10x_adc.h"
#include "stm32f10x_gpio.h"

void Rain_Init(void);
uint8_t Rain_Read(void);
char* Rain_String(void);

#endif
