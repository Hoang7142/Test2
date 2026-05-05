//#ifndef FLOW_H
//#define FLOW_H

//#include "stm32f10x.h"
//#include "stm32f10x_gpio.h"

//void FlowSensor_Init(void);
//float Flow_GetLitersPerMinute(void); // Tinh luu luong L/phut

//#endif

////////////////////////////////////////////////////////////
#ifndef FLOW_SENSOR_H
#define FLOW_SENSOR_H

#include "stm32f10x.h"

void FlowSensor_Init(void);
float Flow_GetLitersPerMinute(uint32_t sampling_time_ms); // Thêm th?i gian l?y m?u

#endif

