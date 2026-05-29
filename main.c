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

void LED_Init(void) {
    GPIO_InitTypeDef gpioInit;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioInit.GPIO_Pin = GPIO_Pin_13;
    gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &gpioInit);
    GPIO_ResetBits(GPIOC, GPIO_Pin_13);
}

void LED_Blink(void) {
    if (GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_13) == SET) {
        GPIO_ResetBits(GPIOC, GPIO_Pin_13);
    } else {
        GPIO_SetBits(GPIOC, GPIO_Pin_13);
    }
}

// ===== MAIN =====
int main(void) {
    LED_Init();
    char buf[64];

    TIM2_Init();
    UART1_Init(115200);
    UART_SendString("\r\n===== TEST SPI LORA =====\r\n");

    // Init SPI + LoRa
    if (LoRa_Init() == 0) {
        UART_SendString("LoRa_Init failed\r\n");
        while (1) { }
    }
    UART_SendString("LoRa_Init OK\r\n");
    LoRa_StartReceive();
    UART_SendString("LoRa RX listening (DIO0 interrupt)\r\n");

    while (1) {
        /* No SPI/register polling — wait for EXTI from DIO0 (RxDone) */
        if (LoRa_RxDonePending()) {
            uint8_t rx_buf[64];
            uint8_t n = LoRa_Receive(rx_buf, sizeof(rx_buf) - 1);
            if (n > 0) {
                int8_t rssi = LoRa_GetPacketRssi();
                rx_buf[n] = '\0';
                sprintf(buf, "RX %u bytes, RSSI %d dBm: ", n, (int)rssi);
                UART_SendString(buf);
                UART_SendString((char *)rx_buf);
                UART_SendString("\r\n");

                LoRa_SendString((char *)rx_buf); // Send the received data back to the sender
                LED_Blink();
            }
        } else {
            __WFI(); /* sleep until EXTI or other IRQ */
        }
    }
}

