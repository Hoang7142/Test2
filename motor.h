#ifndef __MOTOR_H
#define __MOTOR_H

#include "stm32f10x.h"

/* Dinh nghia huong quay motor */
#define MOTOR_STOP      0
#define MOTOR_FORWARD   1
#define MOTOR_BACKWARD  2

/* Cac ham giao tiep ngoai vi */
void Motor_Init(void);
void Motor1_SetSpeed(uint16_t speed); /* Toc do tu 0 - 100 */
void Motor1_Dir(uint8_t dir);         /* Huong: Stop, Forward, Backward */


#endif
