/****************************************************
  * @Name	Mid_Firmware.c
  * @Brief	
  ***************************************************/
  
/*-------------Header Files Include-----------------*/
#include "stm32f10x.h"
#include "mid_firmware.h"
#include "crc16.h"

/*-------------Internal Functions Declaration------*/


/*-------------Module Variables Declaration--------*/
unsigned short CombinedCRC16;			// current combiend CRC16 check value
unsigned short CombinedCRC16Last;	// last combined CRC16 check value

stu_Firmware_t stu_Firmware;

/*---Module Call-Back function pointer Definition---*/


/*-------------Module Functions Definition---------*/
/**
  * @Brief	Initialize Firmware module
  * @Param	None
  * @Retval	None
  */
void Mid_Firmware_Init(void)
{
	stu_Firmware.UpdateState = FIRMWARE_UPDATE_STA_IDLE;
	
}
  
/**
  * @Brief	Update the current Firmware parameters according to the new firmware info from server
  * @Param	FirmwarePara: provided Firmware parameters captured from Server response message
  * @Retval	None
  */
void Mid_Firmware_InfoUpdate(stu_Firmware_t FirmwarePara)
{
	if(stu_Firmware.UpdateState == FIRMWARE_UPDATE_STA_IDLE)
	{
		stu_Firmware.NewVersion.Version[0] = FirmwarePara.NewVersion.Version[0];
		stu_Firmware.NewVersion.Version[1] = FirmwarePara.NewVersion.Version[1];
	
		stu_Firmware.FirmwareSize.FirmwareSize[0] = FirmwarePara.FirmwareSize.FirmwareSize[0];
		stu_Firmware.FirmwareSize.FirmwareSize[1] = FirmwarePara.FirmwareSize.FirmwareSize[1];
		stu_Firmware.FirmwareSize.FirmwareSize[2] = FirmwarePara.FirmwareSize.FirmwareSize[2];
		stu_Firmware.FirmwareSize.FirmwareSize[3] = FirmwarePara.FirmwareSize.FirmwareSize[3];
		
		stu_Firmware.PackageNumber.PackageNumber[0] = FirmwarePara.PackageNumber.PackageNumber[0];
		stu_Firmware.PackageNumber.PackageNumber[1] = FirmwarePara.PackageNumber.PackageNumber[1];
		
		stu_Firmware.CRC16[0] = FirmwarePara.CRC16[0];
		stu_Firmware.CRC16[1] = FirmwarePara.CRC16[1];
		
		CombinedCRC16 = 0;
		CombinedCRC16Last = 0xFFFF;
		
		stu_Firmware.UpdateState = FIRMWARE_UPDATE_STA_NEW_VERSION;
		stu_Firmware.DownloadPackageNumber = 0;
		stu_Firmware.DownloadByteNumber = 0;
	}
}

/**
  * @Brief	Set Firmware current Version in byte 
  * @Param	ByteIndex: index of CurrentVersion.Version[] union
  *			Value	 : target value
  * @Retval	None
  */
void Mid_Firmware_SetCurrentVersion(uint8_t ByteIndex, uint8_t Value)
{
	stu_Firmware.CurrentVersion.Version[ByteIndex] = Value;
}

/**
  * @Brief	Get Firmware UpdateState 
  * @Param	None
  * @Retval	Current UpdateState
  */
uint8_t Mid_Firmware_GetUpdateState(void)
{
	return (uint8_t)stu_Firmware.UpdateState;
}

/**
  * @Brief	Set Firmware UpdateState 
  * @Param	State: target state
  * @Retval	None
  */
void Mid_Firmware_SetUpdateState(en_FirmwareUpdateState_t State)
{
	stu_Firmware.UpdateState = State;
}

/**
  * @Brief	Start Firmware Update
  * @Param	None
  * @Retval	None
  */
void Mid_Firmware_StartDownload(void)
{
	stu_Firmware.UpdateState = FIRMWARE_UPDATE_STA_DOWNLOAD_START;
	stu_Firmware.DownloadPackageNumber = 0;
	stu_Firmware.DownloadByteNumber = 0;
	
	CombinedCRC16 = 0;
	CombinedCRC16Last = 0xFFFF;
}

/**
  * @Brief	Polling function of downloading data from received data packages
				check the CRC16 value of each received package 
  * @Param	pFlashWriteData	: function pointer of FlashWriteData
  *			pFlashReadData	: function pointer of FlashReadData
  *			pData			: point to the Data received
  * @Retval	0->Package download not complete yet, 
  *			1->Package download complete
  */
uint8_t Mid_Firmware_Download_Pro(void (*pFlashWriteData)(uint8_t *pBuffer, uint32_t Addr, uint16_t Num), void (*pFlashReadData)(uint8_t *pBuffer, uint32_t Addr, uint16_t Num), uint8_t *pData)
{
	uint16_t CRC16_fromCalculation;
	uint16_t CRC16_fromPackage;
	uint16_t CRC16_Firmware;	// grab from server info
	
	uint32_t WriteInAddress;	// start address for data write-in
	
	uint16_t PackageIndex;		// index of Download Package(0->PackageNumber-1)
	
	uint8_t DataBuff[13];		// buffer of Firmware info, ahead of effective-data 
	
	stu_DownloadData_t *DownloadDataBuff;
	
	/* start parsing download data */
	if(stu_Firmware.UpdateState == FIRMWARE_UPDATE_STA_DOWNLOAD_START)
	{
		DownloadDataBuff = (stu_DownloadData_t *)pData;
		
		PackageIndex = ((DownloadDataBuff->PackageIndex[0] << 8) | (DownloadDataBuff->PackageIndex[1]));
		
		/* PackageIndex == stu_Firmware.DownloadPackageNumber indicates that the current processing datapackage is the next one of packages already downloaded */
		if(PackageIndex == stu_Firmware.DownloadPackageNumber)
		{
			/* calculate CRC16 of the new datapackage */
			CRC16_fromCalculation = Mid_CRC16_Modbus(&DownloadDataBuff->DataBuff[0], DownloadDataBuff->DataLen);
		
			/* get the CRC16 provided by the package(last 2 byte) */
			CRC16_fromPackage = (DownloadDataBuff->DataBuff[DownloadDataBuff->DataLen] << 8) | (DownloadDataBuff->DataBuff[DownloadDataBuff->DataLen + 1]);
		
			/* CRC16 Check of this package succeed */
			if(CRC16_fromCalculation == CRC16_fromPackage)
			{
				CombinedCRC16 = Mid_CRC16_Modbus_Continuous(&DownloadDataBuff->DataBuff[0], DownloadDataBuff->DataLen, CombinedCRC16Last);
				CombinedCRC16Last = CombinedCRC16;
				
				WriteInAddress = stu_Firmware.DownloadByteNumber + FLASH_ADDRESS_FIRMWARE_BASE_ADDRESS;
			
				/* write-in effective data to Flash */
				pFlashWriteData(&DownloadDataBuff->DataBuff[0], WriteInAddress, DownloadDataBuff->DataLen);
			
				stu_Firmware.DownloadPackageNumber += 1;
				stu_Firmware.DownloadByteNumber += DownloadDataBuff->DataLen;
				
				/* check whether all data write-in complete */
				if(stu_Firmware.DownloadPackageNumber == stu_Firmware.PackageNumber.PackageNumberTotal)
				{
					if(stu_Firmware.DownloadByteNumber == stu_Firmware.FirmwareSize.FirmwareSizeTotal)
					{
						CRC16_Firmware = ((stu_Firmware.CRC16[0] << 8) | (stu_Firmware.CRC16[1]));
				
						/* CRC16 check of the whole file succeed */
						if(CombinedCRC16 == CRC16_Firmware)
						{
							DataBuff[0] = FIRMWARE_NEW_VERSION_FLAG;
							
							DataBuff[1] = stu_Firmware.CurrentVersion.Version[0];
							DataBuff[2] = stu_Firmware.CurrentVersion.Version[1];
							DataBuff[3] = stu_Firmware.NewVersion.Version[0];
							DataBuff[4] = stu_Firmware.NewVersion.Version[1];
							
							DataBuff[5] = stu_Firmware.FirmwareSize.FirmwareSize[0];
							DataBuff[6] = stu_Firmware.FirmwareSize.FirmwareSize[1];
							DataBuff[7] = stu_Firmware.FirmwareSize.FirmwareSize[2];
							DataBuff[8] = stu_Firmware.FirmwareSize.FirmwareSize[3];
							
							DataBuff[9] = stu_Firmware.PackageNumber.PackageNumber[0];
							DataBuff[10] = stu_Firmware.PackageNumber.PackageNumber[1];
							
							DataBuff[11] = stu_Firmware.CRC16[0];
							DataBuff[12] = stu_Firmware.CRC16[1];
							
							/* write-in Firmware info to Flash */
							pFlashWriteData(&DataBuff[0], FLASH_ADDRESS_FIRMWARE_NEW_VERSION_FLAG, 13);
							
							Mid_Firmware_SetUpdateState(FIRMWARE_UPDATE_STA_SUCCESS);
						}
						/* CRC16 check of the whole file fail*/
						else
						{
							Mid_Firmware_SetUpdateState(FIRMWARE_UPDATE_STA_DOWNLOAD_FAIL);
						}
					}
					else
					{
						Mid_Firmware_SetUpdateState(FIRMWARE_UPDATE_STA_DOWNLOAD_FAIL);
					}
				}
				/* download not complete yet */
				else
				{
					return 0;
				}
			}
		}
	}
	
	return 1;
}

/**
  * @Brief	Polling function of tracking and updating the download progress
  * @Param	CommType				: communication type(0->WiFi, 1->LTE)
  *			pGetNewFirmware_DataPack: function pointer of MQTTProtocol_GetNewFirmware_DataPack
  * @Retval	Firmware download progress, experessed in percentage:
  *			66.6% 		  -> return 666
  *			100.0% 		  -> return 1000
  *			download fail -> return 0xFFFF
  */
uint16_t Mid_Firmware_DownloadProgress_Pro(uint8_t CommType, void (*pGetNewFirmware_DataPack)(uint8_t CommType, uint16_t PackageIdnex, uint8_t *pVersion))
{
	static uint32_t DownloadProgress = 0xFFFF;	// indicate the progress percentage with 2-byte
	static uint16_t Counter = 0;
	static uint8_t ResendCounter = 0;
	
	/* Firmware is downloading */
	if(stu_Firmware.UpdateState == FIRMWARE_UPDATE_STA_DOWNLOAD_START)
	{
		/* check and update DownloadProgress */
		if(DownloadProgress != stu_Firmware.DownloadPackageNumber)
		{
			Counter = 0;
			ResendCounter = 0;
			DownloadProgress = stu_Firmware.DownloadPackageNumber;
			
			/* Pack the dataframe to request the next package data */
			/* use stu_Firmware.DownloadPackageNumber as the PackageIndex of the next package data */
			pGetNewFirmware_DataPack(CommType, stu_Firmware.DownloadPackageNumber, &stu_Firmware.CurrentVersion.Version[0]);
		}
		else
		{
			Counter++;
			
			/* resend the request if no response after 3s */
			if(Counter > 300)
			{
				Counter = 0;
				
				DownloadProgress = stu_Firmware.DownloadPackageNumber;
				
				pGetNewFirmware_DataPack(CommType, stu_Firmware.DownloadPackageNumber, &stu_Firmware.CurrentVersion.Version[0]);
			
				ResendCounter++;
				
				/* resend 20 times, timeout */
				/* download fail */
				if(ResendCounter > 20)
				{
					ResendCounter = 0;
					DownloadProgress = 0xFFFF;
					
					return DownloadProgress;
				}
			}
		}
	}
	
	/* return updated download progress */
	return ((stu_Firmware.DownloadPackageNumber * 1000) / stu_Firmware.PackageNumber.PackageNumberTotal);
}


/*-------------Internal Functions Definition--------*/


/*-------------Interrupt Functions Definition--------*/


