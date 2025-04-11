
#ifndef I2C_LCD_H
#define I2C_LCD_H

#include "stm32f10x.h"
#include <stdint.h>

#define I2C_LCD_ADDR 0x4E  

void I2C_LCD_Init(void);
void I2C_LCD_Puts(char *szStr);
void I2C_LCD_Clear(void);
void I2C_LCD_NewLine(void);
void I2C_LCD_BackLight(uint8_t u8BackLight);

#endif