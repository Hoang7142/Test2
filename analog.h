#ifndef __ANALOG_H
#define __ANALOG_H

#include "stm32f10x.h"

/* ===== STRUCT LUU DU LIEU ===== */
typedef struct {
    uint16_t raw_soil;
    uint8_t  soil_percent;

    uint16_t raw_water;
    uint8_t  water_percent;

    float current_ampe;
} Analog_Data_t;

/* ===== KHOI TAO ADC ===== */
void Analog_Init(void);

/* ===== HAM MOI: HIEU CHUAN DONG DIEN ===== */
void Analog_Calibrate(void); 

/* ===== DOC ADC ===== */
uint16_t ADC_Read_Raw(uint8_t channel);

/* ===== LOC NHIEU ===== */
uint16_t ADC_Read_Median(uint8_t channel);
uint16_t ADC_Read_Filter(uint8_t channel);

/* ===== CAP NHAT TOAN BO CAM BIEN ===== */
void Analog_UpdateAll(Analog_Data_t *data);

#endif




