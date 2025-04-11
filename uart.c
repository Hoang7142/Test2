#include "uart.h"

void UART_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA,ENABLE);//bat clock len
	
	GPIO_InitTypeDef gpioInit;// kieu struct
	/*PB10 tx*/
	gpioInit.GPIO_Mode = GPIO_Mode_AF_PP  ;//che do Output Push-Pull
	gpioInit.GPIO_Pin = GPIO_Pin_2;// Chon chan PC10
	gpioInit.GPIO_Speed = GPIO_Speed_50MHz;// toc do 
	GPIO_Init(GPIOA,&gpioInit);//ham trong vi du
	/*PB11 rx*/
	gpioInit.GPIO_Mode = GPIO_Mode_IN_FLOATING   ;//che do dau ra
	gpioInit.GPIO_Pin = GPIO_Pin_3;// Chon chan PC11
	gpioInit.GPIO_Speed = GPIO_Speed_50MHz;// toc do 
	GPIO_Init(GPIOA,&gpioInit);//ham trong vi du
	
	USART_InitTypeDef usartInit;// kieu struct
    usartInit.USART_BaudRate = 9600;
    usartInit.USART_WordLength = USART_WordLength_8b;//chonj truyen 8 bit
    usartInit.USART_StopBits = USART_StopBits_1;//1 bit stop
    usartInit.USART_Parity = USART_Parity_No;//bit kiem trar chan le
    usartInit.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;//che do vua truyen vua nhan
    usartInit.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//che do bat tay, chon khong bat tay

    USART_Init(USART2, &usartInit);
    USART_Cmd(USART2, ENABLE);  // Bat USART3
	
}

void UART_SendChar(char c) {
    USART_SendData(USART2, c);
    while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
}

/* Gui mot chuoi qua UART */
void UART_SendString(char *str) {
    while (*str) {
        USART_SendData(USART2, *str++);
        while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
    }
}