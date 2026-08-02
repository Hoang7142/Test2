#include "menu_control.h"
#include "delay.h"  // ?? THÊM: d? dùng millis() ho?c tuong t?

static uint8_t lcd_page = 0; // Bi?n n?i b? qu?n lý trang màn hình hi?n t?i (0 -> 3)

// ?? FIX BUG #7: Debounce state machine thay vì blocking for loop
#define BTN_DEBOUNCE_MS 20  // 20ms debounce
#define BTN_HOLD_TIMEOUT_MS 500  // Timeout d? tránh nút dính

typedef struct {
    uint32_t last_press_ms;
    uint8_t state; // 0: idle, 1: pressed
} btn_state_t;

static btn_state_t btn_next_state = {0, 0};
static btn_state_t btn_up_state = {0, 0};
static btn_state_t btn_down_state = {0, 0};

/**
 * @brief L?y th?i gian hi?n t?i (ms) - c?n implement t? timer.c
 * N?u b?n không có, dùng TIM_GetCounter ho?c tuong t?
 */
extern uint32_t millis(void);  // ?? YÊU C?U: c?n có hàm này t? timer.c

/**
 * @brief Hàm debounce không blocking (dùng state machine + timestamp)
 */
static uint8_t btn_debounce(GPIO_TypeDef* port, uint16_t pin, btn_state_t* state, uint32_t now_ms) {
    uint8_t pin_level = GPIO_ReadInputDataBit(port, pin);
    
    // Nút du?c nh?n (m?c LOW)
    if (pin_level == Bit_RESET) {
        if (state->state == 0) { // T? idle chuy?n sang pressed
            if (now_ms - state->last_press_ms >= BTN_DEBOUNCE_MS) {
                state->state = 1;
                state->last_press_ms = now_ms;
                return 1; // ? Phát hi?n nh?n h?p l?
            }
        } else {
            // Ðang trong tr?ng thái pressed - ki?m tra timeout d? tránh nút dính
            if (now_ms - state->last_press_ms > BTN_HOLD_TIMEOUT_MS) {
                // Nút gi? quá lâu -> reset state d? tránh "ghost press"
                state->state = 0;
            }
        }
    } 
    // Nút du?c th? (m?c HIGH)
    else {
        state->state = 0; // Reset v? idle
    }
    
    return 0; // Chua phát hi?n nh?n
}

/**
 * @brief Kh?i t?o 3 chân GPIO m?i dùng IPU và C?u hình gi?i phóng chân JTAG cho PA15
 */
void Menu_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 1. C?p xung clock cho PORT A và ngo?i vi AFIO (B?t bu?c ph?i có AFIO d? remap)
    RCC_APB2PeriphClockCmd(BTN_CLK | RCC_APB2Periph_AFIO, ENABLE);
    
    // 2. L?NH QUAN TR?NG: T?t JTAG d? gi?i phóng chân PA15 làm GPIO thu?ng (v?n gi? l?i SWD d? n?p code)
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    
    // 3. C?u hình chân nút nh?n Input Pull-up
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = BTN_NEXT_PIN | BTN_UP_PIN | BTN_DOWN_PIN;
    GPIO_Init(BTN_PORT, &GPIO_InitStructure);
    
    // Kh?i d?ng màn hình LCD
    I2C_LCD_Init();
    I2C_LCD_Clear();
    
    // Kh?i t?o state debounce
    btn_next_state.last_press_ms = millis();
    btn_up_state.last_press_ms = millis();
    btn_down_state.last_press_ms = millis();
}

/**
 * @brief Quét nút nh?n v?t lý (PA15, PA11, PA12) và thay d?i các bi?n tr?ng thái
 * ?? FIX: Không blocking - dùng debounce state machine
 */
void Menu_Button_Scan(menu_control_data_t *control, uint8_t *update_flag) {
    uint32_t now_ms = millis(); // ?? YÊU C?U: hàm millis() t? timer.c
    
    // ====================================================================
    // 1. LU?NG NÚT NEXT (PA15): Chuy?n d?i qua l?i gi?a 4 trang màn hình
    // ====================================================================
    if (btn_debounce(BTN_PORT, BTN_NEXT_PIN, &btn_next_state, now_ms)) {
        lcd_page++;
        if (lcd_page > 3) lcd_page = 0; 
        I2C_LCD_Clear();
        *update_flag = 1;
    }

    // ====================================================================
    // 2. LU?NG NÚT UP (PA11) / DOWN (PA12): Thay d?i giá tr? theo trang
    // ====================================================================
    
    // --- TRANG 1: CH?N CH? Ð? V?N HÀNH (AUTO HO?C MANUAL) ---
    if (lcd_page == 1) {
        if (btn_debounce(BTN_PORT, BTN_UP_PIN, &btn_up_state, now_ms)) {
            if (control->system_mode != 1) {
                control->system_mode = 1; // AUTO
                *update_flag = 1;         
            }
        }
        if (btn_debounce(BTN_PORT, BTN_DOWN_PIN, &btn_down_state, now_ms)) {
            if (control->system_mode != 0) {
                control->system_mode = 0; // MANUAL
                *update_flag = 1;         
            }
        }
    }
    
    // --- CÁC TRANG THI?T B?: Ch? cho phép tuong tác khi ? MANUAL ---
    else if (control->system_mode == 0) { 
        
        // --- TRANG 2: ÐI?U KHI?N MÁY BOM NU?C ---
        if (lcd_page == 2) {
            if (btn_debounce(BTN_PORT, BTN_UP_PIN, &btn_up_state, now_ms)) {
                if (control->pump_status != 1) {
                    control->pump_status = 1; // B?t Bom
                    *update_flag = 1;
                }
            }
            if (btn_debounce(BTN_PORT, BTN_DOWN_PIN, &btn_down_state, now_ms)) {
                if (control->pump_status != 0) {
                    control->pump_status = 0; // T?t Bom
                    *update_flag = 1;
                }
            }
        }
        
        // --- TRANG 3: ÐI?U KHI?N MÁI CHE ---
        else if (lcd_page == 3) {
            if (btn_debounce(BTN_PORT, BTN_UP_PIN, &btn_up_state, now_ms)) {
                if (control->roof_status != MOTOR_FORWARD) {
                    control->roof_status = MOTOR_FORWARD; // M? mái
                    *update_flag = 1;
                }
            }
            if (btn_debounce(BTN_PORT, BTN_DOWN_PIN, &btn_down_state, now_ms)) {
                if (control->roof_status == MOTOR_BACKWARD) {
                    control->roof_status = MOTOR_STOP;    // B?m DOWN ti?p khi dang dóng -> D?NG
                } else {
                    control->roof_status = MOTOR_BACKWARD; // Ðóng mái
                }
                *update_flag = 1;
            }
        }
    }
}

/**
 * @brief C?p nh?t giao di?n ch? hi?n th? ra màn hình LCD 16x2
 */
void Menu_Display_Update(const menu_control_data_t *control) {
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
    }
}