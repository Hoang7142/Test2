#include "menu_control.h"

static uint8_t lcd_page = 0; // Bi?n n?i b? qu?n lý trang màn hình hi?n t?i (0 -> 3)

/**
 * @brief Kh?i t?o 3 chân GPIO m?i d?ng IPU và C?u hình gi?i phóng chân JTAG cho PA15
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
}

/**
 * @brief Quét nút nh?n v?t lý (PA15, PA11, PA12) và thay d?i các bi?n tr?ng thái
 */
void Menu_Button_Scan(menu_control_data_t *control, uint8_t *update_flag) {
    
    // ====================================================================
    // 1. LU?NG NÚT NEXT (PA15): Chuy?n d?i qua l?i gi?a 4 trang màn hình
    // ====================================================================
    if (GPIO_ReadInputDataBit(BTN_PORT, BTN_NEXT_PIN) == Bit_RESET) {
        for(volatile uint32_t i = 0; i < 200000; i++); // Ch?ng rung
        if (GPIO_ReadInputDataBit(BTN_PORT, BTN_NEXT_PIN) == Bit_RESET) {
            lcd_page++;
            if (lcd_page > 3) lcd_page = 0; 
            I2C_LCD_Clear();                
            while (GPIO_ReadInputDataBit(BTN_PORT, BTN_NEXT_PIN) == Bit_RESET); // Ch? nh? nút
        }
    }

    // ====================================================================
    // 2. LU?NG NÚT UP (PA11) / DOWN (PA12): Thay d?i giá tr? theo trang
    // ====================================================================
    
    // --- TRANG 1: CH?N CH? Ð? V?N HÀNH (AUTO HO?C MANUAL) ---
    if (lcd_page == 1) {
        if (GPIO_ReadInputDataBit(BTN_PORT, BTN_UP_PIN) == Bit_RESET) {
            for(volatile uint32_t i = 0; i < 200000; i++);
            if (control->system_mode != 1) {
                control->system_mode = 1; // AUTO
                *update_flag = 1;         
            }
            while (GPIO_ReadInputDataBit(BTN_PORT, BTN_UP_PIN) == Bit_RESET);
        }
        if (GPIO_ReadInputDataBit(BTN_PORT, BTN_DOWN_PIN) == Bit_RESET) {
            for(volatile uint32_t i = 0; i < 200000; i++);
            if (control->system_mode != 0) {
                control->system_mode = 0; // MANUAL
                *update_flag = 1;         
            }
            while (GPIO_ReadInputDataBit(BTN_PORT, BTN_DOWN_PIN) == Bit_RESET);
        }
    }
    
    // --- CÁC TRANG THI?T B?: Ch? cho phép tuong tác khi ? MANUAL ---
    else if (control->system_mode == 0) { 
        
        // --- TRANG 2: ÐI?U KHI?N MÁY BOM NU?C ---
        if (lcd_page == 2) {
            if (GPIO_ReadInputDataBit(BTN_PORT, BTN_UP_PIN) == Bit_RESET) {
                for(volatile uint32_t i = 0; i < 200000; i++);
                if (control->pump_status != 1) {
                    control->pump_status = 1; // B?t Bom
                    *update_flag = 1;
                }
                while (GPIO_ReadInputDataBit(BTN_PORT, BTN_UP_PIN) == Bit_RESET);
            }
            if (GPIO_ReadInputDataBit(BTN_PORT, BTN_DOWN_PIN) == Bit_RESET) {
                for(volatile uint32_t i = 0; i < 200000; i++);
                if (control->pump_status != 0) {
                    control->pump_status = 0; // T?t Bom
                    *update_flag = 1;
                }
                while (GPIO_ReadInputDataBit(BTN_PORT, BTN_DOWN_PIN) == Bit_RESET);
            }
        }
        
        // --- TRANG 3: ÐI?U KHI?N MÁI CHE ---
        else if (lcd_page == 3) {
            if (GPIO_ReadInputDataBit(BTN_PORT, BTN_UP_PIN) == Bit_RESET) {
                for(volatile uint32_t i = 0; i < 200000; i++);
                if (control->roof_status != MOTOR_FORWARD) {
                    control->roof_status = MOTOR_FORWARD; // M? mái
                    *update_flag = 1;
                }
                while (GPIO_ReadInputDataBit(BTN_PORT, BTN_UP_PIN) == Bit_RESET);
            }
            if (GPIO_ReadInputDataBit(BTN_PORT, BTN_DOWN_PIN) == Bit_RESET) {
                for(volatile uint32_t i = 0; i < 200000; i++);
                if (control->roof_status == MOTOR_BACKWARD) {
                    control->roof_status = MOTOR_STOP;    // B?m DOWN ti?p khi dang dóng -> D?NG
                } else {
                    control->roof_status = MOTOR_BACKWARD; // Ðóng mái
                }
                *update_flag = 1;
                while (GPIO_ReadInputDataBit(BTN_PORT, BTN_DOWN_PIN) == Bit_RESET);
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