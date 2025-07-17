/****************************************************
  * @Name	Hal_SPI.c
  * @Brief	Config SPI
  ***************************************************/

/*-------------Header Files Include-----------------*/
#include "stm32f10x.h"                 
#include "hal_spi.h"
#include "hal_gpio.h"

/*-------------Internal Functions Declaration------*/
static void Hal_SPI2_Config(void);
static void Hal_SPI3_Config(void);

/*-------------Module Variables Declaration--------*/


/*-------------Module Functions Definition---------*/
/**
  * @Brief	Initialize SPI2 module
  * @Param	None
  * @Retval	None
  */
void Hal_SPI_Init(void)
{
	Hal_SPI2_Config();
	Hal_SPI3_Config();
	
}

/**
  * @Brief	SPI2 CS state driver
  * @Param	state
  *		1: CS pull high
  *		0: CS pull low
  * @Retval	None
  */
void Hal_SPI2_CSDriver(uint8_t state)
{
	if(state)
	{
		GPIO_SetBits(SPI2_NSS_PORT, SPI2_NSS_PIN);
	}
	else
	{
		GPIO_ResetBits(SPI2_NSS_PORT, SPI2_NSS_PIN);
	}
}

/**
  * @Brief	SPI2 read/write data
  * @Param	TxData
  *		data to send
  * @Retval	Received data
  */
uint8_t Hal_SPI2_ReadWriteByte(uint8_t TxData)
{
	uint8_t retry = 0;
	
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET)
	{
		retry++;
		
		if(retry > 200)
		{
			return 0; // timeout
		}
	}
	
	SPI_I2S_SendData(SPI2, TxData);
	
	retry = 0;
	
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET)
	{
		retry++;
		
		if(retry > 200)
		{
			return 0;
		}
	}
	
	return SPI_I2S_ReceiveData(SPI2);
}


/*-------------Internal Functions Definition--------*/
/**
  * @Brief	Config SPI2
  * @Param	None
  * @Retval	None
  */
static void Hal_SPI2_Config(void)
{
	SPI_InitTypeDef SPI_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; // transmission speed = 36MHz/8 
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b; // 8 bit
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_Init(SPI2, &SPI_InitStructure);
	
	SPI_Cmd(SPI2, ENABLE);

	Hal_SPI2_CSDriver(1);
}

/**
  * @Brief	Config SPI3
  * @Param	None
  * @Retval	None
  */
static void Hal_SPI3_Config(void)
{
	SPI_InitTypeDef SPI_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
	
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_Init(SPI3, &SPI_InitStructure);
	
	SPI_I2S_DMACmd(SPI3, SPI_I2S_DMAReq_Tx, ENABLE);
	
	SPI_Cmd(SPI3, ENABLE);
}



/*-------------Interrupt Functions Definition--------*/


