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
 
/* TEST ONLY: dat 1 = tat bao ve bom (overload/dry-run/can bon). Demo that: de 0 */
#define PUMP_PROTECT_DISABLE  0
 
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
 
 /* In UART: boot OK + [ERR] khi su co (sensor/LoRa/bao ve bom). Chuoi ngan de fit 32KB. */
 static void debug_log(const char *fmt, ...)
 {
     char buf[128];
     va_list ap;
 
     va_start(ap, fmt);
     vsnprintf(buf, sizeof(buf), fmt, ap);
     va_end(ap);
     UART_SendString(buf);
 }
 
 /* Tranh spam UART: chi in lai [ERR] sau min_ms */
 static void debug_err_throttle(uint32_t *last_ms, uint32_t min_ms, const char *msg)
 {
     uint32_t now = millis();
     if (*last_ms != 0u && (now - *last_ms) < min_ms) {
         return;
     }
     *last_ms = now;
     debug_log("%s", msg);
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
        debug_log("[Status] alive, listening( node  0x%02X)\r\n",
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
         debug_log("[Sensor] read failed: buffer too small (%u)\r\n",
           (unsigned)max_len);
         return 0;
     }
 
     memset(&sample, 0, sizeof(sample));
     
     // 1. Doc DHT11 — loi: day long / mat ket noi / checksum
     {
         uint8_t dht_st = DHT11_ReadData(&temp, &humi);
         static uint32_t s_dht_err_ms;
         if (dht_st == DHT11_OK) {
             sample.temperature_c10 = (int16_t)temp * 10;
             sample.humidity_pct10 = (uint16_t)humi * 10;
         } else if (dht_st == DHT11_ERR_NO_RESP) {
             debug_err_throttle(&s_dht_err_ms, 3000u,
                                "[ERR] DHT no resp (loose/power)\r\n");
         } else {
             debug_err_throttle(&s_dht_err_ms, 3000u,
                                "[ERR] DHT checksum\r\n");
         }
     }
 
     // 2. Dung ADC da doc san (khong doc lai)
     sample.soil_moisture = (uint16_t)g_current_adc_data.soil_percent;
     sample.water_level   = (uint16_t)g_current_adc_data.water_percent;
     sample.current_mA    = (uint16_t)(g_current_adc_data.current_ampe * 1000.0f);
     
     // 3. Cam bien mua (Digital)
     sample.rain_status = Rain_Read();
 
     // 4. Flow TIM3_CH1 — cache ~500ms
     uint16_t flow_x10 = Flow_GetLpmX10();
     sample.flow_rate_Lmin_x10 = (uint8_t)((flow_x10 > 255u) ? 255u : flow_x10);
 
     // 5. Trang thai vuon len Web
     sample.system_mode = g_system_control.system_mode;
     sample.pump_status = g_system_control.pump_status;
     sample.roof_status = g_system_control.roof_status;
     sample.pump_diagnostic = (uint8_t)g_pump_diagnostic;
 
     /* Snapshot cam bien: chi in ~5s/lan (HB) de UART uu tien [ERR] */
     {
         static uint32_t s_sensor_ok_ms;
         uint32_t now = millis();
         if (s_sensor_ok_ms == 0u || (now - s_sensor_ok_ms) >= 5000u) {
             s_sensor_ok_ms = now;
             debug_log("[S] T=%d H=%u S=%u W=%u I=%u R=%s F=%u.%u\r\n",
                       (int)sample.temperature_c10 / 10,
                       (unsigned)sample.humidity_pct10 / 10,
                       (unsigned)sample.soil_moisture,
                       (unsigned)sample.water_level,
                       (unsigned)sample.current_mA,
                       (sample.rain_status == 0) ? "YES" : "NO",
                       (unsigned)(sample.flow_rate_Lmin_x10 / 10),
                       (unsigned)(sample.flow_rate_Lmin_x10 % 10));
         }
     }
 
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
 
                                         /* Cam bien loi kieu long/tuot day / ADC short-open / khong xung */
                                         {
                                                 static uint32_t s_soil_err_ms;
                                                 static uint32_t s_water_err_ms;
                                                 static uint32_t s_acs_err_ms;
                                                 static uint32_t s_flow_err_ms;
                                                 static uint8_t  s_flow_pump_was_on;
                                                 static uint32_t s_flow_pump_on_ms;
                                                 /* Rain DO + IPU: day tin hieu long => luon "kho" (giong khong mua)
                                                  * — khong the in [ERR] open ma khong bao nham. */
 
                                                 if (shared_adc_data.raw_soil <= 5u
                                                         || shared_adc_data.raw_soil >= 4090u) {
                                                         debug_err_throttle(&s_soil_err_ms, 5000u,
                                                                 "[ERR] Soil ADC open/short?\r\n");
                                                 }
                                                 if (shared_adc_data.raw_water <= 5u
                                                         || shared_adc_data.raw_water >= 4090u) {
                                                         debug_err_throttle(&s_water_err_ms, 5000u,
                                                                 "[ERR] Water ADC open/short?\r\n");
                                                 }
                                                 /* ACS712: OUT long/short -> raw ~0 hoac ~4095 */
                                                 if (shared_adc_data.raw_current <= 5u
                                                         || shared_adc_data.raw_current >= 4090u) {
                                                         debug_err_throttle(&s_acs_err_ms, 5000u,
                                                                 "[ERR] ACS712 ADC open/short?\r\n");
                                                 } else if (g_system_control.pump_status == 0
                                                         && shared_adc_data.current_ampe > 1.0f) {
                                                         /* Bom tat ma dong van cao: day / cam bien / zero lech */
                                                         debug_err_throttle(&s_acs_err_ms, 5000u,
                                                                 "[ERR] ACS712 idle high (loose/fault?)\r\n");
                                                 }
                                                 /* Flow: bom chay qua grace ma khong xung — day / sensor / ong kho */
                                                 if (g_system_control.pump_status == 1) {
                                                         if (!s_flow_pump_was_on) {
                                                                 s_flow_pump_on_ms = millis();
                                                                 s_flow_pump_was_on = 1u;
                                                         }
                                                         if ((millis() - s_flow_pump_on_ms) >= PUMP_PROTECT_GRACE_MS
                                                                 && flow_now_x10 < 2u) {
                                                                 debug_err_throttle(&s_flow_err_ms, 5000u,
                                                                         "[ERR] Flow no pulse (loose/sensor?)\r\n");
                                                         }
                                                 } else {
                                                         s_flow_pump_was_on = 0u;
                                                 }
                                         }
 
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
                                         /* Ma chuan doan bom van gui len Web (SENSOR_DATA) — khong in UART */
                                         // Gui ACK mang TRANG THAI THAT (sau khi AUTO + an toan da xu ly xong)
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
                                         } else if (g_system_control.roof_status == MOTOR_BACKWARD) {
                                                 Motor1_SetSpeed(current_roof_pwm);
                                         } else {
                                                 Motor1_SetSpeed(0);
                                         }

                                         /* In UART ngay khi trang thai bom/mai doi (nut / Web / Auto) */
                                         {
                                                 static uint8_t s_prev_pump = 0xFFu;
                                                 static uint8_t s_prev_roof = 0xFFu;
                                                 static uint8_t s_prev_ppwm = 0xFFu;
                                                 static uint8_t s_prev_rpwm = 0xFFu;
                                                 uint8_t pump = g_system_control.pump_status;
                                                 uint8_t roof = g_system_control.roof_status;
                                                 uint8_t ppwm = current_pump_pwm;
                                                 uint8_t rpwm = current_roof_pwm;

                                                 if (s_prev_pump == 0xFFu) {
                                                         s_prev_pump = pump;
                                                         s_prev_roof = roof;
                                                         s_prev_ppwm = ppwm;
                                                         s_prev_rpwm = rpwm;
                                                 } else {
                                                         if (pump != s_prev_pump
                                                                         || (pump != 0u && ppwm != s_prev_ppwm)) {
                                                                 debug_log("[M] Pump %s pwm=%u%%\r\n",
                                                                           pump ? "ON" : "OFF",
                                                                           (unsigned)(pump ? ppwm : 0u));
                                                                 s_prev_pump = pump;
                                                                 s_prev_ppwm = ppwm;
                                                         }
                                                         if (roof != s_prev_roof
                                                                         || ((roof == MOTOR_FORWARD
                                                                                         || roof == MOTOR_BACKWARD)
                                                                                         && rpwm != s_prev_rpwm)) {
                                                                 const char *rs =
                                                                         (roof == MOTOR_FORWARD) ? "OPEN" :
                                                                         (roof == MOTOR_BACKWARD) ? "CLOSE" : "STOP";
                                                                 debug_log("[M] Roof %s pwm=%u%%\r\n",
                                                                           rs,
                                                                           (unsigned)((roof == MOTOR_STOP) ? 0u : rpwm));
                                                                 s_prev_roof = roof;
                                                                 s_prev_rpwm = rpwm;
                                                         }
                                                 }
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
 