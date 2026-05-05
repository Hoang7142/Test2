#include "water_level.h"
#include <stdio.h>

// khoi tao ADC cho PA7
void WaterLevel_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE);

    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin = GPIO_Pin_7;       // PA7
    gpio.GPIO_Mode = GPIO_Mode_AIN;   // analog input
    GPIO_Init(GPIOA, &gpio);

    ADC_InitTypeDef adc;
    adc.ADC_Mode = ADC_Mode_Independent;
    adc.ADC_ScanConvMode = DISABLE;
    adc.ADC_ContinuousConvMode = DISABLE;
    adc.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    adc.ADC_DataAlign = ADC_DataAlign_Right;
    adc.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &adc);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 1, ADC_SampleTime_55Cycles5);

    ADC_Cmd(ADC1, ENABLE);

    // hieu chuan ADC
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));

    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
}

// doc gia tri ADC
uint16_t WaterLevel_ReadRaw(void)
{
    ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 1, ADC_SampleTime_55Cycles5);

    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));

    return ADC_GetConversionValue(ADC1);
}

// chuyen sang phan tram
uint8_t WaterLevel_GetPercent(void)
{
    uint16_t adc = WaterLevel_ReadRaw();

    // map 0-4095 -> 0-100%
    uint8_t percent = (adc * 100) / 4095;

    return percent;
}

// tra chuoi hien thi
char* WaterLevel_String(void)
{
    static char buf[20];

    uint8_t percent = WaterLevel_GetPercent();

    sprintf(buf, "WATER: %d%%", percent);

    return buf;
}