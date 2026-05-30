#ifndef LORA_RADIO_H
#define LORA_RADIO_H

#include "stm32f10x.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* SPI1 (SX1278) — change pins here only */
#define LORA_SPI_PORT       GPIOA
#define LORA_SPI_SCK_PIN    GPIO_Pin_5   /* PA5 — SPI1_SCK */
#define LORA_SPI_MISO_PIN   GPIO_Pin_6   /* PA6 — SPI1_MISO */
#define LORA_SPI_MOSI_PIN   GPIO_Pin_7   /* PA7 — SPI1_MOSI */
#define LORA_NSS_PORT       GPIOA
#define LORA_NSS_PIN        GPIO_Pin_4   /* PA4 — chip select */
#define LORA_RST_PORT       GPIOA
#define LORA_RST_PIN        GPIO_Pin_3   /* PA3 — RESET */
#define LORA_DIO0_PORT      GPIOB
#define LORA_DIO0_PIN       GPIO_Pin_5   /* PB5 — DIO0 / RxDone (EXTI) */
#define LORA_DIO0_EXTI_LINE EXTI_Line5

#define LORA_MAX_RAW_PACKET 255

typedef struct {
    uint32_t frequency_hz;
    uint8_t  spreading_factor;
    uint8_t  bandwidth;
    uint8_t  coding_rate;
    uint8_t  tx_power;
    uint8_t  preamble_len;
    uint8_t  sync_word;
    uint8_t  crc_on;
} lora_modem_config_t;

/** Init SPI, reset SX1278, apply modem config, enter continuous RX. */
bool lora_radio_begin(void);

bool lora_radio_send(const uint8_t* data, size_t len);
bool lora_radio_rx_pending(void);
int  lora_radio_receive(uint8_t* data, size_t max_len, int16_t* rssi_out);

/** Called from EXTI9_5_IRQHandler when DIO0 fires. */
void lora_radio_on_dio0_irq(void);

#endif
