/**
 * @file lora_radio.c
 * @brief SX1278 HAL driver for STM32F103 (StdPeriph). Matches ESP32_LORA modem settings.
 */
#include "lora_radio.h"

#include "delay.h"
#include "misc.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_spi.h"

#include <string.h>

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
#define REG_MODEM_CONFIG_1          0x1D
#define REG_MODEM_CONFIG_2          0x1E
#define REG_SYMB_TIMEOUT_LSB        0x1F
#define REG_PREAMBLE_MSB            0x20
#define REG_PREAMBLE_LSB            0x21
#define REG_PAYLOAD_LENGTH          0x22
#define REG_MODEM_CONFIG_3          0x26
#define REG_SYNC_WORD               0x39
#define REG_DIO_MAPPING_1           0x40
#define REG_DIO_MAPPING_2           0x41
#define REG_VERSION                 0x42
#define REG_PA_DAC                  0x4D

#define SX1278_VERSION              0x12

#define LORA_LONG_RANGE             0x80
#define MODE_SLEEP                  0x00
#define MODE_STDBY                  0x01
#define MODE_TX                     0x03
#define MODE_RX_CONTINUOUS          0x05

#define IRQ_TX_DONE                 0x08
#define IRQ_PAYLOAD_CRC_ERR         0x20
#define IRQ_RX_DONE                 0x40
#define IRQ_ALL                     0xFF

#define DIOMAP1_DIO0_RX_DONE        0x00
#define DIOMAP1_DIO0_TX_DONE        0x40

#define IRQ_MASK_RX                 0x3F
#define IRQ_MASK_TX                 0xF7

/* Must match include/lora_config.h on ESP32 gateway */
#define LORA_FREQ_HZ                433000000UL
#define LORA_SF                     7
#define LORA_BW_INDEX               7
#define LORA_CR_INDEX               1
#define LORA_TX_DBM                 17
#define LORA_PREAMBLE               8
#define LORA_SYNC                   0xF1

static lora_modem_config_t lora_modem;
static volatile uint8_t lora_rx_done_pending = 0;
static volatile uint8_t lora_rx_irq_enabled = 0;

static void LoRa_NSS(uint8_t select)
{
    if (select)
        GPIO_SetBits(LORA_NSS_PORT, LORA_NSS_PIN);
    else
        GPIO_ResetBits(LORA_NSS_PORT, LORA_NSS_PIN);
}

static uint8_t SPI_Transfer(uint8_t data)
{
    uint16_t timeout = 1000;
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET && --timeout);
    if (timeout == 0) return 0xFF;
    SPI_I2S_SendData(SPI1, data);
    timeout = 1000;
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET && --timeout);
    if (timeout == 0) return 0xFF;
    return (uint8_t)SPI_I2S_ReceiveData(SPI1);
}

static void LoRa_Write_Reg(uint8_t reg, uint8_t data)
{
    LoRa_NSS(0);
    SPI_Transfer(reg | 0x80);
    SPI_Transfer(data);
    LoRa_NSS(1);
}

static uint8_t LoRa_Read_Reg(uint8_t reg)
{
    uint8_t val;
    LoRa_NSS(0);
    SPI_Transfer(reg & 0x7F);
    val = SPI_Transfer(0x00);
    LoRa_NSS(1);
    return val;
}

static void LoRa_Write_Buffer(uint8_t reg, const uint8_t *data, uint8_t len)
{
    uint8_t i;
    LoRa_NSS(0);
    SPI_Transfer(reg | 0x80);
    for (i = 0; i < len; i++)
        SPI_Transfer(data[i]);
    LoRa_NSS(1);
}

static void LoRa_Read_Buffer(uint8_t reg, uint8_t *data, uint8_t len)
{
    uint8_t i;
    LoRa_NSS(0);
    SPI_Transfer(reg & 0x7F);
    for (i = 0; i < len; i++)
        data[i] = SPI_Transfer(0x00);
    LoRa_NSS(1);
}

static void LoRa_SetOpMode(uint8_t mode)
{
    LoRa_Write_Reg(REG_OP_MODE, LORA_LONG_RANGE | mode);
}

static void LoRa_SetStandby(void)
{
    LoRa_SetOpMode(MODE_STDBY);
}

static void LoRa_ClearIrq(void)
{
    LoRa_Write_Reg(REG_IRQ_FLAGS, IRQ_ALL);
}

static void LoRa_SetFrequency(uint32_t freq_hz)
{
    uint32_t frf = (uint32_t)(((uint64_t)freq_hz << 19) / 32000000UL);
    LoRa_Write_Reg(REG_FRF_MSB, (uint8_t)(frf >> 16));
    LoRa_Write_Reg(REG_FRF_MID, (uint8_t)(frf >> 8));
    LoRa_Write_Reg(REG_FRF_LSB, (uint8_t)(frf));
}

static uint8_t LoRa_MapTxPower(uint8_t dbm)
{
    if (dbm > 20) dbm = 20;
    if (dbm < 2)  dbm = 2;
    return (uint8_t)(0x80 | (dbm - 2));
}

static void LoRa_ApplyModemConfig(const lora_modem_config_t *cfg)
{
    uint8_t mc1, mc2, mc3;

    mc1 = (uint8_t)((cfg->bandwidth << 4) | (cfg->coding_rate << 1) | 0x00);
    mc2 = (uint8_t)((cfg->spreading_factor << 4) | (cfg->crc_on ? 0x04 : 0x00));
    mc3 = (cfg->spreading_factor >= 11) ? 0x0C : 0x04;

    LoRa_Write_Reg(REG_MODEM_CONFIG_1, mc1);
    LoRa_Write_Reg(REG_MODEM_CONFIG_2, mc2);
    LoRa_Write_Reg(REG_MODEM_CONFIG_3, mc3);
    LoRa_Write_Reg(REG_PREAMBLE_MSB, 0x00);
    LoRa_Write_Reg(REG_PREAMBLE_LSB, cfg->preamble_len);
    LoRa_Write_Reg(REG_SYMB_TIMEOUT_LSB, 0x08);
    LoRa_Write_Reg(REG_SYNC_WORD, cfg->sync_word);
    LoRa_SetFrequency(cfg->frequency_hz);
    LoRa_Write_Reg(REG_PA_CONFIG, LoRa_MapTxPower(cfg->tx_power));
    LoRa_Write_Reg(REG_OCP, 0x0B);
    LoRa_Write_Reg(REG_LNA, 0x23);
    LoRa_Write_Reg(REG_FIFO_TX_BASE_ADDR, 0x00);
    LoRa_Write_Reg(REG_FIFO_RX_BASE_ADDR, 0x00);
    LoRa_Write_Reg(REG_DIO_MAPPING_2, 0x00);
    LoRa_ClearIrq();
}

static void LoRa_SPI_Init(void)
{
    GPIO_InitTypeDef gpio;
    SPI_InitTypeDef spi;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_SPI1, ENABLE);

    gpio.GPIO_Pin = LORA_SPI_SCK_PIN | LORA_SPI_MOSI_PIN;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LORA_SPI_PORT, &gpio);

    gpio.GPIO_Pin = LORA_SPI_MISO_PIN;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(LORA_SPI_PORT, &gpio);

    gpio.GPIO_Pin = LORA_NSS_PIN;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(LORA_NSS_PORT, &gpio);
    LoRa_NSS(1);

    spi.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    spi.SPI_Mode = SPI_Mode_Master;
    spi.SPI_DataSize = SPI_DataSize_8b;
    spi.SPI_CPOL = SPI_CPOL_Low;
    spi.SPI_CPHA = SPI_CPHA_1Edge;
    spi.SPI_NSS = SPI_NSS_Soft;
    spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
    spi.SPI_FirstBit = SPI_FirstBit_MSB;
    spi.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &spi);
    SPI_Cmd(SPI1, ENABLE);
}

static void LoRa_Reset(void)
{
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    gpio.GPIO_Pin = LORA_RST_PIN;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LORA_RST_PORT, &gpio);

    GPIO_ResetBits(LORA_RST_PORT, LORA_RST_PIN);
    Delay_Ms(10);
    GPIO_SetBits(LORA_RST_PORT, LORA_RST_PIN);
    Delay_Ms(10);
}

static void LoRa_DIO0_Init(void)
{
    GPIO_InitTypeDef gpio;
    EXTI_InitTypeDef exti;
    NVIC_InitTypeDef nvic;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB, ENABLE);

    gpio.GPIO_Pin = LORA_DIO0_PIN;
    gpio.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(LORA_DIO0_PORT, &gpio);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource5);

    EXTI_StructInit(&exti);
    exti.EXTI_Line = LORA_DIO0_EXTI_LINE;
    exti.EXTI_Mode = EXTI_Mode_Interrupt;
    exti.EXTI_Trigger = EXTI_Trigger_Rising;
    exti.EXTI_LineCmd = ENABLE;
    EXTI_Init(&exti);

    nvic.NVIC_IRQChannel = EXTI9_5_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 1;
    nvic.NVIC_IRQChannelSubPriority = 1;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);
}

static void LoRa_RxIrqEnable(uint8_t enable)
{
    lora_rx_irq_enabled = enable ? 1 : 0;
}

static void LoRa_SetDioMappingRx(void)
{
    LoRa_Write_Reg(REG_DIO_MAPPING_1, DIOMAP1_DIO0_RX_DONE);
    LoRa_Write_Reg(REG_IRQ_FLAGS_MASK, IRQ_MASK_RX);
}

static void LoRa_SetDioMappingTx(void)
{
    LoRa_Write_Reg(REG_DIO_MAPPING_1, DIOMAP1_DIO0_TX_DONE);
    LoRa_Write_Reg(REG_IRQ_FLAGS_MASK, IRQ_MASK_TX);
}

static void LoRa_StartReceive(void)
{
    uint8_t rx_base;

    LoRa_SetStandby();
    LoRa_ClearIrq();
    lora_rx_done_pending = 0;
    LoRa_SetDioMappingRx();

    rx_base = LoRa_Read_Reg(REG_FIFO_RX_BASE_ADDR);
    LoRa_Write_Reg(REG_FIFO_ADDR_PTR, rx_base);
    LoRa_Write_Reg(REG_PA_DAC, 0x84);
    LoRa_SetOpMode(MODE_RX_CONTINUOUS);
    LoRa_RxIrqEnable(1);
}

static void LoRa_DefaultModem(lora_modem_config_t *cfg)
{
    cfg->frequency_hz     = LORA_FREQ_HZ;
    cfg->spreading_factor = LORA_SF;
    cfg->bandwidth        = LORA_BW_INDEX;
    cfg->coding_rate      = LORA_CR_INDEX;
    cfg->tx_power         = LORA_TX_DBM;
    cfg->preamble_len     = LORA_PREAMBLE;
    cfg->sync_word        = LORA_SYNC;
    cfg->crc_on           = 1;
}

bool lora_radio_begin(void)
{
    LoRa_DefaultModem(&lora_modem);

    LoRa_SPI_Init();
    LoRa_Reset();

    LoRa_SetOpMode(MODE_SLEEP);
    Delay_Ms(10);

    if (LoRa_Read_Reg(REG_VERSION) != SX1278_VERSION)
        return false;

    LoRa_ApplyModemConfig(&lora_modem);
    LoRa_DIO0_Init();
    LoRa_StartReceive();
    return true;
}

void lora_radio_on_dio0_irq(void)
{
    if (!lora_rx_irq_enabled)
        return;

    if (GPIO_ReadInputDataBit(LORA_DIO0_PORT, LORA_DIO0_PIN) == SET)
        lora_rx_done_pending = 1;
}

bool lora_radio_rx_pending(void)
{
    return lora_rx_done_pending ? true : false;
}

bool lora_radio_send(const uint8_t* data, size_t len)
{
    uint8_t tx_base;
    uint32_t timeout;
    uint8_t irq;

    if (data == NULL || len == 0 || len > LORA_MAX_RAW_PACKET)
        return false;

    LoRa_RxIrqEnable(0);
    lora_rx_done_pending = 0;

    LoRa_SetStandby();
    LoRa_ClearIrq();
    LoRa_SetDioMappingTx();

    tx_base = LoRa_Read_Reg(REG_FIFO_TX_BASE_ADDR);
    LoRa_Write_Reg(REG_FIFO_ADDR_PTR, tx_base);
    LoRa_Write_Reg(REG_PAYLOAD_LENGTH, (uint8_t)len);
    LoRa_Write_Buffer(REG_FIFO, data, (uint8_t)len);

    LoRa_Write_Reg(REG_PA_DAC, 0x87);
    LoRa_SetOpMode(MODE_TX);

    timeout = 200;
    do {
        irq = LoRa_Read_Reg(REG_IRQ_FLAGS);
        if (irq & IRQ_TX_DONE) {
            LoRa_Write_Reg(REG_IRQ_FLAGS, IRQ_TX_DONE);
            LoRa_SetStandby();
            LoRa_StartReceive();
            return true;
        }
        Delay_Ms(1);
    } while (--timeout);

    LoRa_SetStandby();
    LoRa_StartReceive();
    return false;
}

int lora_radio_receive(uint8_t* data, size_t max_len, int16_t* rssi_out)
{
    uint8_t len, addr, irq;

    if (data == NULL || max_len == 0)
        return -1;

    if (!lora_rx_done_pending)
        return 0;

    lora_rx_done_pending = 0;

    irq = LoRa_Read_Reg(REG_IRQ_FLAGS);
    if (!(irq & IRQ_RX_DONE)) {
        LoRa_ClearIrq();
        LoRa_StartReceive();
        return 0;
    }

    if (irq & IRQ_PAYLOAD_CRC_ERR) {
        LoRa_ClearIrq();
        LoRa_StartReceive();
        return -1;
    }

    if (lora_modem.spreading_factor == 6) {
        len = LoRa_Read_Reg(REG_PAYLOAD_LENGTH);
    } else {
        len = LoRa_Read_Reg(REG_RX_NB_BYTES);
    }

    if (len > max_len)
        len = (uint8_t)max_len;

    addr = LoRa_Read_Reg(REG_FIFO_RX_CURRENT_ADDR);
    LoRa_Write_Reg(REG_FIFO_ADDR_PTR, addr);
    LoRa_Read_Buffer(REG_FIFO, data, len);

    if (rssi_out != NULL)
        *rssi_out = (int16_t)LoRa_Read_Reg(0x1A) - 137;

    LoRa_ClearIrq();
    LoRa_StartReceive();
    return (int)len;
}
