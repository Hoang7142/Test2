#ifndef FLOW_SENSOR_H
#define FLOW_SENSOR_H

#include "stm32f10x.h"

void FlowSensor_Init(void);

/* Tra ve L/min * 10 (vd 15 = 1.5 L/min). Cache ~500ms, an toan goi nhieu noi. */
uint16_t Flow_GetLpmX10(void);

#endif
