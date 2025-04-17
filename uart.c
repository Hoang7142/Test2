#include "uart.h"

//void UART_Init(void)
//{
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
//	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA,ENABLE);//bat clock len
//	
//	GPIO_InitTypeDef gpioInit;// kieu struct
//	/*PB10 tx*/
//	gpioInit.GPIO_Mode = GPIO_Mode_AF_PP  ;//che do Output Push-Pull
//	gpioInit.GPIO_Pin = GPIO_Pin_2;// Chon chan PC10
//	gpioInit.GPIO_Speed = GPIO_Speed_50MHz;// toc do 
//	GPIO_Init(GPIOA,&gpioInit);//ham trong vi du
//	/*PB11 rx*/
//	gpioInit.GPIO_Mode = GPIO_Mode_IN_FLOATING   ;//che do dau ra
//	gpioInit.GPIO_Pin = GPIO_Pin_3;// Chon chan PC11
//	gpioInit.GPIO_Speed = GPIO_Speed_50MHz;// toc do 
//	GPIO_Init(GPIOA,&gpioInit);//ham trong vi du
//	
//	USART_InitTypeDef usartInit;// kieu struct
//    usartInit.USART_BaudRate = 9600;
//    usartInit.USART_WordLength = USART_WordLength_8b;//chonj truyen 8 bit
//    usartInit.USART_StopBits = USART_StopBits_1;//1 bit stop
//    usartInit.USART_Parity = USART_Parity_No;//bit kiem trar chan le
//    usartInit.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;//che do vua truyen vua nhan
//    usartInit.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//che do bat tay, chon khong bat tay

//    USART_Init(USART2, &usartInit);
//    USART_Cmd(USART2, ENABLE);  // Bat USART3
//	
//}

//void UART_SendChar(char c) {
//    USART_SendData(USART2, c);
//    while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
//}

///* Gui mot chuoi qua UART */
//void UART_SendString(char *str) {
//    while (*str) {
//        USART_SendData(USART2, *str++);
//        while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
//    }
//}

#define UART_BUFFER_SIZE 100
volatile char uartBuffer[UART_BUFFER_SIZE];
volatile uint8_t uartIndex = 0;

void UART_Init(void)
{
    // B?t clock cho USART2 và GPIOA
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef gpioInit;

    // PA2 - TX
    gpioInit.GPIO_Mode = GPIO_Mode_AF_PP;
    gpioInit.GPIO_Pin = GPIO_Pin_2;
    gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpioInit);

    // PA3 - RX
    gpioInit.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    gpioInit.GPIO_Pin = GPIO_Pin_3;
    GPIO_Init(GPIOA, &gpioInit);

    USART_InitTypeDef usartInit;
    usartInit.USART_BaudRate = 9600;
    usartInit.USART_WordLength = USART_WordLength_8b;
    usartInit.USART_StopBits = USART_StopBits_1;
    usartInit.USART_Parity = USART_Parity_No;
    usartInit.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    usartInit.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

    USART_Init(USART2, &usartInit);
    USART_Cmd(USART2, ENABLE);

    // B?t ng?t nh?n
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    NVIC_EnableIRQ(USART2_IRQn);
}

void USART2_IRQHandler(void)
{
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
        char c = USART_ReceiveData(USART2); // Ð?c ký t? nh?n du?c

        if (uartIndex < UART_BUFFER_SIZE - 1) {
            uartBuffer[uartIndex++] = c;
            if (c == '\n') {
                uartBuffer[uartIndex] = '\0'; // K?t thúc chu?i
                uartIndex = 0;

                UART_SendString("Received: ");
                UART_SendString((char*)uartBuffer);
            }
        } else {
            uartIndex = 0; // reset n?u tràn b? d?m
        }
    }
}

void UART_SendChar(char c) {
    USART_SendData(USART2, c);
    while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
}

void UART_SendString(char *str) {
    while (*str) {
        UART_SendChar(*str++);
    }
}