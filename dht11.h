#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
void DHT11_Init(void);
void DHT11_Check(void);
char* DHT11_Data(void);
uint8_t DHT11_GetHumidity(void);
uint8_t DHT11_GetTemperature(void);