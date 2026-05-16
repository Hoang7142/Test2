#ifndef WATER_LEVEL_H
#define WATER_LEVEL_H

#include "stm32f10x.h"
#include <stdint.h>
#include "stm32f10x_adc.h"
#include "stm32f10x_gpio.h"

// khoi tao cam bien muc nuoc
void WaterLevel_Init(void);

// doc gia tri ADC (0-4095)
uint16_t WaterLevel_ReadRaw(void);

// tra muc nuoc %
uint8_t WaterLevel_GetPercent(void);

// tra chuoi hien thi
char* WaterLevel_String(void);

#endif
