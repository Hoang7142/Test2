//#include "flow_sensor.h"

//volatile uint32_t pulse_count = 0;

//void FlowSensor_Init(void) {
//    GPIO_InitTypeDef gpio;
//    EXTI_InitTypeDef exti;
//    NVIC_InitTypeDef nvic;

//    // B?t clock AFIO l� b?t bu?c d? d�ng Remap v� EXTI
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
//    
//    // GI?I C?U CH�N PB3: T?t t�nh nang JTAG d? d�ng PB3 l�m ch�n Input thu?ng
//    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

//    gpio.GPIO_Pin = GPIO_Pin_3;
//    gpio.GPIO_Mode = GPIO_Mode_IPU; // D�ng Pull-up n?i b?
//    GPIO_Init(GPIOB, &gpio);

//    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource3);

//    exti.EXTI_Line = EXTI_Line3;
//    exti.EXTI_Mode = EXTI_Mode_Interrupt;
//    exti.EXTI_Trigger = EXTI_Trigger_Falling; // Ng?t khi c� xung c?nh xu?ng
//    exti.EXTI_LineCmd = ENABLE;
//    EXTI_Init(&exti);

//    nvic.NVIC_IRQChannel = EXTI3_IRQn;
//    nvic.NVIC_IRQChannelPreemptionPriority = 0;
//    nvic.NVIC_IRQChannelSubPriority = 1;
//    nvic.NVIC_IRQChannelCmd = ENABLE;
//    NVIC_Init(&nvic);
//}

//void EXTI3_IRQHandler(void) {
//    if (EXTI_GetITStatus(EXTI_Line3) != RESET) {
//        pulse_count++; // Tang bi?n d?m xung
//        EXTI_ClearITPendingBit(EXTI_Line3);
//    }
//}

//float Flow_GetLitersPerMinute(void) {
//    float flow_rate;
//    
//    // T?m th?i t?t ng?t d? d?c bi?n pulse_count an to�n (tr�nh tranh ch?p)
//    __disable_irq();
//    uint32_t temp_pulses = pulse_count;
//    pulse_count = 0; 
//    __enable_irq();
//    
//    // C�ng th?c YF-S201: Q = F / 7.5
//    flow_rate = (float)temp_pulses / 7.5; 
//    
//    return flow_rate;
//}

/////////////////////////////////////////////////////

#include "flow_sensor.h"

volatile uint32_t pulse_count = 0;

void FlowSensor_Init(void) {
    GPIO_InitTypeDef gpio;
    EXTI_InitTypeDef exti;
    NVIC_InitTypeDef nvic;

    // 1. B?t clock AFIO v� GPIOB
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    
    // 2. GI?I C?U CH�N PB3: T?t JTAG d? d�ng l�m ch�n GPIO thu?ng
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    // 3. C?u h�nh ch�n PB3 l� Input Pull-up
    gpio.GPIO_Pin = GPIO_Pin_3;
    gpio.GPIO_Mode = GPIO_Mode_IPU; 
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpio);

    // 4. K?t n?i du?ng ng?t EXTI3 v?i ch�n PB3
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource3);

    // 5. C?u h�nh EXTI Line 3
    exti.EXTI_Line = EXTI_Line3;
    exti.EXTI_Mode = EXTI_Mode_Interrupt;
    exti.EXTI_Trigger = EXTI_Trigger_Falling; 
    exti.EXTI_LineCmd = ENABLE;
    EXTI_Init(&exti);

    // 6. C?u h�nh uu ti�n ng?t
    nvic.NVIC_IRQChannel = EXTI3_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 1; // Uu ti�n th?p hon c�c t�c v? h? th?ng quan tr?ng
    nvic.NVIC_IRQChannelSubPriority = 0;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);
}

void EXTI3_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line3) != RESET) {
        pulse_count++; 
        EXTI_ClearITPendingBit(EXTI_Line3);
    }
}

/**
 * sampling_time_ms: Kho?ng th?i gian gi?a 2 l?n g?i h�m (v� d?: 1000ms)
 */
float Flow_GetLitersPerMinute(uint32_t sampling_time_ms) {
    if (sampling_time_ms == 0) return 0;

    __disable_irq();
    uint32_t temp_pulses = pulse_count;
    pulse_count = 0; 
    __enable_irq();
    
    // T?n s? Hz = S? xung / Th?i gian (gi�y)
    float frequency = (float)temp_pulses / (sampling_time_ms / 1000.0f);
    
    // C�ng th?c YF-S201: Q = F / 7.5 (L/min)
    return frequency / 7.5f; 
}
