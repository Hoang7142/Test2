#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>
#include "delay.h"
#include "uart.h"
#include "timer.h"
#include "dht11.h"
#include "i2c_lcd.h"
#include "pwm.h"
#include "adc.h"




//int main(void)
//{ 
//	
//	GPIO_InitTypeDef gpioInit;
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
//    gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
//    gpioInit.GPIO_Pin = GPIO_Pin_13;
//    gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
//	 
//    GPIO_Init(GPIOC, &gpioInit);
//	  


//	TIM2_Init();
//	UART_Init();
//	DHT11_Init();


//	I2C_LCD_Init();
//	I2C_LCD_Clear();
//	I2C_LCD_BackLight(1);
//	I2C_LCD_Puts("hoang");
//	I2C_LCD_NewLine();
//	I2C_LCD_Puts("I2C: PA0 - PA1");







//	while(1)
//	{
//		
//	  DHT11_Check();
//		char* dulieu=DHT11_Data();
//		UART_SendString(dulieu);
//		GPIO_SetBits(GPIOC, GPIO_Pin_13);
//		Delay_Ms(500);
//		GPIO_ResetBits(GPIOC, GPIO_Pin_13);
//		Delay_Ms(500);
//		    
//	}
	
//	return 0;
//	
//}
//int main(void)
//{
//	GPIO_InitTypeDef gpioInit;
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
//    gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
//    gpioInit.GPIO_Pin = GPIO_Pin_13;
//    gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_Init(GPIOC, &gpioInit);

//	TIM2_Init();
//	UART_Init();
//	DHT11_Init();

//	// Delay nh? d? c?m bi?n DHT11 ?n d?nh tru?c
//	Delay_Ms(1000); 

//	DHT11_Check(); // ?? ki?m tra DHT11 tru?c
//	char* dulieu = DHT11_Data();  // ?? d?c d? li?u t? c?m bi?n
//	UART_SendString(dulieu);      // G?i UART ki?m tra
//	Delay_Ms(1000);

//	// Bây gi? m?i kh?i d?ng LCD
//	I2C_LCD_Init();
//	I2C_LCD_Clear();
//	I2C_LCD_BackLight(1);
//	I2C_LCD_Puts("DHT11 Ready");
//	I2C_LCD_NewLine();
//	I2C_LCD_Puts(dulieu);

//	while(1) {
//		DHT11_Check();
//		dulieu = DHT11_Data();

//		I2C_LCD_Clear();
//		
//		I2C_LCD_Puts("DHT11:");
//		I2C_LCD_NewLine();
//		I2C_LCD_Puts(dulieu);
//		I2C_LCD_NewLine();
//		UART_SendString(dulieu);
//		Delay_Ms(2000);
//	}
//}

//int main(void)
//{
//    PWM_GPIO_Config();      // C?u hình PB4 cho PWM
//    PWM_Init();             // C?u hình TIM3 PWM
//    HBridge_GPIO_Config();  // C?u hình c?u H

//    Motor_Forward();        // Cho quay thu?n
//    PWM_SetDutyCycle(30);   // Ð? r?ng xung 30%

//    while (1)
//    {
//        // PWM ch?y t? d?ng
//    }
//}


int main(void) {
    UART_Init();     
//    I2C_LCD_Init();         // Kh?i t?o LCD
//    I2C_LCD_Clear();        // Xóa màn hình

    ADC_InitConfig();       // Kh?i t?o ADC

    while (1) {
        char* do_am = GetDoAmDatString();  // L?y chu?i d? ?m d?t
//        I2C_LCD_Clear();                   // Xóa LCD
//        I2C_LCD_Puts(do_am);              // In chu?i lên LCD
//        Delay_Ms(1000);                   // Delay 1s
			UART_SendString(do_am);
 	    Delay_Ms(500);
    }
}



