/****************************************************
  * @Name	MQTT_Protocol.c
  * @Brief	
  *  
  *	DataFrame:
  *	Terminal-->Server:
  *	---------------------------------------------------------------------------------------------
  *	|	Header	|	DataLength	|	Command		|	DataPayload		|	CheckValue	|	Tail	|
  *	---------------------------------------------------------------------------------------------
  *		0xAA		2 byte			1 byte			 	N byte			 1 byte			0x55	
  *	
  *	Server-->Terminal:
  *	---------------------------------------------------------------------------------------------
  *	|	Header	|	DataLength	|	Command		|	DataPayload		|	CheckValue	|	Tail	|
  *	---------------------------------------------------------------------------------------------
  *		0xAA		  2 byte 		1 byte				N byte			 1 byte			0x55	
  *		
  *	-> Header	 	: 0xAA, indicate the beginning of the DataFrame
  *	-> DataLength	: 2 byte data, indicate the bytes followed: (N + 3) = Command + DataPayload + CheckValue + Tail
  *	-> Command		: 1 byte data, indicate the defined command type
  *	-> DataPayload	: N byte data, the length of Payload varies(can set nested protocol inside)	(0-256 bytes)
  *	-> CheckValue	: 1 byte data, CheckValue = XOR(DataLength + Command + DataPayload)
  *	-> Tail			: 0x55, indicate the end of the DAtaFrame
  *	
  *	Terminal-->Server:
  *	---------------------------------------------------------------------------------------------------------
  *	|				Command					|							DataPayload							|
  *	---------------------------------------------------------------------------------------------------------
  *			0x29 : Get SystemTime				DataFrameID	: 1 byte
  *												UTC-Offset	: 2 byte(highest bit: 1->positive, 0->negative)
  *	---------------------------------------------------------------------------------------------------------
  *	
  *	
  *	Server-->Terminal:
  *	---------------------------------------------------------------------------------------------------------
  *	|				Command					|							DataPayload							|
  *	---------------------------------------------------------------------------------------------------------
  *			0x2A : MessageDown SystemTime		DataFrameID	: 1 byte(same as the client send Command 0x29)
  *												Year		: 2 byte
  *												Month		: 1 byte
  *												Day			: 1 byte
  *												Weekday		: 1 byte
  *												Hour		: 1 byte
  *												Minute		: 1 byte
  *												Second		: 1 byte
  *	---------------------------------------------------------------------------------------------------------
  *	
  *	DataFrame examples:
  *	-> EventUpload from Terminal:
  *	HomeArm:(Terminal Key)
  *	AA 		00 2A 			51 			00 25 			  323032352D30352D32352D31353A313020 	20 48 6F 6D 65 61 72 6D 20 62 79 20 68 6F 73 74 	20202020 	01 			55
  *	Head	Datalength		Command		Payload length    SystemTime						   	   H  o  m  e  a  r  m     b  y     h  o  s  t  		 		XOR value	Tail
  *	
  *	AwayArm:(Terminal Key)
  *	AA 		00 2A 			51 			00 25 			  323032352D30352D32352D31353A313120 	20 41 77 61 79 61 72 6D 20 62 79 20 68 6F 73 74 	20202020 	01 			55
  *	Head	Datalength		Command		Payload length	  SystemTime						   	   A  w  a  y  a  r  m     b  y     h  o  s  t  		 		XOR value	Tail
  *	
  *	DisArm:(Terminal Key)
  *	AA		00 2A			51			00 25			  323032352D30352D32352D31353A333820 	20 44 69 73 61 72 6D 20 62 79 20 68 6F 73 74 20 	20202020	5A			55
  *	Head	Datalength		Command		Payload length	  SystemTime						   	   D  i  s  a  r  m     b  y     h  o  s  t						XOR value	Tail
  *	
  *	Alarm:(Terminal Key)
  *	AA		00 38			51			00 33			  323032352D30352D32352D31363A343020	48 4F 53 54 20 20 20 20 20 20 20 20 20 20 20 20 48 6F 73 74 20 53 4F 53 20 61 6C 61 72 6D 20202020 	75 			55
  *	Head	Datalength		Command		Payload length	  SystemTime							H  O  S  T										H  o  s  t     S  O  S     a  l  a  r  m			XOR value	Tail
  *	
  *	Alarm:(Remote)
  *	AA		00 38			51			00 33			  323032352D30352D32362D31373A303020	44 65 76 69 63 65 2D 30 30 32 00 00 00 00 00 00 53 65 6E 73 6F 72 20 61 6C 61 72 6D 202020202020	2D			55
  *	Head	Datalength		Command		Payload length	  SystemTime							D  e  v  i  c  e  _  0  0  2					S  e  n  s  o  r     a  l  a  r  m					XOR value	Tail
  *	
  *	Alarm:(Server)
  *	AA		00 38			51			00 33			  323032352D30352D32372D31353A323720	48 4F 53 54 20 20 20 20 20 20 20 20 20 20 20 20 48 6F 73 74 20 53 4F 53 20 61 6C 61 72 6D 20202020 	75			55
  *	Head	Datalength		Command		Payload length	  SystemTime							H  O  S  T										H  o  s  t     S  O  S     a  l  a  r  m			XOR value	Tail	
  *
  *	Door Close:
  *	AA		00 38			51			00 33			  323032352D30352D32352D31353A343320 	44 65 76 69 63 65 2D 30 30 31 00 00 00 00 00 00 44 6F 6F 72 20 63 6C 6F 73 65 	2020202020202020 	2B 			55
  *	Head	Datalength		Command		Payload length	  SystemTime						 	D  e  v  i  c  e  _  0  0  1					D  o  o  r     c  l  o  s  e				   		XOR value	Tail
  *	
  *	Door Open:
  *	AA		00 38			51			00 33			  323032352D30352D32352D31353A353420	44 65 76 69 63 65 2D 30 30 31 00 00 00 00 00 00 44 6F 6F 72 20 6F 70 65 6E 20	2020202020202020	6F			55		
  *	Head	Datalength		Command		Payload length	  SystemTime							D  e  v  i  c  e  _  0  0  1					D  o  o  r     o  p  e  n							XOR value	Tail
  *	
  *	Host Battery-low:
  *	AA		00 38			51			00 33			  000000000000000000002D000000000020	48 4F 53 54 20 20 20 20 20 20 20 20 20 20 20 20 48 6F 73 74 20 76 6F 6C 74 20 6C 6F 77 	2020202020 	22			55
  *	Head	Datalength		Command		Payload length	  SystemTime(MQTT not connected yet)	H  O  S  T  						 			H  o  s  t     v  o  l  t     l  o  w				XOR value	Tail
  *  
  * Check New Firmware:
  *	AA 		00 05 			21 			00 00 			  	24 			55
  *	Head	Datalength		Command		Payload length		XOR value	Tail
  *  
  *	-> Response from Terminal:
  *	Host Info:
  *	AA		00 16			51			00 11			  00 64 				00 00 00 		38 FF D9 05 34 42 36 36 36 44 18 43 	76 			55
  *	Head	Datalength		Command		Payload length	  FirmwareVersion               		MCU-UID									XOR value	Tail
  *	
  *	Terminal WorkMode:(AwayArm->00, HomeArm->01, Alarm->02, Disarm->03)
  *	Head	Datalength		Command		Payload length	  WorkMode		XOR value	Tail
  *	AA		00 06			51			00 01			  00 			56			55
  *	AA		00 06			51			00 01			  01			57			55
  *	AA		00 06			51			00 01			  02			54			55
  *	AA		00 06			51			00 01			  03			55			55
  *	
  *	Get Device_1 Info:
  *	AA		00 18			51			00 13			  01 			01 			44 65 76 69 63 65 2D 30 30 31 00 00 00 00 00 00 	01 				7F 			55
  *	Head	Datalength		Command		Payload length	  SensorID		SensorType	D  e  v  i  c  e  _  0  0  1 (SensorName)			SensorArmType	XOR value	Tail
  *	
  *	Get Device_2 Info:
  *	AA		00 18			51			00 13			  02			02			44 65 76 69 63 65 2D 30 30 32 00 00 00 00 00 00 	01 				7C			55
  *	Head	Datalength		Command		Payload length	  SensorID		SensorType	D  e  v  i  c  e  _  0  0  2 (SensorName)			SensorArmType	XOR value	Tail
  *	
  *	Server Change WorkMode Successfully:
  *	AA		00 0B			51			00 06			  53 65 74 20 4F 4B 	3A 			55
  *	Head	Datalength		Command		Payload length	  S  e  t     O  K		XOR value	Tail
  *  
  *	Server Request Invalid / Dataframe Corrupted:(Error)
  *	AA		00 0A			51			00 05			  45 72 72 6F 72 	06 			55
  *	Head	Datalength		Command		Payload length	  E  r  r  o  r  	XOR value	Tail
  *	
  *  
  *  
  *	-> Server Request Code DataFrame:
  *		PROTOCOL_SERVER_REQUEST_HOST_INFO: 				AA0006500001005755
  *		AA 00 06 50 00 01 00 57 55
  *		
  *		PROTOCOL_SERVER_REQUEST_WORKMODE: 				AA0006500001015655
  *		AA 00 06 50 00 01 01 56 55
  *		
  *		PROTOCOL_SERVER_REQUEST_DEVICE_1_INFO: 			AA0006500001025555
  *		AA 00 06 50 00 01 02 55 55
  *		
  *		PROTOCOL_SERVER_REQUEST_DEVICE_2_INFO: 			AA0006500001035455
  *		AA 00 06 50 00 01 03 54 55
  *		
  *		...												...
  *		
  *		PROTOCOL_SERVER_REQUEST_DEVICE_20_INFO: 		AA0006500001154255
  *		AA 00 06 50 00 01 15 42 55
  *	
  *		PROTOCOL_SERVER_REQUEST_SET_WORKMODE_AWAYARM: 	AA0006500001164155
  *		AA 00 06 50 00 01 16 41 55
  *		
  *		PROTOCOL_SERVER_REQUEST_SET_WORKMODE_HOMEARM:	AA0006500001174055
  *		AA 00 06 50 00 01 17 40 55
  *		
  *		PROTOCOL_SERVER_REQUEST_SET_WORKMODE_DISARM:	AA0006500001184F55
  *		AA 00 06 50 00 01 18 4F 55
  *		
  *		PROTOCOL_SERVER_REQUEST_SET_WORKMODE_ALARM:		AA0006500001194E55
  *		AA 00 06 50 00 01 19 4E 55
  *		
  ***************************************************/
  
/*-------------Header Files Include-----------------*/
#include "stm32f10x.h"                  // Device header
#include "mqtt_protocol.h"
#include "mid_wifi.h"
#include "mid_firmware.h"
#include "mid_flash.h"
#include "device.h"
#include "os_system.h"
#include "app.h"

/*-------------Internal Functions Declaration------*/
static void 							MQTTProtocol_DataPack(en_Protocol_CommType_t CommType, unsigned char *pData);
static en_Protocol_ServerRequestCode_t 	MQTTProtocol_ReceiveDataParse(en_Protocol_CommType_t CommType, unsigned char *pData);
static void 							MQTTProtocol_TerminalRequest_SystemTime(en_Protocol_CommType_t CommType);


/*-------------Module Variables Declaration--------*/
stu_SystemTime_t	stu_SystemTime;

/* EventUpload buffer for MQTT server */
Queue32 Queue_MQTTEventUpload;

/* message payload of EventUpload */
const unsigned char MQTTEventUpload_FunctionMessage[][18] = 
{
		"Sensor alarm      ",
    "Sensor offline    ",
    "Sensor online     ",
    "Sensor volt low   ",
    "Sensor tamper     ",
    "Host SOS alarm    ",
    "Host volt low     ",
    "Host AC disconnect",
    "Host AC connect   ",
    "Door open         ",
    "Door close        ",
};

/* message payload of Terminal WorkMode(ArmType) change */
const unsigned char MQTTEventUpload_ArmMessage[][20] = 
{
		" Awayarm by host    ",
    " Awayarm by remote  ",
    " Awayarm by server  ",
    " Homearm by host    ",
    " Homearm by remote  ",
    " Homearm by server  ",
    " Disarm by host     ",
    " Disarm by remote   ",
    " Disarm by server   ",
};

/*---Module Call-Back function pointer Definition---*/


/*-------------Module Functions Definition---------*/
/**
  * @Brief	Initialize MQTT_Protocol module
  * @Param	None
  * @Retval	None
  */
void MQTTProtocol_Init(void)
{
	QueueEmpty(Queue_MQTTEventUpload);
}

/**
  * @Brief	
  * @Param	CommType: communication type
  * @Retval	None
  */
void MQTTProtocol_Pro(en_Protocol_CommType_t CommType)
{

}

/**
  * @Brief	Process the data received from MQTT server
  * @Param	CommType: communication type
  *			pData	: point to the data received
  *			Len		: data length
  * @Retval	None
  */
void MQTTProtocol_ReceiveDataHandler(en_Protocol_CommType_t CommType, unsigned char *pData, unsigned char Len)
{
	unsigned short DataLen;
	en_Protocol_ServerRequestCode_t ServerRequestCode;
	
	stu_Firmware_t FirmwareBuff;
	unsigned short VersionBuff;
	
	if(pData[0] == 0xAA)	// detect DataFrame Header
	{
		if(Len > 3)
		{
			DataLen = pData[1] << 8;
			DataLen += pData[2];
		}
		else
		{
			return;
		}
		
		if(pData[DataLen+2] == 0x55)	// detect Dataframe Tail, indicate a complete dataframe
		{
			switch(pData[3])	// get CommandCode
			{
				case PROTOCOL_SERVER_RESPONSE_GET_SYSTIME:
				{
					//AA 00 12 2A 00 07 E6 02 11 04 0B 0D 47 B7 55
					if(DataLen == 0x0C)
					{
						Set_SystemTime_Year((pData[5] << 8) + (pData[6]));
						Set_SystemTime_Month(pData[7]);
						Set_SystemTime_Day(pData[8]);
						Set_SystemTime_Week(pData[9]);
						Set_SystemTime_Hour(pData[10]);
						Set_SystemTime_Minute(pData[11]);
						Set_SystemTime_Second(pData[12]);
					}
				}
				break;
				
				case PROTOCOL_SERVER_RESPONSE_UPDATE_CHECK:
				{
					/*  Server Response DataFrame format:
						AA 00 0D 22 	XX XX 	XX XX XX XX		XX XX		XX XX	XX 	55
										version	FirmwareSize	PackageSize CRC16	XOR	
					*/
					FirmwareBuff.NewVersion.Version[0] = pData[5];
					FirmwareBuff.NewVersion.Version[1] = pData[6];
					
					VersionBuff = (FirmwareBuff.NewVersion.Version[0] << 8) + (FirmwareBuff.NewVersion.Version[1]);
					
					/* different version of firmware detect */
					if( VersionBuff != ((Device_Get_SystemPara_FirmwareVersion(0) << 8) + (Device_Get_SystemPara_FirmwareVersion(1))) )
					{
						FirmwareBuff.FirmwareSize.FirmwareSize[0] = pData[7];
						FirmwareBuff.FirmwareSize.FirmwareSize[1] = pData[8];
						FirmwareBuff.FirmwareSize.FirmwareSize[2] = pData[9];
						FirmwareBuff.FirmwareSize.FirmwareSize[3] = pData[10];
						
						FirmwareBuff.PackageNumber.PackageNumber[0] = pData[11];
						FirmwareBuff.PackageNumber.PackageNumber[1] = pData[12];
						
						FirmwareBuff.CRC16[0] = pData[13];
						FirmwareBuff.CRC16[1] = pData[14];
						
						// Update the Firmware Info according to the NewFirmware from server
						Mid_Firmware_InfoUpdate(FirmwareBuff);
					}
				}
				break;
				
				case PROTOCOL_SERVER_RESPONSE_UPDATE_FIRMWARE:
				{
					// AA LEN1 LEN2 0x25 DATA DATA DATA ... 0x55, effective data start from pData[4]
					Mid_Firmware_Download_Pro(&Mid_Flash_WriteData, &Mid_Flash_ReadData, &pData[4]);
				}
				break;
				
				case PROTOCOL_SERVER_COMMAND:
				{
					/* extract ServerRequestCode from dataframe */
					ServerRequestCode = MQTTProtocol_ReceiveDataParse(CommType, &pData[1]);
					
					/* prepare and package response dataframe */
					MQTTProtocol_ServerRequestResponse_DataPack(CommType, ServerRequestCode);
				}
				break;
				
			}
		}
	}
}

/**
  * @Brief	Queue-in Event and Message payload index to Queue_MQTTEventUpload
  * @Param	Event: Event index
  *			Data : Message index
  * @Retval	None
  */
void MQTTProtocol_EventUpQueueIn(unsigned char Event, unsigned char Data)
{
	QueueDataIn(Queue_MQTTEventUpload, &Event, 1);
	QueueDataIn(Queue_MQTTEventUpload, &Data, 1);
}

/**
  * @Brief	Polling function, if Queue_MQTTEventUpload has data to process, pack the data according to the protocol
  *			upload the packed dataframe to the server through WiFi-module/LTE-module
  * @Param	CommType: communication type(0->WiFi, 1->LTE)
  * @Retval	None
  */
void MQTTProtocol_EventUpload_Pro(en_Protocol_CommType_t CommType)
{
	stu_MQTTEventUpload_t EventUploadBuff;
	
	if(QueueDataLen(Queue_MQTTEventUpload) > 1)
	{
		if(Mid_WiFi_GetMQTTState() == MQTT_STA_READY)
		{
			QueueDataOut(Queue_MQTTEventUpload, (unsigned char *)&EventUploadBuff.Event);
			QueueDataOut(Queue_MQTTEventUpload, &EventUploadBuff.Buff);
			
			switch((unsigned char)EventUploadBuff.Event)
			{
				case TERMINAL_UPEVENT_UPDATE_CHECK:
				{
					MQTTProtocol_NewFirmwareCheck_DataPack(CommType);
				}
				break;
				
				case TERMINAL_UPEVENT_GET_SYSTIME:
				{
				
				}
				break;
				
				case TERMINAL_UPEVENT_DETECTOR_ALARM:
				{
					MQTTProtocol_EventUpload_DataPack(CommType, EventUploadBuff.Buff, TERMINAL_UPEVENT_DETECTOR_ALARM, ENDPOINT_MESSAGE_UPLOAD);
				}			
				break;
				
				case TERMINAL_UPEVENT_DETECTOR_OFFLINE:
				{
					MQTTProtocol_EventUpload_DataPack(CommType, EventUploadBuff.Buff, TERMINAL_UPEVENT_DETECTOR_OFFLINE, ENDPOINT_MESSAGE_UPLOAD);
				}
				break;
				
				case TERMINAL_UPEVENT_DETECTOR_ONLINE:
				{
					MQTTProtocol_EventUpload_DataPack(CommType, EventUploadBuff.Buff, TERMINAL_UPEVENT_DETECTOR_ONLINE, ENDPOINT_MESSAGE_UPLOAD);
				}
				break;
				
				case TERMINAL_UPEVENT_DETECTOR_BATLOW:
				{
					MQTTProtocol_EventUpload_DataPack(CommType, EventUploadBuff.Buff, TERMINAL_UPEVENT_DETECTOR_BATLOW, ENDPOINT_MESSAGE_UPLOAD);
				}					
				break;
				
				case TERMINAL_UPEVENT_DETECTOR_ALARM_TEMPER:
				{
					
				}
				break;
				
				case TERMINAL_UPEVENT_HOST_ALARM_SOS:
				{
					MQTTProtocol_EventUpload_DataPack(CommType, 0xFF, TERMINAL_UPEVENT_HOST_ALARM_SOS, ENDPOINT_MESSAGE_UPLOAD);
				}	
				break;
				
				case TERMINAL_UPEVENT_HOST_BATLOW:
				{
					MQTTProtocol_EventUpload_DataPack(CommType, 0xFF, TERMINAL_UPEVENT_HOST_BATLOW, ENDPOINT_MESSAGE_UPLOAD);
				}
				break;
				
				case TERMINAL_UPEVENT_HOST_AC_DISCONN:
				{
					MQTTProtocol_EventUpload_DataPack(CommType, 0xFF, TERMINAL_UPEVENT_HOST_AC_DISCONN, ENDPOINT_MESSAGE_UPLOAD);
				}
				break;
				
				case TERMINAL_UPEVENT_HOST_AC_CONNECT:
				{
					MQTTProtocol_EventUpload_DataPack(CommType, 0xFF, TERMINAL_UPEVENT_HOST_AC_CONNECT, ENDPOINT_MESSAGE_UPLOAD);
				}
				break;
				
				case TERMINAL_UPEVENT_DOOR_OPEN:
				{
					MQTTProtocol_EventUpload_DataPack(CommType, EventUploadBuff.Buff, TERMINAL_UPEVENT_DOOR_OPEN, ENDPOINT_MESSAGE_UPLOAD);
				}
				break;
				
				case TERMINAL_UPEVENT_DOOR_CLOSE:
				{
					MQTTProtocol_EventUpload_DataPack(CommType, EventUploadBuff.Buff, TERMINAL_UPEVENT_DOOR_CLOSE, ENDPOINT_MESSAGE_UPLOAD);
				}
				break;
				
				case TERMINAL_UPEVENT_AWAYARM_BY_HOST:
				{
					MQTTProtocol_EventUpload_DataPack(CommType, 0xFF, TERMINAL_UPEVENT_AWAYARM_BY_HOST, ENDPOINT_TERMINAL_WORMODE_CHANGE);
				}
				break;
				
				case TERMINAL_UPEVENT_AWAYARM_BY_REMOTE:
				{
					MQTTProtocol_EventUpload_DataPack(CommType, 0xFF, TERMINAL_UPEVENT_AWAYARM_BY_REMOTE, ENDPOINT_TERMINAL_WORMODE_CHANGE);
				}
				break;
				
				case TERMINAL_UPEVENT_AWAYARM_BY_SERVER:
				{
					MQTTProtocol_EventUpload_DataPack(CommType, 0xFF, TERMINAL_UPEVENT_AWAYARM_BY_SERVER, ENDPOINT_TERMINAL_WORMODE_CHANGE);
				}
				break;
				
				case TERMINAL_UPEVENT_HOMEARM_BY_HOST:
				{
					MQTTProtocol_EventUpload_DataPack(CommType, 0xFF, TERMINAL_UPEVENT_HOMEARM_BY_HOST, ENDPOINT_TERMINAL_WORMODE_CHANGE);
				}
				break;
				
				case TERMINAL_UPEVENT_HOMEARM_BY_REMOTE:
				{
					MQTTProtocol_EventUpload_DataPack(CommType, 0xFF, TERMINAL_UPEVENT_HOMEARM_BY_REMOTE, ENDPOINT_TERMINAL_WORMODE_CHANGE);
				}
				break;
				
				case TERMINAL_UPEVENT_HOMEARM_BY_SERVER:
				{
					MQTTProtocol_EventUpload_DataPack(CommType, 0xFF, TERMINAL_UPEVENT_HOMEARM_BY_SERVER, ENDPOINT_TERMINAL_WORMODE_CHANGE);
				}
				break;
			
				case TERMINAL_UPEVENT_DISARM_BY_HOST:
				{
					MQTTProtocol_EventUpload_DataPack(CommType, 0xFF, TERMINAL_UPEVENT_DISARM_BY_HOST, ENDPOINT_TERMINAL_WORMODE_CHANGE);
				}
				break;
				
				case TERMINAL_UPEVENT_DISARM_BY_REMOTE:
				{
					MQTTProtocol_EventUpload_DataPack(CommType, 0xFF, TERMINAL_UPEVENT_DISARM_BY_REMOTE, ENDPOINT_TERMINAL_WORMODE_CHANGE);
				}
				break;
				
				case TERMINAL_UPEVENT_DISARM_BY_SERVER:
				{
					MQTTProtocol_EventUpload_DataPack(CommType, 0xFF, TERMINAL_UPEVENT_DISARM_BY_SERVER, ENDPOINT_TERMINAL_WORMODE_CHANGE);
				}
				break;
				
			}
		}
	}
}

/**
  * @Brief	Pack the provided data prepare the Upload payload Data
  * @Param	CommType	: communication type
  *			ZoneNo		: 0xFF->Terminal / Server, 1-20->Sensor
  *			EventType	: event type
  *			Endpoint	: manage the different datapackage (messgae upload, Terminal arm mode change)
  * @Retval	None
  */
void MQTTProtocol_EventUpload_DataPack(en_Protocol_CommType_t CommType, unsigned char ZoneNo, en_Terminal_UpEvent_t EventType, unsigned short Endpoint)
{
	unsigned char DataBuff[100];
	unsigned char i, j;
	unsigned short Len;
	
	i = 2;
	
	DataBuff[i++] = PROTOCOL_TERMINAL_RESPONSE;		// Terminal response command
	
	DataBuff[i++] = 0;								// payload Length highbyte
	DataBuff[i++] = 0;								// payload Length lowbyte
	Len = 0;
	DataBuff[i++] = SystemTime[0];					// year
	DataBuff[i++] = SystemTime[1];					// year
	DataBuff[i++] = SystemTime[2];					// year
	DataBuff[i++] = SystemTime[3];					// year
	DataBuff[i++] = SystemTime[4];					// '-'
	DataBuff[i++] = SystemTime[5];					// month
	DataBuff[i++] = SystemTime[6];					// month
	DataBuff[i++] = SystemTime[7];					// '-'
	DataBuff[i++] = SystemTime[8];					// day
	DataBuff[i++] = SystemTime[9];					// day
	DataBuff[i++] = '-';							
	DataBuff[i++] = SystemTime[11];					// hour
	DataBuff[i++] = SystemTime[12];					// hour
	DataBuff[i++] = SystemTime[13];					// ':'
	DataBuff[i++] = SystemTime[14];					// minute
	DataBuff[i++] = SystemTime[15];					// minute
	DataBuff[i++] = ' ';
	Len = 17;
	
	if(Endpoint == ENDPOINT_MESSAGE_UPLOAD)
	{
		/* event from Terminal */
		if(ZoneNo == 0xFF)
		{
			DataBuff[i++] = 'H';
			DataBuff[i++] = 'O';
			DataBuff[i++] = 'S';
			DataBuff[i++] = 'T';
		
			for(j=0; j<12; j++)
			{
				DataBuff[i++] = ' '; 	// leave SensorName blank
			}
		}
		/* event from Sensor */
		else
		{
			ZoneNo -= 1;	// SensorIndex = ZoneNo - 1
		
			for(j=0; j<16; j++)
			{
				DataBuff[i++] = Device_Get_SensorPara_SensorName(ZoneNo, j);	// SensorName
			}
		}
	
		Len += 16;
	
		/* add payload message */
		if((EventType >= TERMINAL_UPEVENT_DETECTOR_ALARM) && (EventType <= TERMINAL_UPEVENT_DOOR_CLOSE))
		{
			for(j=0; j<18; j++)
			{
				DataBuff[i++] = MQTTEventUpload_FunctionMessage[EventType - TERMINAL_UPEVENT_DETECTOR_ALARM][j];
			}
		
			Len += 18;
		}
	}
	else if(Endpoint == ENDPOINT_TERMINAL_WORMODE_CHANGE)
	{
		if((EventType >= TERMINAL_UPEVENT_AWAYARM_BY_HOST) && (Endpoint <= TERMINAL_UPEVENT_DISARM_BY_SERVER))
		{
			for(j=0; j<20; j++)
			{
				DataBuff[i++] = MQTTEventUpload_ArmMessage[EventType - TERMINAL_UPEVENT_AWAYARM_BY_HOST][j];
			}
			
			Len += 20;
		}
	}
	
	DataBuff[3] = (Len >> 8) & 0xFF;	// payload length
	DataBuff[4] = Len & 0xFF;			
	
	DataBuff[0] = (i >> 8) & 0xFF;		// DataFrame length
	DataBuff[1] = i & 0xFF;
	
	// pack processed payload data
	MQTTProtocol_DataPack(CommType, &DataBuff[0]);
}

/**
  * @Brief	According to the ServerRequestCode, prepare the corresponding final MQTT_Protocol DataFrame reply the server
  * @Param	CommType			: communication type
  *			ServerRequestCode	: Server Request Code extracted
  * @Retval	None
  */
void MQTTProtocol_ServerRequestResponse_DataPack(en_Protocol_CommType_t CommType, en_Protocol_ServerRequestCode_t ServerRequestCode)
{
	unsigned char DataBuff[100];
	unsigned char i, j;
	unsigned short Len;
	
	i = 2;
	
	DataBuff[i++] = PROTOCOL_TERMINAL_RESPONSE;		// Terminal response command
	DataBuff[i++] = 0;								// payload Length highbyte
	DataBuff[i++] = 0;								// payload Length lowbyte
	
	Len = 0;
	
	switch((unsigned char)ServerRequestCode)
	{
		case PROTOCOL_SERVER_REQUEST_HOST_INFO:
		{
			DataBuff[i++] = Device_Get_SystemPara_FirmwareVersion(0);		// FirmwareVersion high byte
			DataBuff[i++] = Device_Get_SystemPara_FirmwareVersion(1);		// FirmwareVersion low byte
		
			DataBuff[i++] = 0x00;
			DataBuff[i++] = 0x00;
			DataBuff[i++] = 0x00;
			
			for(j=0; j<12; j++)
			{
				DataBuff[i++] = Device_Get_MCU_UID(j);		// MCU-UID
			}

			Len += 17;
		}
		break;
		
		case PROTOCOL_SERVER_REQUEST_WORKMODE:
		{
			/* obtain the current Terminal WorkMode from App module */
			DataBuff[i++] = pTerminalMode->WorkMode;
			
			Len += 1;
		}
		break;
		
		case PROTOCOL_SERVER_REQUEST_DEVICE_1_INFO:
		{
			DataBuff[i++] = Device_Get_SensorPara_ID(0);					// SensorID
			DataBuff[i++] = Device_Get_SensorPara_Sensor_Type(0);			// SensorType
			
			for(j=0; j<16; j++)
			{
				DataBuff[i++] = Device_Get_SensorPara_SensorName(0, j);		// SensorName
			}
			
			DataBuff[i++] = Device_Get_SensorPara_Sensor_ArmType(0);		// Sensor ArmType
			
			Len += 19;
		}
		break;
		
		case PROTOCOL_SERVER_REQUEST_DEVICE_2_INFO:
		{
			DataBuff[i++] = Device_Get_SensorPara_ID(1);					// SensorID
			DataBuff[i++] = Device_Get_SensorPara_Sensor_Type(1);			// SensorType
			
			for(j=0; j<16; j++)
			{
				DataBuff[i++] = Device_Get_SensorPara_SensorName(1, j);		// SensorName
			}
			
			DataBuff[i++] = Device_Get_SensorPara_Sensor_ArmType(1);		// Sensor ArmType
			
			Len += 19;
		}
		break;
		
		case PROTOCOL_SERVER_REQUEST_DEVICE_3_INFO:
		{
			DataBuff[i++] = Device_Get_SensorPara_ID(2);					// SensorID
			DataBuff[i++] = Device_Get_SensorPara_Sensor_Type(2);			// SensorType
			
			for(j=0; j<16; j++)
			{
				DataBuff[i++] = Device_Get_SensorPara_SensorName(2, j);		// SensorName
			}
			
			DataBuff[i++] = Device_Get_SensorPara_Sensor_ArmType(2);		// Sensor ArmType
			
			Len += 19;
		}
		break;
		
		case PROTOCOL_SERVER_REQUEST_DEVICE_4_INFO:
		{
			DataBuff[i++] = Device_Get_SensorPara_ID(3);					// SensorID
			DataBuff[i++] = Device_Get_SensorPara_Sensor_Type(3);			// SensorType
			
			for(j=0; j<16; j++)
			{
				DataBuff[i++] = Device_Get_SensorPara_SensorName(3, j);		// SensorName
			}
			
			DataBuff[i++] = Device_Get_SensorPara_Sensor_ArmType(3);		// Sensor ArmType
			
			Len += 19;
		}
		break;
		
		case PROTOCOL_SERVER_REQUEST_DEVICE_5_INFO:
		{
			DataBuff[i++] = Device_Get_SensorPara_ID(4);					// SensorID
			DataBuff[i++] = Device_Get_SensorPara_Sensor_Type(4);			// SensorType
			
			for(j=0; j<16; j++)
			{
				DataBuff[i++] = Device_Get_SensorPara_SensorName(4, j);		// SensorName
			}
			
			DataBuff[i++] = Device_Get_SensorPara_Sensor_ArmType(4);		// Sensor ArmType
			
			Len += 19;
		}
		break;
		
		case PROTOCOL_SERVER_REQUEST_DEVICE_6_INFO:
		{
			DataBuff[i++] = Device_Get_SensorPara_ID(5);					// SensorID
			DataBuff[i++] = Device_Get_SensorPara_Sensor_Type(5);			// SensorType
			
			for(j=0; j<16; j++)
			{
				DataBuff[i++] = Device_Get_SensorPara_SensorName(5, j);		// SensorName
			}
			
			DataBuff[i++] = Device_Get_SensorPara_Sensor_ArmType(5);		// Sensor ArmType
			
			Len += 19;
		}
		break;
		
		case PROTOCOL_SERVER_REQUEST_DEVICE_7_INFO:
		{
			DataBuff[i++] = Device_Get_SensorPara_ID(6);					// SensorID
			DataBuff[i++] = Device_Get_SensorPara_Sensor_Type(6);			// SensorType
			
			for(j=0; j<16; j++)
			{
				DataBuff[i++] = Device_Get_SensorPara_SensorName(6, j);		// SensorName
			}
			
			DataBuff[i++] = Device_Get_SensorPara_Sensor_ArmType(6);		// Sensor ArmType
			
			Len += 19;
		}
		break;
		
		case PROTOCOL_SERVER_REQUEST_DEVICE_8_INFO:
		{
			DataBuff[i++] = Device_Get_SensorPara_ID(7);					// SensorID
			DataBuff[i++] = Device_Get_SensorPara_Sensor_Type(7);			// SensorType
			
			for(j=0; j<16; j++)
			{
				DataBuff[i++] = Device_Get_SensorPara_SensorName(7, j);		// SensorName
			}
			
			DataBuff[i++] = Device_Get_SensorPara_Sensor_ArmType(7);		// Sensor ArmType
			
			Len += 19;
		}
		break;
		
		case PROTOCOL_SERVER_REQUEST_DEVICE_9_INFO:
		{
			DataBuff[i++] = Device_Get_SensorPara_ID(8);					// SensorID
			DataBuff[i++] = Device_Get_SensorPara_Sensor_Type(8);			// SensorType
			
			for(j=0; j<16; j++)
			{
				DataBuff[i++] = Device_Get_SensorPara_SensorName(8, j);		// SensorName
			}
			
			DataBuff[i++] = Device_Get_SensorPara_Sensor_ArmType(8);		// Sensor ArmType
			
			Len += 19;
		}
		break;
		
		case PROTOCOL_SERVER_REQUEST_DEVICE_10_INFO:
		{
			DataBuff[i++] = Device_Get_SensorPara_ID(9);					// SensorID
			DataBuff[i++] = Device_Get_SensorPara_Sensor_Type(9);			// SensorType
			
			for(j=0; j<16; j++)
			{
				DataBuff[i++] = Device_Get_SensorPara_SensorName(9, j);		// SensorName
			}
			
			DataBuff[i++] = Device_Get_SensorPara_Sensor_ArmType(9);		// Sensor ArmType
			
			Len += 19;
		}
		break;
		
		case PROTOCOL_SERVER_REQUEST_DEVICE_11_INFO:
		{
			DataBuff[i++] = Device_Get_SensorPara_ID(10);					// SensorID
			DataBuff[i++] = Device_Get_SensorPara_Sensor_Type(10);			// SensorType
			
			for(j=0; j<16; j++)
			{
				DataBuff[i++] = Device_Get_SensorPara_SensorName(10, j);		// SensorName
			}
			
			DataBuff[i++] = Device_Get_SensorPara_Sensor_ArmType(10);		// Sensor ArmType
			
			Len += 19;
		}
		break;
		
		case PROTOCOL_SERVER_REQUEST_DEVICE_12_INFO:
		{
			DataBuff[i++] = Device_Get_SensorPara_ID(11);					// SensorID
			DataBuff[i++] = Device_Get_SensorPara_Sensor_Type(11);			// SensorType
			
			for(j=0; j<16; j++)
			{
				DataBuff[i++] = Device_Get_SensorPara_SensorName(11, j);		// SensorName
			}
			
			DataBuff[i++] = Device_Get_SensorPara_Sensor_ArmType(11);		// Sensor ArmType
			
			Len += 19;
		}
		break;
		
		case PROTOCOL_SERVER_REQUEST_DEVICE_13_INFO:
		{
			DataBuff[i++] = Device_Get_SensorPara_ID(12);					// SensorID
			DataBuff[i++] = Device_Get_SensorPara_Sensor_Type(12);			// SensorType
			
			for(j=0; j<16; j++)
			{
				DataBuff[i++] = Device_Get_SensorPara_SensorName(12, j);		// SensorName
			}
			
			DataBuff[i++] = Device_Get_SensorPara_Sensor_ArmType(12);		// Sensor ArmType
			
			Len += 19;
		}
		break;
		
		case PROTOCOL_SERVER_REQUEST_DEVICE_14_INFO:
		{
			DataBuff[i++] = Device_Get_SensorPara_ID(13);					// SensorID
			DataBuff[i++] = Device_Get_SensorPara_Sensor_Type(13);			// SensorType
			
			for(j=0; j<16; j++)
			{
				DataBuff[i++] = Device_Get_SensorPara_SensorName(13, j);		// SensorName
			}
			
			DataBuff[i++] = Device_Get_SensorPara_Sensor_ArmType(13);		// Sensor ArmType
			
			Len += 19;
		}
		break;
		
		case PROTOCOL_SERVER_REQUEST_DEVICE_15_INFO:
		{
			DataBuff[i++] = Device_Get_SensorPara_ID(14);					// SensorID
			DataBuff[i++] = Device_Get_SensorPara_Sensor_Type(14);			// SensorType
			
			for(j=0; j<16; j++)
			{
				DataBuff[i++] = Device_Get_SensorPara_SensorName(14, j);		// SensorName
			}
			
			DataBuff[i++] = Device_Get_SensorPara_Sensor_ArmType(14);		// Sensor ArmType
			
			Len += 19;
		}
		break;
		
		case PROTOCOL_SERVER_REQUEST_DEVICE_16_INFO:
		{
			DataBuff[i++] = Device_Get_SensorPara_ID(15);					// SensorID
			DataBuff[i++] = Device_Get_SensorPara_Sensor_Type(15);			// SensorType
			
			for(j=0; j<16; j++)
			{
				DataBuff[i++] = Device_Get_SensorPara_SensorName(15, j);		// SensorName
			}
			
			DataBuff[i++] = Device_Get_SensorPara_Sensor_ArmType(15);		// Sensor ArmType
			
			Len += 19;
		}
		break;
		
		case PROTOCOL_SERVER_REQUEST_DEVICE_17_INFO:
		{
			DataBuff[i++] = Device_Get_SensorPara_ID(16);					// SensorID
			DataBuff[i++] = Device_Get_SensorPara_Sensor_Type(16);			// SensorType
			
			for(j=0; j<16; j++)
			{
				DataBuff[i++] = Device_Get_SensorPara_SensorName(16, j);		// SensorName
			}
			
			DataBuff[i++] = Device_Get_SensorPara_Sensor_ArmType(16);		// Sensor ArmType
			
			Len += 19;
		}
		break;
		
		case PROTOCOL_SERVER_REQUEST_DEVICE_18_INFO:
		{
			DataBuff[i++] = Device_Get_SensorPara_ID(17);					// SensorID
			DataBuff[i++] = Device_Get_SensorPara_Sensor_Type(17);			// SensorType
			
			for(j=0; j<16; j++)
			{
				DataBuff[i++] = Device_Get_SensorPara_SensorName(17, j);		// SensorName
			}
			
			DataBuff[i++] = Device_Get_SensorPara_Sensor_ArmType(17);		// Sensor ArmType
			
			Len += 19;
		}
		break;
		
		case PROTOCOL_SERVER_REQUEST_DEVICE_19_INFO:
		{
			DataBuff[i++] = Device_Get_SensorPara_ID(18);					// SensorID
			DataBuff[i++] = Device_Get_SensorPara_Sensor_Type(18);			// SensorType
			
			for(j=0; j<16; j++)
			{
				DataBuff[i++] = Device_Get_SensorPara_SensorName(18, j);		// SensorName
			}
			
			DataBuff[i++] = Device_Get_SensorPara_Sensor_ArmType(18);		// Sensor ArmType
			
			Len += 19;
		}
		break;
		
		case PROTOCOL_SERVER_REQUEST_DEVICE_20_INFO:
		{
			DataBuff[i++] = Device_Get_SensorPara_ID(19);					// SensorID
			DataBuff[i++] = Device_Get_SensorPara_Sensor_Type(19);			// SensorType
			
			for(j=0; j<16; j++)
			{
				DataBuff[i++] = Device_Get_SensorPara_SensorName(19, j);		// SensorName
			}
			
			DataBuff[i++] = Device_Get_SensorPara_Sensor_ArmType(19);		// Sensor ArmType
			
			Len += 19;
		}
		break;	
		
		case PROTOCOL_SERVER_REQUEST_SET_WORKMODE_AWAYARM:
		{
			App_Terminal_ModeChange(0xFF, TERMINAL_WORK_MODE_AWAYARM, TERMINAL_CMD_SOURCE_SERVER);
			
			DataBuff[i++] = 'S';
			DataBuff[i++] = 'e';
			DataBuff[i++] = 't';
			DataBuff[i++] = ' ';
			DataBuff[i++] = 'O';
			DataBuff[i++] = 'K';

			Len += 6;
		}
		break;
		
		case PROTOCOL_SERVER_REQUEST_SET_WORKMODE_HOMEARM:
		{
			App_Terminal_ModeChange(0xFF, TERMINAL_WORK_MODE_HOMEARM, TERMINAL_CMD_SOURCE_SERVER);
			
			DataBuff[i++] = 'S';
			DataBuff[i++] = 'e';
			DataBuff[i++] = 't';
			DataBuff[i++] = ' ';
			DataBuff[i++] = 'O';
			DataBuff[i++] = 'K';

			Len += 6;
		}
		break;
		
		case PROTOCOL_SERVER_REQUEST_SET_WORKMODE_DISARM:
		{
			App_Terminal_ModeChange(0xFF, TERMINAL_WORK_MODE_DISARM, TERMINAL_CMD_SOURCE_SERVER);
			
			DataBuff[i++] = 'S';
			DataBuff[i++] = 'e';
			DataBuff[i++] = 't';
			DataBuff[i++] = ' ';
			DataBuff[i++] = 'O';
			DataBuff[i++] = 'K';

			Len += 6;
		}
		break;
		
		case PROTOCOL_SERVER_REQUEST_SET_WORKMODE_ALARM:
		{
			App_Terminal_ModeChange(0xFF, TERMINAL_WORK_MODE_ALARMING, TERMINAL_CMD_SOURCE_SERVER);
			
			DataBuff[i++] = 'S';
			DataBuff[i++] = 'e';
			DataBuff[i++] = 't';
			DataBuff[i++] = ' ';
			DataBuff[i++] = 'O';
			DataBuff[i++] = 'K';

			Len += 6;
		}
		break;
		
		case PROTOCOL_SERVER_REQUEST_ERROR:
		{
			DataBuff[i++] = 'E';
			DataBuff[i++] = 'r';
			DataBuff[i++] = 'r';
			DataBuff[i++] = 'o';
			DataBuff[i++] = 'r';

			Len += 5;
		}
		break;	
	}
	
	DataBuff[3] = (Len >> 8) & 0xFF;	// payload length
	DataBuff[4] = Len & 0xFF;			
	
	DataBuff[0] = (i >> 8) & 0xFF;		// DataFrame length
	DataBuff[1] = i & 0xFF;
	
	// pack processed payload data
	MQTTProtocol_DataPack((en_Protocol_CommType_t)CommType, &DataBuff[0]);
}

/**
  * @Brief	Pack the Check New Firmware command payload
  * @Param	CommType: communication type
  * @Retval	None
  */
void MQTTProtocol_NewFirmwareCheck_DataPack(unsigned char CommType)
{
	unsigned char DataBuff[100];
	unsigned short i;
	
	i = 2;
	
	DataBuff[i++] = PROTOCOL_TERMINAL_REQUEST_UPDATE_CHECK;		// CommandCode
	DataBuff[i++] = 0;	// payload length
	DataBuff[i++] = 0;	// payload length
	
	DataBuff[0] = (i >> 8) & 0xFF;		// dataframe length high byte
	DataBuff[1] = i & 0xFF;				// dataframe length low byte
	
	// pack processed payload data
	MQTTProtocol_DataPack((en_Protocol_CommType_t)CommType, &DataBuff[0]);
}

/**
  * @Brief	Pack the Get New Firmware Version command payload
  * @Param	CommType	: communication type
  *			PackageIndex: index of target package 
  *			pVersion	: Version of New Firmware to download
  * @Retval	None
  *	@Note	FrameID	: used for identifying dataframe, server will response the same FrameID to indicate receiving the corresponding dataframe
  */
void MQTTProtocol_GetNewFirmware_DataPack(unsigned char CommType, unsigned short PackageIndex, unsigned char *pVersion)
{
	unsigned char DataBuff[100];
	unsigned short i;
	
	static unsigned char FrameID = 0;	// indicate the index of request dataframe
	
	i = 2;
	
	DataBuff[i++] = PROTOCOL_TERMINAL_REQUEST_UPDATE_FIRMWARE;	// CommandCode
	DataBuff[i++] = 0;			// payload length
	DataBuff[i++] = 0;			// payload length
	DataBuff[i++] = FrameID++;
	
	DataBuff[i++] = *pVersion;	// Version high byte
	pVersion++;
	DataBuff[i++] = *pVersion;	// Version low byte
	
	DataBuff[i++] = (PackageIndex >> 8) & 0xFF;
	DataBuff[i++] = PackageIndex & 0xFF;
	
	DataBuff[0] = (i >> 8) & 0xFF;
	DataBuff[1] = i & 0xFF;
	
	DataBuff[3] = 0;
	DataBuff[4] = 5;
	
	// pack processed payload data
	MQTTProtocol_DataPack((en_Protocol_CommType_t)CommType, &DataBuff[0]);
}


/*-------------Internal Functions Definition--------*/
/**
  * @Brief	Pack the provided payload to prepare(add Header and XORCheck) the final MQTT_Protocol DataFrame to the server
  * @Param	CommType: communication type
  *			pData	: point to the provided data(Data Length: first 2 byte)
  * @Retval	None
  */
static void MQTTProtocol_DataPack(en_Protocol_CommType_t CommType, unsigned char *pData)
{
	unsigned char XORCheck;
	unsigned char DataBuff[200];
	unsigned short Len;
	unsigned short i;
	
	Len = pData[0];
	Len <<= 8;
	Len |= pData[1];
	
	XORCheck = 0;
	
	DataBuff[2] = 0xAA;								// Header
	DataBuff[3] = (Len >> 8) & 0xFF;	// Length highbyte
	DataBuff[4] = Len & 0xFF;					// Length lowbyte
	
	Len -= 2;
	
	XORCheck = DataBuff[3];
	XORCheck ^= DataBuff[4];
	
	for(i=0; i<Len; i++)				// DataPayload
	{
		DataBuff[i+5] = pData[i+2];
		XORCheck ^= pData[i+2];
	}
	
	Len = Len + 5;
	
	DataBuff[Len++] = XORCheck;		// CheckValue
	DataBuff[Len++] = 0x55;				// Tail
	
	Len -= 2;
	
	DataBuff[0] = (Len >> 8) & 0xFF;	// Length of whole DataFrame(First 2 byte)
	DataBuff[1] = Len & 0xFF;
	
	/* sendout packed dataframe to the MQTT server according to the specified module */
	if(CommType == PROTOCOL_COMM_TYPE_WIFI)
	{
		Mid_WiFi_MQTT_PublishMessage(&DataBuff[0]);
	}
	else if(CommType == PROTOCOL_COMM_TYPE_4G)
	{
		
	}
}

/**
  * @Brief	Get the ServerRequestCode from the dataframe received
  * @Param	CommType: communication type
  *			pData	: point to the provided data
  * @Retval	ServerRequestCode / 0xFF = Error
  *	@Note	pData[0], pData[1]: Data Length(high byte, low byte)
  *			pData[2]		  : CommandCode
  *			pData[3], pData[4]: Payload Length(high byte, low byte)
  *			pData[5]		  : ServerRequestCode
  *			...
  *			pData[N-1]		  : XOR value
  *			pData[N]		  : Tail(0x55)
  */
static en_Protocol_ServerRequestCode_t MQTTProtocol_ReceiveDataParse(en_Protocol_CommType_t CommType, unsigned char *pData)
{
	unsigned char i;
	
	unsigned char ServerRequestCode;
	unsigned char XORCheckValue;
	unsigned short Len;					// dataframe length
	
	Len = (pData[0] << 8) & 0xFF00;
	Len |= pData[1];
	
	XORCheckValue = pData[0];
	XORCheckValue ^= pData[1];
	
	for(i=0; i<Len-2; i++)
	{
		XORCheckValue ^= pData[i+2];
	}
	
	ServerRequestCode = pData[5];
	
	/* check whether the dataframe is valid and complete */
	if(XORCheckValue == pData[Len + 2 - 1 - 1])
	{
		return (en_Protocol_ServerRequestCode_t)ServerRequestCode;
	}
	else
	{
		return PROTOCOL_SERVER_REQUEST_ERROR;
	}
}

/**
  * @Brief	Prepare the Command-Payload to request system time from server
  * @Param	CommType: communication type
  * @Retval	None
  */
static void MQTTProtocol_TerminalRequest_SystemTime(en_Protocol_CommType_t CommType)
{
	unsigned char DataBuff[128];
	unsigned short Index;
	
	Index = 2;
	
	DataBuff[Index++] = PROTOCOL_TERMINAL_REQUEST_GET_SYSTIME;	// Command
	DataBuff[Index++] = 0;																			// DataFrameID
	DataBuff[Index++] = (SYSTETIME_UTC_ZONE >> 8) & 0xFF;				// UTC-Offset highbyte
	DataBuff[Index++] = SYSTETIME_UTC_ZONE & 0xFF;							// UTC-Offset lowbyte
	
	DataBuff[0] = (Index >> 8) & 0xFF;		// Datalength highbyte
	DataBuff[1] = Index & 0xFF;						// Datalength lowbyte
	
	MQTTProtocol_DataPack(CommType, &DataBuff[0]);
}

/*-------------Interrupt Functions Definition--------*/


