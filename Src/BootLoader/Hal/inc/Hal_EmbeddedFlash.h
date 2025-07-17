#ifndef __HAL_EMBEDDEDFLASH_H_
#define __HAL_EMBEDDEDFLASH_H_

void Hal_EmbeddedFlash_ProgramHalfWord(uint32_t Address, uint16_t Data);
void Hal_EmbeddedFlash_Lock(void);
void Hal_EmbeddedFlash_Unlock(void);
void Hal_EmbeddedFlash_EarsePage(uint32_t PageAddress);

#endif
