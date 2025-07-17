/****************************************************
  * @Name	App.c
  * @Brief	

  ***************************************************/
  
/*-------------Header Files Include-----------------*/
#include "stm32f10x.h"        
#include "app.h"
#include "hal_tftlcd.h"
#include "hal_timer.h"
#include "hal_embeddedflash.h"
#include "hal_jumptoapp.h"
#include "mid_tftlcd.h"
#include "mid_flash.h"
#include "mid_embeddedflash.h"

/*-------------Internal Functions Declaration------*/

	

/*-------------Module Variables Declaration--------*/
stu_FirmwareInfo_t *pFirmwareInfo;

uint32_t WriteInFlashAddressOffset;		// Write-in address offset of EmbeddedFlash
uint32_t ReadOutFlashAddressOffset;		// Read-out address offset of ExternalFlash

uint16_t UpdateUnit_256Byte;	// use 256bytes as the counting unit of updating progress
uint16_t UpdateUint_Residue;	// remaining data that less than 256bytes
uint16_t UpdatePercentage;

uint32_t NewFirmwareSize;
uint8_t NewFirmwareVersion[2];



/*-------------Module Functions Definition---------*/
/**
  * @Brief	Initialize App module
  * @Param	None
  * @Retval	None
  */
void App_Init(void)
{
	uint8_t DataBuff[20];
	
	/* Read out the FirmwareInfo from external Flash */
	Mid_Flash_ReadData(&DataBuff[0], FLASH_ADDRESS_FIRMWARE_NEW_VERSION_FLAG, 13);
	
	pFirmwareInfo = (stu_FirmwareInfo_t *)DataBuff;
	
	/* find new Firmware is ready in external Flash */
	if(pFirmwareInfo->NewVersionFlag == FIRMWARE_NEW_VERSION_FLAG)
	{
		ReadOutFlashAddressOffset = FLASH_ADDRESS_FIRMWARE_BASE_ADDRESS;
		
		NewFirmwareSize = pFirmwareInfo->FirmwareSize[0] << 24;
		NewFirmwareSize |= pFirmwareInfo->FirmwareSize[1] << 16;
		NewFirmwareSize |= pFirmwareInfo->FirmwareSize[2] << 8;
		NewFirmwareSize |= pFirmwareInfo->FirmwareSize[3];
		
		NewFirmwareVersion[0] = pFirmwareInfo->NewVersion[0];
		NewFirmwareVersion[1] = pFirmwareInfo->NewVersion[1];
	
		UpdateUnit_256Byte = NewFirmwareSize >> 8;
		UpdateUint_Residue = NewFirmwareSize % 256;
		
		WriteInFlashAddressOffset = 0;
		UpdatePercentage = 0;
		
		Mid_TFTLCD_ScreenClear();
		Mid_TFTLCD_ShowString(60, 60, "Firmware Updating...", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		Mid_TFTLCD_ShowString(130, 120, "0%", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
	}
	/* no new Firmware detected, jump and execute the current version code */
	else
	{
		Hal_JumpToApp_Jump();
	}
}
  
/**
  * @Brief	Polling in sequence for Application layer
  * @Param	None
  * @Retval	None
  */
void App_Pro(void)
{
	uint16_t i;
	uint16_t Len;
	
	uint16_t EmbeddedFlashBuff[128];	// store halfword data
	uint8_t DataBuff[256];
	
	static uint16_t DownloadUnitNumber_256Byte = 0;
	
	if(DownloadUnitNumber_256Byte < UpdateUnit_256Byte)
	{
		/* read-out next uint data(256bytes) from external Flash */
		Mid_Flash_ReadData(&DataBuff[0], ReadOutFlashAddressOffset, 256);
		
		ReadOutFlashAddressOffset += 256;
		
		/* transfer the read-out data into halfword aligned */
		for(i=0; i<128; i++)
		{
			EmbeddedFlashBuff[i] = (DataBuff[i*2 + 1] << 8) & 0xFF00;
			EmbeddedFlashBuff[i] |= (DataBuff[i*2]) & 0xFF;
		}
		
		/* write-in the halfword aligned data into EmbeddedFlash */
		Mid_EmbeddedFlash_WriteHalfWord(EMBEDDED_FLASH_Address_APP_BASE + WriteInFlashAddressOffset, &EmbeddedFlashBuff[0], 128);
		WriteInFlashAddressOffset += 256;
		
		DownloadUnitNumber_256Byte++;
		

	}
	else if(DownloadUnitNumber_256Byte == UpdateUnit_256Byte)
	{
		/* read-out the residue data from external Flash */
		Mid_Flash_ReadData(&DataBuff[0], ReadOutFlashAddressOffset, UpdateUint_Residue);
		
		Len = UpdateUint_Residue;
		
		/* if the residue is odd, to make sure the data alignment add 1 byte(0xFF) at the end */
		if(Len % 2)
		{
			DataBuff[Len] = 0xFF;
			Len++;
		}
		
		Len /= 2;	// halfword align
		
		/* transfer the read-out data into halfword aligned */
		for(i=0; i<Len; i++)
		{
			EmbeddedFlashBuff[i] = (DataBuff[i*2 + 1] << 8) & 0xFF00;
			EmbeddedFlashBuff[i] |= (DataBuff[i*2]) & 0xFF;
		}
		
		Mid_EmbeddedFlash_WriteHalfWord(EMBEDDED_FLASH_Address_APP_BASE + WriteInFlashAddressOffset, &EmbeddedFlashBuff[0], Len);
		
		WriteInFlashAddressOffset += UpdateUint_Residue;
	
		UpdatePercentage = (WriteInFlashAddressOffset * 1000) / NewFirmwareSize;
		
		if(UpdatePercentage < 1000)
		{
			DataBuff[0] = (UpdatePercentage / 100) + '0';
			DataBuff[1] = ((UpdatePercentage % 100) / 10) + '0';
			DataBuff[2] = '.';
			DataBuff[3] = (UpdatePercentage % 10) + '0';
			DataBuff[4] = '%';
			DataBuff[5] = '\0';
		}
		else
		{
			DataBuff[0] = '1';
			DataBuff[1] = '0';
			DataBuff[2] = '0';
			DataBuff[3] = '.';
			DataBuff[4] = '0';
			DataBuff[5] = '%';
			DataBuff[6] = '\0';
		}
	
		/* Display download progress */
		Mid_TFTLCD_ShowString(130, 120, &DataBuff[0], LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
		
		DataBuff[0] = FIRMWARE_NEW_VERSION_READY;	// New Firmware is ready
		DataBuff[1] = NewFirmwareVersion[0];
		DataBuff[2] = NewFirmwareVersion[1];
		
		/* update the FirmwareInfo to external Flash */
		Mid_Flash_WriteData(&DataBuff[0], FLASH_ADDRESS_FIRMWARE_NEW_VERSION_FLAG, 3);
		
		Mid_TFTLCD_ShowString(0, 120, "Update Successfully", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
		
		/* jump and execute the new version code */
		Hal_JumpToApp_Jump();
	}
}


/*-------------Internal Functions Definition--------*/


/*-------------Interrupt Functions Definition--------*/


