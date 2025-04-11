
#include "i2c_lcd.h"
#include "delay.h"

#define LCD_EN 2
#define LCD_RW 1
#define LCD_RS 0
#define LCD_D4 4
#define LCD_D5 5
#define LCD_D6 6
#define LCD_D7 7
#define LCD_BL 3

#define MODE_4_BIT 0x28
#define CLR_SCR 0x01
#define DISP_ON 0x0C
#define CURSOR_ON 0x0E
#define CURSOR_HOME 0x80

static uint8_t u8LCD_Buff[8];
static uint8_t u8LcdTmp;

static void I2C2_Init(void) {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    // PB10 - SCL, PB11 - SDA
    GPIO_InitTypeDef gpio;
    gpio.GPIO_Mode = GPIO_Mode_AF_OD;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_Init(GPIOB, &gpio);
    I2C_DeInit(I2C2); 
    I2C_InitTypeDef i2c;
    i2c.I2C_ClockSpeed = 100000;
    i2c.I2C_Mode = I2C_Mode_I2C;
    i2c.I2C_DutyCycle = I2C_DutyCycle_2;
    i2c.I2C_OwnAddress1 = 0x00;
    i2c.I2C_Ack = I2C_Ack_Enable;
    i2c.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;

    I2C_Init(I2C2, &i2c);
    I2C_Cmd(I2C2, ENABLE);
}

static void I2C2_Write(uint8_t addr, uint8_t *data, uint8_t len) {
    while (I2C_GetFlagStatus(I2C2, I2C_FLAG_BUSY));
    I2C_GenerateSTART(I2C2, ENABLE);
    while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT));

    I2C_Send7bitAddress(I2C2, addr, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    for (uint8_t i = 0; i < len; i++) {
        I2C_SendData(I2C2, data[i]);
        while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    }

    I2C_GenerateSTOP(I2C2, ENABLE);
}

static void I2C_LCD_FlushVal(void) {
    uint8_t i;
    u8LcdTmp = 0;
    for (i = 0; i < 8; ++i) {
        u8LcdTmp >>= 1;
        if(u8LCD_Buff[i]) u8LcdTmp |= 0x80;
    }
    I2C2_Write(I2C_LCD_ADDR, &u8LcdTmp, 1);
}

static void I2C_LCD_Write_4bit(uint8_t u8Data) {
    u8LCD_Buff[LCD_D4] = u8Data & 0x01;
    u8LCD_Buff[LCD_D5] = (u8Data >> 1) & 0x01;
    u8LCD_Buff[LCD_D6] = (u8Data >> 2) & 0x01;
    u8LCD_Buff[LCD_D7] = (u8Data >> 3) & 0x01;

    u8LCD_Buff[LCD_EN] = 1;
    I2C_LCD_FlushVal();
    Delay_Ms(1);

    u8LCD_Buff[LCD_EN] = 0;
    I2C_LCD_FlushVal();
    Delay_Ms(1);
}

static void I2C_LCD_WriteCmd(uint8_t u8Cmd) {
    u8LCD_Buff[LCD_RS] = 0;
    u8LCD_Buff[LCD_RW] = 0;
    I2C_LCD_FlushVal();

    I2C_LCD_Write_4bit(u8Cmd >> 4);
    I2C_LCD_Write_4bit(u8Cmd & 0x0F);
}

static void LCD_Write_Chr(char chr) {
    u8LCD_Buff[LCD_RS] = 1;
    u8LCD_Buff[LCD_RW] = 0;
    I2C_LCD_FlushVal();

    I2C_LCD_Write_4bit(chr >> 4);
    I2C_LCD_Write_4bit(chr & 0x0F);
}

void I2C_LCD_Init(void) {
    I2C2_Init();
    Delay_Ms(50);

    for (uint8_t i = 0; i < 8; ++i)
        u8LCD_Buff[i] = 0;

    I2C_LCD_FlushVal();

    I2C_LCD_Write_4bit(0x03);
    Delay_Ms(5);
    I2C_LCD_Write_4bit(0x03);
    Delay_Ms(1);
    I2C_LCD_Write_4bit(0x03);
    Delay_Ms(1);
    I2C_LCD_Write_4bit(0x02);
    Delay_Ms(1);

    I2C_LCD_WriteCmd(MODE_4_BIT);
    I2C_LCD_WriteCmd(DISP_ON);
    I2C_LCD_WriteCmd(CURSOR_ON);
    I2C_LCD_WriteCmd(CLR_SCR);
}

void I2C_LCD_Puts(char *sz) {
    while (*sz) {
        LCD_Write_Chr(*sz++);
    }
}

void I2C_LCD_Clear(void) {
    I2C_LCD_WriteCmd(CLR_SCR);
    Delay_Ms(2);
}

void I2C_LCD_NewLine(void) {
    I2C_LCD_WriteCmd(0xC0);
}

void I2C_LCD_BackLight(uint8_t u8BackLight) {
    u8LCD_Buff[LCD_BL] = u8BackLight ? 1 : 0;
    I2C_LCD_FlushVal();
}

