//#include "lora.h"
//#include "delay.h" // De dung ham Delay_Ms cho khoi tao

//// Ham khoi tao SPI1 va cac chan dieu khien LoRa
//void LoRa_SPI_Init(void) {
//    GPIO_InitTypeDef GPIO_InitStructure;
//    SPI_InitTypeDef  SPI_InitStructure;

//    // Cap xung nhip cho GPIOA, GPIOB va SPI1
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_SPI1, ENABLE);

//    // Cau hinh chan SCK (PA5) va MOSI (PA7) la Alternate Function Push-Pull de SPI dieu khien
//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_Init(GPIOA, &GPIO_InitStructure);

//    // Cau hinh chan MISO (PA6) la Input Floating de nhan du lieu
//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//    GPIO_Init(GPIOA, &GPIO_InitStructure);

//    // Cau hinh chan NSS (PA4) la Output Push-Pull dieu khien chon chip bang phan mem
//    GPIO_InitStructure.GPIO_Pin = LORA_NSS_PIN;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//    GPIO_Init(LORA_NSS_PORT, &GPIO_InitStructure);

//    // Cau hinh chan DIO0 (PB5) la Input Pull-down de kiem tra ngat/polling
//    GPIO_InitStructure.GPIO_Pin = LORA_DIO0_PIN;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
//    GPIO_Init(LORA_DIO0_PORT, &GPIO_InitStructure);

//    // Keo chan NSS len muc cao (Khong chon chip LoRa luc moi khoi dong)
//    GPIO_SetBits(LORA_NSS_PORT, LORA_NSS_PIN);

//    // Cau hinh thong so cho SPI1
//    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
//    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
//    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
//    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
//    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
//    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft; // Dung phan mem dieu khien chan NSS
//    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; // Chia tan so clock SPI
//    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
//    SPI_InitStructure.SPI_CRCPolynomial = 7;
//    SPI_Init(SPI1, &SPI_InitStructure);

//    // Kich hoat SPI1 hoat dong
//    SPI_Cmd(SPI1, ENABLE);
//}

//// Ham noi bo de truyen va nhan 1 byte qua SPI
//static uint8_t SPI_Transfer(uint8_t data) {
//    // Cho den khi buffer truyen san sang
//    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
//    // Gui 1 byte du lieu di
//    SPI_I2S_SendData(SPI1, data);
//    
//    // Cho den khi nhan duoc 1 byte du lieu ve
//    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
//    // Tra ve byte nhan duoc
//    return SPI_I2S_ReceiveData(SPI1);
//}

//// Ham ghi du lieu vao thanh ghi cua LoRa
//void LoRa_Write_Reg(uint8_t reg, uint8_t data) {
//    // Keo NSS xuong muc thap de chon chip LoRa
//    GPIO_ResetBits(LORA_NSS_PORT, LORA_NSS_PIN);
//    
//    // Gui dia chi thanh ghi (Bit MSB bang 1 de bao hieu lenh Ghi)
//    SPI_Transfer(reg | 0x80);
//    // Gui du lieu can ghi vao
//    SPI_Transfer(data);
//    
//    // Keo NSS len muc cao de ket thuc giao tiep voi chip
//    GPIO_SetBits(LORA_NSS_PORT, LORA_NSS_PIN);
//}

//// Ham doc du lieu tu thanh ghi cua LoRa
//uint8_t LoRa_Read_Reg(uint8_t reg) {
//    uint8_t val;
//    
//    // Keo NSS xuong muc thap de chon chip LoRa
//    GPIO_ResetBits(LORA_NSS_PORT, LORA_NSS_PIN);
//    
//    // Gui dia chi thanh ghi (Bit MSB bang 0 de bao hieu lenh Doc)
//    SPI_Transfer(reg & 0x7F);
//    // Gui ma byte 0x00 de tao xung clock day du lieu tu LoRa ve STM32
//    val = SPI_Transfer(0x00);
//    
//    // Keo NSS len muc cao de ket thuc giao tiep voi chip
//    GPIO_SetBits(LORA_NSS_PORT, LORA_NSS_PIN);
//    
//    return val;
//}

//// Ham khoi tao thong so ban dau cho LoRa (Tan so 433MHz, cong suat phat)
//uint8_t LoRa_Init(void) {
//    // Goi ham khoi tao SPI truoc
//    LoRa_SPI_Init();

//    // Chuyen LoRa ve che do Sleep de kich hoat che do LoRa mode
//    LoRa_Write_Reg(LORA_REG_OP_MODE, LORA_MODE_SLEEP);
//    Delay_Ms(10); 

//    // Bat che do LoRa va giu o che do Sleep
//    LoRa_Write_Reg(LORA_REG_OP_MODE, LORA_MODE_SLEEP | LORA_LONG_RANGE_MODE);
//    Delay_Ms(10);

//    // Kiem tra ket noi SPI voi chip bang cach doc thanh ghi Version (Thuong la 0x12)
//    uint8_t version = LoRa_Read_Reg(LORA_REG_VERSION);
//    if (version == 0x00 || version == 0xFF) {
//        return 0; // Loi khong ket noi duoc SPI hoac LoRa hong
//    }

//    // Cau hinh tan so 433 MHz (Frf = 433MHz / (32MHz / 2^19) = 0x6C4000)
//    LoRa_Write_Reg(LORA_REG_FRF_MSB, 0x6C);
//    LoRa_Write_Reg(LORA_REG_FRF_MID, 0x40);
//    LoRa_Write_Reg(LORA_REG_FRF_LSB, 0x00);

//    // Cau hinh cong suat phat toi da dung chan PA_BOOST
//    LoRa_Write_Reg(LORA_REG_PA_CONFIG, 0xCF);

//    // Chuyen sang che do Standby de san sang truyen nhan
//    LoRa_Write_Reg(LORA_REG_OP_MODE, LORA_MODE_STANDBY | LORA_LONG_RANGE_MODE);

//    return 1; // Khoi tao thanh cong
//}

//// Ham phat du lieu di
//void LoRa_Transmit(uint8_t *data, uint8_t length) {
//    // Chuyen ve che do Standby de nap du lieu
//    LoRa_Write_Reg(LORA_REG_OP_MODE, LORA_MODE_STANDBY | LORA_LONG_RANGE_MODE);

//    // Chi dinh con tro FIFO ve dung dia chi co so cua bo dem TX
//    LoRa_Write_Reg(LORA_REG_FIFO_ADDR_PTR, LoRa_Read_Reg(LORA_REG_FIFO_TX_BASE_ADDR));

//    // Cau hinh so luong byte can truyen
//    LoRa_Write_Reg(LORA_REG_PAYLOAD_LENGTH, length);

//    // Dua tung byte du lieu vao bo dem FIFO cua LoRa
//    for (uint8_t i = 0; i < length; i++) {
//        LoRa_Write_Reg(LORA_REG_FIFO, data[i]);
//    }

//    // Chuyen LoRa sang che do TX de bat dau phat
//    LoRa_Write_Reg(LORA_REG_OP_MODE, LORA_MODE_TX | LORA_LONG_RANGE_MODE);

//    // Cho den khi phat xong bang cach doc co ngat TxDone (Bit 3) trong thanh ghi IRQ
//    while ((LoRa_Read_Reg(LORA_REG_IRQ_FLAGS) & 0x08) == 0) {
//        // Vong lap cho phat xong
//    }

//    // Xoa co ngat TxDone sau khi phat xong bang cach ghi so 1 vao chinh no
//    LoRa_Write_Reg(LORA_REG_IRQ_FLAGS, 0x08);
//}

//// Ham dat LoRa vao che do luon lang nghe du lieu toi
//void LoRa_Start_Receive(void) {
//    LoRa_Write_Reg(LORA_REG_OP_MODE, LORA_MODE_RX_CONTINUOUS | LORA_LONG_RANGE_MODE);
//}

//// Ham kiem tra va doc du lieu nhan duoc (Polling)
//uint8_t LoRa_Receive(uint8_t *data) {
//    // Doc thanh ghi ngat xem co co RxDone (Bit 6) bat len khong
//    uint8_t irqFlags = LoRa_Read_Reg(LORA_REG_IRQ_FLAGS);

//    if ((irqFlags & 0x40) != 0) {
//        // Da co du lieu den, tien hanh xoa co ngat RxDone
//        LoRa_Write_Reg(LORA_REG_IRQ_FLAGS, 0x40);

//        // Doc xem nhan duoc bao nhieu byte
//        uint8_t length = LoRa_Read_Reg(LORA_REG_RX_NB_BYTES);

//        // Di chuyen con tro FIFO den dung vi tri du lieu vua nhan duoc
//        LoRa_Write_Reg(LORA_REG_FIFO_ADDR_PTR, LoRa_Read_Reg(LORA_REG_FIFO_RX_CURRENT_ADDR));

//        // Doc lay tung byte du lieu tu FIFO luu vao mang
//        for (uint8_t i = 0; i < length; i++) {
//            data[i] = LoRa_Read_Reg(LORA_REG_FIFO);
//        }

//        return length; // Tra ve so luong byte lay duoc
//    }

//    return 0; // Khong co du lieu moi
//}


//#include "lora.h"
//#include "delay.h"
//#include <string.h>

//// Dinh nghia chan Reset PA3
//#define LORA_RST_PORT  GPIOA
//#define LORA_RST_PIN   GPIO_Pin_3

//// --- HAM KHOI TAO SPI ---
//void LoRa_SPI_Init(void) {
//    GPIO_InitTypeDef GPIO_InitStructure;
//    SPI_InitTypeDef  SPI_InitStructure;

//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_SPI1, ENABLE);

//    // PA5: SCK, PA7: MOSI
//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7; 
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_Init(GPIOA, &GPIO_InitStructure);

//    // PA6: MISO
//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; 
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
//    GPIO_Init(GPIOA, &GPIO_InitStructure);

//    // PA4: NSS (Soft CS) - Phai keo len CAO ngay lap tuc
//    GPIO_InitStructure.GPIO_Pin = LORA_NSS_PIN; 
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//    GPIO_Init(LORA_NSS_PORT, &GPIO_InitStructure);
//    GPIO_SetBits(LORA_NSS_PORT, LORA_NSS_PIN); // MAC DINH LA CAO (Khong chon chip)

//    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
//    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
//    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
//    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
//    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
//    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
//    
//    // �� S?A: Tang b? chia t? 8 l�n 32 d? h? t?c d? Clock t? 12MHz xu?ng c�n 3MHz
//    // �?m b?o chip LoRa SX1278 c� th? d?c k?p t�n hi?u.
//    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256; 
//    
//    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
//    SPI_Init(SPI1, &SPI_InitStructure);
//    SPI_Cmd(SPI1, ENABLE);
//}

//// --- HAM RESET ---
//void LoRa_Reset(void) {
//    GPIO_InitTypeDef GPIO_InitStructure;
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
//    
//    GPIO_InitStructure.GPIO_Pin = LORA_RST_PIN;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_Init(LORA_RST_PORT, &GPIO_InitStructure);

//    // Chu trinh keo RST: Thap (10ms) -> Cao (10ms)
//    GPIO_ResetBits(LORA_RST_PORT, LORA_RST_PIN);
//    Delay_Ms(20);
//    GPIO_SetBits(LORA_RST_PORT, LORA_RST_PIN);
//    Delay_Ms(20);
//}

//// --- HAM TRUYEN NHAN SPI NOI BO ---
//static uint8_t SPI_Transfer(uint8_t data) {
//    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
//    SPI_I2S_SendData(SPI1, data);
//    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
//    return SPI_I2S_ReceiveData(SPI1);
//}

//// --- CAC HAM DOC/GHI THANH GHI ---
//void LoRa_Write_Reg(uint8_t reg, uint8_t data) {
//    GPIO_ResetBits(LORA_NSS_PORT, LORA_NSS_PIN);
//    
//    // T?o d? tr? nh? d? NSS ?n d?nh
//    for(volatile int i = 0; i < 200; i++); 

//    // NG: Ghi th bit 7 ph?i l 1 (dng | 0x80)
//    SPI_Transfer(reg | 0x80); 
//    SPI_Transfer(data);

//    GPIO_SetBits(LORA_NSS_PORT, LORA_NSS_PIN);
//}

//uint8_t LoRa_Read_Reg(uint8_t reg) {
//    uint8_t val;

//    GPIO_ResetBits(LORA_NSS_PORT, LORA_NSS_PIN);
//    
//    // T?o d? tr? nh?
//    for(volatile int i = 0; i < 200; i++); 

//    // NG:?c th bit 7 ph?i l 0 (dng & 0x7F)
//    SPI_Transfer(reg & 0x7F); 
    
//    // T?o d? tr? d? chip chu?n b? d? li?u
//    for(volatile int i = 0; i < 200; i++); 

//    // G?i 0x00 d? t?o xung clock l?y d? li?u v?
//    val = SPI_Transfer(0x00);

//    GPIO_SetBits(LORA_NSS_PORT, LORA_NSS_PIN);

//    return val;
//}

//// --- HAM KHOI TAO CHINH ---
//uint8_t LoRa_Init(void) {
//    // 1. KHOI TAO SPI TRUOC DE CHAN NSS ON DINH
//    LoRa_SPI_Init(); 
//    
//    // 2. RESET MODULE
//    LoRa_Reset();    
//    
//    // 3. CAU HINH MODULE
//    LoRa_Write_Reg(REG_OP_MODE, 0x00); // Sleep mode
//    Delay_Ms(10);
//    LoRa_Write_Reg(REG_OP_MODE, 0x80); // Chuyen sang LoRa Mode
//    Delay_Ms(10);
//    
//    // Kiem tra Version
//    if (LoRa_Read_Reg(REG_VERSION) != 0x12) {
//        return 0; // That bai
//    }

//    // Dat tan so 433MHz
//    LoRa_Write_Reg(REG_FRF_MSB, 0x6C);
//    LoRa_Write_Reg(REG_FRF_MID, 0x40);
//    LoRa_Write_Reg(REG_FRF_LSB, 0x00);

//    // Kich cong suat phat (Tuy chon)
//    LoRa_Write_Reg(REG_PA_CONFIG, 0xFF); 

//    // Dong bo hoa voi ESP32 (Gia tri 0xF1)
//    LoRa_Write_Reg(REG_SYNC_WORD, 0xF1);

//    // Chuyen ve Standby Mode de san sang hoat dong
//    LoRa_Write_Reg(REG_OP_MODE, 0x81);
//    return 1;
//}

//// --- HAM GUI CHUOI KY TU ---
//void LoRa_SendString(char* str) {
//    uint8_t len = strlen(str);
//    LoRa_Write_Reg(REG_OP_MODE, 0x81); // Standby
//    LoRa_Write_Reg(REG_FIFO_ADDR_PTR, LoRa_Read_Reg(REG_FIFO_TX_BASE));
//    LoRa_Write_Reg(REG_PAYLOAD_LENGTH, len);
//    for (uint8_t i = 0; i < len; i++) {
//        LoRa_Write_Reg(REG_FIFO, (uint8_t)str[i]);
//    }
//    LoRa_Write_Reg(REG_OP_MODE, 0x83); // TX Mode
//    
//    // Cho den khi phat xong
//    while ((LoRa_Read_Reg(REG_IRQ_FLAGS) & 0x08) == 0);
//    
//    // Xoa co ngat
//    LoRa_Write_Reg(REG_IRQ_FLAGS, 0x08);
//}



#include "lora.h"
#include "delay.h"
#include <string.h>

// Dinh nghia chan Reset PA3
#define LORA_RST_PORT  GPIOA
#define LORA_RST_PIN   GPIO_Pin_3

// --- HAM KHOI TAO SPI ---
void LoRa_SPI_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;

    RCC_PCLK2Config(RCC_HCLK_Div8); // Chia tan so clock PCLK2 xuong 4MHz

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_SPI1, ENABLE);

    // PA5: SCK, PA7: MOSI
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // PA6: MISO
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // PA4: NSS (Soft CS) - Phai keo len CAO ngay lap tuc
    GPIO_InitStructure.GPIO_Pin = LORA_NSS_PIN; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(LORA_NSS_PORT, &GPIO_InitStructure);
    GPIO_SetBits(LORA_NSS_PORT, LORA_NSS_PIN); // MAC DINH LA CAO (Khong chon chip)

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128; 
    
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_Init(SPI1, &SPI_InitStructure);
    SPI_Cmd(SPI1, ENABLE);
}

// --- HAM RESET ---
void LoRa_Reset(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = LORA_RST_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LORA_RST_PORT, &GPIO_InitStructure);

    // Chu trinh keo RST: Thap (10ms) -> Cao (10ms)
    GPIO_ResetBits(LORA_RST_PORT, LORA_RST_PIN);
    Delay_Ms(20);
    GPIO_SetBits(LORA_RST_PORT, LORA_RST_PIN);
    Delay_Ms(20);
}

// --- HAM TRUYEN NHAN SPI NOI BO ---
static uint8_t SPI_Transfer(uint8_t data) {
    uint16_t timeout = 1000;  // Arbitrary timeout value
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET && --timeout);
    if (timeout == 0) return 0xFF;  // Error indicator
    SPI_I2S_SendData(SPI1, data);
    
    timeout = 1000;
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET && --timeout);
    if (timeout == 0) return 0xFF;
    return SPI_I2S_ReceiveData(SPI1);
}

// --- H�M GHI THANH GHI (WRITE) ---
void LoRa_Write_Reg(uint8_t reg, uint8_t data) {
    GPIO_ResetBits(LORA_NSS_PORT, LORA_NSS_PIN); // Ch?n chip
    
    // T?o d? tr? nh? d? NSS ?n d?nh
    for(volatile int i = 0; i < 200; i++); 

    // ��NG: Ghi th� bit 7 ph?i l� 1 (d�ng | 0x80)
    SPI_Transfer(reg | 0x80); 
    SPI_Transfer(data);

    GPIO_SetBits(LORA_NSS_PORT, LORA_NSS_PIN); // B? ch?n chip
}

// --- H�M �?C THANH GHI (READ) ---
uint8_t LoRa_Read_Reg(uint8_t reg) {
    uint8_t val;

    GPIO_ResetBits(LORA_NSS_PORT, LORA_NSS_PIN);
    Delay_Ms(1);

    SPI_Transfer(reg & 0x7F);
    val = SPI_Transfer(0x00);

    Delay_Ms(1);
    GPIO_SetBits(LORA_NSS_PORT, LORA_NSS_PIN);

    return val;
}

// --- HAM KHOI TAO CHINH ---
uint8_t LoRa_Init(void) {
    // 1. KHOI TAO SPI TRUOC DE CHAN NSS ON DINH
    LoRa_SPI_Init(); 
    
    // 2. RESET MODULE
    LoRa_Reset();    
    
    // 3. CAU HINH MODULE
    LoRa_Write_Reg(REG_OP_MODE, 0x00); // Sleep mode
    Delay_Ms(10);
    LoRa_Write_Reg(REG_OP_MODE, 0x80); // Chuyen sang LoRa Mode
    Delay_Ms(10);
    
    // Kiem tra Version
    if (LoRa_Read_Reg(REG_VERSION) != 0x12) {
        return 0; // That bai
    }

    // Dat tan so 433MHz
    LoRa_Write_Reg(REG_FRF_MSB, 0x6C);
    LoRa_Write_Reg(REG_FRF_MID, 0x40);
    LoRa_Write_Reg(REG_FRF_LSB, 0x00);

    // Kich cong suat phat (Tuy chon)
    LoRa_Write_Reg(REG_PA_CONFIG, 0xFF); 

    // Dong bo hoa voi ESP32 (Gia tri 0xF1)
    LoRa_Write_Reg(REG_SYNC_WORD, 0xF1);

    // Chuyen ve Standby Mode de san sang hoat dong
    LoRa_Write_Reg(REG_OP_MODE, 0x81);
    return 1;
}

// --- HAM GUI CHUOI KY TU ---
void LoRa_SendString(char* str) {
    uint8_t len = strlen(str);
    LoRa_Write_Reg(REG_OP_MODE, 0x81); // Standby
    LoRa_Write_Reg(REG_FIFO_ADDR_PTR, LoRa_Read_Reg(REG_FIFO_TX_BASE));
    LoRa_Write_Reg(REG_PAYLOAD_LENGTH, len);
    for (uint8_t i = 0; i < len; i++) {
        LoRa_Write_Reg(REG_FIFO, (uint8_t)str[i]);
    }
    LoRa_Write_Reg(REG_OP_MODE, 0x83); // TX Mode
    
    // Cho den khi phat xong
    while ((LoRa_Read_Reg(REG_IRQ_FLAGS) & 0x08) == 0);
    
    // Xoa co ngat
    LoRa_Write_Reg(REG_IRQ_FLAGS, 0x08);
}




