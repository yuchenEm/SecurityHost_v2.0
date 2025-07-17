/****************************************************
  * @Name	Device.c
  * @Brief	
  ***************************************************/
  
/*-------------Header Files Include-----------------*/
#include "device.h"
#include "mid_eeprom.h"

/*-------------Internal Functions Declaration------*/
static void 			Device_ParaCheck_System(void);
static unsigned char 	Device_ParaCheck_Sensor(void);
static void 			Device_SensorGenerate_Debug(void);


/*-------------Module Variables Declaration--------*/
unsigned char STM32_UID[12];
stu_Sensor_t 		stu_Sensor[SENSOR_NUMBER_MAX];
stu_SystemPara_t 	stu_SystemPara;


/*---Module Call-Back function pointer Definition---*/


/*-------------Module Functions Definition---------*/
/**
  * @Brief	Initialize Device module
  * @Param	None
  * @Retval	None
  */
void Device_Init(void)
{
    unsigned char i;
	
	/* obtain MCU UID from the Flash memory */
	for(i=0; i<12; i++)
	{
		STM32_UID[i] = *((unsigned char *)(STM32_UID_ADDR + i));
	}
	
	/* read out the saved SystemPara and SensorPara from EEPROM */
	Mid_EEPROM_SequentialRead(EEPROM_ADDRESS_SYSTEMPARA_OFFSET, (unsigned char *)(&stu_SystemPara), sizeof(stu_SystemPara));
	Mid_EEPROM_SequentialRead(EEPROM_ADDRESS_SENSORPARA_OFFSET, (unsigned char *)(&stu_Sensor), sizeof(stu_Sensor));
	
	/* Check parameters */
	Device_ParaCheck_System();
	if(Device_ParaCheck_Sensor())
	{
		Device_ParaReset();
	}
	
	//Device_SensorGenerate_Debug();
}

/**
  * @Brief	Reset SystemPara and SensorPara to FactorySettings
  * @Param	None
  * @Retval	None
  */
void Device_ParaReset(void)
{
	unsigned short i;
	unsigned char j;
	
	for(i=0; i<SENSOR_NUMBER_MAX; i++)
	{
		stu_Sensor[i].ID = 0;
		stu_Sensor[i].PairFlag = 0;
		stu_Sensor[i].SensorNo = 0;
		
		for(j=0; j<26; j++)
		{
			stu_Sensor[i].SensorName[j] = 0;
		}
		
		stu_Sensor[i].Sensor_Type = SENSOR_TYPE_DOOR;
		stu_Sensor[i].Sensor_ArmType = SENSOR_ARMTYPE_1_AWAYARM;
		
		for(j=0; j<12; j++)
		{
			stu_Sensor[i].MACAddress[j] = 0;
		}
	}
	
	for(i=0; i<4; i++)
	{
		Device_Set_SystemPara_AdminPassword(i, 0);
	}
	
	Mid_EEPROM_PageWrite(EEPROM_ADDRESS_SENSORPARA_OFFSET, (unsigned char *)(&stu_Sensor), sizeof(stu_Sensor));
	Mid_EEPROM_SequentialRead(EEPROM_ADDRESS_SENSORPARA_OFFSET, (unsigned char *)(&stu_Sensor), sizeof(stu_Sensor));
}

/**
  * @Brief	Check if the Sensor is saved by the Terminal
  * @Param	Index: Sensor Index(0->SENSOR_NUMBER_MAX)
  * @Retval	Result: 0->sensor info not exist, 1->paired sensor info found
  */
unsigned char Device_CheckSensorExistence(unsigned char Index)
{
	unsigned char Result = 0;
	
	if(Index < SENSOR_NUMBER_MAX)
	{
		if(stu_Sensor[Index].PairFlag)
		{
			Result = 1;
		}
	}
	
	return Result;
}

/**
  * @Brief	According to the Sensor Index, provide the corresponding Sensor info to the buff
  * @Param	pSensorPara : point to the Sensor parameters buff provided 
  *			Index		: Sensor Index
  * @Retval	None
  */
void Device_GetSensorPara(stu_Sensor_t *pSensorPara, unsigned char Index)
{
	unsigned char i;
	
	if(Index >= SENSOR_NUMBER_MAX)
	{
		return;
	}
	
	pSensorPara->ID = stu_Sensor[Index].ID;
	pSensorPara->Sensor_Type = stu_Sensor[Index].Sensor_Type;
	pSensorPara->PairFlag = stu_Sensor[Index].PairFlag;
	pSensorPara->SensorNo = stu_Sensor[Index].SensorNo;
	pSensorPara->SleepTime = stu_Sensor[Index].SleepTime;
	
	for(i=0; i<16; i++)
	{
		pSensorPara->SensorName[i] = stu_Sensor[Index].SensorName[i];
	}
	
	pSensorPara->Sensor_ArmType = stu_Sensor[Index].Sensor_ArmType;
	
	for(i=0; i<12; i++)
	{
		pSensorPara->MACAddress[i] = stu_Sensor[Index].MACAddress[i];
	}
	
	pSensorPara->Node[0] = stu_Sensor[Index].Node[0];
	pSensorPara->Node[1] = stu_Sensor[Index].Node[1];
}

/**
  * @Brief	Check if there is a Terminal-saved Sensor has the same NodeNo with the given Node Data
  * @Param	pData : pointer of the given sensor's Node
  * @Retval	Matching result: 
  *				0xFF	  -> match fail, 
  *				Sensor.ID -> match succeed
  */
unsigned char Device_SensorNodeMatch(unsigned char *pData)
{
	unsigned char i = 0;
	
	for(i=0; i<SENSOR_NUMBER_MAX; i++)
	{
		/* comapre the given sensor's Node(CRC16 result) and the NodeNo saved in the Terminal */
		if( ( stu_Sensor[i].PairFlag == 1) &&
		    ( stu_Sensor[i].Node[0] == pData[0]) &&
			( stu_Sensor[i].Node[1] == pData[1]) 
		)
		{
			stu_Sensor[i].SleepTime = 0;
			
			return ((unsigned char)stu_Sensor[i].ID);
		}
	}
	
	return 0xFF;
}


/*-------------Internal Functions Definition--------*/
/**
  * @Brief	Check if System parameters valid
  * @Param	None
  * @Retval	None
  */
static void Device_ParaCheck_System(void)
{
//	unsigned char i, j;
	
	/* reset FirmwareVersion to v1.00, if FirmwareVersion = 0x00 or 0xFF */
	if( ((Device_Get_SystemPara_FirmwareVersion(0) == 0) && (Device_Get_SystemPara_FirmwareVersion(1) == 0)) ||
		((Device_Get_SystemPara_FirmwareVersion(0) == 0xFF) && (Device_Get_SystemPara_FirmwareVersion(1) == 0xFF)) )
	{
		Device_Set_SystemPara_FirmwareVersion(0, 0);
		Device_Set_SystemPara_FirmwareVersion(1, 0x64);	// reset the FirmwareVersion to v1.00
	}
	
	/* reset AdminPassword to 0000, if any 'A'-'F' exist */
	if( (Device_Get_SystemPara_AdminPassword(0) > 9) || 
		(Device_Get_SystemPara_AdminPassword(1) > 9) || 
		(Device_Get_SystemPara_AdminPassword(2) > 9) ||
		(Device_Get_SystemPara_AdminPassword(3) > 9) )
	{
		Device_Set_SystemPara_AdminPassword(0, 0);
		Device_Set_SystemPara_AdminPassword(1, 0);
		Device_Set_SystemPara_AdminPassword(2, 0);
		Device_Set_SystemPara_AdminPassword(3, 0);
	}
	
}

/**
  * @Brief	Check if Sensor parameters valid
  * @Param	None
  * @Retval	Error: 0->parameters correct, 1->parameters invalid
  */
static unsigned char Device_ParaCheck_Sensor(void)
{
	unsigned char i;
	unsigned char Error = 0;
	
	for(i=0; i<SENSOR_NUMBER_MAX; i++)
	{
		if(stu_Sensor[i].ID >= SENSOR_NUMBER_MAX)
		{
			Error = 1;
		}
		if(stu_Sensor[i].PairFlag > 1)
		{
			Error = 1;
		}
		if(stu_Sensor[i].Sensor_Type > SENSOR_TYPE_SUM)
		{
			Error = 1;
		}
		if(stu_Sensor[i].Sensor_ArmType > SENSOR_ARMTYPE_SUM)
		{
			Error = 1;
		}
		
		stu_Sensor[i].SleepTime = 0;
	}
	
	return Error;
}

/**
  * @Brief	Generate Sensor info for debug
  * @Param	None
  * @Retval	None
  */
static void Device_SensorGenerate_Debug(void)
{
	unsigned char i, j, k;
	
	for(i=2; i<SENSOR_NUMBER_MAX; i++)
	{
		stu_Sensor[i].ID = i + 1;
		stu_Sensor[i].PairFlag = 1;
		stu_Sensor[i].SensorNo = 1;
		
		stu_Sensor[i].SensorName[0] = 'Z';
		stu_Sensor[i].SensorName[1] = 'o';
		stu_Sensor[i].SensorName[2] = 'n';
		stu_Sensor[i].SensorName[3] = 'e';
		stu_Sensor[i].SensorName[4] = '-';
		stu_Sensor[i].SensorName[5] = ((i + 1) / 100) + '0';
		stu_Sensor[i].SensorName[6] = (((i + 1) % 100) / 10) + '0';
		stu_Sensor[i].SensorName[7] = (((i + 1) % 100) % 10) + '0';
		
		for(j=8; j<26; j++)
		{
			stu_Sensor[i].SensorName[j] = 0;
		}
		
		if(i % 2)
		{
			stu_Sensor[i].Sensor_Type = SENSOR_TYPE_DOOR;
			
			stu_Sensor[i].SleepTime = 86401;
		}
		else
		{
			stu_Sensor[i].Sensor_Type = SENSOR_TYPE_REMOTE;
		}
		
		stu_Sensor[i].Sensor_ArmType = SENSOR_ARMTYPE_1_AWAYARM;
		
		for(k=0; k<12; k++)
		{
			stu_Sensor[i].MACAddress[k] = k;
		}
		
		stu_Sensor[i].Node[0] = 0;
		stu_Sensor[i].Node[1] = i;

	}
}


/*-------------Interrupt Functions Definition--------*/


