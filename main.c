/**
 * STM32F103 LoRa slave sensor node — interoperates with ESP32_LORA gateway.
 *
 * Set MY_NODE_ID per board (0x11, 0x12, 0x13) in lora/lora_network_config.h
 * or via compiler define -DMY_NODE_ID=0x12
 */
#include "stm32f10x.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "delay.h"
#include "timer.h"
#include "dht11.h"
#include "i2c_lcd.h"
#include "pwm.h"
#include "adc.h"
#include "rain.h"
#include "water_level.h"
#include "relay.h"
#include "flow_sensor.h"
#include "analog.h"
#include "motor.h"

#include "lora_node.h"
#include "lora_protocol.h"
#include "lora_network_config.h"
#include "lora_radio.h"

/* ===== UART debug ===== */
static void UART1_Init(uint32_t baudrate)
{
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef uart;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    // TX PA9
    gpio.GPIO_Pin = GPIO_Pin_9;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    // RX PA10
    gpio.GPIO_Pin = GPIO_Pin_10;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio);

    uart.USART_BaudRate = baudrate;
    uart.USART_WordLength = USART_WordLength_8b;
    uart.USART_StopBits = USART_StopBits_1;
    uart.USART_Parity = USART_Parity_No;
    uart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    uart.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART1, &uart);
    USART_Cmd(USART1, ENABLE);
}

static void UART_SendChar(char c)
{
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, c);
}

static void UART_SendString(const char *s)
{
    while (s && *s)
        UART_SendChar(*s++);
}

static void debug_log(const char *fmt, ...)
{
    char buf[128];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    UART_SendString(buf);
}

/** Heartbeat every ~5 s on TIM4 (free-running; TIM2 reserved for delay.c). */
static void debug_heartbeat(void)
{
    static uint32_t acc_us;
    static uint16_t last_tick;
    static uint8_t primed;
    uint16_t now;
    uint32_t delta;

    now = (uint16_t)TIM_GetCounter(TIM4);

    if (!primed) {
        last_tick = now;
        primed = 1;
        return;
    }

    if (now >= last_tick)
        delta = (uint32_t)(now - last_tick);
    else
        delta = (uint32_t)(0x10000u - last_tick + now);

    last_tick = now;
    acc_us += delta;

    if (acc_us >= 5000000UL) {
        acc_us -= 5000000UL;
        debug_log("[Status] alive, listening (node 0x%02X)\r\n",
                  (unsigned)MY_NODE_ID);
    }
}

static void LED_Init(void)
{
    GPIO_InitTypeDef gpioInit;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioInit.GPIO_Pin = GPIO_Pin_13;
    gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &gpioInit);
    GPIO_SetBits(GPIOC, GPIO_Pin_13);
}
void LED_Blink(void) {
    if (GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_13) == SET) {
        GPIO_ResetBits(GPIOC, GPIO_Pin_13);
    } else {
        GPIO_SetBits(GPIOC, GPIO_Pin_13);
    }
}

/**
 * Read sensors and fill lora_sensor_payload_t for CMD_SENSOR_DATA reply.
 */
static uint8_t my_read_sensor(uint8_t *payload, uint8_t max_len)
{
    lora_sensor_payload_t sample;
    uint8_t temp = 0;
    uint8_t humi = 0;

    if (max_len < sizeof(sample)) {
        debug_log("[Sensor] read failed: buffer too small (%u)\r\n",
                  (unsigned)max_len);
        return 0;
    }

    memset(&sample, 0, sizeof(sample));

    if (DHT11_ReadData(&temp, &humi) == DHT11_OK) {
        sample.temperature_c10 = (int16_t)temp * 10;
        sample.humidity_pct10 = (uint16_t)humi * 10;
    }

    sample.soil_moisture = (uint16_t)GetDoAmDatValue();
    sample.water_level = (uint16_t)WaterLevel_GetPercent();

    debug_log("[Sensor] temp_c10=%d  hum_c10=%u  soil=%u  water=%u%%\r\n",
              (int)sample.temperature_c10,
              (unsigned)sample.humidity_pct10,
              (unsigned)sample.soil_moisture,
              (unsigned)sample.water_level);

    memcpy(payload, &sample, sizeof(sample));
    return (uint8_t)sizeof(sample);
}

// ===== MAIN =====
int main(void) {
    LED_Init();

    UART1_Init(115200);
    UART_SendString("\r\n===== STM32 LoRa Slave Node =====\r\n");
    debug_log("[Boot] UART1 115200 OK\r\n");

    debug_log("[Boot] TIM2 (delay) init...\r\n");
    TIM2_Init();
    debug_log("[Boot] TIM4 (heartbeat) init...\r\n");
    TIM4_HeartbeatInit();
    debug_log("[Boot] timers OK\r\n");

    debug_log("[Boot] DHT11 init...\r\n");
    DHT11_Init();
    debug_log("[Boot] DHT11 OK\r\n");

    debug_log("[Boot] ADC init...\r\n");
    ADC_InitConfig();
    debug_log("[Boot] ADC OK\r\n");

    debug_log("[Boot] Water level init...\r\n");
    WaterLevel_Init();
    debug_log("[Boot] Water level OK\r\n");

    Delay_Ms(100);

    {
        uint8_t payload[LORA_MAX_PAYLOAD];
        debug_log("[Boot] Sensor snapshot:\r\n");
        my_read_sensor(payload, sizeof(payload));
    }

    debug_log("[Boot] LoRa radio init...\r\n");
    if (!lora_radio_begin()) {
        UART_SendString("[Boot] ERROR: lora_radio_begin failed\r\n");
        while (1) { }
    }

    debug_log("[Boot] Radio OK  node=0x%02X  gateway=0x%02X\r\n",
              (unsigned)MY_NODE_ID, (unsigned)GATEWAY_ID);
    debug_log("[Boot] Entering main loop (continuous RX)\r\n");

    {
        const lora_node_config_t cfg = {
            .node_id = MY_NODE_ID,
            .gateway_id = GATEWAY_ID,
            .log = debug_log,
        };

        const lora_node_radio_t radio = {
            .send = lora_radio_send,
            .rx_pending = lora_radio_rx_pending,
            .receive = lora_radio_receive,
            .read_sensor = my_read_sensor,
        };

        lora_node_t node;
        lora_node_init(&node, &cfg, &radio);

        while (1) {
            lora_node_poll(&node);
            debug_heartbeat();
            // __WFI();
        }
    }
}
