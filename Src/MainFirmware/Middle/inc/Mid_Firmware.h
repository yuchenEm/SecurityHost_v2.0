#ifndef __MID_FIRMWARE_H_
#define __MID_FIRMWARE_H_

/* Firmware update process macro define: */
#define FIRMWARE_NEW_VERSION_FLAG		0xAA	// new Firmware version detected, is ready to download from ExternalFlash 
#define FIRMWARE_NEW_VERSION_READY		0xBB	// new Firmware is ready in EmbeddedFlash
#define FIRMWARE_NEW_VERSION_DEFAULT	0xCC

/* Flash Address offset of FirmwareInfo define */
typedef enum
{
	FLASH_ADDRESS_FIRMWARE_NEW_VERSION_FLAG = 0,	//
	
	FLASH_ADDRESS_FIRMWARE_CURRENTVERSION 	= 1,	// 2 byte
	FLASH_ADDRESS_FIRMWARE_NEWVERSION 		= 3,	// 2 byte
	
	FLASH_ADDRESS_FIRMWARE_BYTE_SIZE 		= 5,	// 4 byte
	FLASH_ADDRESS_FIRMWARE_PACKAGE_NUMBER 	= 9,	// 2 byte
	
	FLASH_ADDRESS_FIRMWARE_CRC16_CHECKVALUE = 11,	// 2 byte
	
	FLASH_ADDRESS_FIRMWARE_BASE_ADDRESS		= 13,
	
}en_FlashAddress_FirmwareInfo_t;

/* FirmwareUpdate State define */
typedef enum
{
	FIRMWARE_UPDATE_STA_IDLE = 0,		// no FirmwareUpdate mission
	FIRMWARE_UPDATE_STA_NEW_VERSION,	// detect new version 
	FIRMWARE_UPDATE_STA_NOT_UPDATE,		// donot update
	FIRMWARE_UPDATE_STA_DOWNLOAD_START,	// start downloading the Firmware
	FIRMWARE_UPDATE_STA_DOWNLOAD_FAIL,	// Firmware downloading fail
	FIRMWARE_UPDATE_STA_SUCCESS,		// update succeed
	
	FIRMWARE_UPDATE_STA_SUM,
}en_FirmwareUpdateState_t;

typedef union
{
	unsigned char Version[2];
}un_Verion_t;

typedef union
{
	unsigned int FirmwareSizeTotal;
	unsigned char FirmwareSize[4];
}un_FirmwareSize_t;

typedef union
{
	unsigned short PackageNumberTotal;
	unsigned char PackageNumber[2];
}un_PackageNumber_t;

/* Firmware structure define */
typedef struct
{
	en_FirmwareUpdateState_t 	UpdateState;			// FirmwareUpdate State
	un_Verion_t 				CurrentVersion;			// current Firmware Version
	un_Verion_t 				NewVersion;				// New Firmware Version
	un_FirmwareSize_t 			FirmwareSize;			// size of New Firmware (byte)
	un_PackageNumber_t 			PackageNumber;			// number of Packages
	unsigned char 				CRC16[2];				// CRC16 check value of the whole file, grab from Server info
	unsigned short 				DownloadPackageNumber;	// number of already downloaded packages
	unsigned int				DownloadByteNumber;		// number of already downloaded bytes
	
}stu_Firmware_t;

/* Firmware Downloading Data structure define */
typedef struct
{
	unsigned char 	DataID;
//	unsigned char	UpType;
	un_Verion_t 	DownloadVersion;	// FirmwareVersion downloading
//	unsigned char 	State;
	unsigned char 	PackageIndex[2];	// index of Package(0->PackageNumber-1)
	unsigned char 	DataLen;			// effective data length
	unsigned char 	DataBuff[102];		// effective data(100) + CRC16(2): CRC16 value of current package
	
}stu_DownloadData_t;


extern stu_Firmware_t stu_Firmware;



void 	 Mid_Firmware_Init(void);

void 	 Mid_Firmware_InfoUpdate(stu_Firmware_t FirmwarePara);
void 	 Mid_Firmware_SetCurrentVersion(uint8_t ByteIndex, uint8_t Value);
uint8_t  Mid_Firmware_GetUpdateState(void);
void 	 Mid_Firmware_SetUpdateState(en_FirmwareUpdateState_t State);

void 	 Mid_Firmware_StartDownload(void);
uint8_t  Mid_Firmware_Download_Pro(void (*pFlashWriteData)(uint8_t *pBuffer, uint32_t Addr, uint16_t Num), void (*pFlashReadData)(uint8_t *pBuffer, uint32_t Addr, uint16_t Num), uint8_t *pData);
uint16_t Mid_Firmware_DownloadProgress_Pro(uint8_t CommType, void (*pGetNewFirmware_DataPack)(uint8_t CommType, uint16_t PackageIdnex, uint8_t *pVersion));


#endif
