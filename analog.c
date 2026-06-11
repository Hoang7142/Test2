#include "analog.h"

static float global_zero_point = 1.13f; // Bien luu diem 0 cho ACS712

void Analog_Init(void) {
    GPIO_InitTypeDef gpio;
    ADC_InitTypeDef adc;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);

    gpio.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2;
    gpio.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &gpio);

    gpio.GPIO_Pin = GPIO_Pin_0; // PB0 cho ACS712
    GPIO_Init(GPIOB, &gpio);

    adc.ADC_Mode = ADC_Mode_Independent;
    adc.ADC_ScanConvMode = DISABLE;
    adc.ADC_ContinuousConvMode = DISABLE;
    adc.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    adc.ADC_DataAlign = ADC_DataAlign_Right;
    adc.ADC_NbrOfChannel = 1;

    ADC_Init(ADC1, &adc);
    ADC_Cmd(ADC1, ENABLE);

    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
}

// Ham nay goi khi bat nguon de lay diem zero thuc te
void Analog_Calibrate(void) {
    uint32_t sum = 0;
    for(int i = 0; i < 50; i++) {
        sum += ADC_Read_Filter(ADC_Channel_8);
    }
    global_zero_point = (float)(sum / 50) * 3.3f / 4095.0f;
}

uint16_t ADC_Read_Raw(uint8_t channel) {
    // Tang thoi gian lay mau len muc cao nhat 239.5 cycles de on dinh
    ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_239Cycles5);
    
    // Doc lan 1 bo qua (xoa du lieu kenh truoc do)
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
    
    // Doc lan 2 lay ket qua
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);

    return ADC_GetConversionValue(ADC1);
}

// Giu nguyen logic loc Median cua ban
uint16_t ADC_Read_Median(uint8_t channel) {
    uint16_t samples[7];
    uint16_t temp;
    int i, j;

    for(i = 0; i < 7; i++) {
        samples[i] = ADC_Read_Raw(channel);
    }

    for(i = 0; i < 6; i++) {
        for(j = i + 1; j < 7; j++) {
            if(samples[i] > samples[j]) {
                temp = samples[i];
                samples[i] = samples[j];
                samples[j] = temp;
            }
        }
    }
    return samples[3];
}

uint16_t ADC_Read_Filter(uint8_t channel) {
    uint32_t sum = 0;
    for(uint8_t i = 0; i < 10; i++) {
        sum += ADC_Read_Median(channel);
    }
    return sum / 10;
}

void Analog_UpdateAll(Analog_Data_t *data) {
    // Cap nhat Soil
    data->raw_soil = ADC_Read_Filter(ADC_Channel_0);
    int16_t s_res = (4000 - (int16_t)data->raw_soil) * 100 / (4000 - 1500);
    if (s_res > 100) data->soil_percent = 100;
    else if (s_res < 0) data->soil_percent = 0;
    else data->soil_percent = s_res;

    // Cap nhat Nuoc
    data->raw_water = ADC_Read_Filter(ADC_Channel_2);
    data->water_percent = (uint8_t)((uint32_t)data->raw_water * 100 / 4095);

    // Cap nhat Dong dien (Dung global_zero_point da calib)
    uint16_t raw_i = ADC_Read_Filter(ADC_Channel_8);
    float v_pin = (float)raw_i * 3.3f / 4095.0f;
    float curr = (v_pin - global_zero_point) / 0.185f;

    if (curr < 0.03f && curr > -0.03f) curr = 0;
    if (curr < 0) data->current_ampe = -curr;
    else data->current_ampe = curr;
}


