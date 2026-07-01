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
#include "relay.h"
#include "flow_sensor.h"
#include "analog.h"
#include "motor.h"

#include "lora_node.h"
#include "lora_protocol.h"
#include "lora_network_config.h"
#include "lora_radio.h"
#include "menu_control.h"
///* Biến toàn cục lưu trạng thái nút nhấn nhận từ LoRa Gateway */
//volatile lora_control_payload_t g_remote_control;
//volatile uint8_t g_control_updated = 0; // Cờ báo hiệu có lệnh mới

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

//    // 2. Đọc nhóm cảm biến ADC gộp (Độ ẩm đất, Mực nước, Dòng điện)
//    Analog_UpdateAll(&analog_sensor_data);
//    sample.soil_moisture = (uint16_t)analog_sensor_data.soil_percent;
//    sample.water_level   = (uint16_t)analog_sensor_data.water_percent;
//    sample.current_mA    = (uint16_t)(analog_sensor_data.current_ampe * 1000.0f); // Quy đổi A -> mA
//		
//		// 3. Đọc cảm biến mưa (Digital)
//    sample.rain_status = Rain_Read();

//    // 4. Đọc cảm biến lưu lượng nước (Tính toán dựa trên thời gian lấy mẫu, ví dụ 1000ms)
//    // Để chính xác, ở đây lấy mẫu tạm thời 1000ms hoặc bạn có thể tối ưu theo thời gian thực vòng lặp
//    float flow_real = Flow_GetLitersPerMinute(1000); 
//    sample.flow_rate_Lmin_x10 = (uint8_t)(flow_real * 10.0f);

//    // In log debug ra UART1 để giám sát tại chỗ
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

//    // --- ĐỔI KHÚC NÀY THÀNH FILE GỘP ANALOG MỚI ---
//    debug_log("[Boot] Analog ADC channels init...\r\n");
//    Analog_Init(); 
//    debug_log("[Boot] Calibrating ACS712 Current Sensor...\r\n");
//    Analog_Calibrate(); // Tự động lấy điểm 0 dòng điện lúc khởi động
//    debug_log("[Boot] Analog ADC OK\r\n");
//    // ----------------------------------------------

//    // --- THÊM CÁC CẢM BIẾN MỚI KHỞI TẠO Ở ĐÂY ---
//    debug_log("[Boot] Rain sensor init...\r\n");
//    Rain_Init();
//    
//    debug_log("[Boot] Flow sensor init...\r\n");
//		Motor_Init();
//    FlowSensor_Init(); // Đã giải phóng chân PB4 bên trong hàm
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
//    // 1. Luôn luôn quét sóng LoRa để nhận dữ liệu
//    lora_node_poll(&node);
//    
//    // 2. Kiểm tra xem có lệnh nút nhấn mới từ Web đổ xuống không
//    if (g_control_updated) {
//        g_control_updated = 0; // Xóa cờ ngay lập tức để tránh xử lý lặp
//        
//        debug_log("[Hardware] Thực thi lệnh điều khiển từ Web...\r\n");
//        
//        /* ---------------- A. ĐIỀU KHIỂN MÁY BƠM ---------------- */
//        if (g_remote_control.pump_status == 1) {
//            // Nhóm của bạn hãy gọi hàm kích hoạt Relay/Bơm thật ở đây, ví dụ:
//            // Relay_Pump_On();
//            debug_log("-> Phần cứng: BẬT BƠM - Tốc độ PWM: %u%%\r\n", g_remote_control.pump_pwm);
//        } else {
//            // Relay_Pump_Off();
//            debug_log("-> Phần cứng: TẮT BƠM\r\n");
//        }
//        
//        /* ---------------- B. ĐIỀU KHIỂN MÁI CHE ---------------- */
//        if (g_remote_control.roof_status == 1) {
//            // Nhóm của bạn hãy gọi hàm kích hoạt Motor quay thuận ở đây, ví dụ:
//            // Motor_Roof_Open(g_remote_control.roof_pwm);
//            debug_log("-> Phần cứng: MỞ MÁI CHE - PWM: %u%%\r\n", g_remote_control.roof_pwm);
//        } 
//        else if (g_remote_control.roof_status == 2) {
//            // Nhóm của bạn hãy gọi hàm kích hoạt Motor quay ngược ở đây, ví dụ:
//            // Motor_Roof_Close(g_remote_control.roof_pwm);
//            debug_log("-> Phần cứng: ĐÓNG MÁI CHE - PWM: %u%%\r\n", g_remote_control.roof_pwm);
//        } 
//        else {
//            // Nhóm của bạn hãy gọi hàm dừng Motor ở đây, ví dụ:
//            // Motor_Roof_Stop();
//            debug_log("-> Phần cứng: DỪNG MÁI CHE\r\n");
//        }
//        
//        /* ------------- C. ĐỒNG BỘ CHẾ ĐỘ HỆ THỐNG ------------- */
//        if (g_remote_control.system_mode == 1) {
//            debug_log("-> Hệ thống chuyển sang chế độ: TỰ ĐỘNG (Auto)\r\n");
//        } else {
//            debug_log("-> Hệ thống chuyển sang chế độ: THỦ CÔNG (Manual)\r\n");
//        }
//        
//        // Nhấp nháy đèn LED PC13 để báo hiệu xử lý lệnh thành công
//        LED_Blink();
//    }
//    
//    // 3. Giữ nhịp đập heartbeat giám sát tại chỗ
//    debug_heartbeat();
//}

//    }
//}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/* Bi?n to�n c?c luu tr?ng th�i n�t nh?n nh?n t? LoRa Gateway */
volatile lora_control_payload_t g_remote_control;
volatile uint8_t g_control_updated = 0; // C? b�o hi?u c� l?nh m?i

/* ========================================================================== */
/* ?? TH�M: BI?N TO�N C?C QU?N L� TR?NG TH�I TH?C T? T?I VU?N                 */
/* ========================================================================== */
menu_control_data_t g_system_control = {
    .system_mode = 0, // Ban d?u m?c d?nh: 0 = MANUAL, 1 = AUTO
    .pump_status = 0, // 0 = OFF, 1 = ON
    .roof_status = 0  // 0 = STOP, MOTOR_FORWARD = OPEN, MOTOR_BACKWARD = CLOSE
};
uint8_t g_lcd_update = 1; // C? b�o hi?u c?n v? l?i m�n h�nh LCD
uint8_t current_roof_pwm = 100; // Luu t?c d? m�i che nh?n t? Web
/* Bi?n qu?n l� ch?n do�n l?i tr?m bom th?c t? */
volatile uint8_t g_pump_diagnostic = 0; // 0: B�nh thu?ng, 1: Ch?y kh�, 2: Qu� t?i

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
    Analog_Data_t analog_sensor_data;

    if (max_len < sizeof(sample)) {
        debug_log("[Sensor] read failed: buffer too small (%u)\r\n",
                  (unsigned)max_len);
        return 0;
    }

    memset(&sample, 0, sizeof(sample));
    //doc DHT11
    if (DHT11_ReadData(&temp, &humi) == DHT11_OK) {
        sample.temperature_c10 = (int16_t)temp * 10;
        sample.humidity_pct10 = (uint16_t)humi * 10;
    }

    // 2. �?c nh�m c?m bi?n ADC g?p (�? ?m d?t, M?c nu?c, D�ng di?n)
    Analog_UpdateAll(&analog_sensor_data);
    sample.soil_moisture = (uint16_t)analog_sensor_data.soil_percent;
    sample.water_level   = (uint16_t)analog_sensor_data.water_percent;
    sample.current_mA    = (uint16_t)(analog_sensor_data.current_ampe * 1000.0f); // Quy d?i A -> mA
        
    // 3. �?c c?m bi?n mua (Digital)
    sample.rain_status = Rain_Read();

    // 4. �?c c?m bi?n luu lu?ng nu?c (T�nh to�n d?a tr�n th?i gian l?y m?u, v� d? 1000ms)
    // �? ch�nh x�c, ? d�y l?y m?u t?m th?i 1000ms ho?c b?n c� th? t?i uu theo th?i gian th?c v�ng l?p
    float flow_real = Flow_GetLitersPerMinute(1000); 
    sample.flow_rate_Lmin_x10 = (uint8_t)(flow_real * 10.0f);

    /* ================================================================== */
    /* ?? TH�M: �?NG B? NGU?C TR?NG TH�I TH?C T? T?I VU?N L�N WEB GUI     */
    /* ================================================================== */
    sample.system_mode = g_system_control.system_mode;
    sample.pump_status = g_system_control.pump_status;
    sample.roof_status = g_system_control.roof_status;
		
		sample.pump_diagnostic = g_pump_diagnostic;

    // In log debug ra UART1 d? gi�m s�t t?i ch?
    debug_log("[Sensor] Temp=%d|Hum=%u|Soil=%u%%|Water=%u%%|Current=%umA|Rain=%s|Flow=%.1f L/m\r\n",
              (int)sample.temperature_c10/10,
              (unsigned)sample.humidity_pct10/10,
              (unsigned)sample.soil_moisture,
              (unsigned)sample.water_level,
              (unsigned)sample.current_mA,
              (sample.rain_status == 0) ? "YES" : "NO",
              flow_real);

    memcpy(payload, &sample, sizeof(sample));
    return (uint8_t)sizeof(sample);
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

    debug_log("[Boot] DHT11 init...\r\n");
    DHT11_Init();
    debug_log("[Boot] DHT11 OK\r\n");

    // --- �?I KH�C N�Y TH�NH FILE G?P ANALOG M?I ---
    debug_log("[Boot] Analog ADC channels init...\r\n");
    Analog_Init(); 
    debug_log("[Boot] Calibrating ACS712 Current Sensor...\r\n");
    Analog_Calibrate(); // T? d?ng l?y di?m 0 d�ng di?n l�c kh?i d?ng
    debug_log("[Boot] Analog ADC OK\r\n");
    // ----------------------------------------------

    // --- TH�M C�C C?M BI?N M?I KH?I T?O ? ��Y ---
    debug_log("[Boot] Rain sensor init...\r\n");
    Rain_Init();
    
    debug_log("[Boot] Flow sensor init...\r\n");
    Motor_Init();
    FlowSensor_Init(); // �� gi?i ph�ng ch�n PB4 b�n trong h�m
    
    // --- TH�M: Kh?i t?o h? th?ng Menu n�t b?m v� m�n h�nh LCD ---
    debug_log("[Boot] Menu Button & LCD init...\r\n");
    Menu_Init(); 
    // --------------------------------------------

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
										// ==================================================================
										// LAY MAU CAM BIEN TAP TRUNG (CHI DOC 1 LAN DUY NHAT CHO TOAN BO VONG LAP)
										// ==================================================================
										Analog_Data_t shared_adc_data;
										Analog_UpdateAll(&shared_adc_data); // Doc tat ca kenh ADC (Do am, Muc nuoc, Dong dien)
										
										// Tinh toan nhanh dong dien (mA) va luu luong tu du lieu vua doc
										uint16_t current_now_mA = (uint16_t)(shared_adc_data.current_ampe * 1000.0f);
										float flow_now = Flow_GetLitersPerMinute(20); // Lay mau nhanh phan hoi luu luong nuoc

										// ------------------------------------------------------------------
										// A. THEM: LUON QUET NUT BAM VAT LY NGOAI VUON DE CAP NHAT TRANG THAI
										// ------------------------------------------------------------------
										Menu_Button_Scan(&g_system_control, &g_lcd_update);

										// 1. Luon luon quet song LoRa de nhan du lieu
										lora_node_poll(&node);
										
										// 2. Kiem tra xem co lenh nut nhan moi tu Web do xuong khong
										if (g_control_updated) {
												g_control_updated = 0; // Xoa co ngay lap tuc de tranh xu ly lap
												
												debug_log("[Hardware] Thuc thi lenh dieu khien tu Web...\r\n");
												
												/* ================================================================== */
												/* DONG BO LENH WEB VAO BO DIEU KHIEN TRUNG TAM                        */
												/* ================================================================== */
												g_system_control.system_mode = g_remote_control.system_mode;
												g_system_control.pump_status = g_remote_control.pump_status;
												g_system_control.roof_status = g_remote_control.roof_status;
												current_roof_pwm = g_remote_control.roof_pwm; // Nhan them PWM tu Web
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
																		if (shared_adc_data.soil_percent < 30) {
																				g_system_control.pump_status = 1; // Dat kho -> Bat bom
																		} else if (shared_adc_data.soil_percent > 80) {
																				g_system_control.pump_status = 0; // Du am -> Tat bom
																		}

																		// C?p nh?t LCD cho bom n?u tr?ng th�i bom thay d?i
																		if (old_pump != g_system_control.pump_status) {
																				g_lcd_update = 1;
																		}

																		// 2. Tu dong dieu khien MAI CHE dua vao cam bien mua & cong tac hanh trinh (?NH �EN CHU?N)
																		if (Rain_Read() == 0) { // �ang MUA -> Mu?n ��NG m�i
																				// Ch? ra l?nh ��NG n?u hi?n t?i m�i chua ? tr?ng th�i ��NG 
																				// V� c�ng t?c h�nh tr�nh ��NG (PB8) chua b? ch?m (m?c 1 l� chua ch?m)
																				if (g_system_control.roof_status != MOTOR_BACKWARD && GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_8) == Bit_SET) {
																						g_system_control.roof_status = MOTOR_BACKWARD;
																						g_lcd_update = 1; // D?ng c? v? LCD 1 l?n duy nh?t khi b?t d?u ch?y
																				}
																		} else { // T?nh mua -> Mu?n M? m�i
																				// Ch? ra l?nh M? n?u hi?n t?i m�i chua ? tr?ng th�i M? 
																				// V� c�ng t?c h�nh tr�nh M? (PB7) chua b? ch?m (m?c 1 l� chua ch?m)
																				if (g_system_control.roof_status != MOTOR_FORWARD && GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_7) == Bit_SET) {
																						g_system_control.roof_status = MOTOR_FORWARD;
																						g_lcd_update = 1; // D?ng c? v? LCD 1 l?n duy nh?t khi b?t d?u ch?y
																				}
																		}
																}

										// ------------------------------------------------------------------
										// C. THEM: LOGIC CHAN DOAN LOI BAO VE BOM (Chi chay khi bom dang bat)
										// ------------------------------------------------------------------
										if (g_system_control.pump_status == 1) {
												
												// Truong hop A: Dong dien vuot nguong an toan (> 2A) -> Bom ket / Qua tai
												if (current_now_mA > 2000) {
														g_pump_diagnostic = 2; // Ma loi: Overload
														g_system_control.pump_status = 0; // Hanh dong: Ngat bom vat ly khan cap
														g_lcd_update = 1;
														debug_log("[ALERT] Overload detected (%u mA)! Hard shutdown pump.\r\n", current_now_mA);
												}

												// Truong hop B: Dong cuc thap hoac khong co nuoc chay qua -> Chay kho / Hut nuoc
												else if (current_now_mA < 100 || flow_now < 0.2f) {
														g_pump_diagnostic = 1; // Ma loi: Dry-run
														g_system_control.pump_status = 0; // Hanh dong: Ngat bom tranh chay may
														g_lcd_update = 1;
														debug_log("[ALERT] Dry-run detected! Hard shutdown pump.\r\n");
												}

												// Truong hop C: Moi thu on dinh
												else {
														g_pump_diagnostic = 0; // Trang thai: Binh thuong
												}

										} else {

												// Neu nguoi dung chu dong tat bom (bang tay hoac qua Web),
												// reset trang thai loi cu de san sang cho lan van hanh tiep theo
												if (g_pump_diagnostic != 1 && g_pump_diagnostic != 2) {
														g_pump_diagnostic = 0;
												}
										}

										// ------------------------------------------------------------------
										// D. XUAT DAU RA PHAN CUNG THAT THEO BIEN TRUNG TAM
										// ------------------------------------------------------------------
                    Motor_Roof_Safety_Supervisor((uint8_t *)&g_system_control.roof_status, &g_lcd_update);
										/* ---------------- A. DIEU KHIEN MAY BOM ---------------- */

										if (g_system_control.pump_status == 1) {
												Pump_On();
												debug_log("-> Phan cung: BAT BOM (Dong: %u mA | Luu luong: %.1f L/m)\r\n",
																	current_now_mA, flow_now);
										} else {
												Pump_Off();
												debug_log("-> Phan cung: TAT BOM\r\n");
										}

										/* ---------------- B. DIEU KHIEN MAI CHE ---------------- */

										Motor1_Dir(g_system_control.roof_status);

										if (g_system_control.roof_status == MOTOR_FORWARD) {
												Motor1_SetSpeed(current_roof_pwm);
												debug_log("-> Phan cung: MO MAI CHE - PWM: %u%%\r\n", current_roof_pwm);

										} else if (g_system_control.roof_status == MOTOR_BACKWARD) {
												Motor1_SetSpeed(current_roof_pwm);
												debug_log("-> Phan cung: DONG MAI CHE - PWM: %u%%\r\n", current_roof_pwm);

										} else {
												Motor1_SetSpeed(0);
												debug_log("-> Phan cung: DUNG MAI CHE\r\n");
										}

										/* ------------- C. DONG BO CHE DO HE THONG ------------- */

										if (g_system_control.system_mode == 1) {
												debug_log("-> He thong dang o che do: TU DONG (Auto)\r\n");
										} else {
												debug_log("-> He thong dang o che do: THU CONG (Manual)\r\n");
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
/////////////////////////////////main-da co logic nhung goi (2 lan cac ham doc cam bien)/////////////////////////////

///* Bi?n to�n c?c luu tr?ng th�i n�t nh?n nh?n t? LoRa Gateway */
//volatile lora_control_payload_t g_remote_control;
//volatile uint8_t g_control_updated = 0; // C? b�o hi?u c� l?nh m?i

///* B? �?M C?M BI?N TO�N C?C: Kh?c ph?c tri?t d? vi?c ngh?n c? chai khi d?c ph?n c?ng */
//volatile lora_sensor_payload_t g_sensor_cache = {0};

///* Bi?n qu?n l� tr?ng th�i th?c t? t?i vu?n */
//menu_control_data_t g_system_control = {
//    .system_mode = 0, // Ban d?u: 0 = MANUAL, 1 = AUTO
//    .pump_status = 0, // 0 = OFF, 1 = ON
//    .roof_status = 0  // 0 = STOP, MOTOR_FORWARD = OPEN, MOTOR_BACKWARD = CLOSE
//};
//uint8_t g_lcd_update = 1;       // C? b�o hi?u c?n v? l?i m�n h�nh LCD
//uint8_t current_roof_pwm = 100; // Luu t?c d? m�i che nh?n t? Web

///* Bi?n qu?n l� ch?n do�n l?i tr?m bom th?c t? */
//volatile uint8_t g_pump_diagnostic = 0; // 0: B�nh thu?ng, 1: Ch?y kh�, 2: Qu� t?i

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

///** Heartbeat every ~5 s on TIM4 */
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
//        debug_log("[Status] alive, listening (node 0x%02X)\r\n", (unsigned)MY_NODE_ID);
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
// * H�M CALLBACK LORA: T?c d? cao, ch? b?c d? li?u t? Cache n�m di
// */
//static uint8_t my_read_sensor(uint8_t *payload, uint8_t max_len)
//{
//    if (max_len < sizeof(lora_sensor_payload_t)) {
//        debug_log("[Sensor] read failed: buffer too small (%u)\r\n", (unsigned)max_len);
//        return 0;
//    }

//    // C?p nh?t tr?ng th�i h? th?ng v�o Cache tru?c khi g?i
//    g_sensor_cache.system_mode = g_system_control.system_mode;
//    g_sensor_cache.pump_status = g_system_control.pump_status;
//    g_sensor_cache.roof_status = g_system_control.roof_status;
//    g_sensor_cache.pump_diagnostic = g_pump_diagnostic;

//    // In log debug ra UART1
//    debug_log("[LoRa TX] Temp=%d|Hum=%u|Soil=%u%%|Water=%u%%|Current=%umA|Rain=%s|Flow=%u (x0.1 L/m)\r\n",
//              (int)g_sensor_cache.temperature_c10/10,
//              (unsigned)g_sensor_cache.humidity_pct10/10,
//              (unsigned)g_sensor_cache.soil_moisture,
//              (unsigned)g_sensor_cache.water_level,
//              (unsigned)g_sensor_cache.current_mA,
//              (g_sensor_cache.rain_status == 0) ? "YES" : "NO",
//              (unsigned)g_sensor_cache.flow_rate_Lmin_x10);

//    // B?c d? li?u t? cache v�o payload
//    memcpy(payload, (void*)&g_sensor_cache, sizeof(lora_sensor_payload_t));
//    return (uint8_t)sizeof(lora_sensor_payload_t);
//}

//// ===== MAIN =====
//int main(void) {
//    LED_Init();

//    UART1_Init(115200);
//    UART_SendString("\r\n===== STM32 LoRa Slave Node (Optimized) =====\r\n");
//    debug_log("[Boot] UART1 115200 OK\r\n");

//    debug_log("[Boot] TIM2 (delay) init...\r\n");
//    TIM2_Init();
//    debug_log("[Boot] TIM4 (heartbeat) init...\r\n");
//    TIM4_HeartbeatInit();

//    debug_log("[Boot] DHT11 init...\r\n");
//    DHT11_Init();

//    debug_log("[Boot] Analog ADC channels init...\r\n");
//    Analog_Init(); 
//    debug_log("[Boot] Calibrating ACS712 Current Sensor...\r\n");
//    Analog_Calibrate();

//    debug_log("[Boot] Rain sensor init...\r\n");
//    Rain_Init();
//    
//    debug_log("[Boot] Motor & Flow sensor init...\r\n");
//    Motor_Init();
//    FlowSensor_Init();
//    
//    debug_log("[Boot] Menu Button & LCD init...\r\n");
//    Menu_Init(); 

//    Delay_Ms(100);

//    debug_log("[Boot] LoRa radio init...\r\n");
//    if (!lora_radio_begin()) {
//        UART_SendString("[Boot] ERROR: lora_radio_begin failed\r\n");
//        while (1) { }
//    }

//    debug_log("[Boot] Radio OK  node=0x%02X  gateway=0x%02X\r\n", (unsigned)MY_NODE_ID, (unsigned)GATEWAY_ID);
//    debug_log("[Boot] Entering main loop\r\n");

//    const lora_node_config_t cfg = {
//        .node_id = MY_NODE_ID,
//        .gateway_id = GATEWAY_ID,
//        .log = debug_log,
//    };

//    const lora_node_radio_t radio = {
//        .send = lora_radio_send,
//        .rx_pending = lora_radio_rx_pending,
//        .receive = lora_radio_receive,
//        .read_sensor = my_read_sensor,
//    };

//    lora_node_t node;
//    lora_node_init(&node, &cfg, &radio);

//    // Bi?n d?m ph?c v? d?nh th?i l?y m?u c?m bi?n
//    static uint8_t flow_timer_count = 0;
//    static uint8_t dht11_timer_count = 0;

//    while (1) {
//        /* ================================================================== */
//        /* 1. C?P NH?T D? LI?U C?M BI?N V�O CACHE TO�N C?C                    */
//        /* ================================================================== */
//        
//        // �?c ADC (Nhanh)
//        Analog_Data_t adc_data;
//        Analog_UpdateAll(&adc_data);
//        g_sensor_cache.soil_moisture = (uint16_t)adc_data.soil_percent;
//        g_sensor_cache.water_level   = (uint16_t)adc_data.water_percent;
//        g_sensor_cache.current_mA    = (uint16_t)(adc_data.current_ampe * 1000.0f);
//        
//        // �?c c?m bi?n mua (Nhanh)
//        g_sensor_cache.rain_status = Rain_Read();

//        // T�nh luu lu?ng (Chu k? ~1 gi�y = 50 v�ng l?p x 20ms)
//        flow_timer_count++;
//        if (flow_timer_count >= 50) {
//            float flow = Flow_GetLitersPerMinute(1000); 
//            g_sensor_cache.flow_rate_Lmin_x10 = (uint8_t)(flow * 10.0f);
//            flow_timer_count = 0;
//        }

//        // �?c DHT11 (Ch?m, Chu k? ~2 gi�y = 100 v�ng l?p x 20ms)
//        dht11_timer_count++;
//        if (dht11_timer_count >= 100) {
//            uint8_t temp = 0, humi = 0;
//            if (DHT11_ReadData(&temp, &humi) == DHT11_OK) {
//                g_sensor_cache.temperature_c10 = (int16_t)temp * 10;
//                g_sensor_cache.humidity_pct10 = (uint16_t)humi * 10;
//            }
//            dht11_timer_count = 0;
//        }

//        /* ================================================================== */
//        /* 2. KI?M TRA L?NH T? M?NG LORA & N�T NH?N T?I VU?N                  */
//        /* ================================================================== */
//        
//        // L?ng nghe s�ng LoRa
//        lora_node_poll(&node);
//        
//        // Qu�t n�t b?m t?i ch?
//        Menu_Button_Scan(&g_system_control, &g_lcd_update);

//        // Kiem tra l?nh Web do xu?ng
//        if (g_control_updated) {
//            g_control_updated = 0; 
//            debug_log("[Hardware] Thuc thi lenh dieu khien tu Web...\r\n");
//            
//            g_system_control.system_mode = g_remote_control.system_mode;
//            g_system_control.pump_status = g_remote_control.pump_status;
//            g_system_control.roof_status = g_remote_control.roof_status;
//            current_roof_pwm = g_remote_control.roof_pwm;
//            g_lcd_update = 1; 

//            LED_Blink();
//        }

//        /* ================================================================== */
//        /* 3. LOGIC T? �?NG & B?O V? BOM (D�ng d? li?u t? Cache)              */
//        /* ================================================================== */
//        
//        if (g_system_control.system_mode == 1) { // Ch? d? AUTO
//            uint8_t old_pump = g_system_control.pump_status;
//            uint8_t old_roof = g_system_control.roof_status;

//            if (g_sensor_cache.soil_moisture < 30) {
//                g_system_control.pump_status = 1; 
//            } else if (g_sensor_cache.soil_moisture > 80) {
//                g_system_control.pump_status = 0; 
//            }

//            if (g_sensor_cache.rain_status == 0) {
//                g_system_control.roof_status = MOTOR_BACKWARD; 
//            } else {
//                g_system_control.roof_status = MOTOR_FORWARD; 
//            }

//            if(old_pump != g_system_control.pump_status || old_roof != g_system_control.roof_status) {
//                g_lcd_update = 1;
//            }
//        }

//        // Ch?n do�n l?i b?o v? bom
//        if (g_system_control.pump_status == 1) {
//            if (g_sensor_cache.current_mA > 2000) {
//                g_pump_diagnostic = 2; // Overload
//                g_system_control.pump_status = 0; 
//                g_lcd_update = 1;
//                debug_log("[ALERT] Overload detected! Hard shutdown pump.\r\n");
//            }
//            // �i?u ki?n luu lu?ng < 0.2 L/min quy d?i ra x10 l� < 2
//            else if (g_sensor_cache.current_mA < 100 || g_sensor_cache.flow_rate_Lmin_x10 < 2) {
//                g_pump_diagnostic = 1; // Dry-run
//                g_system_control.pump_status = 0; 
//                g_lcd_update = 1;
//                debug_log("[ALERT] Dry-run detected! Hard shutdown pump.\r\n");
//            }
//            else {
//                g_pump_diagnostic = 0; 
//            }
//        } else {
//            if (g_pump_diagnostic != 1 && g_pump_diagnostic != 2) {
//                g_pump_diagnostic = 0;
//            }
//        }

//        /* ================================================================== */
//        /* 4. XU?T T�N HI?U RA PH?N C?NG TH?T & C?P NH?T M�N H�NH             */
//        /* ================================================================== */

//        // A. �i?u khi?n bom
//        if (g_system_control.pump_status == 1) {
//            Pump_On();
//        } else {
//            Pump_Off();
//        }

//        // B. �i?u khi?n m�i che
//        Motor1_Dir(g_system_control.roof_status);
//        if (g_system_control.roof_status == MOTOR_FORWARD || g_system_control.roof_status == MOTOR_BACKWARD) {
//            Motor1_SetSpeed(current_roof_pwm);
//        } else {
//            Motor1_SetSpeed(0);
//        }

//        // C. C?p nh?t giao di?n m�n h�nh LCD
//        if (g_lcd_update) {
//            Menu_Display_Update(&g_system_control);
//            g_lcd_update = 0;
//        }

//        // Nh?p d?p h? th?ng v� ch?ng d?i (Debounce)
//        debug_heartbeat();
//        Delay_Ms(20);
//    }
//}



