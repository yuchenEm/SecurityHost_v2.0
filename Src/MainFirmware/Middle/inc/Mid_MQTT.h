#ifndef __MID_MQTT_H_
#define __MID_MQTT_H_

/* Server IP, Port macro define: */
#define MQTT_SERVER_IP		10.0.0.148
#define MQTT_SERVER_PORT	1883

/* Size of MQTT Device-Attributes define: */
#define MQTT_CLIENT_ID_SIZE    	30
#define MQTT_USERNAME_SIZE 		30
#define MQTT_PASSWORD_SIZE  	32
#define MQTT_SERVER_IP_SIZE  	16
#define MQTT_SERVER_PORT_SIZE 	10
#define MQTT_TOPIC_SIZE         40


typedef struct
{
	unsigned char event;
	unsigned char buffer[1];
}str_mqtt_Upevent;


/* NewFirmwareFlag to indicate the Firmware update state: */
typedef enum
{
	MQTT_FIRMWARE_UPDATE_AVAILABLE, // Firware new version available
	MQTT_FIRMWARE_UPDATE_NONE,
}en_MQTT_FirmwareUpdateFlag_t;

/* Structure of MQTT Device-Attributes: */
typedef struct
{
	unsigned char ClientID[MQTT_CLIENT_ID_SIZE];
	unsigned char Username[MQTT_USERNAME_SIZE];
	unsigned char Password[MQTT_PASSWORD_SIZE];
	unsigned char ServerIp[MQTT_SERVER_IP_SIZE];
	unsigned char ServerPort[MQTT_SERVER_PORT_SIZE];
	unsigned char SubTopic[MQTT_TOPIC_SIZE];
	unsigned char SubTopic_DataTime[MQTT_TOPIC_SIZE];
	unsigned char SubTopic_FirmwareUpdate[MQTT_TOPIC_SIZE];
	unsigned char PubTopic[MQTT_TOPIC_SIZE];
	
	en_MQTT_FirmwareUpdateFlag_t FirmwareUpdateFlag;  
	
}stu_MQTT_Device_t;

extern stu_MQTT_Device_t  stu_MQTT_ESP8266;

void Mid_MQTT_Init(void);
void Mid_MQTT_SetFirmwareUpdateFlag(en_MQTT_FirmwareUpdateFlag_t Flag);
uint8_t Mid_MQTT_GetFirmwareUpdateFlag(void);
void Mid_MQTT_SystemTimeProcess(uint8_t *pData, uint8_t *pDateTime);

#endif
