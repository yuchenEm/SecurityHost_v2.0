/****************************************************
  * @Name	Hal_DMA.c
  * @Brief	Config DMA
  ***************************************************/

/*-------------Header Files Include-----------------*/
#include "stm32f10x.h"                
#include "hal_dma.h"
#include "hal_gpio.h"

/*-------------Internal Functions Declaration------*/
static void Hal_DMA2_Config(void);	// RAM --> SPI3_TX(TFTLCD)
static void Hal_DMA1_Config(void);	// RAM --> USART1_Tx(Debug_USART)

/*-------------Module Variables Declaration--------*/


/*-------------Module Functions Definition---------*/
/**
  * @Brief	Initialize DMA module
  * @Param	None
  * @Retval	None
  */
void Hal_DMA_Init(void)
{
	Hal_DMA2_Config();
	Hal_DMA1_Config();
}

/**
  * @Brief	DMA_SPI3 transmit function
  * @Param	pBuffer: pointer to the RAM buffer
  *			Len: data length
  * @Retval	None
  */
/* Config Register */
void Hal_DMA_SPI3Tx_Reg(uint8_t *pBuffer, uint16_t Len)
{
	DMA2->IFCR |= (0xF0); 										// clear DMA_ISR of DMA_channel2
	DMA2_Channel2->CNDTR = Len;								// number of data to transfer
	DMA2_Channel2->CMAR = (uint32_t)pBuffer;	// memory address of DMA2_channel2
	DMA2_Channel2->CCR |= 0x01; 							// enable DMA_channel2
	
	// wait until transfer finish(TCIF2=1)
	while(!(DMA2->ISR & (1<<5)));							// DMA_channel2 transfer complete flag=1 --> transfer complete
	
	DMA2_Channel2->CCR &= (uint32_t)(~0x01);	// disable DMA2_channel2
}

/* Config stdlib */
void Hal_DMA_SPI3Tx_Stdlib(uint8_t *pBuffer, uint16_t Len)
{
	DMA_ClearITPendingBit(DMA2_IT_GL2);
	DMA_SetCurrDataCounter(DMA2_Channel2, Len);
	DMA2_Channel2->CMAR = (uint32_t)pBuffer;
	DMA_Cmd(DMA2_Channel2, ENABLE);
	
	while(!DMA_GetITStatus(DMA2_IT_TC2));
	
	DMA_Cmd(DMA2_Channel2, DISABLE);
}

/**
  * @Brief	DMA_USART1 transfer function
  * @Param	pBuffer: pointer to the RAM buffer
  *			Len: data length
  * @Retval	None
  */
/* Config stdlib */
void Hal_DMA_USART1Tx_Stdlib(uint8_t *pBuffer, uint16_t Len)
{
	DMA_Cmd(DMA1_Channel4, DISABLE);
	
	DMA_SetCurrDataCounter(DMA1_Channel4, Len);
	DMA1_Channel4->CMAR = (uint32_t)pBuffer;		// set starting memory address for DMA1_channel4
	
	DMA_Cmd(DMA1_Channel4, ENABLE);	
}


/*-------------Internal Functions Definition--------*/
/**
  * @Brief	DMA2 config
  * @Param	None
  * @Retval	None
  */
static void Hal_DMA2_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
	
	DMA_DeInit(DMA2_Channel2);	// DMA2_Channel2 --> SPI3_TX
	
	DMA_InitStructure.DMA_BufferSize = BUFFER_SPI3_TX_SIZE;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
//	DMA_InitStructure.DMA_MemoryBaseAddr = 
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &SPI3->DR;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	DMA_Init(DMA2_Channel2, &DMA_InitStructure);
	
}

/**
  * @Brief	DMA1 config
  * @Param	None
  * @Retval	None
  */
static void Hal_DMA1_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
	DMA_DeInit(DMA1_Channel4);	// USART1_Tx
	
	DMA_InitStructure.DMA_BufferSize = BUFFER_DEBUG_TX_SIZE;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
//	DMA_InitStructure.DMA_MemoryBaseAddr = 
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &USART1->DR;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_Init(DMA1_Channel4, &DMA_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStructure);
	
	DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);	
}

/*-------------Interrupt Functions Definition--------*/


