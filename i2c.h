#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

#define SDA_0 GPIO_ResetBits(GPIOA, GPIO_Pin_0)
#define SDA_1 GPIO_SetBits(GPIOA, GPIO_Pin_0)
#define SCL_0 GPIO_ResetBits(GPIOA, GPIO_Pin_1)
#define SCL_1 GPIO_SetBits(GPIOA, GPIO_Pin_1)
#define SDA_VAL (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0))



void i2c_init(void);
void i2c_start(void);
void i2c_stop(void);
uint8_t i2c_write(uint8_t u8Data);
uint8_t i2c_read(uint8_t u8Ack);
void My_I2C_Init(void);
uint8_t I2C_Write(uint8_t Address, uint8_t *pData, uint8_t length);
uint8_t I2C_Read(uint8_t Address, uint8_t *pData, uint8_t length);






// void send(uint8_t u8Data)
// {
// 	uint8_t i;
	
// 	for (i = 0; i < 8; ++i) {
// 		if (u8Data & 0x80) {
// 			GPIO_SetBits(GPIOA, GPIO_Pin_0);
// 			Delay_Ms(4);
// 			GPIO_ResetBits(GPIOA, GPIO_Pin_0);
// 			Delay_Ms(1);
// 		} else {
// 			GPIO_SetBits(GPIOA, GPIO_Pin_0);
// 			Delay_Ms(1);
// 			GPIO_ResetBits(GPIOA, GPIO_Pin_0);
// 			Delay_Ms(4);
// 		}
// 		u8Data <<= 1;
// 	}
// }