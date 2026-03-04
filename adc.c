#include "adc.h"
#include <stdio.h>
// C?u hình ngu?ng hi?u chu?n th?c t? (b?n có th? do l?i d? di?u ch?nh cho chính xác)
#define ADC_KHO 4000  // Giá tr? ADC khi c?m bi?n ? ngoài không khí
#define ADC_AM  2000  // Giá tr? ADC khi c?m bi?n nhúng vào nu?c

// Kh?i t?o ADC dùng chân PA0 (ADC_Channel_0)
void ADC_InitConfig(void) {
    // B?t clock cho GPIOA và ADC1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE);

    // C?u hình PA0 là analog input
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // C?u hình ADC1
    ADC_InitTypeDef ADC_InitStruct;
    ADC_InitStruct.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStruct.ADC_ScanConvMode = DISABLE;
    ADC_InitStruct.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStruct.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStruct);

    // C?u hình kênh ADC_Channel_0 (PA0), th?i gian m?u 55.5 cycle
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);

    // B?t ADC
    ADC_Cmd(ADC1, ENABLE);

    // Hi?u chu?n ADC
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1));
}

//// Ð?c giá tr? ADC
//static uint16_t ADC_Read(void) {
//    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
//    while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
//    return ADC_GetConversionValue(ADC1);
//}

//// Tr? l?i chu?i d? ?m d?t d?ng "Do am dat: XX%"
//char* GetDoAmDatString(void) {
//    static char buf[20];
//    uint16_t adc_val = ADC_Read();
//    uint8_t do_am = (uint8_t)((4095 - adc_val) * 100 / 4095);
//    sprintf(buf, "Do am dat: %d%%", do_am);
//    return buf;
//}

//uint8_t GetDoAmDatValue(void) {
//    uint16_t adc_val = ADC_Read();
//    uint8_t do_am = (uint8_t)((4095 - adc_val) * 100 / 4095);
//    return do_am;
//}

// Ð?c giá tr? ADC
static uint16_t ADC_Read(void) {
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
    return ADC_GetConversionValue(ADC1);
}

// Tr? l?i chu?i d? ?m d?t d?ng "Do am dat: XX%"
char* GetDoAmDatString(void) {
    static char buf[20];
    uint16_t adc_val = ADC_Read();

    int16_t do_am = (ADC_KHO - adc_val) * 100 / (ADC_KHO - ADC_AM);
    if (do_am > 100) do_am = 100;
    if (do_am < 0)   do_am = 0;

    sprintf(buf, "Do am dat: %d%%", do_am);
    return buf;
}

// Tr? l?i giá tr? d? ?m d?t du?i d?ng s? %
uint8_t GetDoAmDatValue(void) {
    uint16_t adc_val = ADC_Read();

    int16_t do_am = (ADC_KHO - adc_val) * 100 / (ADC_KHO - ADC_AM);
    if (do_am > 100) do_am = 100;
    if (do_am < 0)   do_am = 0;

    return (uint8_t)do_am;
}