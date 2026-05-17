#ifndef __LORA_H
#define __LORA_H

#include "stm32f10x.h"

/* SPI1 (SX1278) — change pins here only */
#define LORA_SPI_PORT       GPIOA
#define LORA_SPI_SCK_PIN    GPIO_Pin_5   /* PA5 — SPI1_SCK */
#define LORA_SPI_MISO_PIN   GPIO_Pin_6   /* PA6 — SPI1_MISO */
#define LORA_SPI_MOSI_PIN   GPIO_Pin_7   /* PA7 — SPI1_MOSI */
#define LORA_NSS_PORT       GPIOA
#define LORA_NSS_PIN        GPIO_Pin_4   /* PA4 — chip select (software NSS) */
#define LORA_RST_PORT       GPIOA
#define LORA_RST_PIN        GPIO_Pin_3   /* PA3 — RESET */
#define LORA_DIO0_PORT      GPIOB
#define LORA_DIO0_PIN       GPIO_Pin_5   /* PB5 — DIO0 / RxDone (EXTI) */
#define LORA_DIO0_EXTI_LINE EXTI_Line5   /* PB5 -> EXTI9_5_IRQHandler */

/* SX1278 registers (LoRa mode) */
#define REG_FIFO                    0x00
#define REG_OP_MODE                 0x01
#define REG_FRF_MSB                 0x06
#define REG_FRF_MID                 0x07
#define REG_FRF_LSB                 0x08
#define REG_PA_CONFIG               0x09
#define REG_OCP                     0x0B
#define REG_LNA                     0x0C
#define REG_FIFO_ADDR_PTR           0x0D
#define REG_FIFO_TX_BASE_ADDR       0x0E
#define REG_FIFO_RX_BASE_ADDR       0x0F
#define REG_FIFO_RX_CURRENT_ADDR    0x10
#define REG_IRQ_FLAGS_MASK          0x11
#define REG_IRQ_FLAGS               0x12
#define REG_RX_NB_BYTES             0x13
#define REG_MODEM_STAT              0x18
#define REG_PKT_SNR_VALUE           0x19
#define REG_PKT_RSSI_VALUE          0x1A
#define REG_RSSI_VALUE              0x1B
#define REG_MODEM_CONFIG_1          0x1D
#define REG_MODEM_CONFIG_2          0x1E
#define REG_SYMB_TIMEOUT_LSB        0x1F
#define REG_PREAMBLE_MSB            0x20
#define REG_PREAMBLE_LSB            0x21
#define REG_PAYLOAD_LENGTH          0x22
#define REG_MODEM_CONFIG_3          0x26
#define REG_DIO_MAPPING_1           0x40
#define REG_DIO_MAPPING_2           0x41
#define REG_VERSION                 0x42
#define REG_PA_DAC                  0x4D
#define REG_SYNC_WORD               0x39

#define SX1278_VERSION              0x12

/* OP_MODE: bit7 = LoRa, bits[2:0] = mode */
#define LORA_LONG_RANGE             0x80
#define MODE_SLEEP                  0x00
#define MODE_STDBY                  0x01
#define MODE_TX                     0x03
#define MODE_RX_CONTINUOUS          0x05

/* IRQ flags */
#define IRQ_TX_DONE                 0x08
#define IRQ_RX_DONE                 0x40
#define IRQ_RX_TIMEOUT              0x80
#define IRQ_ALL                     0xFF

/* RegDioMapping1: DIO0[7:6] in LoRa mode — 00=RxDone, 01=TxDone, 10=CadDone */
#define DIOMAP1_DIO0_RX_DONE        0x00
#define DIOMAP1_DIO0_TX_DONE        0x40

/* IRQ mask: 1=masked; leave RxDone (bit6) open so DIO0 toggles on packet RX */
#define IRQ_MASK_RX                 0x3F
#define IRQ_MASK_TX                 0xF7

#define LORA_MAX_PAYLOAD            255

typedef struct {
    uint32_t frequency_hz;  /* e.g. 433000000 */
    uint8_t  spreading_factor; /* 6..12 */
    uint8_t  bandwidth;     /* 0=7.8k .. 9=500k (7 = 125kHz) */
    uint8_t  coding_rate;   /* 1=4/5 .. 4=4/8 */
    uint8_t  tx_power;      /* 2..20 dBm (PA_BOOST path) */
    uint8_t  preamble_len;
    uint8_t  sync_word;
    uint8_t  crc_on;
} LoRa_Config_t;

void LoRa_SPI_Init(void);
void LoRa_Reset(void);
void LoRa_Write_Reg(uint8_t reg, uint8_t data);
uint8_t LoRa_Read_Reg(uint8_t reg);
void LoRa_Write_Buffer(uint8_t reg, const uint8_t *data, uint8_t len);
void LoRa_Read_Buffer(uint8_t reg, uint8_t *data, uint8_t len);

uint8_t LoRa_Init(void);
uint8_t LoRa_InitWithConfig(const LoRa_Config_t *cfg);
uint8_t LoRa_GetVersion(void);
void LoRa_SetStandby(void);
void LoRa_ClearIrq(void);
void LoRa_DIO0_Init(void);
void LoRa_SetDioMappingRx(void);  /* DIO0 = RxDone + EXTI (LoRa_StartReceive) */
void LoRa_SetDioMappingTx(void);  /* optional: DIO0 = TxDone; TX uses REG_IRQ_FLAGS instead */
uint8_t LoRa_RxDonePending(void);
void LoRa_ClearRxDone(void);
void LoRa_OnDio0Irq(void);   /* called from EXTI9_5_IRQHandler in stm32f10x_it.c */

uint8_t LoRa_Transmit(const uint8_t *data, uint8_t len);
void LoRa_SendString(char *str);
void LoRa_StartReceive(void);
uint8_t LoRa_IsPacketReceived(void);
uint8_t LoRa_Receive(uint8_t *data, uint8_t max_len);
int8_t  LoRa_GetPacketRssi(void);

#endif
