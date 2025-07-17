/****************************************************
  * @Name	Mid_EmbeddedFlash.c
  * @Brief	
  ***************************************************/
  
/*-------------Header Files Include-----------------*/
#include "stm32f10x.h"
#include "mid_embeddedflash.h"
#include "hal_embeddedflash.h"

/*-------------Internal Functions Declaration------*/


/*-------------Module Variables Declaration--------*/


/*---Module Call-Back function pointer Definition---*/


/*-------------Module Functions Definition---------*/
/**
  * @Brief	Read a halword data(16bits) at the specified address from EmbeddedFlash
  * @Param	Address: The address of the halfword to be read
  * @Retval	None
  *	@Note	Address must be multiples of uint 2
  */
uint16_t Mid_EmbeddedFlash_ReadHalfWord(uint32_t Address)
{
	return *(volatile uint16_t *)Address;
}

/**
* @Brief	Write-in specified number of halfwords to EmbeddedFlash without check(can only write-in Flash address without data-> 0xFFFF)
  * @Param	Address	  : The address of the halfword to write-in
  *			pData	  : point to the Data to write-in
  *			Number	  : number of halfword to write-in
  * @Retval	None
  */
void Mid_EmbeddedFlash_WriteHalfWord_NoCheck(uint32_t Address, uint16_t *pData, uint16_t Number)
{
	uint16_t i;
	
	for(i=0; i<Number; i++)
	{
		Hal_EmbeddedFlash_ProgramHalfWord(Address, pData[i]);
		
		Address += 2;
	}
}

/**
  * @Brief	Read-out specified number of halfwords from EmbeddedFlash
  * @Param	Address	  : The start address of the halfword to read
  *			pData	  : point to the Data to read
  *			Number	  : number of halfword to read
  * @Retval	None
  */
void Mid_EmbeddedFlash_ReadHalfWord_Sequence(uint32_t Address, uint16_t *pData, uint16_t Number)
{
	uint16_t i;
	
	for(i=0; i<Number; i++)
	{
		pData[i] = Mid_EmbeddedFlash_ReadHalfWord(Address);
		
		Address += 2;
	}
}

/**
  * @Brief	Write-in specified number of halfwords to EmbeddedFlash in sequence(cross Sectors)
  * @Param	Address	  : The address of the halfwords to write-in
  *			pData	  : point to the Data to write-in
  *			Number	  : number of halfword to write-in
  * @Retval	None
  *	@Note	Address must be multiples of uint 2
  */
void Mid_EmbeddedFlash_WriteHalfWord(uint32_t Address, uint16_t *pData, uint16_t Number)
{
	uint16_t i;
	
	uint16_t DataBuff[EMBEDDED_FLASH_SECTOR_SIZE / 2]; // buffer store the halfwords data
	
	uint32_t SectorIndex;
	uint16_t SectorOffset;			// offset in halfword(16bits)
	uint16_t SectorRemainHalfword;
	
	uint32_t AddressOffset;			// address offset higher than 0x08000000
	
	/* Check Write-in Address, found invalid Address return: */
	if(Address < EMBEDDED_FLASH_Address_Base || (Address >= EMBEDDED_FLASH_Address_Base + 1024 * EMBEDDED_FLASH_SIZE))
	{
		return;
	}
	
//	Hal_EmbeddedFlash_Unlock();		// unlock Flash write
	
	AddressOffset = Address - EMBEDDED_FLASH_Address_Base;
	
	SectorIndex = AddressOffset / EMBEDDED_FLASH_SECTOR_SIZE;
	SectorOffset = (AddressOffset % EMBEDDED_FLASH_SECTOR_SIZE) / 2;
	SectorRemainHalfword = EMBEDDED_FLASH_SECTOR_SIZE / 2 - SectorOffset;
	
	if(Number <= SectorRemainHalfword)
	{
		SectorRemainHalfword = Number;
	}
	
	while(1)
	{
		/* save a copy of current Sector data */
		Mid_EmbeddedFlash_ReadHalfWord_Sequence(SectorIndex * EMBEDDED_FLASH_SECTOR_SIZE + EMBEDDED_FLASH_Address_Base, &DataBuff[0], EMBEDDED_FLASH_SECTOR_SIZE / 2);
	
		/* check whether data exist on the Sector-remained space */
		for(i=0; i<SectorRemainHalfword; i++)
		{
			/* data exists, need erase the existing data before write-in */
			if(DataBuff[SectorOffset + i] != 0xFFFF)
			{
				break;
			}
		}
		
		if(i < SectorRemainHalfword)
		{
			/* erase the whole Sector */
			Hal_EmbeddedFlash_EarsePage(SectorIndex * EMBEDDED_FLASH_SECTOR_SIZE + EMBEDDED_FLASH_Address_Base);
			
			/* copy target halfword data to DataBuff from SectorOffset */
			for(i=0; i<SectorRemainHalfword; i++)
			{
				DataBuff[i + SectorOffset] = pData[i];
			}
			
			/* write-in updated Sector data to EmbeddedFlash */
			Mid_EmbeddedFlash_WriteHalfWord_NoCheck(SectorIndex * EMBEDDED_FLASH_SECTOR_SIZE + EMBEDDED_FLASH_Address_Base, &DataBuff[0], EMBEDDED_FLASH_SECTOR_SIZE / 2);
		}
		/* no existing data found on the Sector-remained space, write-in directly */
		else
		{
			Mid_EmbeddedFlash_WriteHalfWord_NoCheck(Address, &pData[0], SectorRemainHalfword);
		}
		
		/* write-in complete */
		if(Number == SectorRemainHalfword)
		{
			break;
		}
		/* write-in not complete */
		else
		{
			SectorIndex++;
			SectorOffset = 0;
			pData += SectorRemainHalfword;
			Address += SectorRemainHalfword;
			Number -= SectorRemainHalfword;
			
			if(Number > (EMBEDDED_FLASH_SECTOR_SIZE / 2))
			{
				SectorRemainHalfword = EMBEDDED_FLASH_SECTOR_SIZE / 2;
			}
			else
			{
				SectorRemainHalfword = Number;
			}
		}
	}
	
	Hal_EmbeddedFlash_Lock();	// lock Flash write
}
	

/*-------------Internal Functions Definition--------*/


/*-------------Interrupt Functions Definition--------*/


