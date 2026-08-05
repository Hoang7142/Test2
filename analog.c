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
		{
				uint32_t timeout = 100000;
				while(ADC_GetResetCalibrationStatus(ADC1)) {
						if (--timeout == 0) break; // Thoat neu ADC khong phan hoi, tranh treo may khi Boot
				}
		}
		ADC_StartCalibration(ADC1);
		{
				uint32_t timeout = 100000;
				while(ADC_GetCalibrationStatus(ADC1)) {
						if (--timeout == 0) break;
				}
		}
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
    ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_239Cycles5);
    uint32_t timeout;

    // Doc lan 1 bo qua (xoa du lieu kenh truoc do)
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    timeout = 10000;
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET) {
        if (--timeout == 0) return 0; // ADC treo -> tra ve 0, khong cho MCU dung hinh
    }
    
    // Doc lan 2 lay ket qua
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    timeout = 10000;
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET) {
        if (--timeout == 0) return 0;
    }

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
    int16_t s_res = (4084 - (int16_t)data->raw_soil) * 100 / (4084 - 1250);
    if (s_res > 100) data->soil_percent = 100;
    else if (s_res < 0) data->soil_percent = 0;
    else data->soil_percent = s_res;

    // Cap nhat Nuoc — map 2 diem (cam bien PCB khong dat ADC=4095 khi full)
    // Calib: kho ~0% cu (~ADC 0), nhung full ~55% cu (~ADC 2470) -> 100%
    #define WATER_ADC_DRY  0
    #define WATER_ADC_WET  2470  /* 55% * 4095 / 100 */
    data->raw_water = ADC_Read_Filter(ADC_Channel_2);
    {
        int16_t w_res;
        if (data->raw_water <= WATER_ADC_DRY) {
            w_res = 0;
        } else if (data->raw_water >= WATER_ADC_WET) {
            w_res = 100;
        } else {
            w_res = (int16_t)((data->raw_water - WATER_ADC_DRY) * 100
                              / (WATER_ADC_WET - WATER_ADC_DRY));
        }
        data->water_percent = (uint8_t)w_res;
    }

    // Cap nhat Dong dien (Dung global_zero_point da calib)
    data->raw_current = ADC_Read_Filter(ADC_Channel_8);
    {
        float v_pin = (float)data->raw_current * 3.3f / 4095.0f;
        float curr = (v_pin - global_zero_point) / 0.185f;

        if (curr < 0.03f && curr > -0.03f) curr = 0;
        if (curr < 0) data->current_ampe = -curr;
        else data->current_ampe = curr;
    }
}


