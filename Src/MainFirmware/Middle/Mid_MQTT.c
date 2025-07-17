/****************************************************
  * @Name	Mid_MQTT.c
  * @Brief	Set parameters for connecting the MQTT server
  ***************************************************/
  
/*-------------Header Files Include-----------------*/
#include "stm32f10x.h"
#include "mid_mqtt.h"
#include "md5.h"
#include "device.h"
#include "timestamp.h"
#include "os_system.h"

/*-------------Internal Functions Declaration------*/
static void Mid_MQTT_SetClientID(uint8_t *pClientID);
static void Mid_MQTT_SetUsername(uint8_t *pUsername);
static void Mid_MQTT_SetServerIP(uint8_t *pServerIP);
static void Mid_MQTT_SetServerPort(uint8_t *pServerPort);
static void Mid_MQTT_SetSubTopic(uint8_t *pSubTopic);
static void Mid_MQTT_SetPubTopic(uint8_t *pPubTopic);
static void Mid_MQTT_SetSubTopic_DateTime(uint8_t *pDateTimeTopic);

/*-------------Module Variables Declaration--------*/
stu_MQTT_Device_t  stu_MQTT_ESP8266;


/*---Module Call-Back function pointer Definition---*/


/*-------------Module Functions Definition---------*/
/**
  * @Brief	Initialize MQTT module
  *			use <MCU-UID> as the @Username of device 
  *			use <Flag> + <Username> as the @ClientID of device
  *			use <Username> as recource for MD5-Salt encryption get final @Password
  * @Param	None
  * @Retval	None
  */
void Mid_MQTT_Init(void)
{
	uint8_t i;
	uint8_t buf[40];
	uint8_t dat;
	
	
	/* Username-Frame: [0x00 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF '\0'] 
			 26 bytes: 	Flag(1 byte)	MCU-UID(12 bytes)	String-end-symbol(1 byte) */
	buf[0] = '0';
	
	for(i=0; i<12; i++)
    {
		/* use MCU UID as the Username here */
		dat = Device_Get_MCU_UID(i);
		
		dat >>= 4;
		dat &= 0x0F;
		
		if(dat < 10)
		{
			buf[2*i + 1] = dat + '0';
		}
		else
		{
			buf[2*i + 1] = dat + 'A' -10;
		}
		
		dat = Device_Get_MCU_UID(i);
		dat &= 0x0F;
		
		if(dat < 10)
		{
			buf[2*i + 2] = dat + '0';
		}
		else
		{
			buf[2*i + 2] = dat + 'A' -10;
		}
	}
	
	buf[25] = 0;	// '\0'
	
	/* get the MD5-salted pasword: */
	SaltEncryption((char *)&buf[1], (char *)&buf[1], stu_MQTT_ESP8266.Password);
	
	/* set the ClientID: */
	Mid_MQTT_SetClientID(&buf[0]);
	
	/* set the Username: */
	Mid_MQTT_SetUsername(&buf[1]);
	
	/* set the ServerIP: */
	Mid_MQTT_SetServerIP("10.0.0.148");
	
	/* set the ServerPort: */
	Mid_MQTT_SetServerPort("1883");
	
	/* set the SubTopic: (<MCU-UID>_MessageDown\0) */
	buf[25] = '_';
	buf[26] = 'M';
	buf[27] = 'e';
	buf[28] = 's';
	buf[29] = 's';
	buf[30] = 'a';
	buf[31] = 'g';
	buf[32] = 'e';
	buf[33] = 'D';
	buf[34] = 'o';
	buf[35] = 'w';
	buf[36] = 'n';
	buf[37] = '\0';
	Mid_MQTT_SetSubTopic(&buf[1]);
	
	/* set the PubTopic: (<MCU-UID>_MessageUp\0) */
	buf[25] = '_';
	buf[26] = 'M';
	buf[27] = 'e';
	buf[28] = 's';
	buf[29] = 's';
	buf[30] = 'a';
	buf[31] = 'g';
	buf[32] = 'e';
	buf[33] = 'U';
	buf[34] = 'p';
	buf[35] = '\0';
	Mid_MQTT_SetPubTopic(&buf[1]);

	/* set the SubTopic_DateTime: ($SYS/brokers/emqx@127.0.0.1/datetime\0) */
	buf[1] = '$';
	buf[2] = 'S';
	buf[3] = 'Y';
	buf[4] = 'S';
	buf[5] = '/';
	buf[6] = 'b';
	buf[7] = 'r';
	buf[8] = 'o';
	buf[9] = 'k';
	buf[10] = 'e';
	buf[11] = 'r';
	buf[12] = 's';
	buf[13] = '/';
	buf[14] = 'e';
	buf[15] = 'm';
	buf[16] = 'q';
	buf[17] = 'x';
	buf[18] = '@';
	buf[19] = '1';
	buf[20] = '2';
	buf[21] = '7';
	buf[22] = '.';
	buf[23] = '0';
	buf[24] = '.';
	buf[25] = '0';
	buf[26] = '.';
	buf[27] = '1';
	buf[28] = '/';
	buf[29] = 'd';
	buf[30] = 'a';
	buf[31] = 't';
	buf[32] = 'e';
	buf[33] = 't';
	buf[34] = 'i';
	buf[35] = 'm';
	buf[36] = 'e';
	buf[37] = '\0';
	Mid_MQTT_SetSubTopic_DateTime(&buf[1]);
	
	/* set the FirmwareUpdateFlag: */
	Mid_MQTT_SetFirmwareUpdateFlag(MQTT_FIRMWARE_UPDATE_NONE);
}

/**
  * @Brief	Set the Flag indicating whether there is new Firmware available on the server
  * @Param	Flag: Firmware state
  * @Retval	None
  */
void Mid_MQTT_SetFirmwareUpdateFlag(en_MQTT_FirmwareUpdateFlag_t Flag)
{
	stu_MQTT_ESP8266.FirmwareUpdateFlag = Flag;
}

/**
  * @Brief	Get the Firmwareupdate flag
  * @Param	None
  * @Retval	None
  */
uint8_t Mid_MQTT_GetFirmwareUpdateFlag(void)
{
	return stu_MQTT_ESP8266.FirmwareUpdateFlag;
}

/* DateTime Frame from EMQX-system: 1970-07-21T23:08:35.841964696-04:00 */
/**
  * @Brief	According to the UNIX TimeStamp received from server, calculate the Coordinate Universal Time(UTC)
  * @Param	pData		: point to the receieved time data(string) trimmed by Mid_WiFi_MQTTRxDataHandler
  *			pDateTime	: point to the processed RealTime datetime data(string)
  * @Retval	None
  *	@Note	The EMQX server provide the TimeStamp with every 100 seconds(Realtime) into 1 overflow of @Second bits
			the suffix <-04:00> means the UTC-4:00 time zone(!!!The timestamp provided by the server deduced this part on @Hour bits!!!)
  */
void Mid_MQTT_SystemTimeProcess(uint8_t *pData, uint8_t *pDateTime)
{
	uint8_t MonthIndex;
	unsigned long UNIXCounter;
	unsigned long CounterBuff;

	// 1970-07-21T23:18:29.823975371-04:00
	/* get the UNIX TimeStamp count: */
	UNIXCounter = ((pData[20] - '0') * 10) + (pData[21] - '0');	
	UNIXCounter += ((((pData[17] - '0') * 10) + (pData[18] - '0')) * 100);
	UNIXCounter += ((((pData[14] - '0') * 10) + (pData[15] - '0')) * 60 * 100);
	UNIXCounter += ((((pData[11] - '0') * 10) + (pData[12] - '0') + 4) * 60 * 60 * 100);
	UNIXCounter += ((((pData[8] - '0') * 10) + (pData[9] - '0')) * 24 * 60 * 60 * 100);
	
	MonthIndex = (((pData[5] - '0') * 10) + (pData[6] - '0'));
	switch(MonthIndex)
	{
		case 7:
		{
			CounterBuff = 180 * (unsigned long)8640000;
		}
		break;

		case 8:
		{
			CounterBuff = 211 * (unsigned long)8640000;
		}
		break;

		case 9:
		{
			CounterBuff = 242 * (unsigned long)8640000;
		}
		break;

		case 10:
		{
			CounterBuff = 272 * (unsigned long)8640000;
		}
		break;

		case 11:
		{
			CounterBuff = 303 * (unsigned long)8640000;
		}
		break;

		case 12:
		{
			CounterBuff = 333 * (unsigned long)8640000;
		}
		break;
	}

	UNIXCounter += CounterBuff;
	
	TimeStamp_DateTime_Conversion(UNIXCounter, &pDateTime[0]);
}


/*-------------Internal Functions Definition--------*/
/**
  * @Brief	Set device ClientID 
  * @Param	pClientID: point to the ClientID string
  * @Retval	None
  */
static void Mid_MQTT_SetClientID(uint8_t *pClientID)
{
	uint8_t Index = 0;
	
	while(*pClientID)
	{
		stu_MQTT_ESP8266.ClientID[Index++] = *pClientID;
		
		pClientID++;
		
		if(Index == MQTT_CLIENT_ID_SIZE)
		{
			break;
		}
	}
}

/**
  * @Brief	Set device Username 
  * @Param	pUsername: point to the Username string
  * @Retval	None
  */
static void Mid_MQTT_SetUsername(uint8_t *pUsername)
{
	uint8_t Index = 0;
	
	while(*pUsername)
	{
		stu_MQTT_ESP8266.Username[Index++] = *pUsername;
		
		pUsername++;
		
		if(Index == MQTT_USERNAME_SIZE)
		{
			break;
		}
	}
}

/**
  * @Brief	Set target ServerIP
  * @Param	pServerIP: point to the ServerIP string
  * @Retval	None
  */
static void Mid_MQTT_SetServerIP(uint8_t *pServerIP)
{
	uint8_t Index = 0;
	
	while(*pServerIP)
	{
		stu_MQTT_ESP8266.ServerIp[Index++] = *pServerIP;
		
		pServerIP++;
		
		if(Index == MQTT_SERVER_IP_SIZE)
		{
			break;
		}
	}
}

/**
  * @Brief	Set target ServerPort
  * @Param	pServerPort: point to the ServerPort string
  * @Retval	None
  */
static void Mid_MQTT_SetServerPort(uint8_t *pServerPort)
{
	uint8_t Index = 0;
	
	while(*pServerPort)
	{
		stu_MQTT_ESP8266.ServerPort[Index++] = *pServerPort;
		
		pServerPort++;
		
		if(Index == MQTT_SERVER_PORT_SIZE)
		{
			break;
		}
	}
}

/**
  * @Brief	Set Subscription Topic
  * @Param	pSubTopic: point to the Subscription Topic string
  * @Retval	None
  */
static void Mid_MQTT_SetSubTopic(uint8_t *pSubTopic)
{
	uint8_t Index = 0;
	
	while(*pSubTopic)
	{
		stu_MQTT_ESP8266.SubTopic[Index++] = *pSubTopic;
		
		pSubTopic++;
		
		if(Index == MQTT_TOPIC_SIZE)
		{
			break;
		}
	}
}

/**
  * @Brief	Set Publication Topic
  * @Param	pPubTopic: point to the Publication Topic string
  * @Retval	None
  */
static void Mid_MQTT_SetPubTopic(uint8_t *pPubTopic)
{
	uint8_t Index = 0;
	
	while(*pPubTopic)
	{
		stu_MQTT_ESP8266.PubTopic[Index++] = *pPubTopic;
		
		pPubTopic++;
		
		if(Index == MQTT_TOPIC_SIZE)
		{
			break;
		}
	}
}

/**
  * @Brief	Set DateTime subscription topic
  * @Param	pDateTimeTopic: point to the DateTime subscription topic string
  * @Retval	None
  */
static void Mid_MQTT_SetSubTopic_DateTime(uint8_t *pDateTimeTopic)
{
	uint8_t Index = 0;
	
	while(*pDateTimeTopic)
	{
		stu_MQTT_ESP8266.SubTopic_DataTime[Index++] = *pDateTimeTopic;
		
		pDateTimeTopic++;
		
		if(Index == MQTT_TOPIC_SIZE)
		{
			break;
		}
	}
}


/*-------------Interrupt Functions Definition--------*/


