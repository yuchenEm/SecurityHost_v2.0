#ifndef __MID_FLASH_H_
#define __MID_FLASH_H_

/** W25Q64: 128 Blocks
  *		    16 Sectors per Block
  *			16 Pages per Sector
  *		    	4 KB per Sector
  *				256 Byte per Page
  *		64 KB * 128 Blocks = 8192/1024 = 8 MB
  */
#define DUMMY						      0xFF       // Dummy value
#define FLASH_SECTOR_SIZE			4096       // 4*1024 byte per Sector
#define FLASH_PAGE_SIZE				256	       // 256 byte per Page

/* Instructions of W25Q64-Standard SPI Instruction */
#define	WRITE_ENABLE				      0x06
#define VOLATILE_SR_WRITE_ENABLE	0x50
#define WRITE_DISABLE				      0x04

#define RELEASE_POWER_DOWN			  0xAB
#define DEVICE_ID					        0xAB
#define MANUFACTURER_ID				    0x90
#define JEDEC_ID					        0x9F
#define READ_UNIQUE_ID				    0x4B

#define READ_DATA					        0x03
#define FAST_READ					        0x0B

#define PAGE_PROGRAM				      0x02
#define SECTOR_ERASE_4KB			    0x20
#define BLOCK_ERASE_32KB			    0x52
#define BLOCK_ERASE_64KB			    0xD8
#define CHIP_ERASE					      0xC7 //or 0x60

#define READ_STATUS_REGISTER_1		0x05
#define WRITE_STATUS_REGISTER_1		0x01
#define READ_STATUS_REGISTER_2		0x35
#define WRITE_STATUS_REGISTER_2		0x31
#define READ_STATUS_REGISTER_3		0x15
#define WRITE_STATUS_REGISTER_3		0x11

#define READ_SFDP_REGISTER			  0x5A
#define ERASE_SECURITY_REGISTER		0x44
#define PROGRAM_SECURITY_REGISTER	0x42
#define READ_SECURITY_REGISTER		0x48

#define GLOBAL_BLOCK_LOCK			    0x7E
#define GLOBAL_BLOCK_UNLOCK			  0x98
#define READ_BLOCK_LOCK				    0x3D
#define INDIVIDUAL_BLOCK_LOCK		  0x36
#define INDIVIDUAL_BLOCK_UNLOCK		0x39

#define ERASE_PROGRAM_SUSPEND		  0x75
#define ERASE_PROGRAM_RESUME		  0x7A
#define POWER_DOWN					      0xB9

#define ENABLE_RESET				      0x66
#define RESET_DEVICE				      0x99


void Mid_Flash_Init(void);

uint16_t Mid_Flash_ReadManufacturerID(void);

void Mid_Flash_WritePage(uint8_t *pBuffer, uint32_t Addr, uint16_t Num);
void Mid_Flash_WriteSector(uint8_t *pBuffer, uint32_t Addr, uint16_t Num);
void Mid_Flash_EraseSector(uint32_t Addr);

void Mid_Flash_ReadData(uint8_t *pBuffer, uint32_t Addr, uint16_t Num);
void Mid_Flash_WriteData(uint8_t *pBuffer, uint32_t Addr, uint16_t Num);

#endif
