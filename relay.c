#include "relay.h"

// bien luu trang thai relay
static uint8_t relay_state = 0;

// khoi tao chan PA5 lam output dieu khien relay
void Relay_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin = GPIO_Pin_5;          // PA5
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;   // output push pull
    gpio.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(GPIOA, &gpio);

    // mac dinh tat relay
    GPIO_ResetBits(GPIOA, GPIO_Pin_5);
    relay_state = 0;
}

// bat relay (kich muc cao)
void Relay_On(void)
{
    GPIO_SetBits(GPIOA, GPIO_Pin_5);   // xuat HIGH
    relay_state = 1;
}

// tat relay
void Relay_Off(void)
{
    GPIO_ResetBits(GPIOA, GPIO_Pin_5); // xuat LOW
    relay_state = 0;
}

// dao trang thai
void Relay_Toggle(void)
{
    if(relay_state == 0)
    {
        Relay_On();
    }
    else
    {
        Relay_Off();
    }
}

// lay trang thai
uint8_t Relay_GetState(void)
{
    return relay_state;
}