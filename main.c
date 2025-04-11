#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>
#include "delay.h"
#include "uart.h"
#include "timer.h"
#include "dht11.h"
#include "i2c_lcd.h"








int main(void)
{ 
	
	GPIO_InitTypeDef gpioInit;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioInit.GPIO_Pin = GPIO_Pin_13;
    gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
	 
    GPIO_Init(GPIOC, &gpioInit);
	  
    //GPIO_PWM_Config();  
    //PWM1_Init(10);  // Duty cycle 10%
    //PWM2_Init(25);  // Duty cycle 25%
    //PWM3_Init(40);  // Duty cycle 40%
    //PWM4_Init(80);  // Duty cycle 80%

	TIM2_Init();
	UART_Init();
	DHT11_Init();

//	GPIO_SetBits (GPIOC,GPIO_Pin_13);
//	Delay_Ms(200);
//	GPIO_ResetBits(GPIOC,GPIO_Pin_13);
//	Delay_Ms(200);
//	GPIO_SetBits (GPIOC,GPIO_Pin_13);
//	Delay_Ms(200);
//	GPIO_ResetBits(GPIOC,GPIO_Pin_13);
//	Delay_Ms(200);
//	UART_SendString("Hello, UART! \r\n");
//	I2C_LCD_Init();
//	I2C_LCD_Clear();
//	I2C_LCD_BackLight(1);
//	I2C_LCD_Puts("huy");
//	I2C_LCD_NewLine();
//	I2C_LCD_Puts("I2C: PA0 - PA1");







	while(1)
	{
		
		DHT11_Check();
		char* dulieu=DHT11_Data();
		UART_SendString(dulieu);
		GPIO_SetBits(GPIOC, GPIO_Pin_13);
		Delay_Ms(500);
		GPIO_ResetBits(GPIOC, GPIO_Pin_13);
		Delay_Ms(500);
	}
	
	return 0;
	
}