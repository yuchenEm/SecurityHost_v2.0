/****************************************************
  * @Name	Hal_EmbeddedFlash.c
  * @Brief	
  ***************************************************/
  
/*-------------Header Files Include-----------------*/
#include "stm32f10x.h" 
#include "hal_embeddedflash.h"

/*-------------Internal Functions Declaration------*/


/*-------------Module Variables Declaration--------*/


/*---Module Call-Back function pointer Definition---*/


/*-------------Module Functions Definition---------*/
/**
  * @Brief	Programs a half word at a specified address
  * @Param	Address	: specifies the address to be programmed
  *  		Data	: specifies the data to be programmed
  * @Retval	None
  */
void Hal_EmbeddedFlash_ProgramHalfWord(uint32_t Address, uint16_t Data)
{
	FLASH_ProgramHalfWord(Address, Data);
}

/**
  * @Brief	Locks the FLASH Program Erase Controller
  * @Param	None
  * @Retval	None
  */
void Hal_EmbeddedFlash_Lock(void)
{
	FLASH_Lock();
}

/**
  * @Brief	Unlocks the FLASH Program Erase Controller
  * @Param	None
  * @Retval	None
  */
void Hal_EmbeddedFlash_Unlock(void)
{
	FLASH_Unlock();
}

/**
  * @Brief	Erases a specified FLASH page
  * @Param	PageAddress: The page address to be erased
  * @Retval	None
  */
void Hal_EmbeddedFlash_EarsePage(uint32_t PageAddress)
{
	FLASH_ErasePage(PageAddress);
}

/*-------------Internal Functions Definition--------*/


/*-------------Interrupt Functions Definition--------*/


