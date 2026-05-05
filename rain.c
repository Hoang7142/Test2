#include "rain.h"
#include <stdio.h>

// khoi tao chan PA1 lam input pull-up
void Rain_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin = GPIO_Pin_1;          // chon PA1
    gpio.GPIO_Mode = GPIO_Mode_IPU;      // input pull-up
    gpio.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(GPIOA, &gpio);
}

// doc trang thai cam bien mua
uint8_t Rain_Read(void)
{
    return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1);
}

// tra chuoi de hien thi lcd/uart
char* Rain_String(void)
{
    static char buf[20];   // buffer luu chuoi

    uint8_t val = Rain_Read();  // doc gia tri

    if(val == 0)
    {
        // co mua
        sprintf(buf, "RAIN: YES");
    }
    else
    {
        // khong mua
        sprintf(buf, "RAIN: NO ");
    }

    return buf;
}