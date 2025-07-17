#ifndef __HAL_SPI_H_
#define __HAL_SPI_H_



void Hal_SPI_Init(void);
void Hal_SPI2_CSDriver(uint8_t state);
uint8_t Hal_SPI2_ReadWriteByte(uint8_t TxData);

#endif
