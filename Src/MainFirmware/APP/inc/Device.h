#ifndef __DEVICE_H_
#define __DEVICE_H_

/**  STM32 Unique Device ID (UID) is a 96-bit value (12 byte)
  *	 stored in Flash memory and is used for device identification and 
  *	 security purposes 
  */
  /* STM3210x Unique ID Base_Address: */
#define STM32_UID_ADDR		0x1FFFF7E8

extern unsigned char STM32_UID[12];

/* UID  */
#define Device_Get_MCU_UID(i)	(STM32_UID[i])




/* Sensor maxium numbers */
#define SENSOR_NUMBER_MAX		20

/* Sensor Offline time define */
#define SENSOR_OFFLINE_COUNT	86400

/* Sensor Type */
typedef enum
{
	SENSOR_TYPE_DOOR = 1,
	SENSOR_TYPE_REMOTE,
	
	SENSOR_TYPE_SUM
}en_Sensor_Type_t;

/* Sensor ArmType */
typedef enum
{
	SENSOR_ARMTYPE_0_24HOURS,	// zone_0
	SENSOR_ARMTYPE_1_AWAYARM,	// zone_1
	SENSOR_ARMTYPE_2_HOMEARM,	// zone_2

	SENSOR_ARMTYPE_SUM,
}en_Sensor_ArmType_t;

/* Sensor structure */
typedef struct
{
	unsigned char 		ID;
	unsigned char 		PairFlag;		// 0->unpair, 1->pair
	unsigned char 		SensorName[26];
	unsigned char 		SensorNo;
	
	en_Sensor_Type_t 	Sensor_Type;
	en_Sensor_ArmType_t Sensor_ArmType;
	
	unsigned char 		MACAddress[12];
	unsigned char 		Node[2];
	unsigned int 		SleepTime;
	
}stu_Sensor_t;

/* Sensor Pairing State */
typedef enum
{
	SENSOR_PAIRSTATE_PAIRED,	// sensor alread paired
	SENSOR_PAIRSTATE_UNPAIR,	// sensor not paired
	SENSOR_PAIRSTATE_FAIL,		// pairing fail
	
}en_Sensor_PairState_t;

/* SensorPair parameters structure */
typedef struct
{
	en_Sensor_PairState_t 	PairState;
	unsigned char 					SensorIndex;
	
}stu_Sensor_PairPara_t;

/* Sensor structure size */
#define STU_SENSOR_SIZE 	sizeof(stu_Sensor_t)

extern stu_Sensor_t stu_Sensor[SENSOR_NUMBER_MAX];

/* Set Sensor Parameters macro define */
#define Device_Set_SensorPara_ID(x, val)							(stu_Sensor[x].ID = val)
#define Device_Set_SensorPara_PairFlag(x, val)				(stu_Sensor[x].PairFlag = val)
#define Device_Set_SensorPara_SensorName(x, i, val)		(stu_Sensor[x].SensorName[i] = val)
#define Device_Set_SensorPara_SensorNo(x, val)				(stu_Sensor[x].SensorNo = val)		 
#define Device_Set_SensorPara_Sensor_Type(x, val)			(stu_Sensor[x].Sensor_Type = val)
#define Device_Set_SensorPara_Sensor_ArmType(x,val)		(stu_Sensor[x].Sensor_ArmType = val)		
#define Device_Set_SensorPara_Code(x, i, val)					(stu_Sensor[x].Code[i] = val)
#define Device_Set_SensorPara_SleepTime(x, val)				(stu_Sensor[x].SleepTime = val)

/* Get Sensor Parameters macro define */
#define Device_Get_SensorPara_ID(x)								(stu_Sensor[x].ID)
#define Device_Get_SensorPara_PairFlag(x)					(stu_Sensor[x].PairFlag)
#define Device_Get_SensorPara_SensorName(x, i)		(stu_Sensor[x].SensorName[i])
#define Device_Get_SensorPara_SensorNo(x)					(stu_Sensor[x].SensorNo)		 
#define Device_Get_SensorPara_Sensor_Type(x)			(stu_Sensor[x].Sensor_Type)
#define Device_Get_SensorPara_Sensor_ArmType(x)		(stu_Sensor[x].Sensor_ArmType)	
#define Device_Get_SensorPara_Code(x, i)					(stu_Sensor[x].Code[i])
#define Device_Get_SensorPara_SleepTime(x)				(stu_Sensor[x].SleepTime)




/* Terminal system parameters structure */
/* Super adminpassword: 0000 */
typedef struct 
{
	unsigned char PhoneNumber[7][20];
	unsigned char AdminPassword[4];
	unsigned char FirmwareVersion[2];
	
}stu_SystemPara_t;

/* Terminal system parameters structre size */
#define STU_SYSTEMPARA_SIZE		sizeof(stu_SystemPara_t)

extern stu_SystemPara_t stu_SystemPara;

/* Set Terminal System Parameters macro define */
#define Device_Set_SystemPara_PhoneNumber(x, i, val)		(stu_SystemPara.PhoneNumber[x][i] = val)
#define Device_Set_SystemPara_AdminPassword(i, val)			(stu_SystemPara.AdminPassword[i] = val)
#define Device_Set_SystemPara_FirmwareVersion(i, val)   (stu_SystemPara.FirmwareVersion[i] = val)


/* Get Terminal System Parameters macro define */
#define Device_Get_SystemPara_PhoneNumber(x, i)				(stu_SystemPara.PhoneNumber[x][i])
#define Device_Get_SystemPara_AdminPassword(i)				(stu_SystemPara.AdminPassword[i])
#define Device_Get_SystemPara_FirmwareVersion(i)      (stu_SystemPara.FirmwareVersion[i])




/* EEPROM address offset */
#define EEPROM_ADDRESS_BASE					0
#define EEPROM_ADDRESS_SYSTEMPARA_OFFSET	EEPROM_ADDRESS_BASE
#define EEPROM_ADDRESS_SENSORPARA_OFFSET	STU_SYSTEMPARA_SIZE


void 			Device_Init(void);
void 			Device_ParaReset(void);
unsigned char 	Device_CheckSensorExistence(unsigned char Index);
void 			Device_GetSensorPara(stu_Sensor_t *pSensorPara, unsigned char Index);
unsigned char 	Device_SensorNodeMatch(unsigned char *pData);

#endif
