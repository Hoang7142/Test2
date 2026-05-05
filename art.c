//#include "art.h"

//void UART_Init(void) {
//    GPIO_InitTypeDef gpio;
//    USART_InitTypeDef usart;

//    // Bat clock cho USART1 va GPIOA
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

//    // PA9 - TX: Alternate Function Push-Pull
//    gpio.GPIO_Pin = GPIO_Pin_9;
//    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
//    gpio.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_Init(GPIOA, &gpio);

//    // PA10 - RX: Input Floating
//    gpio.GPIO_Pin = GPIO_Pin_10;
//    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//    GPIO_Init(GPIOA, &gpio);

//    // Cau hinh USART1: 9600, 8-N-1
//    usart.USART_BaudRate = 9600;
//    usart.USART_WordLength = USART_WordLength_8b;
//    usart.USART_StopBits = USART_StopBits_1;
//    usart.USART_Parity = USART_Parity_No;
//    usart.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
//    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

//    USART_Init(USART1, &usart);
//    USART_Cmd(USART1, ENABLE);
//}

//void UART_SendChar(char c) {
//    USART_SendData(USART1, (uint8_t)c);
//    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
//}

//void UART_SendString(char *str) {
//    while (*str) {
//        UART_SendChar(*str++);
//    }
//}





//#include "art.h"

//void UART_Init(void) {
//    GPIO_InitTypeDef gpio;
//    USART_InitTypeDef usart;

//    // Bat clock cho USART3 va GPIOB
//    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

//    // PB10 - TX: Alternate Function Push-Pull
//    gpio.GPIO_Pin = GPIO_Pin_10;
//    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
//    gpio.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_Init(GPIOB, &gpio);

//    // PB11 - RX: Input Floating
//    gpio.GPIO_Pin = GPIO_Pin_11;
//    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//    GPIO_Init(GPIOB, &gpio);

//    // Cau hinh USART3: 9600, 8-N-1
//    usart.USART_BaudRate = 9600;
//    usart.USART_WordLength = USART_WordLength_8b;
//    usart.USART_StopBits = USART_StopBits_1;
//    usart.USART_Parity = USART_Parity_No;
//    usart.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
//    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

//    USART_Init(USART3, &usart);
//    USART_Cmd(USART3, ENABLE);
//}

//void UART_SendChar(char c) {
//    USART_SendData(USART3, (uint8_t)c);
//    while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
//}

//void UART_SendString(char *str) {
//    while (*str) {
//        UART_SendChar(*str++);
//    }
//}

