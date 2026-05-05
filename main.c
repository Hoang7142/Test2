#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>
#include "delay.h"
#include "timer.h"
#include "dht11.h"
#include "i2c_lcd.h"
#include "pwm.h"
#include "adc.h"
#include "rain.h"
#include "water_level.h"
#include "relay.h"
#include "flow_sensor.h"
#include "analog.h"
#include "motor.h"
#include "lora.h"


/*

int main(void)
{ 
	
	GPIO_InitTypeDef gpioInit;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioInit.GPIO_Pin = GPIO_Pin_13;
    gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
	 
    GPIO_Init(GPIOC, &gpioInit);
	  


	TIM2_Init();
	UART_Init();
	DHT11_Init();


	I2C_LCD_Init();
	I2C_LCD_Clear();
	I2C_LCD_BackLight(1);
	I2C_LCD_Puts("hoang");
	I2C_LCD_NewLine();
	I2C_LCD_Puts("I2C: PA0 - PA1");







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


//int main(void) {
//	  TIM2_Init();
//    UART_Init();  
//    Delay_Ms(500);	
//    I2C_LCD_Init();         // Kh?i t?o LCD
//    I2C_LCD_Clear();        // Xóa màn hình

//    ADC_InitConfig();       // Kh?i t?o ADC

//    while (1) {
//        char* do_am = GetDoAmDatString();  // L?y chu?i d? ?m d?t
//        I2C_LCD_Clear();                   // Xóa LCD
//        I2C_LCD_Puts(do_am);              // In chu?i lên LCD
//        //Delay_Ms(500);                   // Delay 1s
//			  UART_SendString(do_am);
// 	      Delay_Ms(500);
//    }
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










// Dinh nghia cac chan
#define MODE_BUTTON_PIN      GPIO_Pin_9  // PA9 lam nut MODE
#define MODE_BUTTON_PORT     GPIOA
#define MODE_LED_PIN         GPIO_Pin_13 // PC13 lam den bao Mode
#define MODE_LED_PORT        GPIOC

// Bien trang thai he thong
typedef enum {
    MODE_AUTO = 0,
    MODE_MANUAL
} Mode_t;

Mode_t current_mode = MODE_AUTO;
uint8_t mode_button_prev = 1;

void System_Init(void) {
    TIM2_Init();        // Delay
    
    UART_Init();        // UART gui thong tin
    I2C_LCD_Init();     // LCD I2C
    ADC_InitConfig();   // Cam bien do am dat
    DHT11_Init();       // DHT11
    PWM_GPIO_Config();  // Chan PWM
    PWM_Init();         // Timer PWM
    HBridge_GPIO_Config(); // Cau H
    Button_GPIO_Config();  // 2 nut toc do & chieu quay

    // Nut MODE (PA9)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef gpio;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    gpio.GPIO_Pin = MODE_BUTTON_PIN;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MODE_BUTTON_PORT, &gpio);

    // LED bao che do (PC13)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Pin = MODE_LED_PIN;
    GPIO_Init(MODE_LED_PORT, &gpio);
    GPIO_ResetBits(MODE_LED_PORT, MODE_LED_PIN);

    I2C_LCD_Clear();
    I2C_LCD_BackLight(1);
		
		TIM4_Init();        // Ngat 50ms
}

void Toggle_Mode(void) {
    if (current_mode == MODE_AUTO) {
        current_mode = MODE_MANUAL;
        GPIO_SetBits(MODE_LED_PORT, MODE_LED_PIN);  // Bat LED bao manual
    } else {
        current_mode = MODE_AUTO;
        GPIO_ResetBits(MODE_LED_PORT, MODE_LED_PIN); // Tat LED bao auto
    }
    Delay_Ms(200); // Chong doi nut
}

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
void Auto_Mode_Handler(void) {
    DHT11_Check();
    char* temp_humi = DHT11_Data();
    char* doam = GetDoAmDatString();

    I2C_LCD_SetCursor(0, 0); // Ðua con tr? v? d?u dòng 1
    I2C_LCD_Puts(temp_humi);
    I2C_LCD_Puts("        "); // Xóa ph?n du n?u chu?i tru?c dài hon

    I2C_LCD_SetCursor(1, 0); // Ðua con tr? v? d?u dòng 2
    I2C_LCD_Puts(doam);
    I2C_LCD_Puts("        "); // Xóa ph?n du n?u chu?i tru?c dài hon

    uint8_t temp = DHT11_GetTemperature();
    uint16_t do_am_dat = GetDoAmDatValue();

    if (temp > 20 && do_am_dat < 60) {
        Motor_Forward();
        PWM_SetDutyCycle(80); // M? bom t?c d? cao
    }
    else if (do_am_dat > 40) {
        PWM_SetDutyCycle(0);
    }
}

void Manual_Mode_Handler(void) {
    PWM_ControlWithButton();         // Nut tang giam toc do
    Motor_DirectionControlWithButton(); // Nut doi chieu quay
	  Delay_Ms(20);

    I2C_LCD_Clear();
    I2C_LCD_Puts("Manual Mode");
    I2C_LCD_NewLine();
    I2C_LCD_Puts("Dieu khien tay");
}

// Ng?t Timer4
void TIM4_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
        
//        // Ð?o tr?ng thái LED PC13 m?i 50ms
        GPIOC->ODR ^= GPIO_Pin_13;
															  // Xu ly nut MODE
				uint8_t mode_button_now = GPIO_ReadInputDataBit(MODE_BUTTON_PORT, MODE_BUTTON_PIN);
        if (mode_button_prev == 1 && mode_button_now == 0) {
            Toggle_Mode();
        }
        mode_button_prev = mode_button_now;

        // Xu ly theo che do
        if (current_mode == MODE_AUTO) 
				{
            Auto_Mode_Handler();
        } 
				else 
				{
            Manual_Mode_Handler();
        }
//        Delay_Ms(1000);
			
    }
}

int main(void) {
    System_Init();
    while (1) {



    }
}
*/


////////////////////////////////////////////////////////////////////////////////////////////////////////////

//int main(void)
//{
//    TIM2_Init();     // dung delay
//    UART_Init();     // khoi tao uart
//    Rain_Init();     // khoi tao cam bien mua

//    while(1)
//    {
//        char* rain = Rain_String();   // lay trang thai mua

//        UART_SendString(rain);        // gui uart
//        UART_SendString("\r\n");      // xuong dong

//        Delay_Ms(500);               // delay 500ms
//    }
//}


//test cam bien mua//////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

//int main(void)
//{
//    TIM2_Init();
//    UART_Init();
//    WaterLevel_Init();

//    while(1)
//    {
//        char* water = WaterLevel_String();

//        UART_SendString(water);
//        UART_SendString("\r\n");

//        Delay_Ms(500);
//    }
//}

//test cam bien muc nuoc //////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////
//int main(void)
//{
//    // ===== Khoi tao delay =====
//    TIM2_Init();

//    // ===== Khoi tao LED PC13 =====
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
//    GPIO_InitTypeDef gpio;

//    gpio.GPIO_Pin = GPIO_Pin_13;
//    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
//    gpio.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_Init(GPIOC, &gpio);

//    GPIO_SetBits(GPIOC, GPIO_Pin_13); // tat LED (active LOW)

//    // ===== PWM + Motor =====
//    PWM_GPIO_Config();        // PA6 PWM
//    PWM_Init();               // TIM3
//    HBridge_GPIO_Config();    // PA4 PA5

//    // ===== Button =====
//    Button_GPIO_Config();     // PA8 + PA10

//    // ===== Trang thai ban dau =====
//    Motor_Forward();          // quay thuan
//    PWM_SetDutyCycle(0);      // ban dau dung

//    while (1)
//    {
//        // nut tang toc (PA8)
//        PWM_ControlWithButton();

//        // nut doi chieu (PA10)
//        Motor_DirectionControlWithButton();

//        // ===== DEBUG LED =====
//        // nhan PA8 thi LED doi trang thai
//        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8) == 0)
//        {
//            GPIOC->ODR ^= GPIO_Pin_13;
//            Delay_Ms(200); // tranh nhay lien tuc
//        }

//        Delay_Ms(20); // chong doi chung
//    }
//}
////////////////////////test pwm///////////////////////////////////////////



//char display_buffer[16];

//int main(void) {
//    // Khoi tao he thong
//    SystemInit();
//    TIM2_Init();       
//    I2C_LCD_Init();    
//    
//    // Khoi tao 2 cam bien moi
//    ACS712_Init();     
//    FlowSensor_Init(); 

//    I2C_LCD_Clear();
//    I2C_LCD_Puts("TESTING...");
//    Delay_Ms(1000);

//    while(1) {
//        float i_val = ACS712_GetCurrent();
//        float f_val = Flow_GetLitersPerMinute();

//        // In hang 1: Dong dien
//        I2C_LCD_SetCursor(0, 0);
//        sprintf(display_buffer, "I: %.2f A      ", i_val);
//        I2C_LCD_Puts(display_buffer);

//        // In hang 2: Luu luong
//        I2C_LCD_SetCursor(1, 0);
//        sprintf(display_buffer, "F: %.1f L/min  ", f_val);
//        I2C_LCD_Puts(display_buffer);

//        // Cho 1 giay de xem ket qua
//        Delay_Ms(1000); 
//    }
//}
///////////////////////////////////test flow////////////////

//char lcd_buffer[16];

//int main(void)
//{
//    // 1. Kh?i t?o h? th?ng (Clock dã du?c g?i trong các hàm Init con)
//    SystemInit();
//    
//    // 2. Kh?i t?o delay (Timer 2)
//    TIM2_Init();

//    // 3. Kh?i t?o LCD (PB10, PB11)
//    I2C_LCD_Init();
//    I2C_LCD_Clear();
//    I2C_LCD_BackLight(1);
//    I2C_LCD_Puts("System Ready!");
//    Delay_Ms(1000);
//    I2C_LCD_Clear();

//    // 4. Kh?i t?o ACS712 (Ð?c dòng t?i chân PB0)
//    ACS712_Init();

//    // 5. Kh?i t?o LED PC13 (Ð? Debug)
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
//    GPIO_InitTypeDef gpio;
//    gpio.GPIO_Pin = GPIO_Pin_13;
//    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
//    gpio.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_Init(GPIOC, &gpio);
//    GPIO_SetBits(GPIOC, GPIO_Pin_13); // T?t LED (tích c?c m?c th?p)

//    // 6. Kh?i t?o PWM + Motor (PA6, PA4, PA5)
//    PWM_GPIO_Config();
//    PWM_Init();
//    HBridge_GPIO_Config();

//    // 7. Kh?i t?o Button (PA8, PA10)
//    Button_GPIO_Config();

//    // 8. Tr?ng thái ban d?u
//    Motor_Forward();
//    PWM_SetDutyCycle(0); // Ban d?u motor d?ng yên

//    while (1)
//    {
//        // --- X? LÝ NÚT NH?N (Tang t?c & Ð?i chi?u) ---
//        PWM_ControlWithButton();           // Nút PA8: Tang t?c 10% m?i l?n nh?n
//        Motor_DirectionControlWithButton(); // Nút PA10: Ti?n -> Lùi -> D?ng

//        // --- DEBUG LED PC13 ---
//        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8) == 0)
//        {
//            GPIOC->ODR ^= GPIO_Pin_13; // Ð?o tr?ng thái LED khi nh?n PA8
//            // Không dùng Delay_Ms quá l?n ? dây d? tránh làm LCD c?p nh?t ch?m
//        }

//        // --- Ð?C DÒNG ÐI?N VÀ C?P NH?T LCD ---
//        float current_amps = ACS712_GetCurrent();

//        // Hi?n th? hàng 1: Tr?ng thái dòng di?n
//        I2C_LCD_SetCursor(0, 0);
//        sprintf(lcd_buffer, "Current: %.2f A ", current_amps);
//        I2C_LCD_Puts(lcd_buffer);

//        // Hi?n th? hàng 2: Note (Có th? hi?n th? Duty Cycle n?u mu?n)
//        I2C_LCD_SetCursor(1, 0);
//        I2C_LCD_Puts("Monitoring...   ");

//        // Delay ng?n d? vòng l?p ch?y mu?t và nút nh?n v?n nh?y
//        Delay_Ms(50); 
//    }
//}

/////////////////test acs và pwm////////////////////////////////////////////////////////////


//// Khai bao bien toan cuc
//uint8_t nhiet_do = 0, do_am = 0;
//char msg_lcd[16];
//char msg_uart[64];

//int main(void) {
//    // 1. Khoi tao he thong
//    SystemInit();
//    
//    // 2. Khoi tao ngoai vi
//    TIM2_Init();      // Dung cho delay_us va delay_ms
//    UART_Init();      // Test qua UART (9600 baud)
//    I2C_LCD_Init();   // Test qua LCD (I2C2: PB10, PB11)
//	  I2C_LCD_BackLight(1); 
//    DHT11_Init();     // Test cam bien (PB12)

//    // Greeting tren ca 2 kenh
//    I2C_LCD_Clear();
//    I2C_LCD_Puts("DHT11 Testing...");
//    UART_SendString("\r\n--- Kiem tra DHT11 bat dau ---\r\n");
//    
//    Delay_Ms(1000);

//    while(1) {
//        // Doc du lieu tu cam bien
//        uint8_t status = DHT11_ReadData(&nhiet_do, &do_am);
//        
//        I2C_LCD_Clear();
//        I2C_LCD_SetCursor(0, 0);

//        if(status == DHT11_OK) {
//            // --- HIEN THI LEN LCD ---
//            sprintf(msg_lcd, "Temp: %d C", nhiet_do);
//            I2C_LCD_Puts(msg_lcd);
//            I2C_LCD_SetCursor(1, 0);
//            sprintf(msg_lcd, "Humi: %d %%", do_am);
//            I2C_LCD_Puts(msg_lcd);

//            // --- GUI QUA UART ---
//            sprintf(msg_uart, "OK - Nhiet do: %d C, Do am: %d %%\r\n", nhiet_do, do_am);
//            UART_SendString(msg_uart);
//        } 
//        else if(status == DHT11_ERR_NO_RESP) {
//            I2C_LCD_Puts("Error: No Resp");
//            UART_SendString("LOI: Cam bien khong phan hoi!\r\n");
//        } 
//        else if(status == DHT11_ERR_CHKSUM) {
//            I2C_LCD_Puts("Error: Checksum");
//            UART_SendString("LOI: Sai Checksum du lieu!\r\n");
//        }

//        // DHT11 can it nhat 2s de on dinh giua cac lan doc
//        Delay_Ms(2000); 
//    }
//}
///////////////////test dht11 len lcd và uart/////////////////////////////////////


///* Bien luu tru trang thai he thong */
//uint16_t g_speed = 0; 
//uint8_t g_direction = MOTOR_FORWARD;

//void Button_Init(void) {
//    GPIO_InitTypeDef GPIO_InitStructure;
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
//    
//    /* PA0: Nut toc do, PA1: Nut dao chieu */
//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
//    GPIO_Init(GPIOA, &GPIO_InitStructure);
//}

//int main(void) {
//    SystemInit();
//    TIM2_Init();
//    Motor_Init();
//    Button_Init();

//    while(1) {
//        /* --- NUT 1: THAY DOI TOC DO (Tang dan 0-20-40-60-80-100-0) --- */
//        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 0) {
//            Delay_Ms(20); /* Chong rung */
//            if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 0) {
//                g_speed += 20;
//                if (g_speed > 100) g_speed = 0; /* Quay ve 0 neu vuot qua 100 */
//                
//                /* Cap nhat toc do cho ca 2 motor */
//                Motor1_SetSpeed(g_speed);
//                Motor2_SetSpeed(g_speed);
//                
//                /* Neu dang o muc 0 thi cho dung han */
//                if (g_speed == 0) {
//                    Motor1_Dir(MOTOR_STOP);
//                    Motor2_Dir(MOTOR_STOP);
//                } else {
//                    Motor1_Dir(g_direction);
//                    Motor2_Dir(g_direction);
//                }
//                
//                while(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 0); /* Cho nha nut */
//            }
//        }

//        /* --- NUT 2: DAO CHIEU QUAY (Thuan <-> Nghich) --- */
//        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) == 0) {
//            Delay_Ms(20);
//            if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) == 0) {
//                /* Dao trang thai bien direction */
//                if (g_direction == MOTOR_FORWARD) {
//                    g_direction = MOTOR_BACKWARD;
//                } else {
//                    g_direction = MOTOR_FORWARD;
//                }
//                
//                /* Neu motor dang quay (speed > 0) thi moi ap dung dao chieu ngay */
//                if (g_speed > 0) {
//                    Motor1_Dir(g_direction);
//                    Motor2_Dir(g_direction);
//                }
//                
//                while(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) == 0);
//            }
//        }
//        
//        Delay_Ms(10);
//    }
//}
//////////////////////////////////////////test pwm/////////////////////////



///* ===== BI?N ===== */
//Analog_Data_t sensor_test;
//char msg[64];

//int main(void)
//{
//    /* ===== KH?I T?O ===== */
//    SystemInit();
//    TIM2_Init();      // dùng delay
//    UART_Init();      // UART 9600
//    Analog_Init();    // ADC + PB0 (ACS712)

//    UART_SendString("=== TEST ACS712 (KHONG MOTOR) ===\r\n");

//    while(1)
//    {
//        /* ===== Ð?C D? LI?U ÐÃ L?C ===== */
//        Analog_UpdateAll(&sensor_test);

//        /* ===== Ð?C RAW Ð? SO SÁNH ===== */
//        uint16_t raw = ADC_Read_Raw(ADC_Channel_8);

//        /* ===== Ð?C SAU FILTER ===== */
//        uint16_t filtered = ADC_Read_Filter(ADC_Channel_8);

//        /* ===== IN UART ===== */
//        sprintf(msg,
//                "Raw: %4d | Filter: %4d | Current: %.3f A\r\n",
//                raw,
//                filtered,
//                sensor_test.current_ampe);

//        UART_SendString(msg);

//        /* ===== T?C Ð? 10 M?U / GIÂY ===== */
//        Delay_Ms(100);
//    }
//}

//////////////////////////////////////////////////////////// khi khong co moto////////////////////////////
//#include "stm32f10x.h"
//#include "delay.h"
//#include "timer.h"
//#include "art.h"
//#include "analog.h"
//#include "motor.h"
//#include <stdio.h>

///* ===== BI?N ===== */
//Analog_Data_t sensor_test;
//char msg[64];

//uint8_t speed = 0;

///* ===== BUTTON PA1 ===== */
//#define BUTTON_PIN GPIO_Pin_1
//#define BUTTON_PORT GPIOA

///* ===== INIT BUTTON ===== */
//void Button_Init(void)
//{
//    GPIO_InitTypeDef gpio;

//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

//    gpio.GPIO_Pin = BUTTON_PIN;
//    gpio.GPIO_Mode = GPIO_Mode_IPU; // kéo lên n?i
//    GPIO_Init(BUTTON_PORT, &gpio);
//}

///* ===== READ BUTTON ===== */
//uint8_t Button_Read(void)
//{
//    return GPIO_ReadInputDataBit(BUTTON_PORT, BUTTON_PIN);
//}

///* ===== MAIN ===== */
//int main(void)
//{   
//	
//    /* ===== KH?I T?O ===== */
//    SystemInit();
//    TIM2_Init();
//    UART_Init();
//    Analog_Init();
//    Motor_Init();
//    Button_Init();
//	  Delay_Ms(500);        // cho on dinh
//    Analog_Calibrate();   // ? calibrate ACS712

//    UART_SendString("=== TEST MOTOR + ACS712 ===\r\n");

//    /* Motor quay 1 chi?u */
//    Motor1_Dir(MOTOR_FORWARD);
//    Motor1_SetSpeed(0);

//    uint8_t last_state = 1;

//    while(1)
//    {
//        /* ===== NÚT NH?N TANG T?C ===== */
//        uint8_t current_state = Button_Read();

//        /* B?t c?nh nh?n (1 ? 0) */
//        if (last_state == 1 && current_state == 0)
//        {
//            speed += 10;
//            if (speed > 100) speed = 0;

//            Motor1_SetSpeed(speed);
//        }

//        last_state = current_state;

//        /* ===== Ð?C C?M BI?N ===== */
//        Analog_UpdateAll(&sensor_test);

//        /* ===== DEBUG RAW + FILTER ===== */
//        uint16_t raw = ADC_Read_Raw(ADC_Channel_8);
//        uint16_t filter = ADC_Read_Filter(ADC_Channel_8);

//        /* ===== IN UART ===== */
//        sprintf(msg,
//                "Speed: %3d%% | Raw: %4d | Filter: %4d | Current: %.3f A\r\n",
//                speed,
//                raw,
//                filter,
//                sensor_test.current_ampe);

//        UART_SendString(msg);

//        /* ===== DELAY ===== */
//        Delay_Ms(100);
//    }
//}
///////////////////////////////////////////////////////////////////loi khong hien thi len uart////////////////
//uint8_t speed = 0;
//uint8_t last_state = 1;

///* ===== BUTTON PA1 ===== */
//#define BUTTON_PIN GPIO_Pin_1
//#define BUTTON_PORT GPIOA
///* ===== INIT BUTTON ===== */
//void Button_Init(void)
//{
//    GPIO_InitTypeDef gpio;

//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

//    gpio.GPIO_Pin = BUTTON_PIN;
//    gpio.GPIO_Mode = GPIO_Mode_IPU; // kéo lên n?i
//    GPIO_Init(BUTTON_PORT, &gpio);
//}

///* ===== READ BUTTON ===== */
//uint8_t Button_Read(void)
//{
//    return GPIO_ReadInputDataBit(BUTTON_PORT, BUTTON_PIN);
//}
///* ===== BI?N ===== */
//Analog_Data_t sensor_test;
//char msg[64];

//int main(void)
//{
//    /* ===== KH?I T?O ===== */
//	  Motor_Init();
//    Button_Init();
//    SystemInit();
//    TIM2_Init();      // dùng delay
//    UART_Init();      // UART 9600
//    Analog_Init();    // ADC + PB0 (ACS712)
//	  Delay_Ms(500);        // cho on dinh
//    Analog_Calibrate();   // ? calibrate ACS712

//    UART_SendString("=== TEST ACS712 (KHONG MOTOR) ===\r\n");

//while(1)
//{
//    /* ===== BUTTON ===== */
//    uint8_t current_state = Button_Read();

//    if (last_state == 1 && current_state == 0)
//    {
//        speed += 10;
//        if (speed > 100) speed = 0;

//        Motor1_SetSpeed(speed);

//        sprintf(msg, "SET SPEED: %d%%\r\n", speed);
//        UART_SendString(msg);
//    }

//    last_state = current_state;

//    /* ===== ADC ===== */
//    Analog_UpdateAll(&sensor_test);

//    uint16_t raw = ADC_Read_Raw(ADC_Channel_8);
//    uint16_t filtered = ADC_Read_Filter(ADC_Channel_8);

//    sprintf(msg,
//            "Raw: %4d | Filter: %4d | Current: %.3f A\r\n",
//            raw,
//            filtered,
//            sensor_test.current_ampe);

//    UART_SendString(msg);

//    Delay_Ms(100);
//}
//}
///////////////////////////////hien thi len uart ma moto k chay/////////////////////////////
//#include "stm32f10x.h"
//#include "delay.h"
//#include "timer.h"
//#include "analog.h"
//#include "motor.h"

///* ===== BI?N ===== */
//uint8_t speed = 0;
//uint8_t last_state = 1;

///* ===== BUTTON PA1 ===== */
//#define BUTTON_PIN GPIO_Pin_1
//#define BUTTON_PORT GPIOA

///* ===== INIT BUTTON ===== */
//void Button_Init(void)
//{
//    GPIO_InitTypeDef gpio;

//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

//    gpio.GPIO_Pin = BUTTON_PIN;
//    gpio.GPIO_Mode = GPIO_Mode_IPU; // kéo lên
//    GPIO_Init(BUTTON_PORT, &gpio);
//}

///* ===== READ BUTTON ===== */
//uint8_t Button_Read(void)
//{
//    return GPIO_ReadInputDataBit(BUTTON_PORT, BUTTON_PIN);
//}

///* ===== MAIN ===== */
//int main(void)
//{
//    /* ===== KH?I T?O (ÐÚNG TH? T?) ===== */
//    SystemInit();
//    TIM2_Init();      // delay
//    Analog_Init();    // n?u không c?n có th? b?
//    Motor_Init();
//    Button_Init();
//	  Delay_Ms(500);        // cho on dinh
//    Analog_Calibrate();   // ? calibrate ACS712

//    /* Cho motor quay 1 chi?u */
//    Motor1_Dir(MOTOR_FORWARD);
//    Motor1_SetSpeed(0);

//    while(1)
//    {
//        uint8_t current_state = Button_Read();

//        /* Nh?n nút tang t?c */
//        if (last_state == 1 && current_state == 0)
//        {
//            speed += 10;
//            if (speed > 100) speed = 0;

//            Motor1_SetSpeed(speed);
//        }

//        last_state = current_state;

//        Delay_Ms(100);
//    }
//}
////////////////////////////////test co moto k uart///////////////////////////

//#include "stm32f10x.h"
//#include "delay.h"
//#include "art.h"
//#include "i2c_lcd.h"
//#include "dht11.h"
//#include <stdio.h>

//// Khai bao bien toan cuc
//uint8_t nhiet_do = 0, do_am = 0;
//char msg_lcd[17]; 
//char msg_uart[100];

//int main(void) {
//    // 1. Khoi tao he thong
//    SystemInit();
//    
//    // 2. Khoi tao ngoai vi (Da sua theo dung thu vien ban gui)
//    TIM2_Init();        // Dung ten TIM2_Init nhu trong file delay.c cua ban
//    UART_Init();        // Dung ten UART_Init() khong tham so
//    I2C_LCD_Init();     
//    I2C_LCD_BackLight(1); 
//    
//    DHT11_Init();       // Su dung ham Init moi co Delay 1s

//    // Greeting
//    I2C_LCD_Clear();
//    I2C_LCD_SetCursor(0, 0);
//    I2C_LCD_Puts("System Ready!");
//    
//    UART_SendString("\r\n--- TEST DHT11 KET HOP THU VIEN CU ---\r\n");

//    while(1) {
//        // Doc du lieu tu cam bien (Dung ham DHT11_ReadData moi co Timeout)
//        uint8_t status = DHT11_ReadData(&nhiet_do, &do_am);
//        
//        if(status == 0) { // DHT11_OK = 0
//            // Hien thi LCD mu?t, không nháy
//            I2C_LCD_SetCursor(0, 0);
//            sprintf(msg_lcd, "Temp: %2d C      ", nhiet_do); 
//            I2C_LCD_Puts(msg_lcd);
//            
//            I2C_LCD_SetCursor(1, 0);
//            sprintf(msg_lcd, "Humi: %2d %%      ", do_am);
//            I2C_LCD_Puts(msg_lcd);

//            // Gui UART
//            sprintf(msg_uart, "Data: T=%d, H=%d\r\n", nhiet_do, do_am);
//            UART_SendString(msg_uart);
//        } 
//        else {
//            I2C_LCD_SetCursor(0, 0);
//            I2C_LCD_Puts("Sensor Error!   ");
//            UART_SendString("Loi doc DHT11!\r\n");
//        }

//        Delay_Ms(2000); 
//    }
//}
////////////////////////////////////////////test dht11 moi//////////////////////////////////////////////
//#include "stm32f10x.h"
//#include "delay.h"
//#include "timer.h"
//#include "analog.h"
//#include "motor.h"
//#include "i2c_lcd.h"
//#include <stdio.h>

///* ===== BI?N ===== */
//Analog_Data_t sensor_test;
//uint8_t speed = 0;
//uint8_t last_state = 1;

//char line1[16];
//char line2[16];

///* ===== BUTTON PA1 ===== */
//#define BUTTON_PIN GPIO_Pin_1
//#define BUTTON_PORT GPIOA

///* ===== INIT BUTTON ===== */
//void Button_Init(void)
//{
//    GPIO_InitTypeDef gpio;

//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

//    gpio.GPIO_Pin = BUTTON_PIN;
//    gpio.GPIO_Mode = GPIO_Mode_IPU;
//    GPIO_Init(BUTTON_PORT, &gpio);
//}

///* ===== READ BUTTON ===== */
//uint8_t Button_Read(void)
//{
//    return GPIO_ReadInputDataBit(BUTTON_PORT, BUTTON_PIN);
//}

///* ===== MAIN ===== */
//int main(void)
//{
//    /* ===== KH?I T?O ===== */
//    SystemInit();
//    TIM2_Init();
//    Analog_Init();
//    Motor_Init();
//    Button_Init();
//    I2C_LCD_Init();         // ? LCD
//    I2C_LCD_BackLight(1);   // b?t dèn n?n

//    Delay_Ms(1000);
//    Analog_Calibrate();     // ? calibrate ACS712

//    /* Motor quay 1 chi?u */
//    Motor1_Dir(MOTOR_FORWARD);
//    Motor1_SetSpeed(0);

//    I2C_LCD_Clear();

//    while(1)
//    {
//        /* ===== NÚT NH?N ===== */
//        uint8_t current_state = Button_Read();

//        if (last_state == 1 && current_state == 0)
//        {
//            speed += 10;
//            if (speed > 100) speed = 0;

//            Motor1_SetSpeed(speed);
//        }
//        last_state = current_state;

//        /* ===== Ð?C ADC ===== */
//        Analog_UpdateAll(&sensor_test);
//        uint16_t raw = ADC_Read_Filter(ADC_Channel_8);
//				float v_pin = (float)raw * 3.3f / 4095.0f;

//        /* ===== FORMAT LCD ===== */
//        //sprintf(line1, "Raw:%4d Sp:%3d", raw, v_pin);
//				int v_int = (int)v_pin;
//        int v_dec = (int)((v_pin - v_int) * 100);

//        sprintf(line1, "Raw:%4d V:%d.%02d", raw, v_int, v_dec);
//        sprintf(line2, "I:%1.2f A      ", sensor_test.current_ampe);

//        /* ===== HI?N TH? LCD ===== */
//        I2C_LCD_SetCursor(0, 0);
//        I2C_LCD_Puts(line1);

//        I2C_LCD_SetCursor(1, 0);
//        I2C_LCD_Puts(line2);

//        Delay_Ms(200);
//    }
//}

////////////////pwm và lcd và acs////////////////////////





//char display_buffer[16];
//float current_flow = 0.0f;

//int main(void) {
//    // 1. Kh?i t?o h? th?ng
//    SystemInit();
//    TIM2_Init();
//    
//    // 2. Kh?i t?o LCD
//    I2C_LCD_Init(); 
//    I2C_LCD_BackLight(1); // QUAN TR?NG: Ph?i b?t dèn n?n lên vì Init xong nó b? t?t
//    
//    // 3. Kh?i t?o Flow Sensor (PB3)
//    FlowSensor_Init(); 

//    I2C_LCD_Clear();
//    I2C_LCD_Puts("IOT SYSTEM");
//    Delay_Ms(1500);
//    I2C_LCD_Clear();

//    while(1) {
//        // 4. Ð?c luu lu?ng (l?y m?u 1 giây)
//        current_flow = Flow_GetLitersPerMinute(1000);

//        // 5. Hi?n th? dòng 1 (Dùng SetCursor thay vì Clear)
//        I2C_LCD_SetCursor(0, 0); // Dòng 0, C?t 0
//        I2C_LCD_Puts("FLOW MONITOR   "); // Kho?ng tr?ng d? xóa ký t? th?a

//        // 6. Hi?n th? dòng 2
//        sprintf(display_buffer, "%.2f L/Min    ", current_flow);
//        I2C_LCD_SetCursor(1, 0); // Dòng 1, C?t 0
//        I2C_LCD_Puts(display_buffer);

//        // 7. Ch? cho chu k? ti?p theo
//        Delay_Ms(1000);
//    }
//}
/////////////////////flow sensor///////////////




//// Khai báo thêm hàm Reset ? dây d? main.c hi?u du?c mà không c?n s?a lora.h
//extern void LoRa_Reset(void); 

//// Hàm kh?i t?o UART1 (PA9, PA10) d? in log ra Hercules
//void UART1_Init(uint32_t baudrate) {
//    GPIO_InitTypeDef GPIO_InitStructure;
//    USART_InitTypeDef USART_InitStructure;
//    
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
//    
//    // TX - PA9
//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_Init(GPIOA, &GPIO_InitStructure);
//    
//    // RX - PA10
//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//    GPIO_Init(GPIOA, &GPIO_InitStructure);
//    
//    USART_InitStructure.USART_BaudRate = baudrate;
//    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
//    USART_InitStructure.USART_StopBits = USART_StopBits_1;
//    USART_InitStructure.USART_Parity = USART_Parity_No;
//    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
//    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
//    
//    USART_Init(USART1, &USART_InitStructure);
//    USART_Cmd(USART1, ENABLE);
//}

//// Hàm g?i chu?i qua UART
//void UART_SendString(char* str) {
//    while (*str) {
//        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
//        USART_SendData(USART1, *str++);
//    }
//}

//int main(void) {
//    char buffer[100];
//    uint8_t version;

//    // Kh?i t?o Timer/Delay theo thu vi?n c?a b?n
//    TIM2_Init(); 
//    
//    UART1_Init(115200);
//    
//    UART_SendString("\r\n=========================================\r\n");
//    UART_SendString("   BAT DAU TEST LORA TREN STM32 (NGUON NGOAI)\r\n");
//    UART_SendString("=========================================\r\n");

//    // 1. Kh?i t?o SPI và c?u hình chân CS(PA4), RST(PA3)
//    LoRa_SPI_Init(); 
//    
//    // 2. B?T BU?C PH?I RESET MODULE Ð? LORA T?NH GI?C
//    LoRa_Reset();
//    Delay_Ms(20); // Ð?i LoRa kh?i d?ng xong sau khi reset
//    
//    while (1) {
//        UART_SendString("\r\nDang kiem tra ket noi voi LoRa Ra-02...\r\n");
//        
//        // Ð?c thanh ghi phiên b?n c?a LoRa (Ð?a ch? 0x42)
//        version = LoRa_Read_Reg(0x42); 
//        
//        sprintf(buffer, "Gia tri doc duoc tai Reg 0x42: 0x%02X\r\n", version);
//        UART_SendString(buffer);

//        // Phân tích k?t qu?
//        if (version == 0x12) {
//            UART_SendString("-> KHOI TAO THANH CONG! STM32 da nhan ra chip LoRa.\r\n");
//        } 
//        else if (version == 0x00) {
//            UART_SendString("-> LOI (0x00): STM32 khong doc duoc tin hieu tu LoRa.\r\n");

//        } 
//        else if (version == 0xFF) {
//            UART_SendString("-> LOI (0xFF): LoRa khong tra loi hoac mat dien.\r\n");

//        } 
//        else {
//            UART_SendString("-> LOI: Nhieu tin hieu. Kiem tra lai day chung GND.\r\n");
//        }
//        
//        // Ch? 2 giây r?i d?c l?i
//        Delay_Ms(2000); 
//    }
//}



// ===== UART =====
void UART1_Init(uint32_t baudrate) {
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef uart;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    // TX PA9
    gpio.GPIO_Pin = GPIO_Pin_9;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    // RX PA10
    gpio.GPIO_Pin = GPIO_Pin_10;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio);

    uart.USART_BaudRate = baudrate;
    uart.USART_WordLength = USART_WordLength_8b;
    uart.USART_StopBits = USART_StopBits_1;
    uart.USART_Parity = USART_Parity_No;
    uart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    uart.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

    USART_Init(USART1, &uart);
    USART_Cmd(USART1, ENABLE);
}

void UART_SendChar(char c) {
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, c);
}

void UART_SendString(char *s) {
    while (*s) UART_SendChar(*s++);
}

// ===== MAIN =====
int main(void) {
    char buf[64];
    uint8_t ver;

    TIM2_Init();
    UART1_Init(115200);

    UART_SendString("\r\n===== TEST SPI LORA =====\r\n");

    // Init SPI + LoRa
    LoRa_SPI_Init();
    

    Delay_Ms(50);

    while (1) {

        UART_SendString("\r\nDoc REG_VERSION...\r\n");

// d?c nhi?u l?n cho ch?c
        for (int i = 0; i < 5; i++) {
            
            // 1. THÊM "BREAKPOINT" VÀO ÐÂY: Ngh? 1 giây tru?c khi d?c
            Delay_Ms(1000); 

            // 2. L?nh d?c SPI th?c t? ch?y
            ver = LoRa_Read_Reg(0x42);

            // 3. In k?t qu? ra màn hình (Terminal)
            sprintf(buf, "Lan %d: 0x%02X\r\n", i+1, ver);
            UART_SendString(buf);
            
            // B?n có th? xóa cái Delay_Ms(200); cu di cho d? r?i
        }

        // ===== PHÂN TÍCH =====
        if (ver == 0x12) {
            UART_SendString("OK -> LoRa OK\r\n");
        }
        else if (ver == 0x00) {
            UART_SendString("LOI -> Khong co ket noi SPI\r\n");
        }
        else if (ver == 0xFF) {
            UART_SendString("LOI -> MISO dang bi keo len (loi day)\r\n");
        }
        else {
            UART_SendString("LOI -> Tin hieu sai (check GND / clock)\r\n");
        }

        UART_SendString("-------------------------\r\n");

        Delay_Ms(2000);
    }
}

