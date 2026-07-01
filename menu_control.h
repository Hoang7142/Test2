#ifndef __MENU_CONTROL_H
#define __MENU_CONTROL_H

#include "stm32f10x.h"
#include "i2c_lcd.h"
#include "motor.h"

// ?? C?U HÌNH PH?N C?NG NÚT NH?N M?I: PA15, PA11, PA12
#define BTN_PORT        GPIOA
#define BTN_NEXT_PIN    GPIO_Pin_15  // Nút chuy?n trang Menu (PA15)
#define BTN_UP_PIN      GPIO_Pin_11  // Nút UP (PA11)
#define BTN_DOWN_PIN    GPIO_Pin_12  // Nút DOWN (PA12)
#define BTN_CLK         RCC_APB2Periph_GPIOA

// ?? C?U TRÚC D? LI?U Ð?NG B? TR?NG THÁI H? TH?NG
typedef struct {
    uint8_t system_mode;  // 1: AUTO, 0: MANUAL
    uint8_t pump_status;  // 1: ON,   0: OFF
    uint8_t roof_status;  // 1: MOTOR_FORWARD, 2: MOTOR_BACKWARD, 0: MOTOR_STOP
} menu_control_data_t;

// CÁC HÀM GIAO TI?P
void Menu_Init(void);
void Menu_Button_Scan(menu_control_data_t *control, uint8_t *update_flag);
void Menu_Display_Update(const menu_control_data_t *control);

#endif