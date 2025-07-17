#ifndef __MID_EMBEDDEDFLASH_H_
#define __MID_EMBEDDEDFLASH_H_

/* STM32F105RC EmbeddedFlash size(in KByte) */
#define EMBEDDED_FLASH_SIZE				256

/* STM32F105RC EmbeddedFlash Sector size macro(uncomment the target size) */
/* For STM32F105 series
	Flash size >= 128KB -> page size = 2048
	Flash size < 128KB -> page size = 1024
*/
#define EMBEDDED_FLASH_SECTOR_SIZE		2048
//#define EMBEDDED_FLASH_SECTOR_SIZE	1024

/* Base-address of the embedded Flash */
#define EMBEDDED_FLASH_Address_Base		0x08000000

uint16_t 	Mid_EmbeddedFlash_ReadHalfWord(uint32_t Address);
void 		Mid_EmbeddedFlash_WriteHalfWord_NoCheck(uint32_t Address, uint16_t *pData, uint16_t Number);
void 		Mid_EmbeddedFlash_ReadHalfWord_Sequence(uint32_t Address, uint16_t *pData, uint16_t Number);
void 		Mid_EmbeddedFlash_WriteHalfWord(uint32_t Address, uint16_t *pData, uint16_t Number);


#endif
