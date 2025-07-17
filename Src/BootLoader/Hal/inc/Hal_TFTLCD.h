#ifndef __HAL_TFTLCD_H_
#define __HAL_TFTLCD_H_

/* TFTLCD Pinout control function Macro define */
#define Hal_TFTLCD_SCLK_Clear()		GPIO_ResetBits(TFTLCD_CLK_PORT, TFTLCD_CLK_PIN)
#define Hal_TFTLCD_SCLK_Set()		GPIO_SetBits(TFTLCD_CLK_PORT, TFTLCD_CLK_PIN)

#define Hal_TFTLCD_MOSI_Clear()		GPIO_ResetBits(TFTLCD_DIO_PORT, TFTLCD_DIO_PIN)
#define Hal_TFTLCD_MOSI_Set()		GPIO_SetBits(TFTLCD_DIO_PORT, TFTLCD_DIO_PIN)

#define Hal_TFTLCD_CMD_Clear()		GPIO_ResetBits(TFTLCD_CMD_PORT, TFTLCD_CMD_PIN)
#define Hal_TFTLCD_CMD_Set()		GPIO_SetBits(TFTLCD_CMD_PORT, TFTLCD_CMD_PIN)

#define Hal_TFTLCD_CS_Clear()		GPIO_ResetBits(TFTLCD_CS_PORT, TFTLCD_CS_PIN)
#define Hal_TFTLCD_CS_Set()			GPIO_SetBits(TFTLCD_CS_PORT, TFTLCD_CS_PIN)

#define Hal_TFTLCD_RES_Clear()		GPIO_ResetBits(TFTLCD_RES_PORT, TFTLCD_RES_PIN)
#define Hal_TFTLCD_RES_Set()		GPIO_SetBits(TFTLCD_RES_PORT, TFTLCD_RES_PIN)

#define Hal_TFTLCD_LEDA_Clear()		GPIO_ResetBits(TFTLCD_LEDA_EN_PORT, TFTLCD_LEDA_EN_PIN)
#define Hal_TFTLCD_LEDA_Set()		GPIO_SetBits(TFTLCD_LEDA_EN_PORT, TFTLCD_LEDA_EN_PIN)



void Hal_TFTLCD_WriteBus(uint8_t Data);

void Hal_TFTLCD_Display_On(void);
void Hal_TFTLCD_Display_Off(void);

void Hal_TFTLCD_Rest_High(void);
void Hal_TFTLCD_Rest_Low(void);

void Hal_TFTLCD_Write_Data8(uint8_t Data);
void Hal_TFTLCD_Write_Data(uint16_t Data);
void Hal_TFTLCD_Write_Register(uint8_t Data);


#endif
