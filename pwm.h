#ifndef __PWM_H
#define __PWM_H

#include "stm32f10x.h"

// Khởi tạo PWM
void PWM_Init(void);

// Cấu hình GPIO dùng cho PWM (PB4 - TIM3_CH1)
void PWM_GPIO_Config(void);

// Đặt độ rộng xung PWM theo phần trăm (0–100%)
void PWM_SetDutyCycle(uint16_t duty);

// ====== Điều khiển Cầu H L298N ======
// PA4 -> IN1, PA5 -> IN2

void HBridge_GPIO_Config(void);
void Motor_Forward(void);
void Motor_Backward(void);
void Motor_Stop(void);

#endif