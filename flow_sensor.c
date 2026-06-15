#include "flow_sensor.h"

volatile uint32_t pulse_count = 0;

void FlowSensor_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_ICInitTypeDef TIM_ICInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* 1. Cấp xung nhịp cho GPIOB và AFIO */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    // Lưu ý: TIM3 clock đã được bật bên Motor_Init, nhưng nếu muốn chắc chắn bạn có thể bật lại:
    // RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    /* 2. Giải cứu chân PB4 & Cấu hình Remap TIM3 */
    // Tắt JTAG (giữ lại SWD để nạp code) để giải phóng PB4 (NJTRST)
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    
    // Remap 1 phần TIM3: Đưa TIM3_CH1 sang PB4, giữ nguyên TIM3_CH4 ở PB1 (cho Motor)
    GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE);

    /* 3. Cấu hình chân PB4 làm Input Pull-up */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* 4. Cấu hình Input Capture cho TIM3 - Kênh 1 (PB4) */
    // Không thay đổi TimeBase (Prescaler, Period) vì Motor đang dùng
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling; // Bắt cạnh xuống
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;        // Không chia tần số capture
    TIM_ICInitStructure.TIM_ICFilter = 0x0F;                     // LỌC NHIỄU TỐI ĐA (Cực kỳ quan trọng để thay thế EXTI)
    TIM_ICInit(TIM3, &TIM_ICInitStructure);

    /* 5. Cấu hình Ngắt cho TIM3 */
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; // Độ ưu tiên
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* 6. Cho phép ngắt Capture/Compare Kênh 1 của TIM3 */
    TIM_ITConfig(TIM3, TIM_IT_CC1, ENABLE);
    
    // Lưu ý: TIM_Cmd(TIM3, ENABLE); đã được gọi bên Motor_Init nên không cần gọi lại.
}

/* Hàm phục vụ ngắt của TIM3 (Dùng chung nếu sau này Motor cần) */
void TIM3_IRQHandler(void) {
    // Kiểm tra xem ngắt có phải do Input Capture kênh 1 (PB4) sinh ra không
    if (TIM_GetITStatus(TIM3, TIM_IT_CC1) != RESET) {
        pulse_count++; // Tăng biến đếm xung
        
        // Xóa cờ ngắt
        TIM_ClearITPendingBit(TIM3, TIM_IT_CC1);
    }
}

/**
 * sampling_time_ms: Khoảng thời gian giữa 2 lần gọi hàm (ví dụ: 1000ms)
 */
float Flow_GetLitersPerMinute(uint32_t sampling_time_ms) {
    if (sampling_time_ms == 0) return 0;

    __disable_irq();
    uint32_t temp_pulses = pulse_count;
    pulse_count = 0; 
    __enable_irq();
    
    // Tần số Hz = Số xung / Thời gian (giây)
    float frequency = (float)temp_pulses / (sampling_time_ms / 1000.0f);
    
    // Công thức YF-S201: Q = F / 7.5 (L/min)
    return frequency / 7.5f; 
}

/////////////////////////////////////////////////////

//#include "flow_sensor.h"

//volatile uint32_t pulse_count = 0;

//void FlowSensor_Init(void) {
//    GPIO_InitTypeDef gpio;
//    EXTI_InitTypeDef exti;
//    NVIC_InitTypeDef nvic;

//    // 1. B?t clock AFIO v� GPIOB
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
//    
//    // 2. GI?I C?U CH�N PB3: T?t JTAG d? d�ng l�m ch�n GPIO thu?ng
//    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

//    // 3. C?u h�nh ch�n PB3 l� Input Pull-up
//    gpio.GPIO_Pin = GPIO_Pin_3;
//    gpio.GPIO_Mode = GPIO_Mode_IPU; 
//    gpio.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_Init(GPIOB, &gpio);

//    // 4. K?t n?i du?ng ng?t EXTI3 v?i ch�n PB3
//    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource3);

//    // 5. C?u h�nh EXTI Line 3
//    exti.EXTI_Line = EXTI_Line3;
//    exti.EXTI_Mode = EXTI_Mode_Interrupt;
//    exti.EXTI_Trigger = EXTI_Trigger_Falling; 
//    exti.EXTI_LineCmd = ENABLE;
//    EXTI_Init(&exti);

//    // 6. C?u h�nh uu ti�n ng?t
//    nvic.NVIC_IRQChannel = EXTI3_IRQn;
//    nvic.NVIC_IRQChannelPreemptionPriority = 1; // Uu ti�n th?p hon c�c t�c v? h? th?ng quan tr?ng
//    nvic.NVIC_IRQChannelSubPriority = 0;
//    nvic.NVIC_IRQChannelCmd = ENABLE;
//    NVIC_Init(&nvic);
//}

//void EXTI3_IRQHandler(void) {
//    if (EXTI_GetITStatus(EXTI_Line3) != RESET) {
//        pulse_count++; 
//        EXTI_ClearITPendingBit(EXTI_Line3);
//    }
//}

///**
// * sampling_time_ms: Kho?ng th?i gian gi?a 2 l?n g?i h�m (v� d?: 1000ms)
// */
//float Flow_GetLitersPerMinute(uint32_t sampling_time_ms) {
//    if (sampling_time_ms == 0) return 0;

//    __disable_irq();
//    uint32_t temp_pulses = pulse_count;
//    pulse_count = 0; 
//    __enable_irq();
//    
//    // T?n s? Hz = S? xung / Th?i gian (gi�y)
//    float frequency = (float)temp_pulses / (sampling_time_ms / 1000.0f);
//    
//    // C�ng th?c YF-S201: Q = F / 7.5 (L/min)
//    return frequency / 7.5f; 
//}
