#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>
#include "delay.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_gpio.h"

void UART_Init(void);
void UART_SendString(char *str);
void UART_SendChar(char c);