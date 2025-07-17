#ifndef __MID_WIFI_H_
#define __MID_WIFI_H_

/** Comment this macro to enable <Working Mode> of WiFi-Module
  * Uncomment this macro to enable <Debug Mode> of WiFi-Module 				*/ 
#define	WIFI_Module_DEBUG_MODE

/** Comment this macro to enable <Working Mode> of Mid_WiFi_RxDataHandler 
  * Uncomment this macro to enable <Debug Mode> of Mid_WiFi_RxDataHandler 				*/ 
//#define	WIFI_RX_DEBUG_MODE

/** Comment this macro to enable <Working Mode> of Mid_WiFi_TxDataHandler
  * Uncomment this macro to enable <Debug Mode> of Mid_WiFi_TxDataHandler 				*/ 
//#define	WIFI_TX_DEBUG_MODE


/* Tx_Queue Number */
#define WIFI_TX_QUEUE_SUM		10	
/* Tx_Buffer Size */
#define WIFI_TX_BUFFER_SIZE		200	

/* Rx_Buffer Size */
#define WIFI_RX_BUFFER_SIZE		800	

/* SSID Length */
#define WIFI_SSID_LENGTH_MAX	20

/* MQTT TxData Size */
#define WIFI_MQTT_TX_DATA_SIZE	(WIFI_TX_BUFFER_SIZE - 2)

/* ESP8266 AT-Command */
typedef enum
{
	ESP8266_AT_RESET = 0,				// Restart module
	ESP8266_AT_AT,              // AT test
	ESP8266_AT_ATE1,            // AT command echo enable
	
	ESP8266_AT_GETWIFILIST,			// "AT+CWSTATE?\0"		
	ESP8266_AT_CWMODE,	        // "AT+CWMODE=1\0"		
	ESP8266_AT_CWAUTOCONN,			// "AT+CWAUTOCONN=1\0"
	ESP8266_AT_CWSTARTSMART,		// "AT+CWSTARTSMART=2\0"	
	ESP8266_AT_CWSTOPSMART,	    // "AT+CWSTOPSMART\0"		
	ESP8266_AT_CWSTATE,         // "AT+CWSTATE?\0"	
	ESP8266_AT_CWLAP,	        	// "AT+CWLAP=\"\0"		
	
	ESP8266_AT_MQTTUSERCFG,			// "AT+MQTTUSERCFG=0,1,\""
	ESP8266_AT_MQTTCONN,				// "AT+MQTTCONN=0,\""    
	ESP8266_AT_MQTTPUB,					// "AT+MQTTPUB=0,\""  
	ESP8266_AT_MQTTSUB,					// "AT+MQTTSUB=0,\"" 
	ESP8266_AT_MQTTSUBDATETIME,	// "AT+MQTTSUB=0,\"" 
	ESP8266_AT_MQTTCLEAN,				// "AT+MQTTCLEAN=0" 
 	
	ESP8266_AT_SUM
}en_ESP8266_AT_t;

/* ESP8266 AT-Command Response */
typedef enum
{
	ESP8266_AT_RESPONSE_WIFI_CONNECTED = 0,
	ESP8266_AT_RESPONSE_WIFI_DISCONNECT,
	
	ESP8266_AT_RESPONSE_CWSTATE,	
	ESP8266_AT_RESPONSE_CWJAP,
	ESP8266_AT_RESPONSE_CWLAP,
	
	ESP8266_AT_RESPONSE_SMART_GET_WIFI_INFO,	// SmartConfig WiFi info: <SSID>\r\n<Password>\r\n
	ESP8266_AT_RESPONSE_SMART_SUCCESS,				// SmartConfig successfully

	ESP8266_AT_RESPONSE_MQTTCONN,
	ESP8266_AT_RESPONSE_MQTTDISCONN,
	ESP8266_AT_RESPONSE_MQTTSUB_MESSAGEDOWN_SUCCESS,
	ESP8266_AT_RESPONSE_MQTTSUB_SYSTIME_SUCCESS,
	ESP8266_AT_RESPONSE_MQTTRECV_DOWN,
	ESP8266_AT_RESPONSE_MQTTRECV_SYSTIME,
	
	ESP8266_AT_RESPONSE_OK,
	ESP8266_AT_RESPONSE_ERROR,
	
	ESP8266_AT_RESPONSE_SUM,	
}en_ESP8266_ATResponse_t;

/* WiFi-Module working state */
typedef enum
{
	ESP8266_STA_MODULE_RESET,			// reset module
	ESP8266_STA_MODULE_DETECT,		// detect module
	ESP8266_STA_MODULE_INIT,			// initialize module
	ESP8266_STA_MODULE_STATE,			// check module state
	
	ESP8266_STA_NETPAIR_START,    			// start pairing Access_Point(WAP)
	ESP8266_STA_NETPAIR_IN_PROCESS, 		// pairing Access_Point in process
	ESP8266_STA_GET_SMART_WIFI_INFO,  	// get SmartConfig info
	ESP8266_STA_GET_IP_TIMEOUT,					// pairing Access_Point for more than 5mins, timeout	
	ESP8266_STA_GET_IP_SUCCESS,   			// get IP from Access_Point, WiFi connected
	
	ESP8266_STA_MODULE_READY,     
	ESP8266_STA_PUBLISH_ALARMDATA,     
	
}en_ESP8266_State_t;

/* ESP8266 AP-Connection State */
typedef enum
{
	ESP8266_LINK_0_NOCONNECTION, 	  		// 0->no WiFi connection
	ESP8266_LINK_1_CONNECTED_NOIPV4,  	// 1->AP connected, no IPv4 address
	ESP8266_LINK_2_CONNECTED_GETIPV4, 	// 2->AP connected, IPv4 address obtained
	ESP8266_LINK_3_CONNECTING,	 	  		// 3->connecting
	ESP8266_LINK_4_DISCONNECTED, 	  		// 4->disconnected
	
}en_ESP8266_LinkState_t;

/* ESP8266 Module PowerManage state: */
typedef enum
{
	ESP8266_POWER_STATE_IDLE,
	ESP8266_POWER_STATE_RESET,
	
	ESP8266_POWER_STATE_SUM,
}en_ESP8266_PowerState_t;

/* MQTT communication state: */
typedef enum 
{
	MQTT_STA_IDLE = 0,						// MQTT idle
	MQTT_STA_CONFIG,							// config MQTT attributes
	MQTT_STA_CONNECT,							// connect MQTT server
	MQTT_STA_SUB,									// subscribe topic
	MQTT_STA_SUB_DATETIME,				// subscribe $SYS/brokers/.../datetime topic
	MQTT_STA_SUB_FIRMWARE_UPDATE,
	MQTT_STA_READY,								// MQTT ready
	MQTT_STA_PUB,									// publish message
	MQTT_STA_RECV_SYSTIME,				// receive Systemtime from server
	MQTT_STA_RECV_DOWN,						// receive Message from Message_Down topic
	
}en_MQTT_State_t;


extern uint8_t SystemTime[17];


void Mid_WiFi_Init(void);
void Mid_WiFi_Pro(void);

void 	Mid_WiFi_ATcmdQueueIn(en_ESP8266_AT_t ATcmd, uint8_t *pPara);

uint8_t Mid_WiFi_GetModuleWorkState(void);
void 	Mid_WiFi_ChangeModuleWorkState(en_ESP8266_State_t State);

uint8_t Mid_WiFi_GetMQTTState(void);
void 	Mid_WiFi_ChangeMQTTState(en_MQTT_State_t State);

void 	Mid_WiFi_MQTT_PublishMessage(uint8_t *pData);

uint8_t Mid_WiFi_GetSignalLevel(void);

#endif
