#ifndef RELAY_H
#define RELAY_H

#include "stm32f10x.h"
#include <stdio.h>
#include "stm32f10x_gpio.h"

// khoi tao relay
void Relay_Init(void);

// bat relay (bom chay)
void Relay_On(void);

// tat relay (bom dung)
void Relay_Off(void);

// dao trang thai relay
void Relay_Toggle(void);

// lay trang thai relay (0: tat, 1: bat)
uint8_t Relay_GetState(void);

#endif
