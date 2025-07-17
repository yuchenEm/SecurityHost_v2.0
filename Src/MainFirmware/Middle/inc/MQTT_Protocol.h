#ifndef __MQTT_PROTOCOL_H_
#define __MQTT_PROTOCOL_H_

/* UTC time-zone define: 0x1000 0000 0000 0100(UTC-4:00) */
/* highest bit: 1->negative, 0->positive */
#define SYSTETIME_UTC_ZONE	0x8004

/* SystemTime Macro Define: */
#define Set_SystemTime_Year(x)		(stu_SystemTime.year=x)
#define Set_SystemTime_Month(x)		(stu_SystemTime.month=x)
#define Set_SystemTime_Day(x)		(stu_SystemTime.day=x)
#define Set_SystemTime_Week(x)		(stu_SystemTime.week=x)
#define Set_SystemTime_Hour(x)		(stu_SystemTime.hour=x)
#define Set_SystemTime_Minute(x)	(stu_SystemTime.minute=x)
#define Set_SystemTime_Second(x)	(stu_SystemTime.second=x)

#define Get_SystemTime_Year(x)		(stu_SystemTime.year)
#define Get_SystemTime_Month(x)		(stu_SystemTime.month)
#define Get_SystemTime_Day(x)		(stu_SystemTime.day)
#define Get_SystemTime_Week(x)		(stu_SystemTime.week)
#define Get_SystemTime_Hour(x)		(stu_SystemTime.hour)
#define Get_SystemTime_Minute(x)	(stu_SystemTime.minute)
#define Get_SystemTime_Second(x)	(stu_SystemTime.second)

/* Communication type define: */
typedef enum
{
	PROTOCOL_COMM_TYPE_WIFI = 0,
	PROTOCOL_COMM_TYPE_4G,
	
	PROTOCOL_COMM_TYPE_SUM,
}en_Protocol_CommType_t;

/* Terminal->Server / Server->Terminal request and response CommandCode: */
typedef enum
{
	PROTOCOL_TERMINAL_REQUEST_GET_SYSTIME 		= 0x29,		// request systemtime
	PROTOCOL_TERMINAL_REQUEST_UPDATE_CHECK 		= 0x21,		// check update information request
	PROTOCOL_TERMINAL_REQUEST_UPDATE_FIRMWARE 	= 0x24,		// request Firmware package
	PROTOCOL_TERMINAL_RESPONSE					= 0x51, 	// response of server command / terminal eventupload
	
	PROTOCOL_SERVER_RESPONSE_GET_SYSTIME		= 0x2A,		// server response of systemtime request
	PROTOCOL_SERVER_RESPONSE_UPDATE_CHECK		= 0x22,		// server response of update information request
	PROTOCOL_SERVER_RESPONSE_UPDATE_FIRMWARE 	= 0x25,		// server download Firmware package
	PROTOCOL_SERVER_COMMAND			 			= 0x50,		// server command

}en_Protocol_CMD_t;

/* Server->Terminal, Server Request Code: */
typedef enum
{
	PROTOCOL_SERVER_REQUEST_HOST_INFO = 0,	// request Terminal info
	PROTOCOL_SERVER_REQUEST_WORKMODE,		// request Terminal WorkMode
	
	PROTOCOL_SERVER_REQUEST_DEVICE_1_INFO,	// request Device 1 attribute info
	PROTOCOL_SERVER_REQUEST_DEVICE_2_INFO,	// request Device 2 attribute info
	PROTOCOL_SERVER_REQUEST_DEVICE_3_INFO,	// request Device 3 attribute info
	PROTOCOL_SERVER_REQUEST_DEVICE_4_INFO,	// request Device 4 attribute info
	PROTOCOL_SERVER_REQUEST_DEVICE_5_INFO,	// request Device 5 attribute info
	PROTOCOL_SERVER_REQUEST_DEVICE_6_INFO,	// request Device 6 attribute info
	PROTOCOL_SERVER_REQUEST_DEVICE_7_INFO,	// request Device 7 attribute info
	PROTOCOL_SERVER_REQUEST_DEVICE_8_INFO,	// request Device 8 attribute info
	PROTOCOL_SERVER_REQUEST_DEVICE_9_INFO,	// request Device 9 attribute info
	PROTOCOL_SERVER_REQUEST_DEVICE_10_INFO,	// request Device 10 attribute info
	PROTOCOL_SERVER_REQUEST_DEVICE_11_INFO,	// request Device 11 attribute info
	PROTOCOL_SERVER_REQUEST_DEVICE_12_INFO,	// request Device 12 attribute info
	PROTOCOL_SERVER_REQUEST_DEVICE_13_INFO,	// request Device 13 attribute info
	PROTOCOL_SERVER_REQUEST_DEVICE_14_INFO,	// request Device 14 attribute info
	PROTOCOL_SERVER_REQUEST_DEVICE_15_INFO,	// request Device 15 attribute info
	PROTOCOL_SERVER_REQUEST_DEVICE_16_INFO,	// request Device 16 attribute info
	PROTOCOL_SERVER_REQUEST_DEVICE_17_INFO,	// request Device 17 attribute info
	PROTOCOL_SERVER_REQUEST_DEVICE_18_INFO,	// request Device 18 attribute info
	PROTOCOL_SERVER_REQUEST_DEVICE_19_INFO,	// request Device 19 attribute info
	PROTOCOL_SERVER_REQUEST_DEVICE_20_INFO,	// request Device 20 attribute info
	
	PROTOCOL_SERVER_REQUEST_SET_WORKMODE_AWAYARM,	// change Terminal WorkMode to AwayArm
	PROTOCOL_SERVER_REQUEST_SET_WORKMODE_HOMEARM,	// change Terminal WorkMode to HomeArm
	PROTOCOL_SERVER_REQUEST_SET_WORKMODE_DISARM,	// change Terminal WorkMode to Disarm
	PROTOCOL_SERVER_REQUEST_SET_WORKMODE_ALARM,		// change Terminal WorkMode to Alarm
	
	PROTOCOL_SERVER_REQUEST_ERROR = 0xFF,			// invalid server request / corrupted dataframe
	
	PROTOCOL_SERVER_REQUEST_SUM,
}en_Protocol_ServerRequestCode_t;

/* Terminal UpEvent define: */
typedef enum
{
	TERMINAL_UPEVENT_UPDATE_CHECK = 0,		// check update of Firmware
	
	TERMINAL_UPEVENT_GET_SYSTIME, 			// request system time
	
	TERMINAL_UPEVENT_DETECTOR_ALARM,  		// detector alarm
	TERMINAL_UPEVENT_DETECTOR_OFFLINE,  	// detector offline
	TERMINAL_UPEVENT_DETECTOR_ONLINE,  		// detector online
	TERMINAL_UPEVENT_DETECTOR_BATLOW,		// detector battery low
	TERMINAL_UPEVENT_DETECTOR_ALARM_TEMPER,	//
	TERMINAL_UPEVENT_HOST_ALARM_SOS, 		// host SOS alarm
	TERMINAL_UPEVENT_HOST_BATLOW,			// host battery low
	TERMINAL_UPEVENT_HOST_AC_DISCONN,		// host AC disconnected
	TERMINAL_UPEVENT_HOST_AC_CONNECT,		// host AC connected
	TERMINAL_UPEVENT_DOOR_OPEN,     		// door open
	TERMINAL_UPEVENT_DOOR_CLOSE,    		// door close
 
	TERMINAL_UPEVENT_AWAYARM_BY_HOST,       // Awayarm by Host, Remote, Server
	TERMINAL_UPEVENT_AWAYARM_BY_REMOTE,      
	TERMINAL_UPEVENT_AWAYARM_BY_SERVER,         

	TERMINAL_UPEVENT_HOMEARM_BY_HOST,    	// Homearm by Host, Remote, Server
	TERMINAL_UPEVENT_HOMEARM_BY_REMOTE,  
	TERMINAL_UPEVENT_HOMEARM_BY_SERVER,     

	TERMINAL_UPEVENT_DISARM_BY_HOST,     	// Disarm by Host, Remote, Server
	TERMINAL_UPEVENT_DISARM_BY_REMOTE,   
	TERMINAL_UPEVENT_DISARM_BY_SERVER,      	
		
	TERMINAL_UPEVENT_SUM,
}en_Terminal_UpEvent_t;

/* MQTT server Endpoint define */
typedef enum
{
	ENDPOINT_MESSAGE_UPLOAD,
	ENDPOINT_TERMINAL_WORMODE_CHANGE,
	
}en_ENDPOINT_t;

/* Terminal event to MQTT server structure */
typedef struct
{
	en_Terminal_UpEvent_t Event;
	unsigned char Buff;
	
}stu_MQTTEventUpload_t;

/* SystemTime */
typedef struct 
{
	unsigned short year;
	unsigned char month;
	unsigned char day;
	unsigned char week;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
	
}stu_SystemTime_t;

extern stu_SystemTime_t	stu_SystemTime;

void MQTTProtocol_Init(void);
void MQTTProtocol_Pro(en_Protocol_CommType_t CommType);

void MQTTProtocol_ReceiveDataHandler(en_Protocol_CommType_t CommType, unsigned char *pData, unsigned char Len);
void MQTTProtocol_EventUpQueueIn(unsigned char Event, unsigned char Data);
void MQTTProtocol_EventUpload_Pro(en_Protocol_CommType_t CommType);
void MQTTProtocol_EventUpload_DataPack(en_Protocol_CommType_t CommType, unsigned char ZoneNo, en_Terminal_UpEvent_t EventType, unsigned short Endpoint);
void MQTTProtocol_ServerRequestResponse_DataPack(en_Protocol_CommType_t CommType, en_Protocol_ServerRequestCode_t ServerRequestCode);
void MQTTProtocol_NewFirmwareCheck_DataPack(unsigned char CommType);
void MQTTProtocol_GetNewFirmware_DataPack(unsigned char CommType, unsigned short PackageIndex, unsigned char *pVersion);


#endif
