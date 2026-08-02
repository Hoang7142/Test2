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
#include "rain.h"
#include "flow_sensor.h"
#include "analog.h"
#include "motor.h"

#include "lora_node.h"
#include "lora_protocol.h"
#include "lora_network_config.h"
#include "lora_radio.h"
#include "menu_control.h"

/* ===== DEBUG TEST ÐI?U KHI?N — d?i 0 sau khi test xong ===== */
#define CONTROL_TEST_DEBUG  0
///* Bi?n toàn c?c luu tr?ng thái nút nh?n nh?n t? LoRa Gateway */
//volatile lora_control_payload_t g_remote_control;
//volatile uint8_t g_control_updated = 0; // C? báo hi?u có l?nh m?i

///* ===== UART debug ===== */
//static void UART1_Init(uint32_t baudrate)
//{
//    GPIO_InitTypeDef gpio;
//    USART_InitTypeDef uart;

//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

//    // TX PA9
//    gpio.GPIO_Pin = GPIO_Pin_9;
//    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
//    gpio.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_Init(GPIOA, &gpio);

//    // RX PA10
//    gpio.GPIO_Pin = GPIO_Pin_10;
//    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//    GPIO_Init(GPIOA, &gpio);

//    uart.USART_BaudRate = baudrate;
//    uart.USART_WordLength = USART_WordLength_8b;
//    uart.USART_StopBits = USART_StopBits_1;
//    uart.USART_Parity = USART_Parity_No;
//    uart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
//    uart.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
//    USART_Init(USART1, &uart);
//    USART_Cmd(USART1, ENABLE);
//}

//static void UART_SendChar(char c)
//{
//    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
//    USART_SendData(USART1, c);
//}

//static void UART_SendString(const char *s)
//{
//    while (s && *s)
//        UART_SendChar(*s++);
//}

//static void debug_log(const char *fmt, ...)
//{
//    char buf[128];
//    va_list ap;

//    va_start(ap, fmt);
//    vsnprintf(buf, sizeof(buf), fmt, ap);
//    va_end(ap);
//    UART_SendString(buf);
//}

///** Heartbeat every ~5 s on TIM4 (free-running; TIM2 reserved for delay.c). */
//static void debug_heartbeat(void)
//{
//    static uint32_t acc_us;
//    static uint16_t last_tick;
//    static uint8_t primed;
//    uint16_t now;
//    uint32_t delta;

//    now = (uint16_t)TIM_GetCounter(TIM4);

//    if (!primed) {
//        last_tick = now;
//        primed = 1;
//        return;
//    }

//    if (now >= last_tick)
//        delta = (uint32_t)(now - last_tick);
//    else
//        delta = (uint32_t)(0x10000u - last_tick + now);

//    last_tick = now;
//    acc_us += delta;

//    if (acc_us >= 5000000UL) {
//        acc_us -= 5000000UL;
//        debug_log("[Status] alive, listening (node 0x%02X)\r\n",
//                  (unsigned)MY_NODE_ID);
//    }
//}

//static void LED_Init(void)
//{
//    GPIO_InitTypeDef gpioInit;
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
//    gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
//    gpioInit.GPIO_Pin = GPIO_Pin_13;
//    gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_Init(GPIOC, &gpioInit);
//    GPIO_SetBits(GPIOC, GPIO_Pin_13);
//}
//void LED_Blink(void) {
//    if (GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_13) == SET) {
//        GPIO_ResetBits(GPIOC, GPIO_Pin_13);
//    } else {
//        GPIO_SetBits(GPIOC, GPIO_Pin_13);
//    }
//}

///**
// * Read sensors and fill lora_sensor_payload_t for CMD_SENSOR_DATA reply.
// */
//static uint8_t my_read_sensor(uint8_t *payload, uint8_t max_len)
//{
//    lora_sensor_payload_t sample;
//    uint8_t temp = 0;
//    uint8_t humi = 0;
//	  Analog_Data_t analog_sensor_data;

//    if (max_len < sizeof(sample)) {
//        debug_log("[Sensor] read failed: buffer too small (%u)\r\n",
//                  (unsigned)max_len);
//        return 0;
//    }

//    memset(&sample, 0, sizeof(sample));
//    //doc DHT11
//    if (DHT11_ReadData(&temp, &humi) == DHT11_OK) {
//        sample.temperature_c10 = (int16_t)temp * 10;
//        sample.humidity_pct10 = (uint16_t)humi * 10;
//    }

//    // 2. Ð?c nhóm c?m bi?n ADC g?p (Ð? ?m d?t, M?c nu?c, Dòng di?n)
//    Analog_UpdateAll(&analog_sensor_data);
//    sample.soil_moisture = (uint16_t)analog_sensor_data.soil_percent;
//    sample.water_level   = (uint16_t)analog_sensor_data.water_percent;
//    sample.current_mA    = (uint16_t)(analog_sensor_data.current_ampe * 1000.0f); // Quy d?i A -> mA
//		
//		// 3. Ð?c c?m bi?n mua (Digital)
//    sample.rain_status = Rain_Read();

//    // 4. Ð?c c?m bi?n luu lu?ng nu?c (Tính toán d?a trên th?i gian l?y m?u, ví d? 1000ms)
//    // Ð? chính xác, ? dây l?y m?u t?m th?i 1000ms ho?c b?n có th? t?i uu theo th?i gian th?c vòng l?p
//    float flow_real = Flow_GetLitersPerMinute(1000); 
//    sample.flow_rate_Lmin_x10 = (uint8_t)(flow_real * 10.0f);

//    // In log debug ra UART1 d? giám sát t?i ch?
//    debug_log("[Sensor] Temp=%d|Hum=%u|Soil=%u%%|Water=%u%%|Current=%umA|Rain=%s|Flow=%.1f L/m\r\n",
//              (int)sample.temperature_c10/10,
//              (unsigned)sample.humidity_pct10/10,
//              (unsigned)sample.soil_moisture,
//              (unsigned)sample.water_level,
//              (unsigned)sample.current_mA,
//              (sample.rain_status == 0) ? "YES" : "NO",
//              flow_real);

//    memcpy(payload, &sample, sizeof(sample));
//    return (uint8_t)sizeof(sample);
//}

//// ===== MAIN =====
//int main(void) {
//    LED_Init();

//    UART1_Init(115200);
//    UART_SendString("\r\n===== STM32 LoRa Slave Node (Updated) =====\r\n");
//    debug_log("[Boot] UART1 115200 OK\r\n");

//    debug_log("[Boot] TIM2 (delay) init...\r\n");
//    TIM2_Init();
//    debug_log("[Boot] TIM4 (heartbeat) init...\r\n");
//    TIM4_HeartbeatInit();
//    debug_log("[Boot] timers OK\r\n");

//    debug_log("[Boot] DHT11 init...\r\n");
//    DHT11_Init();
//    debug_log("[Boot] DHT11 OK\r\n");

//    // --- Ð?I KHÚC NÀY THÀNH FILE G?P ANALOG M?I ---
//    debug_log("[Boot] Analog ADC channels init...\r\n");
//    Analog_Init(); 
//    debug_log("[Boot] Calibrating ACS712 Current Sensor...\r\n");
//    Analog_Calibrate(); // T? d?ng l?y di?m 0 dòng di?n lúc kh?i d?ng
//    debug_log("[Boot] Analog ADC OK\r\n");
//    // ----------------------------------------------

//    // --- THÊM CÁC C?M BI?N M?I KH?I T?O ? ÐÂY ---
//    debug_log("[Boot] Rain sensor init...\r\n");
//    Rain_Init();
//    
//    debug_log("[Boot] Flow sensor init...\r\n");
//		Motor_Init();
//    FlowSensor_Init(); // Ðã gi?i phóng chân PB4 bên trong hàm
//    // --------------------------------------------

//    Delay_Ms(100);

//    {
//        uint8_t payload[LORA_MAX_PAYLOAD];
//        debug_log("[Boot] Sensor snapshot:\r\n");
//        my_read_sensor(payload, sizeof(payload));
//    }

//    debug_log("[Boot] LoRa radio init...\r\n");
//    if (!lora_radio_begin()) {
//        UART_SendString("[Boot] ERROR: lora_radio_begin failed\r\n");
//        while (1) { }
//    }

//    debug_log("[Boot] Radio OK  node=0x%02X  gateway=0x%02X\r\n",
//              (unsigned)MY_NODE_ID, (unsigned)GATEWAY_ID);
//    debug_log("[Boot] Entering main loop (continuous RX)\r\n");

//    {
//        const lora_node_config_t cfg = {
//            .node_id = MY_NODE_ID,
//            .gateway_id = GATEWAY_ID,
//            .log = debug_log,
//        };

//        const lora_node_radio_t radio = {
//            .send = lora_radio_send,
//            .rx_pending = lora_radio_rx_pending,
//            .receive = lora_radio_receive,
//            .read_sensor = my_read_sensor,
//        };

//        lora_node_t node;
//        lora_node_init(&node, &cfg, &radio);

//while (1) {
//    // 1. Luôn luôn quét sóng LoRa d? nh?n d? li?u
//    lora_node_poll(&node);
//    
//    // 2. Ki?m tra xem có l?nh nút nh?n m?i t? Web d? xu?ng không
//    if (g_control_updated) {
//        g_control_updated = 0; // Xóa c? ngay l?p t?c d? tránh x? lý l?p
//        
//        debug_log("[Hardware] Th?c thi l?nh di?u khi?n t? Web...\r\n");
//        
//        /* ---------------- A. ÐI?U KHI?N MÁY BOM ---------------- */
//        if (g_remote_control.pump_status == 1) {
//            // Nhóm c?a b?n hãy g?i hàm kích ho?t Relay/Bom th?t ? dây, ví d?:
//            // Relay_Pump_On();
//            debug_log("-> Ph?n c?ng: B?T BOM - T?c d? PWM: %u%%\r\n", g_remote_control.pump_pwm);
//        } else {
//            // Relay_Pump_Off();
//            debug_log("-> Ph?n c?ng: T?T BOM\r\n");
//        }
//        
//        /* ---------------- B. ÐI?U KHI?N MÁI CHE ---------------- */
//        if (g_remote_control.roof_status == 1) {
//            // Nhóm c?a b?n hãy g?i hàm kích ho?t Motor quay thu?n ? dây, ví d?:
//            // Motor_Roof_Open(g_remote_control.roof_pwm);
//            debug_log("-> Ph?n c?ng: M? MÁI CHE - PWM: %u%%\r\n", g_remote_control.roof_pwm);
//        } 
//        else if (g_remote_control.roof_status == 2) {
//            // Nhóm c?a b?n hãy g?i hàm kích ho?t Motor quay ngu?c ? dây, ví d?:
//            // Motor_Roof_Close(g_remote_control.roof_pwm);
//            debug_log("-> Ph?n c?ng: ÐÓNG MÁI CHE - PWM: %u%%\r\n", g_remote_control.roof_pwm);
//        } 
//        else {
//            // Nhóm c?a b?n hãy g?i hàm d?ng Motor ? dây, ví d?:
//            // Motor_Roof_Stop();
//            debug_log("-> Ph?n c?ng: D?NG MÁI CHE\r\n");
//        }
//        
//        /* ------------- C. Ð?NG B? CH? Ð? H? TH?NG ------------- */
//        if (g_remote_control.system_mode == 1) {
//            debug_log("-> H? th?ng chuy?n sang ch? d?: T? Ð?NG (Auto)\r\n");
//        } else {
//            debug_log("-> H? th?ng chuy?n sang ch? d?: TH? CÔNG (Manual)\r\n");
//        }
//        
//        // Nh?p nháy dèn LED PC13 d? báo hi?u x? lý l?nh thành công
//        LED_Blink();
//    }
//    
//    // 3. Gi? nh?p d?p heartbeat giám sát t?i ch?
//    debug_heartbeat();
//}

//    }
//}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///* Bi?n to?n c?c luu tr?ng th?i n?t nh?n nh?n t? LoRa Gateway */
//volatile lora_control_payload_t g_remote_control;
//volatile uint8_t g_control_updated = 0; // C? b?o hi?u c? l?nh m?i
//extern volatile uint8_t g_ack_pending;

///* ========================================================================== */
///* ?? TH?M: BI?N TO?N C?C QU?N L? TR?NG TH?I TH?C T? T?I VU?N                 */
///* ========================================================================== */
//menu_control_data_t g_system_control = {
//    /* Boot MANUAL: tranh Auto bat bom khi ADC dat noi -> PWM nhi?u -> mat LoRa luc demo */
//    .system_mode = 0, // 0 = MANUAL, 1 = AUTO
//    .pump_status = 0, // 0 = OFF, 1 = ON
//    .roof_status = 0  // 0 = STOP, MOTOR_FORWARD = OPEN, MOTOR_BACKWARD = CLOSE
//};
//uint8_t g_lcd_update = 1; // C? b?o hi?u c?n v? l?i m?n h?nh LCD
//uint8_t current_roof_pwm = 100; // Luu t?c d? m?i che nh?n t? Web
///* Bi?n qu?n l? ch?n do?n l?i tr?m bom th?c t? */
//uint8_t current_pump_pwm = 100;
//volatile pump_diagnostic_t g_pump_diagnostic = PUMP_DIAG_OK;

///* Sau khi bat bom: cho nuoc/dong on dinh roi moi check DRY_RUN/OVERLOAD */
//#define PUMP_PROTECT_GRACE_MS  2000u
//static uint8_t s_prev_pump_on = 0;
//static uint32_t s_pump_on_since_ms = 0;

///* === Bi?n t?m d? truy?n ADC data d?n h?m callback LoRa === */
//static Analog_Data_t g_current_adc_data = {0};

//#define WATER_EMPTY_PERCENT  10
//uint8_t g_soil_on_threshold = 30;
//uint8_t g_soil_off_threshold = 80;
//static uint8_t g_runtime_node_id = 0;

//static uint8_t read_dip_node_id(void)
//{
//    GPIO_InitTypeDef gpio;
//    uint8_t b0, b1, b2, dip;

//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
//    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

//    gpio.GPIO_Mode = GPIO_Mode_IPD;
//    gpio.GPIO_Speed = GPIO_Speed_2MHz;
//    gpio.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_6 | GPIO_Pin_3;
//    GPIO_Init(GPIOB, &gpio);

//    b0 = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9) == Bit_SET ? 1 : 0;
//    b1 = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_6) == Bit_SET ? 1 : 0;
//    b2 = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3) == Bit_SET ? 1 : 0;
//    dip = (uint8_t)(b0 | (b1 << 1) | (b2 << 2));

//    if (dip == 0) {
//        return (uint8_t)MY_NODE_ID;
//    }
//    return (uint8_t)(0x10 | dip);
//}

//static void clamp_pump_for_water(uint8_t *pump_status, uint8_t water_percent)
//{
//    if (*pump_status != 0 && water_percent < WATER_EMPTY_PERCENT) {
//        *pump_status = 0;
//    }
//}



///* ===== UART debug ===== */
//static void UART1_Init(uint32_t baudrate)
//{
//    GPIO_InitTypeDef gpio;
//    USART_InitTypeDef uart;

//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

//    // TX PA9
//    gpio.GPIO_Pin = GPIO_Pin_9;
//    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
//    gpio.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_Init(GPIOA, &gpio);

//    // RX PA10
//    gpio.GPIO_Pin = GPIO_Pin_10;
//    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//    GPIO_Init(GPIOA, &gpio);

//    uart.USART_BaudRate = baudrate;
//    uart.USART_WordLength = USART_WordLength_8b;
//    uart.USART_StopBits = USART_StopBits_1;
//    uart.USART_Parity = USART_Parity_No;
//    uart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
//    uart.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
//    USART_Init(USART1, &uart);
//    USART_Cmd(USART1, ENABLE);
//}

//static void UART_SendChar(char c)
//{
//    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
//    USART_SendData(USART1, c);
//}

//static void UART_SendString(const char *s)
//{
//    while (s && *s)
//        UART_SendChar(*s++);
//}

//static void debug_log(const char *fmt, ...)
//{
//    char buf[128];
//    va_list ap;

//    va_start(ap, fmt);
//    vsnprintf(buf, sizeof(buf), fmt, ap);
//    va_end(ap);
//    UART_SendString(buf);
//}
//#if CONTROL_TEST_DEBUG
//static void debug_print_control_rx(const lora_control_payload_t *cmd)
//{
//    const char *roof_str = (cmd->roof_status == 1) ? "OPEN" :
//                           (cmd->roof_status == 2) ? "CLOSE" : "STOP";
//    debug_log("\r\n========== [CTRL-RX] LENH TU WEB ==========\r\n");
//    debug_log("  mode      : %s\r\n", cmd->system_mode ? "AUTO" : "MANUAL");
//    debug_log("  pump      : %s\r\n", cmd->pump_status ? "BAT" : "TAT");
//    debug_log("  pump_pwm  : %u%%\r\n", (unsigned)cmd->pump_pwm);
//    debug_log("  roof      : %s\r\n", roof_str);
//    debug_log("  roof_pwm  : %u%%\r\n", (unsigned)cmd->roof_pwm);
//    debug_log("============================================\r\n");
//}

//static void debug_print_control_ack(const lora_control_payload_t *ack)
//{
//    const char *roof_str = (ack->roof_status == 1) ? "OPEN" :
//                           (ack->roof_status == 2) ? "CLOSE" : "STOP";
//    debug_log("\r\n========== [ACK-TX] GUI LEN ESP ==========\r\n");
//    debug_log("  mode      : %s\r\n", ack->system_mode ? "AUTO" : "MANUAL");
//    debug_log("  pump      : %s\r\n", ack->pump_status ? "BAT" : "TAT");
//    debug_log("  pump_pwm  : %u%%\r\n", (unsigned)ack->pump_pwm);
//    debug_log("  roof      : %s\r\n", roof_str);
//    debug_log("  roof_pwm  : %u%%\r\n", (unsigned)ack->roof_pwm);
//    debug_log("============================================\r\n");
//}
//#endif
///** Heartbeat every ~5 s on TIM4 (free-running; TIM2 reserved for delay.c). */
//static void debug_heartbeat(void)
//{
//    static uint32_t acc_us;
//    static uint16_t last_tick;
//    static uint8_t primed;
//    uint16_t now;
//    uint32_t delta;

//    now = (uint16_t)TIM_GetCounter(TIM4);

//    if (!primed) {
//        last_tick = now;
//        primed = 1;
//        return;
//    }

//    if (now >= last_tick)
//        delta = (uint32_t)(now - last_tick);
//    else
//        delta = (uint32_t)(0x10000u - last_tick + now);

//    last_tick = now;
//    acc_us += delta;

//    if (acc_us >= 5000000UL) {
//        acc_us -= 5000000UL;
//        debug_log("[HB] 0x%02X\r\n",
//                  (unsigned)g_runtime_node_id);
//    }
//}

//static void LED_Init(void)
//{
//    GPIO_InitTypeDef gpioInit;
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
//    gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
//    gpioInit.GPIO_Pin = GPIO_Pin_13;
//    gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_Init(GPIOC, &gpioInit);
//    GPIO_SetBits(GPIOC, GPIO_Pin_13);
//}
//void LED_Blink(void) {
//    if (GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_13) == SET) {
//        GPIO_ResetBits(GPIOC, GPIO_Pin_13);
//    } else {
//        GPIO_SetBits(GPIOC, GPIO_Pin_13);
//    }
//}

///**
// * Read sensors and fill lora_sensor_payload_t for CMD_SENSOR_DATA reply.
// * ?? FIX: S? d?ng ADC data d? d?c s?n t? g_current_adc_data
// */
//static uint8_t my_read_sensor(uint8_t *payload, uint8_t max_len)
//{
//    lora_sensor_payload_t sample;
//    uint8_t temp = 0;
//    uint8_t humi = 0;

//    if (max_len < sizeof(sample)) {
//        debug_log("[S] buf small %u\r\n",
//          (unsigned)max_len);
//        return 0;
//    }

//    memset(&sample, 0, sizeof(sample));
//    
//    // 1. ??c DHT11
//    if (DHT11_ReadData(&temp, &humi) == DHT11_OK) {
//        sample.temperature_c10 = (int16_t)temp * 10;
//        sample.humidity_pct10 = (uint16_t)humi * 10;
//    }

//    // 2. ?? D?NG D? LI?U ADC ?? ??C S?N (kh?ng d?c l?i)
//    sample.soil_moisture = (uint16_t)g_current_adc_data.soil_percent;
//    sample.water_level   = (uint16_t)g_current_adc_data.water_percent;
//    sample.current_mA    = (uint16_t)(g_current_adc_data.current_ampe * 1000.0f);
//    
//    // 3. ??c c?m bi?n mua (Digital)
//    sample.rain_status = Rain_Read();

//    // 4. Doc flow (TIM3_CH1 IC, chung TIM3 voi PWM mai) — cache ~500ms
//    uint16_t flow_x10 = Flow_GetLpmX10();
//    sample.flow_rate_Lmin_x10 = (uint8_t)((flow_x10 > 255u) ? 255u : flow_x10);

//    // 5. ??ng b? tr?ng th?i th?c t? t?i vu?n l?n Web
//    sample.system_mode = g_system_control.system_mode;
//    sample.pump_status = g_system_control.pump_status;
//    sample.roof_status = g_system_control.roof_status;
//    sample.pump_diagnostic = (uint8_t)g_pump_diagnostic;

//    // Debug log (không dùng %.1f — tránh kéo lib printf float vu?t 32KB Keil Lite)
//    debug_log("[S] T=%d H=%u S=%u W=%u I=%u R=%s F=%u.%u\r\n",
//              (int)sample.temperature_c10/10,
//              (unsigned)sample.humidity_pct10/10,
//              (unsigned)sample.soil_moisture,
//              (unsigned)sample.water_level,
//              (unsigned)sample.current_mA,
//              (sample.rain_status == 0) ? "Y" : "N",
//              (unsigned)(sample.flow_rate_Lmin_x10 / 10),
//              (unsigned)(sample.flow_rate_Lmin_x10 % 10));

//    memcpy(payload, &sample, sizeof(sample));
//    return (uint8_t)sizeof(sample);
//}

///**
// * Wrapper function: G?i my_read_sensor (kh?ng c?n tham s? ADC v? d?ng global)
// */
//static uint8_t my_read_sensor_wrapper(uint8_t *payload, uint8_t max_len) {
//    return my_read_sensor(payload, max_len);  // ? Ch? 2 tham s?
//}
//// ===== MAIN =====
//int main(void) {
//    LED_Init();

//    UART1_Init(115200);
//    UART_SendString("\r\n===== STM32 LoRa Slave Node (Updated) =====\r\n");
//    debug_log("[Boot] UART1 115200 OK\r\n");

//    debug_log("[Boot] TIM2 (delay) init...\r\n");
//    TIM2_Init();
//    debug_log("[Boot] TIM4 (heartbeat) init...\r\n");
//    TIM4_HeartbeatInit();
//    debug_log("[Boot] timers OK\r\n");

//    debug_log("[Boot] DHT\r\n");
//    DHT11_Init();
//    debug_log("[Boot] DHT OK\r\n");

//    // --- ??I KH?C N?Y TH?NH FILE G?P ANALOG M?I ---
//    debug_log("[Boot] Analog\r\n");
//    Analog_Init(); 
//    debug_log("[Boot] ACS712 calib\r\n");
//    Analog_Calibrate();
//    debug_log("[Boot] Analog OK\r\n");

//    debug_log("[Boot] Rain\r\n");
//    Rain_Init();
//    
//    debug_log("[Boot] Flow+Motor\r\n");
//    Motor_Init();
//    FlowSensor_Init();
//    
//    debug_log("[Boot] Menu/LCD\r\n");
//    Menu_Init(); 

//    Delay_Ms(100);

//    {
//        uint8_t payload[LORA_MAX_PAYLOAD];
//        debug_log("[Boot] Snap\r\n");
//        my_read_sensor(payload, sizeof(payload));
//    }

//    g_runtime_node_id = read_dip_node_id();
//    debug_log("[Boot] Node 0x%02X\r\n", (unsigned)g_runtime_node_id);

//    debug_log("[Boot] LoRa\r\n");
//    if (!lora_radio_begin()) {
//        UART_SendString("[Boot] ERR radio\r\n");
//        while (1) { }
//    }

//    debug_log("[Boot] OK n=0x%02X gw=0x%02X\r\n",
//              (unsigned)g_runtime_node_id, (unsigned)GATEWAY_ID);
//    debug_log("[Boot] loop\r\n");

//    {
//        const lora_node_config_t cfg = {
//            .node_id = g_runtime_node_id,
//            .gateway_id = GATEWAY_ID,
//            .log = debug_log,
//        };

//        const lora_node_radio_t radio = {
//            .send = lora_radio_send,
//            .rx_pending = lora_radio_rx_pending,
//            .receive = lora_radio_receive,
//            .read_sensor = my_read_sensor_wrapper,
//        };

//        lora_node_t node;
//        lora_node_init(&node, &cfg, &radio);

//        while(1){
//										// ==================================================================
//										// LAY MAU CAM BIEN TAP TRUNG (CHI DOC 1 LAN DUY NHAT CHO TOAN BO VONG LAP)
//										// ==================================================================
//										Analog_Data_t shared_adc_data;
//										Analog_UpdateAll(&shared_adc_data); // Doc tat ca kenh ADC (Do am, Muc nuoc, Dong dien)
//					          g_current_adc_data = shared_adc_data;
//										
//										// Tinh toan nhanh dong dien (mA) va luu luong tu du lieu vua doc
//										uint16_t current_now_mA = (uint16_t)(shared_adc_data.current_ampe * 1000.0f);
//										/* Flow TIM3_CH1 (chung PWM mai CH4) — cache L/min*10 */
//										uint16_t flow_now_x10 = Flow_GetLpmX10();

//										// ------------------------------------------------------------------
//										// A. THEM: LUON QUET NUT BAM VAT LY NGOAI VUON DE CAP NHAT TRANG THAI
//										// ------------------------------------------------------------------
//										Menu_Button_Scan(&g_system_control, &g_lcd_update);

//										// 1. Luon luon quet song LoRa de nhan du lieu
//										lora_node_poll(&node);
//										
//										// 2. Kiem tra xem co lenh nut nhan moi tu Web do xuong khong
//										if (g_control_updated) {
//												g_control_updated = 0; // Xoa co ngay lap tuc de tranh xu ly lap
//												
//												//debug_log("[Hardware] Thuc thi lenh dieu khien tu Web...\r\n");
//												
//												/* ================================================================== */
//												/* DONG BO LENH WEB VAO BO DIEU KHIEN TRUNG TAM                        */
//												/* ================================================================== */
//												g_system_control.system_mode = g_remote_control.system_mode;
//												g_system_control.pump_status = g_remote_control.pump_status;
//												clamp_pump_for_water(&g_system_control.pump_status, shared_adc_data.water_percent);
//												g_system_control.roof_status = g_remote_control.roof_status;
//												current_roof_pwm = g_remote_control.roof_pwm; // Nhan them PWM tu Web
//											    current_pump_pwm = g_remote_control.pump_pwm;
//                                                #if CONTROL_TEST_DEBUG
//                                                    debug_print_control_rx((const lora_control_payload_t*)&g_remote_control);
//                                                #endif
//												g_lcd_update = 1; // Danh dau bat buoc update lai LCD

//												// Nhap nhay den LED PC13 de bao hieu xu ly lenh thanh cong
//												LED_Blink();
//										}

//										// ------------------------------------------------------------------
//										// B. THEM: LOGIC TU DONG (Su dung truc tiep du lieu tap trung o dau loop)
//										// ------------------------------------------------------------------
//										if (g_system_control.system_mode == 1) {
//												uint8_t old_pump = g_system_control.pump_status;
//												
//												// 1. Tu dong dieu khien BOM dua theo do am dat
//												// Sau loi bao ve: KHONG bat lai bom moi vong (tranh PWM nhay -> mat LoRa)
//												if (shared_adc_data.soil_percent < g_soil_on_threshold) {
//														if (g_pump_diagnostic != PUMP_DIAG_DRY_RUN
//																&& g_pump_diagnostic != PUMP_DIAG_OVERLOAD
//																&& g_pump_diagnostic != PUMP_DIAG_WATER_EMPTY) {
//																g_system_control.pump_status = 1; // Dat kho -> Bat bom
//														} else {
//																g_system_control.pump_status = 0; // Dang khoa loi bao ve
//														}
//												} else if (shared_adc_data.soil_percent > g_soil_off_threshold) {
//														g_system_control.pump_status = 0; // Du am -> Tat bom
//														/* Dat uot lai: mo khoa DRY_RUN/OVERLOAD (WATER_EMPTY clear khi co nuoc lai) */
//														if (g_pump_diagnostic == PUMP_DIAG_DRY_RUN
//																|| g_pump_diagnostic == PUMP_DIAG_OVERLOAD) {
//																g_pump_diagnostic = PUMP_DIAG_OK;
//														}
//												}
//												clamp_pump_for_water(&g_system_control.pump_status, shared_adc_data.water_percent);

//												// C?p nh?t LCD cho bom n?u tr?ng th?i bom thay d?i
//												if (old_pump != g_system_control.pump_status) {
//														g_lcd_update = 1;
//												}

//												// 2. Tu dong dieu khien MAI CHE dua vao cam bien mua & cong tac hanh trinh (?NH ?EN CHU?N)
//												if (Rain_Read() == 0) { // ?ang MUA -> Mu?n ??NG m?i
//														// Ch? ra l?nh ??NG n?u hi?n t?i m?i chua ? tr?ng th?i ??NG 
//														// V? c?ng t?c h?nh tr?nh ??NG (PB8) chua b? ch?m (m?c 1 l? chua ch?m)
//														if (g_system_control.roof_status != MOTOR_BACKWARD && GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_8) == Bit_SET) {
//																g_system_control.roof_status = MOTOR_BACKWARD;
//																g_lcd_update = 1; // D?ng c? v? LCD 1 l?n duy nh?t khi b?t d?u ch?y
//														}
//												} else { // Tr?nh kh?ng mua -> Mu?n M? m?i
//														// ?? FIX: N?u m?i dang ??NG (MOTOR_BACKWARD), ph?i D?NG tru?c (STOP)
//														if (g_system_control.roof_status == MOTOR_BACKWARD) {
//																// Chuy?n sang STOP d? motor d?ng quay ngu?c
//																g_system_control.roof_status = 0; // STOP
//																g_lcd_update = 1;
//														} 
//														// Sau khi d? STOP, b?y gi? m?i chuy?n MOTOR_FORWARD
//														else if (g_system_control.roof_status != MOTOR_FORWARD && 
//																		 GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_7) == Bit_SET) {
//																g_system_control.roof_status = MOTOR_FORWARD;
//																g_lcd_update = 1;
//														}
//												}
//										}

//										// ------------------------------------------------------------------
//										// C. CHAN DOAN LOI BOM — uu tien: 3 can bon > 2 overload > 1 dry-run
//										// Can bon: check ngay. DRY_RUN/OVERLOAD: sau grace ~2s khi vua bat bom
//										// ------------------------------------------------------------------
//										if (shared_adc_data.water_percent < WATER_EMPTY_PERCENT) {
//												if (g_system_control.pump_status != 0) {
//														g_system_control.pump_status = 0;
//														g_lcd_update = 1;
//												}
//												g_pump_diagnostic = PUMP_DIAG_WATER_EMPTY;
//										} else if (g_system_control.pump_status == 1) {

//												if (!s_prev_pump_on) {
//														s_pump_on_since_ms = millis();
//												}

//												if ((millis() - s_pump_on_since_ms) < PUMP_PROTECT_GRACE_MS) {
//														/* Grace: cho flow/dong on dinh — chua bat DRY_RUN/OVERLOAD */
//												}
//												// Truong hop A: Dong dien vuot nguong an toan (> 2A) -> Bom ket / Qua tai
//												else if (current_now_mA > 2000) {
//														g_pump_diagnostic = PUMP_DIAG_OVERLOAD;
//														g_system_control.pump_status = 0;
//														g_lcd_update = 1;
//												}
//												// Truong hop B: Dong cuc thap hoac khong co nuoc chay qua -> Chay kho
//												else if (current_now_mA < 100 || flow_now_x10 < 2u) {
//														g_pump_diagnostic = PUMP_DIAG_DRY_RUN;
//														g_system_control.pump_status = 0;
//														g_lcd_update = 1;
//												}
//												// Truong hop C: Moi thu on dinh
//												else {
//														g_pump_diagnostic = PUMP_DIAG_OK;
//												}

//										} else {
//												/* Bom tat + nuoc du: xoa WATER_EMPTY; giu latch DRY_RUN/OVERLOAD */
//												if (g_pump_diagnostic == PUMP_DIAG_WATER_EMPTY) {
//														g_pump_diagnostic = PUMP_DIAG_OK;
//												} else if (g_pump_diagnostic != PUMP_DIAG_DRY_RUN
//																&& g_pump_diagnostic != PUMP_DIAG_OVERLOAD) {
//														g_pump_diagnostic = PUMP_DIAG_OK;
//												}
//										}
//										s_prev_pump_on = (g_system_control.pump_status == 1) ? 1u : 0u;
//										// G?i ACK mang TR?NG TH?I TH?T (sau khi AUTO + an to?n d? x? l? xong)
//										if (g_ack_pending) {
//												g_ack_pending = 0;
//												lora_control_payload_t ack_state;
//												ack_state.system_mode = g_system_control.system_mode;
//												ack_state.pump_status = g_system_control.pump_status;
//												ack_state.roof_status = g_system_control.roof_status;
//												ack_state.pump_pwm    = current_pump_pwm;
//												ack_state.roof_pwm    = current_roof_pwm;
//                                                #if CONTROL_TEST_DEBUG
//                                                debug_print_control_ack(&ack_state);
//                                                #endif
//												lora_node_send_ack(&node, &ack_state);
//										}

//										// ------------------------------------------------------------------
//										// D. XUAT DAU RA PHAN CUNG THAT THEO BIEN TRUNG TAM
//										// ------------------------------------------------------------------
//                    clamp_pump_for_water(&g_system_control.pump_status, shared_adc_data.water_percent);
//                    Motor_Roof_Safety_Supervisor((uint8_t *)&g_system_control.roof_status, &g_lcd_update);
//										/* ---------------- A. DIEU KHIEN MAY BOM ---------------- */

//										if (g_system_control.pump_status == 1) {
//											  Pump_SetSpeed(current_pump_pwm);
//										} else {
//												Pump_Off();
//										}

//										/* ---------------- B. DIEU KHIEN MAI CHE ---------------- */

//										Motor1_Dir(g_system_control.roof_status);

//										if (g_system_control.roof_status == MOTOR_FORWARD) {
//												Motor1_SetSpeed(current_roof_pwm);
//												//debug_log("-> Phan cung: MO MAI CHE - PWM: %u%%\r\n", current_roof_pwm);

//										} else if (g_system_control.roof_status == MOTOR_BACKWARD) {
//												Motor1_SetSpeed(current_roof_pwm);
//												//debug_log("-> Phan cung: DONG MAI CHE - PWM: %u%%\r\n", current_roof_pwm);

//										} else {
//												Motor1_SetSpeed(0);
//												//debug_log("-> Phan cung: DUNG MAI CHE\r\n");
//										}

//										/* ------------- C. DONG BO CHE DO HE THONG ------------- */

//										if (g_system_control.system_mode == 1) {
//												//debug_log("-> He thong dang o che do: TU DONG (Auto)\r\n");
//										} else {
//												//debug_log("-> He thong dang o che do: THU CONG (Manual)\r\n");
//										}

//										// ------------------------------------------------------------------
//										// E. THEM: CAP NHAT GIAO DIEN MAN HINH LCD KHI CO THAY DOI
//										// ------------------------------------------------------------------

//										if (g_lcd_update) {
//												Menu_Display_Update(&g_system_control);
//												g_lcd_update = 0; // Xoa co sau khi cap nhat man hinh xong
//										}

//										// 3. Giu nhip dap heartbeat giam sat tai cho
//										debug_heartbeat();

//										// Chong loop quay qua nhanh gay doi phim khi quet nut
//										Delay_Ms(20);
//        }
//    }
//}
/////////////////////////////////main-da chinh/////////////////////////////

/* Bi?n to?n c?c luu tr?ng th?i n?t nh?n nh?n t? LoRa Gateway */
volatile lora_control_payload_t g_remote_control;
volatile uint8_t g_control_updated = 0; // C? b?o hi?u c? l?nh m?i
extern volatile uint8_t g_ack_pending;

/* ========================================================================== */
/* ?? TH?M: BI?N TO?N C?C QU?N L? TR?NG TH?I TH?C T? T?I VU?N                 */
/* ========================================================================== */
menu_control_data_t g_system_control = {
    /* Boot MANUAL: tranh Auto bat bom khi ADC dat noi -> PWM nhi?u -> mat LoRa luc demo */
    .system_mode = 0, // 0 = MANUAL, 1 = AUTO
    .pump_status = 0, // 0 = OFF, 1 = ON
    .roof_status = 0  // 0 = STOP, MOTOR_FORWARD = OPEN, MOTOR_BACKWARD = CLOSE
};
uint8_t g_lcd_update = 1; // C? b?o hi?u c?n v? l?i m?n h?nh LCD
uint8_t current_roof_pwm = 100; // Luu t?c d? m?i che nh?n t? Web
/* Bi?n qu?n l? ch?n do?n l?i tr?m bom th?c t? */
uint8_t current_pump_pwm = 100;
volatile pump_diagnostic_t g_pump_diagnostic = PUMP_DIAG_OK;

/* TEST ONLY: dat 1 = tat bao ve bom (overload/dry-run/can bon). Xong test doi lai 0! */
#define PUMP_PROTECT_DISABLE  1

/* Sau khi bat bom: cho nuoc/dong on dinh roi moi check DRY_RUN/OVERLOAD */
#define PUMP_PROTECT_GRACE_MS  2000u
static uint8_t s_prev_pump_on = 0;
static uint32_t s_pump_on_since_ms = 0;

/* === Bi?n t?m d? truy?n ADC data d?n h?m callback LoRa === */
static Analog_Data_t g_current_adc_data = {0};

#define WATER_EMPTY_PERCENT  10
uint8_t g_soil_on_threshold = 30;
uint8_t g_soil_off_threshold = 80;
static uint8_t g_runtime_node_id = 0;

static uint8_t read_dip_node_id(void)
{
    GPIO_InitTypeDef gpio;
    uint8_t b0, b1, b2, dip;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    gpio.GPIO_Mode = GPIO_Mode_IPD;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    gpio.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_6 | GPIO_Pin_3;
    GPIO_Init(GPIOB, &gpio);

    b0 = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9) == Bit_SET ? 1 : 0;
    b1 = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_6) == Bit_SET ? 1 : 0;
    b2 = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3) == Bit_SET ? 1 : 0;
    dip = (uint8_t)(b0 | (b1 << 1) | (b2 << 2));

    if (dip == 0) {
        return (uint8_t)MY_NODE_ID;
    }
    return (uint8_t)(0x10 | dip);
}

static void clamp_pump_for_water(uint8_t *pump_status, uint8_t water_percent)
{
#if PUMP_PROTECT_DISABLE
    (void)pump_status;
    (void)water_percent;
    return;
#endif
    if (*pump_status != 0 && water_percent < WATER_EMPTY_PERCENT) {
        *pump_status = 0;
    }
}



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
#if CONTROL_TEST_DEBUG
static void debug_print_control_rx(const lora_control_payload_t *cmd)
{
    const char *roof_str = (cmd->roof_status == 1) ? "OPEN" :
                           (cmd->roof_status == 2) ? "CLOSE" : "STOP";
    debug_log("\r\n========== [CTRL-RX] LENH TU WEB ==========\r\n");
    debug_log("  mode      : %s\r\n", cmd->system_mode ? "AUTO" : "MANUAL");
    debug_log("  pump      : %s\r\n", cmd->pump_status ? "BAT" : "TAT");
    debug_log("  pump_pwm  : %u%%\r\n", (unsigned)cmd->pump_pwm);
    debug_log("  roof      : %s\r\n", roof_str);
    debug_log("  roof_pwm  : %u%%\r\n", (unsigned)cmd->roof_pwm);
    debug_log("============================================\r\n");
}

static void debug_print_control_ack(const lora_control_payload_t *ack)
{
    const char *roof_str = (ack->roof_status == 1) ? "OPEN" :
                           (ack->roof_status == 2) ? "CLOSE" : "STOP";
    debug_log("\r\n========== [ACK-TX] GUI LEN ESP ==========\r\n");
    debug_log("  mode      : %s\r\n", ack->system_mode ? "AUTO" : "MANUAL");
    debug_log("  pump      : %s\r\n", ack->pump_status ? "BAT" : "TAT");
    debug_log("  pump_pwm  : %u%%\r\n", (unsigned)ack->pump_pwm);
    debug_log("  roof      : %s\r\n", roof_str);
    debug_log("  roof_pwm  : %u%%\r\n", (unsigned)ack->roof_pwm);
    debug_log("============================================\r\n");
}
#endif
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
        debug_log("[HB] 0x%02X\r\n",
                  (unsigned)g_runtime_node_id);
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
 * ?? FIX: S? d?ng ADC data d? d?c s?n t? g_current_adc_data
 */
static uint8_t my_read_sensor(uint8_t *payload, uint8_t max_len)
{
    lora_sensor_payload_t sample;
    uint8_t temp = 0;
    uint8_t humi = 0;

    if (max_len < sizeof(sample)) {
        debug_log("[S] buf small %u\r\n",
          (unsigned)max_len);
        return 0;
    }

    memset(&sample, 0, sizeof(sample));
    
    // 1. ??c DHT11
    if (DHT11_ReadData(&temp, &humi) == DHT11_OK) {
        sample.temperature_c10 = (int16_t)temp * 10;
        sample.humidity_pct10 = (uint16_t)humi * 10;
    }

    // 2. ?? D?NG D? LI?U ADC ?? ??C S?N (kh?ng d?c l?i)
    sample.soil_moisture = (uint16_t)g_current_adc_data.soil_percent;
    sample.water_level   = (uint16_t)g_current_adc_data.water_percent;
    sample.current_mA    = (uint16_t)(g_current_adc_data.current_ampe * 1000.0f);
    
    // 3. ??c c?m bi?n mua (Digital)
    sample.rain_status = Rain_Read();

    // 4. Doc flow (TIM3_CH1 IC, chung TIM3 voi PWM mai) — cache ~500ms
    uint16_t flow_x10 = Flow_GetLpmX10();
    sample.flow_rate_Lmin_x10 = (uint8_t)((flow_x10 > 255u) ? 255u : flow_x10);

    // 5. ??ng b? tr?ng th?i th?c t? t?i vu?n l?n Web
    sample.system_mode = g_system_control.system_mode;
    sample.pump_status = g_system_control.pump_status;
    sample.roof_status = g_system_control.roof_status;
    sample.pump_diagnostic = (uint8_t)g_pump_diagnostic;

    // Debug log (không dùng %.1f — tránh kéo lib printf float vu?t 32KB Keil Lite)
    debug_log("[S] T=%d H=%u S=%u W=%u I=%u R=%s F=%u.%u\r\n",
              (int)sample.temperature_c10/10,
              (unsigned)sample.humidity_pct10/10,
              (unsigned)sample.soil_moisture,
              (unsigned)sample.water_level,
              (unsigned)sample.current_mA,
              (sample.rain_status == 0) ? "Y" : "N",
              (unsigned)(sample.flow_rate_Lmin_x10 / 10),
              (unsigned)(sample.flow_rate_Lmin_x10 % 10));

    memcpy(payload, &sample, sizeof(sample));
    return (uint8_t)sizeof(sample);
}

/**
 * Wrapper function: G?i my_read_sensor (kh?ng c?n tham s? ADC v? d?ng global)
 */
static uint8_t my_read_sensor_wrapper(uint8_t *payload, uint8_t max_len) {
    return my_read_sensor(payload, max_len);  // ? Ch? 2 tham s?
}
// ===== MAIN =====
int main(void) {
    LED_Init();

    UART1_Init(115200);
    UART_SendString("\r\n===== STM32 LoRa Slave Node (Updated) =====\r\n");
    debug_log("[Boot] UART1 115200 OK\r\n");

    debug_log("[Boot] TIM2 (delay) init...\r\n");
    TIM2_Init();
    debug_log("[Boot] TIM4 (heartbeat) init...\r\n");
    TIM4_HeartbeatInit();
    debug_log("[Boot] timers OK\r\n");

    debug_log("[Boot] DHT\r\n");
    DHT11_Init();
    debug_log("[Boot] DHT OK\r\n");

    // --- ??I KH?C N?Y TH?NH FILE G?P ANALOG M?I ---
    debug_log("[Boot] Analog\r\n");
    Analog_Init(); 
    debug_log("[Boot] ACS712 calib\r\n");
    Analog_Calibrate();
    debug_log("[Boot] Analog OK\r\n");
#if PUMP_PROTECT_DISABLE
    debug_log("[Boot] PUMP PROT OFF\r\n");
#endif

    debug_log("[Boot] Rain\r\n");
    Rain_Init();
    
    debug_log("[Boot] Flow+Motor\r\n");
    Motor_Init();
    FlowSensor_Init();
    
    debug_log("[Boot] Menu/LCD\r\n");
    Menu_Init(); 

    Delay_Ms(100);

    {
        uint8_t payload[LORA_MAX_PAYLOAD];
        debug_log("[Boot] Snap\r\n");
        my_read_sensor(payload, sizeof(payload));
    }

    g_runtime_node_id = read_dip_node_id();
    debug_log("[Boot] Node 0x%02X\r\n", (unsigned)g_runtime_node_id);

    debug_log("[Boot] LoRa\r\n");
    if (!lora_radio_begin()) {
        UART_SendString("[Boot] ERR radio\r\n");
        while (1) { }
    }

    debug_log("[Boot] OK n=0x%02X gw=0x%02X\r\n",
              (unsigned)g_runtime_node_id, (unsigned)GATEWAY_ID);
    debug_log("[Boot] loop\r\n");

    {
        const lora_node_config_t cfg = {
            .node_id = g_runtime_node_id,
            .gateway_id = GATEWAY_ID,
            .log = debug_log,
        };

        const lora_node_radio_t radio = {
            .send = lora_radio_send,
            .rx_pending = lora_radio_rx_pending,
            .receive = lora_radio_receive,
            .read_sensor = my_read_sensor_wrapper,
        };

        lora_node_t node;
        lora_node_init(&node, &cfg, &radio);

        while(1){
										// ==================================================================
										// LAY MAU CAM BIEN TAP TRUNG (CHI DOC 1 LAN DUY NHAT CHO TOAN BO VONG LAP)
										// ==================================================================
										Analog_Data_t shared_adc_data;
										Analog_UpdateAll(&shared_adc_data); // Doc tat ca kenh ADC (Do am, Muc nuoc, Dong dien)
					          g_current_adc_data = shared_adc_data;
										
										// Tinh toan nhanh dong dien (mA) va luu luong tu du lieu vua doc
										uint16_t current_now_mA = (uint16_t)(shared_adc_data.current_ampe * 1000.0f);
										/* Flow TIM3_CH1 (chung PWM mai CH4) — cache L/min*10 */
										uint16_t flow_now_x10 = Flow_GetLpmX10();

										// ------------------------------------------------------------------
										// A. THEM: LUON QUET NUT BAM VAT LY NGOAI VUON DE CAP NHAT TRANG THAI
										// ------------------------------------------------------------------
										Menu_Button_Scan(&g_system_control, &g_lcd_update);

										// 1. Luon luon quet song LoRa de nhan du lieu
										lora_node_poll(&node);
										
										// 2. Kiem tra xem co lenh nut nhan moi tu Web do xuong khong
										if (g_control_updated) {
												g_control_updated = 0; // Xoa co ngay lap tuc de tranh xu ly lap
												
												//debug_log("[Hardware] Thuc thi lenh dieu khien tu Web...\r\n");
												
												/* ================================================================== */
												/* DONG BO LENH WEB VAO BO DIEU KHIEN TRUNG TAM                        */
												/* ================================================================== */
												g_system_control.system_mode = g_remote_control.system_mode;
												g_system_control.pump_status = g_remote_control.pump_status;
												clamp_pump_for_water(&g_system_control.pump_status, shared_adc_data.water_percent);
												g_system_control.roof_status = g_remote_control.roof_status;
												current_roof_pwm = g_remote_control.roof_pwm; // Nhan them PWM tu Web
											    current_pump_pwm = g_remote_control.pump_pwm;
                                                #if CONTROL_TEST_DEBUG
                                                    debug_print_control_rx((const lora_control_payload_t*)&g_remote_control);
                                                #endif
												g_lcd_update = 1; // Danh dau bat buoc update lai LCD

												// Nhap nhay den LED PC13 de bao hieu xu ly lenh thanh cong
												LED_Blink();
										}

										// ------------------------------------------------------------------
										// B. THEM: LOGIC TU DONG (Su dung truc tiep du lieu tap trung o dau loop)
										// ------------------------------------------------------------------
										if (g_system_control.system_mode == 1) {
												uint8_t old_pump = g_system_control.pump_status;
												
												// 1. Tu dong dieu khien BOM dua theo do am dat
												// Sau loi bao ve: KHONG bat lai bom moi vong (tranh PWM nhay -> mat LoRa)
												if (shared_adc_data.soil_percent < g_soil_on_threshold) {
#if !PUMP_PROTECT_DISABLE
														if (g_pump_diagnostic != PUMP_DIAG_DRY_RUN
																&& g_pump_diagnostic != PUMP_DIAG_OVERLOAD
																&& g_pump_diagnostic != PUMP_DIAG_WATER_EMPTY) {
																g_system_control.pump_status = 1; // Dat kho -> Bat bom
														} else {
																g_system_control.pump_status = 0; // Dang khoa loi bao ve
														}
#else
														g_system_control.pump_status = 1;
#endif
												} else if (shared_adc_data.soil_percent > g_soil_off_threshold) {
														g_system_control.pump_status = 0; // Du am -> Tat bom
														/* Dat uot lai: mo khoa DRY_RUN/OVERLOAD (WATER_EMPTY clear khi co nuoc lai) */
														if (g_pump_diagnostic == PUMP_DIAG_DRY_RUN
																|| g_pump_diagnostic == PUMP_DIAG_OVERLOAD) {
																g_pump_diagnostic = PUMP_DIAG_OK;
														}
												}
												clamp_pump_for_water(&g_system_control.pump_status, shared_adc_data.water_percent);

												// C?p nh?t LCD cho bom n?u tr?ng th?i bom thay d?i
												if (old_pump != g_system_control.pump_status) {
														g_lcd_update = 1;
												}

												// 2. Tu dong dieu khien MAI CHE dua vao cam bien mua & cong tac hanh trinh (?NH ?EN CHU?N)
												if (Rain_Read() == 0) { // ?ang MUA -> Mu?n ??NG m?i
														// Ch? ra l?nh ??NG n?u hi?n t?i m?i chua ? tr?ng th?i ??NG 
														// V? c?ng t?c h?nh tr?nh ??NG (PB8) chua b? ch?m (m?c 1 l? chua ch?m)
														if (g_system_control.roof_status != MOTOR_BACKWARD && GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_8) == Bit_SET) {
																g_system_control.roof_status = MOTOR_BACKWARD;
																g_lcd_update = 1; // D?ng c? v? LCD 1 l?n duy nh?t khi b?t d?u ch?y
														}
												} else { // Tr?nh kh?ng mua -> Mu?n M? m?i
														// ?? FIX: N?u m?i dang ??NG (MOTOR_BACKWARD), ph?i D?NG tru?c (STOP)
														if (g_system_control.roof_status == MOTOR_BACKWARD) {
																// Chuy?n sang STOP d? motor d?ng quay ngu?c
																g_system_control.roof_status = 0; // STOP
																g_lcd_update = 1;
														} 
														// Sau khi d? STOP, b?y gi? m?i chuy?n MOTOR_FORWARD
														else if (g_system_control.roof_status != MOTOR_FORWARD && 
																		 GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_7) == Bit_SET) {
																g_system_control.roof_status = MOTOR_FORWARD;
																g_lcd_update = 1;
														}
												}
										}

										// ------------------------------------------------------------------
										// C. CHAN DOAN LOI BOM — uu tien: 3 can bon > 2 overload > 1 dry-run
										// Can bon: check ngay. DRY_RUN/OVERLOAD: sau grace ~2s khi vua bat bom
										// ------------------------------------------------------------------
#if !PUMP_PROTECT_DISABLE
										if (shared_adc_data.water_percent < WATER_EMPTY_PERCENT) {
												if (g_system_control.pump_status != 0) {
														g_system_control.pump_status = 0;
														g_lcd_update = 1;
												}
												g_pump_diagnostic = PUMP_DIAG_WATER_EMPTY;
										} else if (g_system_control.pump_status == 1) {

												if (!s_prev_pump_on) {
														s_pump_on_since_ms = millis();
												}

												if ((millis() - s_pump_on_since_ms) < PUMP_PROTECT_GRACE_MS) {
														/* Grace: cho flow/dong on dinh — chua bat DRY_RUN/OVERLOAD */
												}
												// Truong hop A: Dong dien vuot nguong an toan (> 2A) -> Bom ket / Qua tai
												else if (current_now_mA > 2000) {
														g_pump_diagnostic = PUMP_DIAG_OVERLOAD;
														g_system_control.pump_status = 0;
														g_lcd_update = 1;
												}
												// Truong hop B: Dong cuc thap hoac khong co nuoc chay qua -> Chay kho
												else if (current_now_mA < 100 || flow_now_x10 < 2u) {
														g_pump_diagnostic = PUMP_DIAG_DRY_RUN;
														g_system_control.pump_status = 0;
														g_lcd_update = 1;
												}
												// Truong hop C: Moi thu on dinh
												else {
														g_pump_diagnostic = PUMP_DIAG_OK;
												}

										} else {
												/* Bom tat + nuoc du: xoa WATER_EMPTY; giu latch DRY_RUN/OVERLOAD */
												if (g_pump_diagnostic == PUMP_DIAG_WATER_EMPTY) {
														g_pump_diagnostic = PUMP_DIAG_OK;
												} else if (g_pump_diagnostic != PUMP_DIAG_DRY_RUN
																&& g_pump_diagnostic != PUMP_DIAG_OVERLOAD) {
														g_pump_diagnostic = PUMP_DIAG_OK;
												}
										}
										s_prev_pump_on = (g_system_control.pump_status == 1) ? 1u : 0u;
#else
										g_pump_diagnostic = PUMP_DIAG_OK;
										s_prev_pump_on = (g_system_control.pump_status == 1) ? 1u : 0u;
#endif
										// G?i ACK mang TR?NG TH?I TH?T (sau khi AUTO + an to?n d? x? l? xong)
										if (g_ack_pending) {
												g_ack_pending = 0;
												lora_control_payload_t ack_state;
												ack_state.system_mode = g_system_control.system_mode;
												ack_state.pump_status = g_system_control.pump_status;
												ack_state.roof_status = g_system_control.roof_status;
												ack_state.pump_pwm    = current_pump_pwm;
												ack_state.roof_pwm    = current_roof_pwm;
                                                #if CONTROL_TEST_DEBUG
                                                debug_print_control_ack(&ack_state);
                                                #endif
												lora_node_send_ack(&node, &ack_state);
										}

										// ------------------------------------------------------------------
										// D. XUAT DAU RA PHAN CUNG THAT THEO BIEN TRUNG TAM
										// ------------------------------------------------------------------
                    clamp_pump_for_water(&g_system_control.pump_status, shared_adc_data.water_percent);
                    Motor_Roof_Safety_Supervisor((uint8_t *)&g_system_control.roof_status, &g_lcd_update);
										/* ---------------- A. DIEU KHIEN MAY BOM ---------------- */

										if (g_system_control.pump_status == 1) {
											  Pump_SetSpeed(current_pump_pwm);
										} else {
												Pump_Off();
										}

										/* ---------------- B. DIEU KHIEN MAI CHE ---------------- */

										Motor1_Dir(g_system_control.roof_status);

										if (g_system_control.roof_status == MOTOR_FORWARD) {
												Motor1_SetSpeed(current_roof_pwm);
												//debug_log("-> Phan cung: MO MAI CHE - PWM: %u%%\r\n", current_roof_pwm);

										} else if (g_system_control.roof_status == MOTOR_BACKWARD) {
												Motor1_SetSpeed(current_roof_pwm);
												//debug_log("-> Phan cung: DONG MAI CHE - PWM: %u%%\r\n", current_roof_pwm);

										} else {
												Motor1_SetSpeed(0);
												//debug_log("-> Phan cung: DUNG MAI CHE\r\n");
										}

										/* ------------- C. DONG BO CHE DO HE THONG ------------- */

										if (g_system_control.system_mode == 1) {
												//debug_log("-> He thong dang o che do: TU DONG (Auto)\r\n");
										} else {
												//debug_log("-> He thong dang o che do: THU CONG (Manual)\r\n");
										}

										// ------------------------------------------------------------------
										// E. THEM: CAP NHAT GIAO DIEN MAN HINH LCD KHI CO THAY DOI
										// ------------------------------------------------------------------

										if (g_lcd_update) {
												Menu_Display_Update(&g_system_control);
												g_lcd_update = 0; // Xoa co sau khi cap nhat man hinh xong
										}

										// 3. Giu nhip dap heartbeat giam sat tai cho
										debug_heartbeat();

										// Chong loop quay qua nhanh gay doi phim khi quet nut
										Delay_Ms(20);
        }
    }
}

//////////////////////////////////////////////fake data sensor////////////////////////////////////////////////////////


