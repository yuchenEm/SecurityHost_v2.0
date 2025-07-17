/****************************************************
  * @Name	Hal_TFTLCD.c
  * @Brief	Driver of TFTLCD screen
  * @API	--> 
  *			--> 
  ***************************************************/
  
/*-------------Header Files Include-----------------*/
#include "stm32f10x.h"                 
#include "hal_tftlcd.h"
#include "hal_gpio.h"
#include "hal_dma.h"

/*-------------Internal Functions Declaration------*/


/*-------------Module Variables Declaration--------*/


/*-------------Module Functions Definition---------*/
/**
  * @Brief	Write 1 byte data to the SPI-DMA bus
  * @Param	None
  * @Retval	None
  */
void Hal_TFTLCD_WriteBus(uint8_t Data)
{
	Hal_TFTLCD_CS_Clear();
	Hal_DMA_SPI3Tx_Reg(&Data, 1);
}

/* TFTLCD Pinout control function */
void Hal_TFTLCD_Display_On(void)
{
	Hal_TFTLCD_LEDA_Set();
}

void Hal_TFTLCD_Display_Off(void)
{
	Hal_TFTLCD_LEDA_Clear();
}

void Hal_TFTLCD_Rest_High(void)
{
	Hal_TFTLCD_RES_Set();
}

void Hal_TFTLCD_Rest_Low(void)
{
	Hal_TFTLCD_RES_Clear();
}

/**
  * @Brief	Write Data to TFTLCD
  * @Param	Data: 1 byte data
  * @Retval	None
  */
void Hal_TFTLCD_Write_Data8(uint8_t Data)
{
	Hal_TFTLCD_WriteBus(Data);
}

/**
  * @Brief	Write Data to TFTLCD
  * @Param	Data: 2 byte data
  * @Retval	None
  */
void Hal_TFTLCD_Write_Data(uint16_t Data)
{
	uint8_t dat[2];
	
	dat[0] = Data >> 8;
	dat[1] = Data;
	
	Hal_DMA_SPI3Tx_Reg(&dat[0], 2);
}

/**
  * @Brief	Send CMD to TFTLCD
  * @Param	Data: 1 byte data
  * @Retval	None
  */
void Hal_TFTLCD_Write_Register(uint8_t Data)
{
	Hal_TFTLCD_CMD_Clear();
	
	Hal_TFTLCD_WriteBus(Data);
	
	Hal_TFTLCD_CMD_Set();
}

/*-------------Internal Functions Definition--------*/


/*-------------Interrupt Functions Definition--------*/


