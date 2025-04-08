#include"dht11.h"
#include"delay.h"
void DHT11_Init(void){

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);//chan PB12 DE NHAN DU LIEU DHT11
	GPIO_InitTypeDef gpioInit;
	gpioInit.GPIO_Mode = GPIO_Mode_Out_OD;
	gpioInit.GPIO_Pin = GPIO_Pin_12;
	gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOB, &gpioInit);
	GPIO_SetBits (GPIOB,GPIO_Pin_12); //giu o muc cao	
}
void DHT11_Check(void){
    uint16_t u16Tim;
	uint8_t u8Buff[5];
	uint8_t u8CheckSum;
	uint8_t i;
    GPIO_ResetBits(GPIOB, GPIO_Pin_12);
    Delay_Ms(20);
    GPIO_SetBits(GPIOB, GPIO_Pin_12);
    
    /* cho chan PB12 len cao */
    TIM_SetCounter(TIM2, 0);
    while (TIM_GetCounter(TIM2) < 10) {
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) {
            break;
        }
    }
    u16Tim = TIM_GetCounter(TIM2);
    if (u16Tim >= 10) {
        while (1) {
        }
    }
    
    /* cho chan PB12 xuong thap */
    TIM_SetCounter(TIM2, 0);
    while (TIM_GetCounter(TIM2) < 45) {
        if (!GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) {
            break;
        }
    }
    u16Tim = TIM_GetCounter(TIM2);
    if ((u16Tim >= 45) || (u16Tim <= 5)) {
        while (1) {
        }
    }
    
    /* cho chan PB12 len cao */
    TIM_SetCounter(TIM2, 0);
    while (TIM_GetCounter(TIM2) < 90) {
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) {
            break;
        }
    }
    u16Tim = TIM_GetCounter(TIM2);
    if ((u16Tim >= 90) || (u16Tim <= 70)) {
        while (1) {
        }
    }
    
    /* cho chan PB12 xuong thap */
    TIM_SetCounter(TIM2, 0);
    while (TIM_GetCounter(TIM2) < 95) {
        if (!GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) {
            break;
        }
    }
    u16Tim = TIM_GetCounter(TIM2);
    if ((u16Tim >= 95) || (u16Tim <= 75)) {
        while (1) {
        }
    }
}

char* DHT11_Data(void){
    uint16_t u16Tim;
	uint8_t u8Buff[5];
	uint8_t u8CheckSum;
	uint8_t i;
    uint8_t buffer[50];
    /* nhan byte so 1 */
		for (i = 0; i < 8; ++i) {
			/* cho chan PB12 len cao */
			TIM_SetCounter(TIM2, 0);
			while (TIM_GetCounter(TIM2) < 65) {
				if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) {
					break;
				}
			}
			u16Tim = TIM_GetCounter(TIM2);
			if ((u16Tim >= 65) || (u16Tim <= 45)) {
				while (1) {
				}
			}
			
			/* cho chan PB12 xuong thap */
			TIM_SetCounter(TIM2, 0);
			while (TIM_GetCounter(TIM2) < 80) {
				if (!GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) {
					break;
				}
			}
			u16Tim = TIM_GetCounter(TIM2);
			if ((u16Tim >= 80) || (u16Tim <= 10)) {
				while (1) {
				}
			}
			u8Buff[0] <<= 1;
			if (u16Tim > 45) {
				/* nhan duoc bit 1 */
				u8Buff[0] |= 1;
			} else {
				/* nhan duoc bit 0 */
				u8Buff[0] &= ~1;
			}
		}
		
		/* nhan byte so 2 */
		for (i = 0; i < 8; ++i) {
			/* cho chan PB12 len cao */
			TIM_SetCounter(TIM2, 0);
			while (TIM_GetCounter(TIM2) < 65) {
				if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) {
					break;
				}
			}
			u16Tim = TIM_GetCounter(TIM2);
			if ((u16Tim >= 65) || (u16Tim <= 45)) {
				while (1) {
				}
			}
			
			/* cho chan PB12 xuong thap */
			TIM_SetCounter(TIM2, 0);
			while (TIM_GetCounter(TIM2) < 80) {
				if (!GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) {
					break;
				}
			}
			u16Tim = TIM_GetCounter(TIM2);
			if ((u16Tim >= 80) || (u16Tim <= 10)) {
				while (1) {
				}
			}
			u8Buff[1] <<= 1;
			if (u16Tim > 45) {
				/* nhan duoc bit 1 */
				u8Buff[1] |= 1;
			} else {
				/* nhan duoc bit 0 */
				u8Buff[1] &= ~1;
			}
		}
		
		/* nhan byte so 3 */
		for (i = 0; i < 8; ++i) {
			/* cho chan PB12 len cao */
			TIM_SetCounter(TIM2, 0);
			while (TIM_GetCounter(TIM2) < 65) {
				if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) {
					break;
				}
			}
			u16Tim = TIM_GetCounter(TIM2);
			if ((u16Tim >= 65) || (u16Tim <= 45)) {
				while (1) {
				}
			}
			
			/* cho chan PB12 xuong thap */
			TIM_SetCounter(TIM2, 0);
			while (TIM_GetCounter(TIM2) < 80) {
				if (!GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) {
					break;
				}
			}
			u16Tim = TIM_GetCounter(TIM2);
			if ((u16Tim >= 80) || (u16Tim <= 10)) {
				while (1) {
				}
			}
			u8Buff[2] <<= 1;
			if (u16Tim > 45) {
				/* nhan duoc bit 1 */
				u8Buff[2] |= 1;
			} else {
				/* nhan duoc bit 0 */
				u8Buff[2] &= ~1;
			}
		}
		
		/* nhan byte so 4 */
		for (i = 0; i < 8; ++i) {
			/* cho chan PB12 len cao */
			TIM_SetCounter(TIM2, 0);
			while (TIM_GetCounter(TIM2) < 65) {
				if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) {
					break;
				}
			}
			u16Tim = TIM_GetCounter(TIM2);
			if ((u16Tim >= 65) || (u16Tim <= 45)) {
				while (1) {
				}
			}
			
			/* cho chan PB12 xuong thap */
			TIM_SetCounter(TIM2, 0);
			while (TIM_GetCounter(TIM2) < 80) {
				if (!GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) {
					break;
				}
			}
			u16Tim = TIM_GetCounter(TIM2);
			if ((u16Tim >= 80) || (u16Tim <= 10)) {
				while (1) {
				}
			}
			u8Buff[3] <<= 1;
			if (u16Tim > 45) {
				/* nhan duoc bit 1 */
				u8Buff[3] |= 1;
			} else {
				/* nhan duoc bit 0 */
				u8Buff[3] &= ~1;
			}
		}
		
		/* nhan byte so 5 */
		for (i = 0; i < 8; ++i) {
			/* cho chan PB12 len cao */
			TIM_SetCounter(TIM2, 0);
			while (TIM_GetCounter(TIM2) < 65) {
				if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) {
					break;
				}
			}
			u16Tim = TIM_GetCounter(TIM2);
			if ((u16Tim >= 65) || (u16Tim <= 45)) {
				while (1) {
				}
			}
			
			/* cho chan PB12 xuong thap */
			TIM_SetCounter(TIM2, 0);
			while (TIM_GetCounter(TIM2) < 80) {
				if (!GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) {
					break;
				}
			}
			u16Tim = TIM_GetCounter(TIM2);
			if ((u16Tim >= 80) || (u16Tim <= 10)) {
				while (1) {
				}
			}
			u8Buff[4] <<= 1;
			if (u16Tim > 45) {
				/* nhan duoc bit 1 */
				u8Buff[4] |= 1;
			} else {
				/* nhan duoc bit 0 */
				u8Buff[4] &= ~1;
			}
		}
		
		u8CheckSum = u8Buff[0] + u8Buff[1] + u8Buff[2] + u8Buff[3];
		if (u8CheckSum != u8Buff[4]) {
			while (1) {
			}
		}
        sprintf(buffer, "Nhiet do: %d.%d, Do am: %d.%d%%\r\n",
            u8Buff[2], u8Buff[3], u8Buff[0], u8Buff[1]);
            return buffer;
}