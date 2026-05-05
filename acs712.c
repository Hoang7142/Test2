#include "acs712.h"
#include <stdio.h>

// Ham khoi tao ADC cho cam bien dong
void ACS712_Init(void) {
    GPIO_InitTypeDef gpio;
    ADC_InitTypeDef adc;

    // Bat clock cho Port B va ADC1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_ADC1, ENABLE);
    
    // Cau hinh chan PB0 la Analog Input
    gpio.GPIO_Pin = GPIO_Pin_0; 
    gpio.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOB, &gpio);

    // Cau hinh ADC1
    adc.ADC_Mode = ADC_Mode_Independent;
    adc.ADC_ScanConvMode = DISABLE;
    adc.ADC_ContinuousConvMode = DISABLE;
    adc.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    adc.ADC_DataAlign = ADC_DataAlign_Right;
    adc.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &adc);
    
    ADC_Cmd(ADC1, ENABLE);

    // Hieu chuan ADC de doc chinh xac hon
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
}

// Ham doc gia tri dong dien hien tai
float ACS712_GetCurrent(void) {
    uint16_t adc_val;
    float v_pin;      // Dien ap tai chan chip PB0
    float v_sensor;   // Dien ap thuc te tai dau ra cam bien
    float current;
    
    // DIEM ZERO THUC TE: Dien ap cam bien khi dong Ip = 0
    // Theo ly thuyet la 2.5V. Neu bi am thi giam so nay, duong thi tang so nay.
    float zero_point = 2.4;  

    // Doc ADC tu kenh 8 (tuong ung chan PB0)
    ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_55Cycles5);
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
    
    adc_val = ADC_GetConversionValue(ADC1);
    
    // BUOC 1: Chuyen gia tri so sang dien ap tai chan PB0 (0V - 3.3V)
    v_pin = (float)adc_val * 3.3 / 4095.0;
    
    // BUOC 2: NHAN NGUOC HE SO CHIA AP (Gia su dung 2 tro 10k chia doi nen nhan 2)
    // Neu ban dung ty le khac, hay thay so 2.0 bang ty le tuong ung
    v_sensor = v_pin * 2.0; 
    
    // BUOC 3: Tinh dong dien dua tren do nhay 0.185V/A (Loai 5A)
    // Cong thuc: I = (V_thuc_te - V_zero) / Sensitivity
    current = (v_sensor - zero_point) / 0.185; 

    // BUOC 4: Bo loc nhieu kim (Deadzone)
    // Neu dong nho hon 0.15A thi ep ve 0 de on dinh hien thi
    if (current < 0.15 && current > -0.15) {
        current = 0.0;
    }
    
    return current;
}




