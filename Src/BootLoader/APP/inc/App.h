#ifndef __APP_H_
#define __APP_H_

/* Keep the following part equal to the macro defines of Mid_Firmware.h in the AppPart: */
/**************************************************************/
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

/**************************************************************/

/* structure of FirmwareInfo part(the first 13 bytes of Flash)*/
typedef struct
{
	unsigned char NewVersionFlag;		// FirmwareUpdate State
	unsigned char CurrentVersion[2];    // current Firmware Version
	unsigned char NewVersion[2];        // New Firmware Version
	unsigned char FirmwareSize[4];      // size of New Firmware (byte)
	unsigned char PackageNumber[2];     // number of Packages
	unsigned char CRC16[2];             // CRC16 check value of the whole file, grab from Server info
}stu_FirmwareInfo_t;


void App_Init(void);
void App_Pro(void);


#endif
