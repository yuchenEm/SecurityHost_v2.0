#ifndef __HAL_DMA_H_
#define __HAL_DMA_H_

/* SPI3_Tx buffer size */
#define BUFFER_SPI3_TX_SIZE		1024

/* Debug_USART_Tx buffer size */
#define BUFFER_DEBUG_TX_SIZE	512


void Hal_DMA_Init(void);
void Hal_DMA_SPI3Tx_Reg(uint8_t *pBuffer, uint16_t Len);
void Hal_DMA_SPI3Tx_Stdlib(uint8_t *pBuffer, uint16_t Len);

void Hal_DMA_USART1Tx_Stdlib(uint8_t *pBuffer, uint16_t Len);

#endif
