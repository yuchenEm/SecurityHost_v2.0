/****************************************************
  * @Name	Mid_Flsh.c
  * @Brief	Driver of W25Q64 flash
  * @API	--> Mid_Flash_ReadData
  *			--> Mid_Flash_WriteData
  ***************************************************/

/*-------------Header Files Include-----------------*/
#include "stm32f10x.h"
#include "mid_flash.h"
#include "hal_spi.h"


/*-------------Internal Functions Declaration------*/
static void 	Mid_Flash_WriteEnable(void);
static uint8_t 	Mid_Flash_ReadSR1(void);
static void 	Mid_Flash_WaitForIdle(void);
//static void 	Mid_Flash_Debug(void);

/*-------------Module Variables Declaration--------*/



/*-------------Module Functions Definition---------*/
/**
  * @Brief	Initialize Flash module
  * @Param	None
  * @Retval	None
  */
void Mid_Flash_Init(void)
{
	
}

/**
  * @Brief	Read ManufacturerID from chip
  * @Param	None
  * @Retval	ManufacturerID
  */
uint16_t Mid_Flash_ReadManufacturerID(void)
{
	uint16_t Temp = 0;
	
	Hal_SPI2_CSDriver(0); // SPI communication start
	
	Hal_SPI2_ReadWriteByte(MANUFACTURER_ID);
	Hal_SPI2_ReadWriteByte(DUMMY);
	Hal_SPI2_ReadWriteByte(DUMMY);
	Hal_SPI2_ReadWriteByte(0x00);
	Temp |= Hal_SPI2_ReadWriteByte(DUMMY) << 8; // High byte
	Temp |= Hal_SPI2_ReadWriteByte(DUMMY);			// Low byte
	
	Hal_SPI2_CSDriver(1); // SPI communication end
	
	return Temp;
}

/**
  * @Brief	Read data from Flash chip
  * @Param	pBuffer: pointer to the address of stored data 
  * 		Addr: the starting address of data (3 bytes)
  * 		Num: the number of bytes to read
  * @Retval	None
  */
void Mid_Flash_ReadData(uint8_t *pBuffer, uint32_t Addr, uint16_t Num)
{
	uint16_t i;
	
	Hal_SPI2_CSDriver(0);
	
	Hal_SPI2_ReadWriteByte(READ_DATA);
	Hal_SPI2_ReadWriteByte((uint8_t)(Addr >> 16));	// High 8 bit address
	Hal_SPI2_ReadWriteByte((uint8_t)(Addr >> 8));		// Middle 8 bit address
	Hal_SPI2_ReadWriteByte((uint8_t)(Addr));				// Low 8 bit address
	
	for(i=0; i<Num; i++)
	{
		pBuffer[i] = Hal_SPI2_ReadWriteByte(DUMMY);
	}
	
	Hal_SPI2_CSDriver(1);
}

/**
  * @Brief	Write Page data to Flash
  * @Param	pBuffer: pointer to the address of data to write in
  * 		Addr: the starting address of data to write in(3 bytes)
  * 		Num: the number of bytes to write(0-256)
  * @Note	Exceeding data will wrap to the beginning of the page
  * @Retval	None
  */
void Mid_Flash_WritePage(uint8_t *pBuffer, uint32_t Addr, uint16_t Num)
{
	uint16_t i;
	
	Mid_Flash_WriteEnable();
	
	Hal_SPI2_CSDriver(0);
	
	Hal_SPI2_ReadWriteByte(PAGE_PROGRAM);
	Hal_SPI2_ReadWriteByte((uint8_t)(Addr >> 16));	// High 8 bit address
	Hal_SPI2_ReadWriteByte((uint8_t)(Addr >> 8));		// Middle 8 bit address
	Hal_SPI2_ReadWriteByte((uint8_t)(Addr));				// Low 8 bit address
	
	for(i=0; i<Num; i++)
	{
		Hal_SPI2_ReadWriteByte(pBuffer[i]);
	}
	
	Hal_SPI2_CSDriver(1);
	
	Mid_Flash_WaitForIdle();	// wait for Flash finish writing
}

/**
  * @Brief	Write Sector data to Flash
  * @Param	pBuffer: pointer to the address of data to write in
  * 		Addr: the starting address of data to write in(3 bytes)
  * 		Num: the number of bytes to write
  * @Note	Exceeding data will continue to the next page
  * @Retval	None
  */
void Mid_Flash_WriteSector(uint8_t *pBuffer, uint32_t Addr, uint16_t Num)
{
	uint16_t PageRemainByte;
	uint8_t *pBuff;
	uint32_t StartAddr;
	
	pBuff = pBuffer;
	StartAddr = Addr;
	
	PageRemainByte = FLASH_PAGE_SIZE - (StartAddr % FLASH_PAGE_SIZE);
	
	if(Num <= PageRemainByte)
	{
		PageRemainByte = Num;
	}
	
	while(1)
	{
		Mid_Flash_WritePage(pBuff, StartAddr, PageRemainByte);
		
		if(Num == PageRemainByte)
		{
			break;
		}
		else
		{
			pBuff += PageRemainByte;
			StartAddr += PageRemainByte;
			Num -= PageRemainByte;
			
			if(Num > FLASH_PAGE_SIZE)
			{
				PageRemainByte = FLASH_PAGE_SIZE;
			}
			else
			{
				PageRemainByte = Num;
			}
		}
	}	
}

/**
  * @Brief	Write data to Flash(unlimited, auto erase before write in)
  * @Param	pBuffer: pointer to the address of data to write in
  * 		Addr: the starting address of data to write in(3 bytes)
  * 		Num: the number of bytes to write
  * @Note	Erase Sector data before write in
  * @Retval	None
  */
void Mid_Flash_WriteData(uint8_t *pBuffer, uint32_t Addr, uint16_t Num)
{
	uint8_t Flash_SectorBuffer[FLASH_SECTOR_SIZE]; // sector data backup buffer
	
	uint16_t i;
	
	uint8_t *pBuff;
	uint32_t SectorIndex;
	uint16_t SectorOffset;
	uint16_t SectorRemainByte;
	uint32_t StartAddr;
	
	pBuff = pBuffer;
	StartAddr = Addr;
	
	SectorIndex = StartAddr / FLASH_SECTOR_SIZE;
	SectorOffset = StartAddr % FLASH_SECTOR_SIZE;
	SectorRemainByte = FLASH_SECTOR_SIZE - SectorOffset;
	
	if(Num <= SectorRemainByte)
	{
		SectorRemainByte = Num;
	}
	
	while(1)
	{
		Mid_Flash_ReadData(Flash_SectorBuffer, SectorIndex * FLASH_SECTOR_SIZE, FLASH_SECTOR_SIZE);
		
		Mid_Flash_EraseSector(SectorIndex);
		
		for(i=0; i<SectorRemainByte; i++)
		{
			Flash_SectorBuffer[i + SectorOffset] = pBuff[i];
		}
		
		Mid_Flash_WriteSector(Flash_SectorBuffer, SectorIndex * FLASH_SECTOR_SIZE, FLASH_SECTOR_SIZE);
		
		if(Num == SectorRemainByte)
		{
			break;
		}
		else
		{
			SectorIndex++;
			SectorOffset = 0;
			
			pBuff += SectorRemainByte;
			StartAddr += SectorRemainByte;
			Num -= SectorRemainByte;
			
			if(Num > FLASH_SECTOR_SIZE)
			{
				SectorRemainByte = FLASH_SECTOR_SIZE;
			}
			else
			{
				SectorRemainByte = Num;
			}
		}
	}
}

/**
  * @Brief	Erase Sector data
  * @Param	SectorNo: the index of target sector to be erased
  * @Note	
  * @Retval	None
  */
void Mid_Flash_EraseSector(uint32_t SectorNo)
{
	uint32_t Addr = SectorNo * FLASH_SECTOR_SIZE;
	
	Mid_Flash_WriteEnable();
	
	Hal_SPI2_CSDriver(0);
	
	Hal_SPI2_ReadWriteByte(SECTOR_ERASE_4KB);
	Hal_SPI2_ReadWriteByte((uint8_t)(Addr >> 16));	
	Hal_SPI2_ReadWriteByte((uint8_t)(Addr >> 8));	
	Hal_SPI2_ReadWriteByte((uint8_t)(Addr));
	
	Hal_SPI2_CSDriver(1);
	
	Mid_Flash_WaitForIdle();
}


/*-------------Internal Functions Definition--------*/
/**
  * @Brief	Flash write enable 
  * @Param	None
  * @Retval	None
  */
static void Mid_Flash_WriteEnable(void)
{
	Hal_SPI2_CSDriver(0);
	
	Hal_SPI2_ReadWriteByte(WRITE_ENABLE);
	
	Hal_SPI2_CSDriver(1);
}

/**
  * @Brief	Get SR1 value from Flash
  * @Param	None
  * @Retval	Flash Status Register-1 value
  */
static uint8_t Mid_Flash_ReadSR1(void)
{
	uint8_t SR1_Byte;
	
	Hal_SPI2_CSDriver(0);
	
	Hal_SPI2_ReadWriteByte(READ_STATUS_REGISTER_1);
	
	SR1_Byte = Hal_SPI2_ReadWriteByte(DUMMY);
	
	Hal_SPI2_CSDriver(1);
	
	return SR1_Byte;
}

/**
  * @Brief	Wait Flash clear the BUSY bit of SR1
  * @Param	None
  * @Retval	None
  */
static void Mid_Flash_WaitForIdle(void)
{
	while((Mid_Flash_ReadSR1() & 0x01) == 0x01); /*! infinite loop potential risk !*/
}



/*-------------Interrupt Functions Definition--------*/


