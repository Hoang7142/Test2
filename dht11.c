//#include "dht11.h"
//#include "delay.h"

//void DHT11_Init(void) {
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
//    GPIO_InitTypeDef gpioInit;
//    gpioInit.GPIO_Mode = GPIO_Mode_Out_OD; // Dung Open Drain de chuyen Input/Output linh hoat
//    gpioInit.GPIO_Pin = GPIO_Pin_12;
//    gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
//    
//    GPIO_Init(GPIOB, &gpioInit);
//    GPIO_SetBits(GPIOB, GPIO_Pin_12); // Keo len muc cao mac dinh
//}

//uint8_t DHT11_ReadData(uint8_t *temp, uint8_t *humi) {
//    uint8_t data[5] = {0,0,0,0,0};
//    uint16_t timeout;
//    
//    // 1. Start Signal
//    GPIO_ResetBits(GPIOB, GPIO_Pin_12);
//    Delay_Ms(20);
//    GPIO_SetBits(GPIOB, GPIO_Pin_12);
//    Delay_Us(30);
//    
//    // 2. Cho DHT11 phan hoi (Timeout tranh treo chip)
//    timeout = 0;
//    while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) {
//        if(++timeout > 500) return DHT11_ERR_NO_RESP; 
//        Delay_Us(1);
//    }
//    
//    timeout = 0;
//    while(!GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) {
//        if(++timeout > 500) return DHT11_ERR_NO_RESP;
//        Delay_Us(1);
//    }
//    
//    timeout = 0;
//    while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) {
//        if(++timeout > 500) return DHT11_ERR_NO_RESP;
//        Delay_Us(1);
//    }
//    
//    // 3. Doc 40 bit du lieu
//    for (int i = 0; i < 40; i++) {
//        timeout = 0;
//        while(!GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) {
//            if(++timeout > 500) return DHT11_ERR_NO_RESP;
//            Delay_Us(1);
//        }
//        
//        Delay_Us(40); // Cho qua thoi gian bit 0 (26-28us)
//        
//        if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) {
//            data[i/8] |= (1 << (7 - (i%8)));
//            
//            timeout = 0;
//            while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) {
//                if(++timeout > 500) return DHT11_ERR_NO_RESP;
//                Delay_Us(1);
//            }
//        }
//    }
//    
//    // 4. Kiem tra Checksum
//    if (data[4] == (data[0] + data[1] + data[2] + data[3])) {
//        *humi = data[0];
//        *temp = data[2];
//        return DHT11_OK;
//    } else {
//        return DHT11_ERR_CHKSUM;
//    }
//}
//////////////////////////////////////////////////////////////////////////////////////////////////


#include "dht11.h"
#include "delay.h"

void DHT11_Init(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    GPIO_InitTypeDef gpioInit;
    // Dung Open Drain k?t h?p tr? kéo ngoài (ho?c trong) 
    // Giúp chân có th? v?a xu?t m?c th?p v?a d?c m?c cao mà không c?n d?i Mode
    gpioInit.GPIO_Mode = GPIO_Mode_Out_OD; 
    gpioInit.GPIO_Pin = DHT11_PIN;
    gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
    
    GPIO_Init(DHT11_PORT, &gpioInit);
    GPIO_SetBits(DHT11_PORT, DHT11_PIN); // Mac dinh de muc cao
    Delay_Ms(1000); // Cho cam bien on dinh sau khi cap nguon
}

// Ham doc 1 byte du lieu tu DHT11
static uint8_t DHT11_ReadByte(void) {
    uint8_t byte = 0;
    uint16_t timeout;
    
    for (int i = 0; i < 8; i++) {
        // Cho tin hieu len cao (bat dau 1 bit)
        timeout = 0;
        while (!GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN)) {
            if (++timeout > 1000) return 0;
            Delay_Us(1);
        }
        
        Delay_Us(40); // Cho 40us de kiem tra muc logic
        
        if (GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN)) {
            byte |= (1 << (7 - i));
            
            // Cho tin hieu xuong thap lai (ket thuc bit 1)
            timeout = 0;
            while (GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN)) {
                if (++timeout > 1000) return 0;
                Delay_Us(1);
            }
        }
    }
    return byte;
}

uint8_t DHT11_ReadData(uint8_t *temp, uint8_t *humi) {
    uint8_t data[5];
    uint16_t timeout;

    // 1. Gui Start Signal
    GPIO_ResetBits(DHT11_PORT, DHT11_PIN);
    Delay_Ms(20); // Keo xuong it nhat 18ms
    GPIO_SetBits(DHT11_PORT, DHT11_PIN);
    Delay_Us(30); // Cho 20-40us

    // 2. Kiem tra phan hoi tu DHT11
    timeout = 0;
    while (GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN)) {
        if (++timeout > 200) return DHT11_ERR_NO_RESP;
        Delay_Us(1);
    }
    
    timeout = 0;
    while (!GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN)) {
        if (++timeout > 200) return DHT11_ERR_NO_RESP;
        Delay_Us(1);
    }
    
    timeout = 0;
    while (GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN)) {
        if (++timeout > 200) return DHT11_ERR_NO_RESP;
        Delay_Us(1);
    }

    // 3. Doc 5 byte du lieu (40 bit)
    for (int i = 0; i < 5; i++) {
        data[i] = DHT11_ReadByte();
    }

    // 4. Kiem tra Checksum
    if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
        *humi = data[0];
        *temp = data[2];
        return DHT11_OK;
    } else {
        return DHT11_ERR_CHKSUM;
    }
}