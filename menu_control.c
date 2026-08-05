#include "menu_control.h"
#include "delay.h"
#include <stdio.h>

static uint8_t lcd_page = 0; /* 0..5 */

#define BTN_DEBOUNCE_MS 20
#define BTN_HOLD_TIMEOUT_MS 500
#define MENU_PAGE_MAX 5u

typedef struct {
    uint32_t last_press_ms;
    uint8_t state;
} btn_state_t;

static btn_state_t btn_next_state = {0, 0};
static btn_state_t btn_up_state = {0, 0};
static btn_state_t btn_down_state = {0, 0};

extern uint32_t millis(void);

static uint8_t btn_debounce(GPIO_TypeDef* port, uint16_t pin, btn_state_t* state, uint32_t now_ms) {
    uint8_t pin_level = GPIO_ReadInputDataBit(port, pin);

    if (pin_level == Bit_RESET) {
        if (state->state == 0) {
            if (now_ms - state->last_press_ms >= BTN_DEBOUNCE_MS) {
                state->state = 1;
                state->last_press_ms = now_ms;
                return 1;
            }
        } else {
            if (now_ms - state->last_press_ms > BTN_HOLD_TIMEOUT_MS) {
                state->state = 0;
            }
        }
    } else {
        state->state = 0;
    }
    return 0;
}

static void pwm_add(uint8_t *pwm, int8_t delta, uint8_t *update_flag) {
    int16_t v = (int16_t)(*pwm) + (int16_t)delta;
    if (v < 0) v = 0;
    if (v > 100) v = 100;
    if ((uint8_t)v != *pwm) {
        *pwm = (uint8_t)v;
        *update_flag = 1;
    }
}

void Menu_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(BTN_CLK | RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = BTN_NEXT_PIN | BTN_UP_PIN | BTN_DOWN_PIN;
    GPIO_Init(BTN_PORT, &GPIO_InitStructure);

    I2C_LCD_Init();
    I2C_LCD_Clear();

    btn_next_state.last_press_ms = millis();
    btn_up_state.last_press_ms = millis();
    btn_down_state.last_press_ms = millis();
}

void Menu_Button_Scan(menu_control_data_t *control,
                      uint8_t *pump_pwm,
                      uint8_t *roof_pwm,
                      uint8_t *update_flag) {
    uint32_t now_ms = millis();

    if (btn_debounce(BTN_PORT, BTN_NEXT_PIN, &btn_next_state, now_ms)) {
        lcd_page++;
        if (lcd_page > MENU_PAGE_MAX) lcd_page = 0;
        I2C_LCD_Clear();
        *update_flag = 1;
    }

    /* Trang 1: mode ? luon cho doi */
    if (lcd_page == 1) {
        if (btn_debounce(BTN_PORT, BTN_UP_PIN, &btn_up_state, now_ms)) {
            if (control->system_mode != 1) {
                control->system_mode = 1;
                *update_flag = 1;
            }
        }
        if (btn_debounce(BTN_PORT, BTN_DOWN_PIN, &btn_down_state, now_ms)) {
            if (control->system_mode != 0) {
                control->system_mode = 0;
                *update_flag = 1;
            }
        }
        return;
    }

    /* Trang thiet bi + PWM: chi Manual */
    if (control->system_mode != 0) {
        return;
    }

    if (lcd_page == 2) {
        if (btn_debounce(BTN_PORT, BTN_UP_PIN, &btn_up_state, now_ms)) {
            if (control->pump_status != 1) {
                control->pump_status = 1;
                *update_flag = 1;
            }
        }
        if (btn_debounce(BTN_PORT, BTN_DOWN_PIN, &btn_down_state, now_ms)) {
            if (control->pump_status != 0) {
                control->pump_status = 0;
                *update_flag = 1;
            }
        }
    } else if (lcd_page == 3) {
        if (btn_debounce(BTN_PORT, BTN_UP_PIN, &btn_up_state, now_ms)) {
            if (control->roof_status != MOTOR_FORWARD) {
                control->roof_status = MOTOR_FORWARD;
                *update_flag = 1;
            }
        }
        if (btn_debounce(BTN_PORT, BTN_DOWN_PIN, &btn_down_state, now_ms)) {
            if (control->roof_status == MOTOR_BACKWARD) {
                control->roof_status = MOTOR_STOP;
            } else {
                control->roof_status = MOTOR_BACKWARD;
            }
            *update_flag = 1;
        }
    } else if (lcd_page == 4) {
        if (btn_debounce(BTN_PORT, BTN_UP_PIN, &btn_up_state, now_ms)) {
            pwm_add(pump_pwm, (int8_t)MENU_PWM_STEP, update_flag);
        }
        if (btn_debounce(BTN_PORT, BTN_DOWN_PIN, &btn_down_state, now_ms)) {
            pwm_add(pump_pwm, -(int8_t)MENU_PWM_STEP, update_flag);
        }
    } else if (lcd_page == 5) {
        if (btn_debounce(BTN_PORT, BTN_UP_PIN, &btn_up_state, now_ms)) {
            pwm_add(roof_pwm, (int8_t)MENU_PWM_STEP, update_flag);
        }
        if (btn_debounce(BTN_PORT, BTN_DOWN_PIN, &btn_down_state, now_ms)) {
            pwm_add(roof_pwm, -(int8_t)MENU_PWM_STEP, update_flag);
        }
    }
}

void Menu_Display_Update(const menu_control_data_t *control,
                         uint8_t pump_pwm,
                         uint8_t roof_pwm) {
    char line[17];

    switch (lcd_page) {
        case 0:
            I2C_LCD_SetCursor(0, 0); I2C_LCD_Puts("1. MONITORING   ");
            I2C_LCD_SetCursor(1, 0); I2C_LCD_Puts("SYS: RUNNING... ");
            break;

        case 1:
            I2C_LCD_SetCursor(0, 0); I2C_LCD_Puts("2. SYSTEM MODE  ");
            I2C_LCD_SetCursor(1, 0);
            if (control->system_mode == 1) I2C_LCD_Puts("Mode: > AUTO    ");
            else                           I2C_LCD_Puts("Mode: > MANUAL  ");
            break;

        case 2:
            I2C_LCD_SetCursor(0, 0); I2C_LCD_Puts("3. WATER PUMP   ");
            I2C_LCD_SetCursor(1, 0);
            if (control->pump_status == 1) I2C_LCD_Puts("Pump: [ ON ]    ");
            else                           I2C_LCD_Puts("Pump: [ OFF ]   ");
            break;

        case 3:
            I2C_LCD_SetCursor(0, 0); I2C_LCD_Puts("4. ROOF ROLLER  ");
            I2C_LCD_SetCursor(1, 0);
            if (control->roof_status == MOTOR_FORWARD)      I2C_LCD_Puts("Roof: [ OPENING ]");
            else if (control->roof_status == MOTOR_BACKWARD) I2C_LCD_Puts("Roof: [ CLOSING ]");
            else                                            I2C_LCD_Puts("Roof: [ STOP ]   ");
            break;

        case 4:
            I2C_LCD_SetCursor(0, 0); I2C_LCD_Puts("5. PUMP SPEED   ");
            I2C_LCD_SetCursor(1, 0);
            snprintf(line, sizeof(line), "PWM: %3u%%      ", (unsigned)pump_pwm);
            I2C_LCD_Puts(line);
            break;

        case 5:
            I2C_LCD_SetCursor(0, 0); I2C_LCD_Puts("6. ROOF SPEED   ");
            I2C_LCD_SetCursor(1, 0);
            snprintf(line, sizeof(line), "PWM: %3u%%      ", (unsigned)roof_pwm);
            I2C_LCD_Puts(line);
            break;

        default:
            break;
    }
}
