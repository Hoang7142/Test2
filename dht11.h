//#ifndef __DHT11_H
//#define __DHT11_H

//#include "stm32f10x.h"

//// Dinh nghia cac ma loi
//#define DHT11_OK       0
//#define DHT11_ERR_NO_RESP 1
//#define DHT11_ERR_CHKSUM  2

//void DHT11_Init(void);
//uint8_t DHT11_ReadData(uint8_t *temp, uint8_t *humi);

//#endif

/////////////////////////////////////////////////////////////////////////////

#ifndef __DHT11_H
#define __DHT11_H

#include "stm32f10x.h"

// Dinh nghia cac ma loi
#define DHT11_OK            0
#define DHT11_ERR_NO_RESP   1
#define DHT11_ERR_CHKSUM    2

// Chân k?t n?i theo b?ng ch?t chân (PB12)
#define DHT11_PORT GPIOB
#define DHT11_PIN  GPIO_Pin_12

void DHT11_Init(void);
uint8_t DHT11_ReadData(uint8_t *temp, uint8_t *humi);

#endif
