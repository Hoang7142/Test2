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
    //SystemInit();
    GPIO_InitTypeDef gpioInit;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioInit.GPIO_Pin = GPIO_Pin_13;
    gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &gpioInit);

    GPIO_ResetBits(GPIOC, GPIO_Pin_13);


    char buf[64];
    uint8_t ver;

    TIM2_Init();
    UART1_Init(115200);

    

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioInit.GPIO_Pin = GPIO_Pin_13;
    gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &gpioInit);

    UART_SendString("\r\n===== TEST SPI LORA =====\r\n");
    
    // Init SPI + LoRa
    LoRa_SPI_Init();
    

    Delay_Ms(50);

    while (1) 
    {

        UART_SendString("\r\nDoc REG_VERSION...\r\n");

// d?c nhi?u l?n cho ch?c
        for (int i = 0; i < 5; i++) {

            GPIOC->ODR ^= GPIO_Pin_13;
            
            // 1. TH�M "BREAKPOINT" V�O ��Y: Ngh? 1 gi�y tru?c khi d?c
            Delay_Ms(500); 

            // 2. L?nh d?c SPI th?c t? ch?y
            ver = LoRa_Read_Reg(0x42);

            // 3. In k?t qu? ra m�n h�nh (Terminal)
            sprintf(buf, "Lan %d: 0x%02X\r\n", i+1, ver);
            UART_SendString(buf);
            
            // B?n c� th? x�a c�i Delay_Ms(200); cu di cho d? r?i
        }

        // ===== PH�N T�CH =====
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

        Delay_Ms(1000);
    }
}

