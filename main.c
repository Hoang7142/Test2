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
//	
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
//	ADC_InitConfig();

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
//		char* do_am = GetDoAmDatString();

//		I2C_LCD_Clear();
//		
//		I2C_LCD_Puts(dulieu);
//		I2C_LCD_NewLine();
//		I2C_LCD_Puts(do_am);
//		I2C_LCD_NewLine();
//		UART_SendString(dulieu);
//		Delay_Ms(2000);
//	}
//}

//int main(void)
//{
//	  TIM2_Init();
//    PWM_GPIO_Config();      // C?u hình PB4 cho PWM
//    PWM_Init();             // C?u hình TIM3 PWM
//    HBridge_GPIO_Config();  // C?u hình c?u H
//	  Button_GPIO_Config();

//    Motor_Forward();       // Cho quay thu?n
//    PWM_SetDutyCycle(70);   // Ð? r?ng xung 30%
//	while(1){
//		PWM_ControlWithButton();         // Tang t?c d?
//    Motor_DirectionControlWithButton(); // Ði?u khi?n chi?u quay
//    Delay_Ms(20); // Ch?ng d?i nút
//	}


//}


int main(void) {
	  TIM2_Init();
    UART_Init();  
    Delay_Ms(500);	
    I2C_LCD_Init();         // Kh?i t?o LCD
    I2C_LCD_Clear();        // Xóa màn hình

    ADC_InitConfig();       // Kh?i t?o ADC

    while (1) {
        char* do_am = GetDoAmDatString();  // L?y chu?i d? ?m d?t
        I2C_LCD_Clear();                   // Xóa LCD
        I2C_LCD_Puts(do_am);              // In chu?i lên LCD
        //Delay_Ms(500);                   // Delay 1s
			  UART_SendString(do_am);
 	      Delay_Ms(500);
    }
}


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
//	ADC_InitConfig();
//	PWM_GPIO_Config();
//  PWM_Init();
//  HBridge_GPIO_Config();
//  Motor_Forward(); // Kh?i d?u quay thu?n
//  Button_GPIO_Config();

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
//	//I2C_LCD_Puts("hoang");
//	I2C_LCD_NewLine();
//	I2C_LCD_Puts(dulieu);

//	while(1) {
//		DHT11_Check();
//		dulieu = DHT11_Data();
//		char* do_am = GetDoAmDatString();

//		I2C_LCD_Clear();
//		
//		I2C_LCD_Puts(dulieu);
//		I2C_LCD_NewLine();
//		I2C_LCD_Puts(do_am);
//		I2C_LCD_NewLine();
//		UART_SendString(dulieu);
//		Delay_Ms(2000);
//		PWM_ControlWithButton();         // Tang t?c d?
//    Motor_DirectionControlWithButton(); // Ði?u khi?n chi?u quay
//    Delay_Ms(20); // Ch?ng d?i nút
//	}
//}










//// Dinh nghia cac chan
//#define MODE_BUTTON_PIN      GPIO_Pin_9  // PA9 lam nut MODE
//#define MODE_BUTTON_PORT     GPIOA
//#define MODE_LED_PIN         GPIO_Pin_13 // PC13 lam den bao Mode
//#define MODE_LED_PORT        GPIOC

//// Bien trang thai he thong
//typedef enum {
//    MODE_AUTO = 0,
//    MODE_MANUAL
//} Mode_t;

//Mode_t current_mode = MODE_AUTO;
//uint8_t mode_button_prev = 1;

//void System_Init(void) {
//    TIM2_Init();        // Delay
//    
//    UART_Init();        // UART gui thong tin
//    I2C_LCD_Init();     // LCD I2C
//    ADC_InitConfig();   // Cam bien do am dat
//    DHT11_Init();       // DHT11
//    PWM_GPIO_Config();  // Chan PWM
//    PWM_Init();         // Timer PWM
//    HBridge_GPIO_Config(); // Cau H
//    Button_GPIO_Config();  // 2 nut toc do & chieu quay

//    // Nut MODE (PA9)
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
//    GPIO_InitTypeDef gpio;
//    gpio.GPIO_Mode = GPIO_Mode_IPU;
//    gpio.GPIO_Pin = MODE_BUTTON_PIN;
//    gpio.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_Init(MODE_BUTTON_PORT, &gpio);

//    // LED bao che do (PC13)
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
//    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
//    gpio.GPIO_Pin = MODE_LED_PIN;
//    GPIO_Init(MODE_LED_PORT, &gpio);
//    GPIO_ResetBits(MODE_LED_PORT, MODE_LED_PIN);

//    I2C_LCD_Clear();
//    I2C_LCD_BackLight(1);
//		
//		TIM4_Init();        // Ngat 50ms
//}

//void Toggle_Mode(void) {
//    if (current_mode == MODE_AUTO) {
//        current_mode = MODE_MANUAL;
//        GPIO_SetBits(MODE_LED_PORT, MODE_LED_PIN);  // Bat LED bao manual
//    } else {
//        current_mode = MODE_AUTO;
//        GPIO_ResetBits(MODE_LED_PORT, MODE_LED_PIN); // Tat LED bao auto
//    }
//    Delay_Ms(200); // Chong doi nut
//}

//void Auto_Mode_Handler(void) {
//    DHT11_Check();
//    char* temp_humi = DHT11_Data();
//    char* doam = GetDoAmDatString();

//    I2C_LCD_Clear(); // ???
//    I2C_LCD_Puts(temp_humi);
//    I2C_LCD_NewLine();
//    I2C_LCD_Puts(doam);

//    uint8_t temp = DHT11_GetTemperature();
//    uint16_t do_am_dat = GetDoAmDatValue();

//    if (temp > 30 && do_am_dat < 30) {
//        Motor_Forward();
//        PWM_SetDutyCycle(80); // Mo bom toc do cao
//    }
//    else if (do_am_dat > 50) {
//        PWM_SetDutyCycle(0);
//    }

//}

//void Manual_Mode_Handler(void) {
//    PWM_ControlWithButton();         // Nut tang giam toc do
//    Motor_DirectionControlWithButton(); // Nut doi chieu quay
//	  Delay_Ms(20);

//    I2C_LCD_Clear();
//    I2C_LCD_Puts("Manual Mode");
//    I2C_LCD_NewLine();
//    I2C_LCD_Puts("Dieu khien tay");
//}

//// Ng?t Timer4
//void TIM4_IRQHandler(void)
//{
//    if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
//    {
//        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
//        
////        // Ð?o tr?ng thái LED PC13 m?i 50ms
//        GPIOC->ODR ^= GPIO_Pin_13;
//															  // Xu ly nut MODE
//				uint8_t mode_button_now = GPIO_ReadInputDataBit(MODE_BUTTON_PORT, MODE_BUTTON_PIN);
//        if (mode_button_prev == 1 && mode_button_now == 0) {
//            Toggle_Mode();
//        }
//        mode_button_prev = mode_button_now;

//        // Xu ly theo che do
//        if (current_mode == MODE_AUTO) 
//				{
//            Auto_Mode_Handler();
//        } 
//				else 
//				{
//            Manual_Mode_Handler();
//        }
////        Delay_Ms(1000);
//			
//    }
//}

//int main(void) {
//    System_Init();
//    while (1) {



//    }
//}



