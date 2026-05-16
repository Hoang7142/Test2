//#ifndef __LORA_H
//#define __LORA_H

//#include "stm32f10x.h"

//// Khai bao chan NSS va DIO0 cho LoRa
//#define LORA_NSS_PORT   GPIOA
//#define LORA_NSS_PIN    GPIO_Pin_4

//#define LORA_DIO0_PORT  GPIOB
//#define LORA_DIO0_PIN   GPIO_Pin_5

//// Dinh nghia cac thanh ghi co ban cua SX1278
//#define LORA_REG_FIFO               0x00
//#define LORA_REG_OP_MODE            0x01
//#define LORA_REG_FRF_MSB            0x06
//#define LORA_REG_FRF_MID            0x07
//#define LORA_REG_FRF_LSB            0x08
//#define LORA_REG_PA_CONFIG          0x09
//#define LORA_REG_FIFO_ADDR_PTR      0x0D
//#define LORA_REG_FIFO_TX_BASE_ADDR  0x0E
//#define LORA_REG_FIFO_RX_BASE_ADDR  0x0F
//#define LORA_REG_FIFO_RX_CURRENT_ADDR 0x10
//#define LORA_REG_IRQ_FLAGS          0x12
//#define LORA_REG_RX_NB_BYTES        0x13
//#define LORA_REG_PAYLOAD_LENGTH     0x22
//#define LORA_REG_VERSION            0x42

//// Dinh nghia cac che do hoat dong
//#define LORA_MODE_SLEEP             0x00
//#define LORA_MODE_STANDBY           0x01
//#define LORA_MODE_TX                0x03
//#define LORA_MODE_RX_CONTINUOUS     0x05
//#define LORA_LONG_RANGE_MODE        0x80

//// Cac ham giao tiep SPI co ban
//void LoRa_SPI_Init(void);
//void LoRa_Write_Reg(uint8_t reg, uint8_t data);
//uint8_t LoRa_Read_Reg(uint8_t reg);

//// Cac ham dieu khien LoRa
//uint8_t LoRa_Init(void);
//void LoRa_Transmit(uint8_t *data, uint8_t length);
//void LoRa_Start_Receive(void);
//uint8_t LoRa_Receive(uint8_t *data);

//#endif

#ifndef __LORA_H
#define __LORA_H

#include "stm32f10x.h"

// Dinh nghia chan NSS (PA4)
#define LORA_NSS_PORT   GPIOA
#define LORA_NSS_PIN    GPIO_Pin_4

// Cac thanh ghi co ban de cau hinh
#define REG_FIFO            0x00
#define REG_OP_MODE         0x01
#define REG_FRF_MSB         0x06
#define REG_FRF_MID         0x07
#define REG_FRF_LSB         0x08
#define REG_PA_CONFIG       0x09
#define REG_FIFO_ADDR_PTR   0x0D
#define REG_FIFO_TX_BASE    0x0E
#define REG_IRQ_FLAGS       0x12
#define REG_PAYLOAD_LENGTH  0x22
#define REG_SYNC_WORD       0x39
#define REG_VERSION         0x42

// Prototypes (Cac ham public)
void LoRa_SPI_Init(void);
void LoRa_Write_Reg(uint8_t reg, uint8_t data);
uint8_t LoRa_Read_Reg(uint8_t reg);
uint8_t LoRa_Init(void);
void LoRa_SendString(char* str);

#endif
