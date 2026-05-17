#include "lora.h"
#include "delay.h"
#include "misc.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include <string.h>

static LoRa_Config_t lora_cfg;
static volatile uint8_t lora_rx_done_pending = 0;

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

void LoRa_SPI_Init(void)
{
    GPIO_InitTypeDef gpio;
    SPI_InitTypeDef spi;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_SPI1, ENABLE);

    gpio.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    gpio.GPIO_Pin = GPIO_Pin_6;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio);

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

void LoRa_Reset(void)
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

void LoRa_Write_Reg(uint8_t reg, uint8_t data)
{
    LoRa_NSS(0);
    SPI_Transfer(reg | 0x80);
    SPI_Transfer(data);
    LoRa_NSS(1);
}

uint8_t LoRa_Read_Reg(uint8_t reg)
{
    uint8_t val;
    LoRa_NSS(0);
    SPI_Transfer(reg & 0x7F);
    val = SPI_Transfer(0x00);
    LoRa_NSS(1);
    return val;
}

void LoRa_Write_Buffer(uint8_t reg, const uint8_t *data, uint8_t len)
{
    uint8_t i;
    LoRa_NSS(0);
    SPI_Transfer(reg | 0x80);
    for (i = 0; i < len; i++)
        SPI_Transfer(data[i]);
    LoRa_NSS(1);
}

void LoRa_Read_Buffer(uint8_t reg, uint8_t *data, uint8_t len)
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

void LoRa_SetStandby(void)
{
    LoRa_SetOpMode(MODE_STDBY);
}

void LoRa_ClearIrq(void)
{
    LoRa_Write_Reg(REG_IRQ_FLAGS, IRQ_ALL);
}

uint8_t LoRa_GetVersion(void)
{
    return LoRa_Read_Reg(REG_VERSION);
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
    return (uint8_t)(0x80 | (dbm - 2)); /* PA_BOOST + power */
}

static void LoRa_ApplyModemConfig(const LoRa_Config_t *cfg)
{
    uint8_t mc1, mc2, mc3;

    /* ModemConfig1: BW[7:4], CR[6:4], implicit header off (bit0=0 = explicit) */
    mc1 = (uint8_t)((cfg->bandwidth << 4) | (cfg->coding_rate << 1) | 0x00);
    /* ModemConfig2: SF[7:4], CRC on/off[2], continuous RX (bit0=0) */
    mc2 = (uint8_t)((cfg->spreading_factor << 4) | (cfg->crc_on ? 0x04 : 0x00));
    /* ModemConfig3: LowDataRateOptimize on when SF11/SF12 (long symbol time) */
    mc3 = (cfg->spreading_factor >= 11) ? 0x0C : 0x04;

    /* 0x1D: Signal bandwidth + forward error coding rate + header mode */
    LoRa_Write_Reg(REG_MODEM_CONFIG_1, mc1);
    /* 0x1E: Spreading factor + payload CRC enable */
    LoRa_Write_Reg(REG_MODEM_CONFIG_2, mc2);
    /* 0x26: Low-data-rate optimization (required at high SF / narrow BW) */
    LoRa_Write_Reg(REG_MODEM_CONFIG_3, mc3);

    /* 0x20/0x21: Preamble length in symbols (MSB=0, LSB=cfg->preamble_len) */
    LoRa_Write_Reg(REG_PREAMBLE_MSB, 0x00);
    LoRa_Write_Reg(REG_PREAMBLE_LSB, cfg->preamble_len);
    /* 0x1F: RX symbol timeout LSB (guard time before RxTimeout IRQ) */
    LoRa_Write_Reg(REG_SYMB_TIMEOUT_LSB, 0x08);
    /* 0x39: Network ID — must match on TX and RX nodes */
    LoRa_Write_Reg(REG_SYNC_WORD, cfg->sync_word);

    /* 0x06..0x08: RF carrier frequency (FRF = freq_hz * 2^19 / 32 MHz) */
    LoRa_SetFrequency(cfg->frequency_hz);
    /* 0x09: TX output power via PA_BOOST (see LoRa_MapTxPower) */
    LoRa_Write_Reg(REG_PA_CONFIG, LoRa_MapTxPower(cfg->tx_power));
    /* 0x0B: Over-current protection ~120 mA (0x0B), protects PA */
    LoRa_Write_Reg(REG_OCP, 0x0B);
    /* 0x0C: LNA gain boost + max gain (improves RX sensitivity) */
    LoRa_Write_Reg(REG_LNA, 0x23);

    /* 0x0E/0x0F: Start of TX/RX FIFO in chip RAM (both at 0 = single buffer) */
    LoRa_Write_Reg(REG_FIFO_TX_BASE_ADDR, 0x00);
    LoRa_Write_Reg(REG_FIFO_RX_BASE_ADDR, 0x00);
    /* DIO0 mapping is set per mode in LoRa_SetDioMappingRx() / LoRa_SetDioMappingTx() */
    LoRa_Write_Reg(REG_DIO_MAPPING_2, 0x00);
    /* 0x12: Clear all pending IRQ flags before TX/RX */
    LoRa_ClearIrq();
}

void LoRa_DIO0_Init(void)
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

void LoRa_SetDioMappingRx(void)
{
    /* DIO0 high on RxDone; other DIOs unused */
    LoRa_Write_Reg(REG_DIO_MAPPING_1, DIOMAP1_DIO0_RX_DONE);
    LoRa_Write_Reg(REG_IRQ_FLAGS_MASK, IRQ_MASK_RX);
}

void LoRa_SetDioMappingTx(void)
{
    /* DIO0 high on TxDone */
    LoRa_Write_Reg(REG_DIO_MAPPING_1, DIOMAP1_DIO0_TX_DONE);
    LoRa_Write_Reg(REG_IRQ_FLAGS_MASK, IRQ_MASK_TX);
}

uint8_t LoRa_RxDonePending(void)
{
    return lora_rx_done_pending;
}

void LoRa_ClearRxDone(void)
{
    lora_rx_done_pending = 0;
}

void LoRa_OnDio0Irq(void)
{
    if (GPIO_ReadInputDataBit(LORA_DIO0_PORT, LORA_DIO0_PIN) == SET) {
        lora_rx_done_pending = 1;
    }
}

static void LoRa_DefaultConfig(LoRa_Config_t *cfg)
{
    cfg->frequency_hz     = 433000000UL;
    cfg->spreading_factor = 7;
    cfg->bandwidth      = 7;   /* 125 kHz */
    cfg->coding_rate    = 1;   /* 4/5 */
    cfg->tx_power       = 17;
    cfg->preamble_len   = 8;
    cfg->sync_word      = 0xF1; /* match common private LoRa network */
    cfg->crc_on         = 1;
}

uint8_t LoRa_InitWithConfig(const LoRa_Config_t *cfg)
{
    if (cfg != NULL)
        lora_cfg = *cfg;
    else
        LoRa_DefaultConfig(&lora_cfg);

    LoRa_SPI_Init();
    LoRa_Reset();

    LoRa_SetOpMode(MODE_SLEEP);
    Delay_Ms(10);

    if (LoRa_GetVersion() != SX1278_VERSION)
        return 0;

    LoRa_ApplyModemConfig(&lora_cfg);
    LoRa_DIO0_Init();
    LoRa_SetStandby();
    return 1;
}

uint8_t LoRa_Init(void)
{
    return LoRa_InitWithConfig(NULL);
}

uint8_t LoRa_Transmit(const uint8_t *data, uint8_t len)
{
    uint8_t tx_base;
    uint32_t timeout;
    uint8_t irq;

    if (data == NULL || len == 0 || len > LORA_MAX_PAYLOAD)
        return 0;

    LoRa_SetStandby();
    LoRa_ClearIrq();
    LoRa_ClearRxDone();

    tx_base = LoRa_Read_Reg(REG_FIFO_TX_BASE_ADDR);
    LoRa_Write_Reg(REG_FIFO_ADDR_PTR, tx_base);
    LoRa_Write_Reg(REG_PAYLOAD_LENGTH, len);
    LoRa_Write_Buffer(REG_FIFO, data, len);

    LoRa_Write_Reg(REG_PA_DAC, 0x87);
    LoRa_SetOpMode(MODE_TX);

    timeout = 5000;
    do {
        irq = LoRa_Read_Reg(REG_IRQ_FLAGS);
        if (irq & IRQ_TX_DONE) {
            LoRa_Write_Reg(REG_IRQ_FLAGS, IRQ_TX_DONE);
            LoRa_SetStandby();
            return 1;
        }
        Delay_Ms(1);
    } while (--timeout);

    LoRa_SetStandby();
    return 0;
}

void LoRa_SendString(char *str)
{
    if (str != NULL)
        LoRa_Transmit((const uint8_t *)str, (uint8_t)strlen(str));
}

void LoRa_StartReceive(void)
{
    uint8_t rx_base;

    LoRa_SetStandby();
    LoRa_ClearIrq();
    LoRa_ClearRxDone();
    LoRa_SetDioMappingRx();

    rx_base = LoRa_Read_Reg(REG_FIFO_RX_BASE_ADDR);
    LoRa_Write_Reg(REG_FIFO_ADDR_PTR, rx_base);
    LoRa_Write_Reg(REG_PA_DAC, 0x84);
    LoRa_SetOpMode(MODE_RX_CONTINUOUS);
}

uint8_t LoRa_IsPacketReceived(void)
{
    /* Interrupt path: EXTI sets lora_rx_done_pending via LoRa_OnDio0Irq() */
    return lora_rx_done_pending;
}

uint8_t LoRa_Receive(uint8_t *data, uint8_t max_len)
{
    uint8_t len, addr;

    if (data == NULL || max_len == 0)
        return 0;

    if (!LoRa_IsPacketReceived())
        return 0;

    if (!(LoRa_Read_Reg(REG_IRQ_FLAGS) & IRQ_RX_DONE))
        return 0;

    if (lora_cfg.spreading_factor == 6) {
        len = LoRa_Read_Reg(REG_PAYLOAD_LENGTH);
    } else {
        len = LoRa_Read_Reg(REG_RX_NB_BYTES);
    }

    if (len > max_len)
        len = max_len;

    addr = LoRa_Read_Reg(REG_FIFO_RX_CURRENT_ADDR);
    LoRa_Write_Reg(REG_FIFO_ADDR_PTR, addr);
    LoRa_Read_Buffer(REG_FIFO, data, len);

    LoRa_ClearIrq();
    LoRa_ClearRxDone();
    LoRa_StartReceive();
    return len;
}

int8_t LoRa_GetPacketRssi(void)
{
    return (int8_t)LoRa_Read_Reg(REG_PKT_RSSI_VALUE) - 137;
}
