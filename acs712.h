#ifndef ACS712_H
#define ACS712_H

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"

void ACS712_Init(void);
float ACS712_GetCurrent(void); // Tra ve gia tri dong dien Ampe

#endif