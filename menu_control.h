#ifndef __MENU_CONTROL_H
#define __MENU_CONTROL_H

#include "stm32f10x.h"
#include "i2c_lcd.h"
#include "motor.h"

/* Nut: PA15 NEXT, PA11 UP, PA12 DOWN */
#define BTN_PORT        GPIOA
#define BTN_NEXT_PIN    GPIO_Pin_15
#define BTN_UP_PIN      GPIO_Pin_11
#define BTN_DOWN_PIN    GPIO_Pin_12
#define BTN_CLK         RCC_APB2Periph_GPIOA

#define MENU_PWM_STEP   5u   /* Buoc +/- % tren LCD */

typedef struct {
    uint8_t system_mode;  /* 1 AUTO, 0 MANUAL */
    uint8_t pump_status;  /* 1 ON, 0 OFF */
    uint8_t roof_status;  /* FORWARD / BACKWARD / STOP */
} menu_control_data_t;

void Menu_Init(void);
/* pump_pwm / roof_pwm: 0..100 ? Manual UP/DOWN tren trang 4/5 */
void Menu_Button_Scan(menu_control_data_t *control,
                      uint8_t *pump_pwm,
                      uint8_t *roof_pwm,
                      uint8_t *update_flag);
void Menu_Display_Update(const menu_control_data_t *control,
                         uint8_t pump_pwm,
                         uint8_t roof_pwm);

#endif
