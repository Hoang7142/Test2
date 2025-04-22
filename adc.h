#ifndef ADC_H
#define ADC_H

#include "stm32f10x.h"
#include <stdint.h>
#include "stm32f10x_adc.h"
#include "stm32f10x_gpio.h"

void ADC_InitConfig(void);
char* GetDoAmDatString(void);

#endif