/****************************************************
  * @Name	Mid_WiFi.c
  * @Brief	Handle the data received from ESP8266 WiFi-module
  *  		Handle the AT-command ready-to-send to the ESP8266 WiFi-module
  * @Instruction: 
  * --> DataStructure: 
  * 	@TxBuffer: 	@Length   indicate effective data length
  *			   		@DataByte store byte data(Number up to WIFI_TX_BUFFSIZE_MAX)
  *		-------------------------------------------------------------------		   
  *		Length		DataByte1	DataByte2	DateByte3	...		Databye200
  *		-------------------------------------------------------------------
  *		0xFF,0xFF	0xFF		0xFF		0xFF		...		0xFF
  *		2 byte		1 byte		1 byte		1 byte		...		1 byte
  *	
  *		@TxBufferMatrix[][]:
  *		-------------------------------------------------------------------
  *		Length		DataByte1	DataByte2	DateByte3	...		Databye200		<0>	<--	@TxQueueIndex: point to the <Index> of current using TxBuffer
  *		Length		DataByte1	DataByte2	DateByte3	...		Databye200		<1>
  *		Length		DataByte1	DataByte2	DateByte3	...		Databye200		<2>
  *	 	 .				.			.			.					.
  *	  	 .				.			.			.					.
  *	  	 .				.			.			.					.
  *		Length		DataByte1	DataByte2	DateByte3	...		Databye200		<200>
  *		-------------------------------------------------------------------
  *		(Matrix Row number up to TX_QUEUE_SUM)
  *	
  * --> WiFi-Module AT-command Transmit Process:
  *			Mid_WiFi_ATcmdQueueIn	: trim given @ATcmd and @Parameters and transfer to Mid_WiFi_TxDataQueueIn function
  *			Mid_WiFi_TxDataQueueIn	: queue-in the preprocessed ATcmd to WiFi_TxBufferMatrix[][], and queue-in the corresponding <Index> to Queue_WiFiTxSequence
  *	 (Poll) Mid_WiFi_TxDataHandler	: queue-out the <Index> from the Queue_WiFiTxSequence, and call Mid_WiFi_TxDataSend function to send the corresponding data in WiFi_TxBufferMatrix[][]
  *			Mid_WiFi_TxDataSend		: use WiFi_USART(USART3) send the data to ESP8266 module
  *  
  * --> WiFi-Module AT-command Receive Process: 
  *			Mid_WiFi_RxDataQueueIn		: queue-in received data from module to Queue_WiFiRx(CBF of WiFi_USART)
  *			Mid_WiFi_ATResponseIdentitfy: check if there is matched @ATResponse in the received data
  *			Mid_WiFi_GetSSID			: extract @SSID from the ATResponse from module
  *			Mid_WiFi_ATResponseProcess	: according to different ATResponse, change @WorkState and @MQTTState
  *	 (Poll) Mid_WiFi_RxDataHandler		: queue-out data from Queue_WiFiRx, cast Mid_WiFi_ATResponseIdentitfy to identify the valid @ATResponse,
  *										  and process the data by casting Mid_WiFi_ATResponseProcess
  
  ***************************************************/
  
/*-------------Header Files Include-----------------*/
#include "stm32f10x.h" 
#include "mid_wifi.h"
#include "mid_mqtt.h"
#include "os_system.h"
#include "hal_gpio.h"
#include "hal_usart.h"
#include "string.h"
#include "stringprocess.h"
#include "mqtt_protocol.h"

/*---------------------- ESP8266 AT Commands: ------------------------*/
const unsigned char ESP8266_AT[ESP8266_AT_SUM][70] = 
{
	"AT+RST\0",				// Restart module
	"AT\0",						// AT test
	"ATE1\0",					// AT command echo enable
	
	"AT+CWSTATE?\0",				// Get WiFi connection state
	"AT+CWMODE=1\0",				// WiFi mode: 1->station mode, 2->softAP mode, 3->softAP + station mode
	"AT+CWAUTOCONN=1\0",		// 1->automatically connect to AP, 0->no automatically connection
	"AT+CWSTARTSMART=2\0",	// Start Smartconfig: 1->ESP-TOUCH, 2->AirKiss, 3->AirKiss + ESP-TOUCH
	"AT+CWSTOPSMART\0",			// Stop Smartconfig
	"AT+CWSTATE?\0",				// Get WiFi connection state: 0->no WiFi connection, 1->AP connected, no IPv4 address, 2->AP connected, IPv4 address obtained, 3->connecting, 4->disconnected
	"AT+CWLAP=\"\0",				// Get AP <ecn>,<SSID>,<RSSI>,<MAC>,<CH>,<FREQ OFFSET>,<FREQ CALIBRATION> with specific SSID
	
	"AT+MQTTUSERCFG=0,1,\"",	// MQTT user config: <LinkID>=0, <scheme>=1: MQTT over TCP, <"client ID">, <"username">, <"password">, <cert_key_ID>, <CA_ID>, <"path">
	"AT+MQTTCONN=0,\"",				// Connect to MQTT broker: <LinkID>=0, <"host">, <port>, <reconnect>0->no auto-reconnect, 1->auto-reconnect
	"AT+MQTTPUB=0,\"",				// Publish MQTT message: <LinkID>=0, <"topic">, <"data">, <qos>:0, 1, 2, default 0, <retain>: retain flag
	"AT+MQTTSUB=0,\"",				// Subscribe to MQTT topic(1): <LinkID>=0, <"topic">, <qos>
	"AT+MQTTSUB=0,\"",				// Subscribe to MQTT topic(2): <LinkID>=0, <"topic">, <qos>
	"AT+MQTTCLEAN=0",					// Close the MQTT connection: <LinkID>=0
};

/*------------------- ESP8266 AT Commands Response: -------------------*/
const unsigned char ESP8266_ATResponse[ESP8266_AT_RESPONSE_SUM][60]=
{
	"WIFI CONNECTED\0",   
	"WIFI DISCONNECT\0",
	
	"+CWSTATE:\0",    
	"+CWJAP:\0",			
	"+CWLAP:\0",
	
	"Smart get wifi info\0",
	"smartconfig connected wifi\0",     

	"+MQTTCONNECTED:0\0",
	"+MQTTDISCONNECTED:0\0",
	"+MQTTSUB=0,\"<UID>_MessageDown\",",
	"+MQTTSUB=0,\"$SYS/brokers/emqx@127.0.0.1/datetime\",",
	"+MQTTSUBRECV:0,\"<UID>_MessageDown\",\0",
	"+MQTTSUBRECV:0,\"$SYS/brokers/emqx@127.0.0.1/datetime\",\0",
	
	"OK\r\n\0",    
	"ERROR\0", 	
};


/*-------------Internal Functions Declaration------*/
static void 	Mid_WiFi_RxDataQueueIn(uint8_t Data);
static uint8_t 	Mid_WiFi_ATResponseIdentitfy(uint8_t *pTarget, uint8_t *pATResponseIndex, uint8_t *pStartMatchIndex, uint16_t Len);
static uint8_t 	Mid_WiFi_GetSSID(uint8_t *pData, uint8_t SSID[]);
static void 	Mid_WiFi_ATResponseProcess(uint8_t *pData, en_ESP8266_ATResponse_t ATResponse, uint16_t Len);
static void 	Mid_WiFi_RxDataHandler(void);

static void 	Mid_WiFi_TxDataQueueIn(uint8_t *pData);
static void 	Mid_WiFi_TxDataSend(uint8_t *pData);
static void 	Mid_WiFi_TxDataHandler(void);

static uint8_t 	Mid_WiFi_PowerManage(en_ESP8266_PowerState_t State);

static uint8_t 	Mid_WiFi_MQTT_Pro(void);
static uint8_t 	Mid_WiFi_MQTTRxDataHandler(uint8_t *pData, uint8_t *pReceiveData);

/*-------------Module Variables Declaration--------*/
uint8_t WiFi_TxQueueIndex;			// pointer of current ready-to-send queue
uint8_t WiFi_TxBufferMatrix[WIFI_TX_QUEUE_SUM][WIFI_TX_BUFFER_SIZE];

uint8_t WiFi_RxBuffer[WIFI_RX_BUFFER_SIZE];

uint8_t WiFi_SSID[WIFI_SSID_LENGTH_MAX];

en_ESP8266_State_t 		WiFi_WorkState;
en_ESP8266_LinkState_t 	WiFi_LinkState;
en_MQTT_State_t			WiFi_MQTTState;

volatile Queue1K Queue_WiFiRx;
		 Queue16 Queue_WiFiTxSequence;	// Index(WiFi_TxQueuePos) of ready-to-send WiFi_Tx-dataframe

/* System time from server: */
uint8_t SystemTime[17];

/* PublishData for Test: */
uint8_t MQTT_PubDataTest[] = {0,6,0xAA,0x06,0x29,0x08,0x27,0x55};


/*---Module Call-Back function pointer Definition---*/



/*-------------Module Functions Definition---------*/
/**
  * @Brief	Initialize WiFi module
  * @Param	None
  * @Retval	None
  */
void Mid_WiFi_Init(void)
{
	uint8_t i;
	
	QueueEmpty(Queue_WiFiRx);
	QueueEmpty(Queue_WiFiTxSequence);
	
	WiFi_TxQueueIndex = 0;
	WiFi_WorkState = ESP8266_STA_MODULE_DETECT;
	WiFi_LinkState = ESP8266_LINK_0_NOCONNECTION;
	WiFi_MQTTState = MQTT_STA_IDLE;
	
	memset(&WiFi_RxBuffer[0], 0, WIFI_RX_BUFFER_SIZE);
	memset(&WiFi_SSID[0], 0, WIFI_SSID_LENGTH_MAX);
	
	for(i=0; i<WIFI_TX_QUEUE_SUM; i++)
	{
		memset(&WiFi_TxBufferMatrix[i], 0, sizeof(WiFi_TxBufferMatrix[i]));
	}
	
	/* register Mid_WiFi_RxDataQueueIn as the CBF for WiFi_USART(USART3) IRQHandler */
	Hal_USART_WiFiRxCBFRegister(Mid_WiFi_RxDataQueueIn);
}

/**
  * @Brief	Polling in sequence for WiFi module
  * @Param	None
  * @Retval	None
  */
void Mid_WiFi_Pro(void)
{
	/* Debug Mode: */
	#ifdef WIFI_Module_DEBUG_MODE
	Mid_WiFi_RxDataHandler();
	Mid_WiFi_TxDataHandler();
	#endif
	
	/* Working Mode: */
	/* WiFi-module works only when the outsource AC is connected */
	#ifndef WIFI_Module_DEBUG_MODE
	if(Mid_WiFi_PowerManage(ESP8266_POWER_STATE_IDLE))
	{
		Mid_WiFi_RxDataHandler();
		Mid_WiFi_TxDataHandler();
	}
	#endif
}

/**
  * @Brief	According to the provided @ATcmd and Para indicator @pPara, 
  *			preprocess and queue-in the AT command
  * @Param	ATcmd: Corresponding AT command 
  *			pPara: AT command parameters, @0xFF indicates there is no parameters followed 
  * @Retval	None
  */
void Mid_WiFi_ATcmdQueueIn(en_ESP8266_AT_t ATcmd, uint8_t *pPara)
{
	uint16_t i;
	uint8_t DataBuff[WIFI_TX_BUFFER_SIZE];
	
	if(ATcmd < ESP8266_AT_SUM)
	{
		for(i=0; i<WIFI_TX_BUFFER_SIZE; i++)	// extract AT command content from ESP8266_AT[][] array
		{
			if(ESP8266_AT[ATcmd][i] != 0)	// content before "\0"
			{
				// Dataframe first 2byte reserved for <Length>
				DataBuff[i+2] = ESP8266_AT[ATcmd][i];
			}
			else	// examine if there is any AT-parameters followed
			{
				if(*pPara != 0xFF)
				{
					if(ATcmd == ESP8266_AT_CWLAP)	// provide specified SSID
					{
						while(*pPara != 0xFF)
						{
							DataBuff[i+2] = *pPara;
							i++;
							pPara++;
						}
						
						DataBuff[i+2] = '"';
						i++;
					}
					else
					{
						while(*pPara != 0)
						{
							DataBuff[i+2] = *pPara;
							i++;
							pPara++;
						}
					}
				}
				
				DataBuff[i+2] = 0x0D;
				i++;
				DataBuff[i+2] = 0x0A;
				i++;
				
				DataBuff[0] = ((i+2) >> 8) & 0xFF;	// Len highbyte
				DataBuff[1] = (i+2) & 0xFF;					// Len lowbyte
				
				Mid_WiFi_TxDataQueueIn(DataBuff);
				
				break;
			}
		}
	}
}

/**
  * @Brief	Get current WiFi-Module working state
  * @Param	None
  * @Retval	Work state
  */
uint8_t Mid_WiFi_GetModuleWorkState(void)
{
	return WiFi_WorkState;
}

/**
  * @Brief	Change WiFi-Module to specific working state 
  * @Param	State: target working state(en_ESP8266_State_t)
  * @Retval	None
  */
void Mid_WiFi_ChangeModuleWorkState(en_ESP8266_State_t State)
{
	WiFi_WorkState = State;
	QueueEmpty(Queue_WiFiRx);	// Clear the Queue_WiFiRx buffer after changing module working state
}

/**
  * @Brief	Get current MQTT state
  * @Param	None
  * @Retval	Current MQTT state
  */
uint8_t Mid_WiFi_GetMQTTState(void)
{
	return WiFi_MQTTState;
}

/**
  * @Brief	Change MQTT state
  * @Param	State: target MQTT state
  * @Retval	None
  */
void Mid_WiFi_ChangeMQTTState(en_MQTT_State_t State)
{
	WiFi_MQTTState = State;
	
	QueueEmpty(Queue_WiFiRx);
}

/**
  * @Brief	Publish specified message to <PubTopic>
  * @Param	pData: point to the ready-to-publish message string
  * @Retval	None
	@Note	"AT+MQTTPUB=0, <"topic">, <"data">, <qos>, <retain>"
			<qos>	: 0, 1, 2, default 0
			<retain>: retain flag
  */
void Mid_WiFi_MQTT_PublishMessage(uint8_t *pData)
{
	uint8_t i;
	uint8_t Index;
	uint8_t Char_H;
	uint8_t Char_L;
	uint16_t Len;
	uint8_t MQTTDataBuff[WIFI_MQTT_TX_DATA_SIZE];
	
	if((Mid_WiFi_GetMQTTState() == MQTT_STA_READY) && 
	   (Mid_WiFi_GetModuleWorkState() == ESP8266_STA_MODULE_READY))
	{
		Index = 0;
		i = 0;
		
		while(stu_MQTT_ESP8266.PubTopic[i])
		{
			MQTTDataBuff[Index++] = stu_MQTT_ESP8266.PubTopic[i++];
			
			if(i == MQTT_TOPIC_SIZE)
			{
				break;
			}
		}
		MQTTDataBuff[Index++] = '\"';
		MQTTDataBuff[Index++] = ',';
		MQTTDataBuff[Index++] = '\"';
		
		/* First 2 bytes of PublishMessage DataFrame indicate <Length> of the data */
		Len = *pData << 8;
		pData++;
		Len += *pData;
		pData++;
		
		while(Len--)
		{
			Hex_ASCII_Conversion_Segment(*pData, &Char_H, &Char_L);
			
			MQTTDataBuff[Index++] = Char_H;
			MQTTDataBuff[Index++] = Char_L;
			pData++;
			
			if(Index >= (WIFI_MQTT_TX_DATA_SIZE - 6))
			{
				break;
			}
		}
			MQTTDataBuff[Index++] = '\"';
			MQTTDataBuff[Index++] = ',';
			MQTTDataBuff[Index++] = '2';
			MQTTDataBuff[Index++] = ',';
			MQTTDataBuff[Index++] = '0';
			MQTTDataBuff[Index++] = '\0';
			
			Mid_WiFi_ATcmdQueueIn(ESP8266_AT_MQTTPUB, &MQTTDataBuff[0]);
			
	}
}

/**
  * @Brief	According to the RSSI, return the WiFi signal level
  * @Param	None
  * @Retval	WiFi signal level(0->3: weak->excellent, 0xFF->disconnected)
  *	@Note	RSSI: Received Signal Strength Indicator
			-30dBm to -70dBm 	-> excellent
			-71dBm to -80dBm 	-> good
			-81dBm to -90dBm	-> fair
			< -90dBm			-> weak
  */
uint8_t Mid_WiFi_GetSignalLevel(void)
{
	if(WiFi_LinkState == ESP8266_LINK_2_CONNECTED_GETIPV4)
	{
		return 3;
		
	}
	else
	{
		return 0xFF;
	}
}

/*-------------Internal Functions Definition--------*/
/**
  * @Brief	Queue-in the recevied data from WiFi-module to Queue_WiFiRx(handler of WiFi_USART_RxCBF)
  * @Param	RxData: byte data ready for queue-in
  * @Retval	None
  */
static void Mid_WiFi_RxDataQueueIn(uint8_t RxData)
{
	QueueDataIn(Queue_WiFiRx, &RxData, 1);
}

/**
  * @Brief	According to the provided target starting @Address(pTarget) and @Length(Len),
  *			Check if there is matched ATResponse,
			if identify succeed, get the corresponding @ATResponseIndex(pATResponseIndex) and @StartMatchIndex(pStartMatchIndex)
  * @Param	pTarget			: point to the target need to be identified
  *			pATResponseIndex: Index of the corresponding ATResponse
  *			pStartMatchIndex: Start Matching Index of target
  *			Len				: length of target
  * @Retval	0-->Identify succeed; 0xFF-->Identify fail(no valid ATResponse matched)
  */
static uint8_t Mid_WiFi_ATResponseIdentitfy(uint8_t *pTarget, uint8_t *pATResponseIndex, uint8_t *pStartMatchIndex, uint16_t Len)
{
	uint32_t StartMatchIndex;
	uint32_t i;
	
	for(i=0; i<ESP8266_AT_RESPONSE_SUM; i++)
	{
		StartMatchIndex = StringMatch(pTarget, (uint8_t*)ESP8266_ATResponse[i], Len);
		
		if(StartMatchIndex != 0xFF)
		{
			*pATResponseIndex = i;
			*pStartMatchIndex = StartMatchIndex;
			
			return 0;
		}
	}
	
	return 0xFF;
}

/**
  * @Brief	According to the given AT_Response, extract the SSID
  * @Param	pData	 : ponit to the received AT_Response
  *			SSID[]	 : store captured SSID
  * @Retval	StateFlag: WiFi connection state(0->4)
  */
static uint8_t Mid_WiFi_GetSSID(uint8_t *pData, uint8_t SSID[])
{
	uint8_t StateFlag;
	
	// +CWSTATE:2,"xxxxxxxx"
	while(*pData != ':')
	{
		pData++;
	}
	
	pData++;
	
	StateFlag = *pData - 0x30;	// '0' ASCII-> 0x30, modify *pData to decimal value

	pData += 3;		// skip ',' and ':'
	
	while(*pData != '"')
	{
		*SSID = *pData;
		
		SSID++;
		pData++;
	}
	
	return StateFlag;
}

/**
  * @Brief	AT-command response process function
  * @Param	pData	  	: point to the target data
  *			ATResponse	: defined AT-command response
  *			Len		  	: length of target data
  * @Retval	None
  */
static void Mid_WiFi_ATResponseProcess(uint8_t *pData, en_ESP8266_ATResponse_t ATResponse, uint16_t Len)
{
	static uint8_t MQTT_ReceiveDataLen;
	static uint8_t DataBuff[WIFI_RX_BUFFER_SIZE];
	static uint8_t HexDataBuff[WIFI_RX_BUFFER_SIZE];
	
	switch((uint8_t)ATResponse)
	{
		case ESP8266_AT_RESPONSE_WIFI_CONNECTED:
		{
			if(Mid_WiFi_GetModuleWorkState() != ESP8266_STA_GET_SMART_WIFI_INFO)
			{
				Mid_WiFi_ChangeMQTTState(MQTT_STA_IDLE);
			}
		}
		break;
		
		case ESP8266_AT_RESPONSE_WIFI_DISCONNECT:
		{
			if(Mid_WiFi_GetModuleWorkState() != ESP8266_STA_NETPAIR_IN_PROCESS)
			{
				Mid_WiFi_ChangeModuleWorkState(ESP8266_STA_MODULE_DETECT);
				Mid_WiFi_ChangeMQTTState(MQTT_STA_IDLE);
			}
		}
		break;
		
		case ESP8266_AT_RESPONSE_CWSTATE:
		{
			memset(&WiFi_SSID[0], 0, WIFI_SSID_LENGTH_MAX);
			
			WiFi_LinkState = (en_ESP8266_LinkState_t)Mid_WiFi_GetSSID(pData, WiFi_SSID);
			
			if(WiFi_LinkState == ESP8266_LINK_2_CONNECTED_GETIPV4)	
			{
				Mid_WiFi_ChangeModuleWorkState(ESP8266_STA_MODULE_READY);
			}
			else
			{
				Mid_WiFi_ChangeModuleWorkState(ESP8266_STA_MODULE_STATE);
			}
		}
		break;
		
		case ESP8266_AT_RESPONSE_CWJAP:
		{
		
		}
		break;
		
		case ESP8266_AT_RESPONSE_CWLAP:
		{
		
		}
		break;
		
		case ESP8266_AT_RESPONSE_SMART_GET_WIFI_INFO:
		{
			if(Mid_WiFi_GetModuleWorkState() == ESP8266_STA_NETPAIR_IN_PROCESS)
			{
				Mid_WiFi_ChangeModuleWorkState(ESP8266_STA_GET_SMART_WIFI_INFO);
			}
		}
		break;
		
		case ESP8266_AT_RESPONSE_SMART_SUCCESS:
		{
			if(Mid_WiFi_GetModuleWorkState() == ESP8266_STA_GET_SMART_WIFI_INFO)
			{
				Mid_WiFi_ChangeModuleWorkState(ESP8266_STA_GET_IP_SUCCESS);
			}
		}
		break;
		
		case ESP8266_AT_RESPONSE_MQTTDISCONN:
		{
			Mid_WiFi_ChangeModuleWorkState(ESP8266_STA_MODULE_DETECT);
			Mid_WiFi_ChangeMQTTState(MQTT_STA_IDLE);
		}
		break;
		
		case ESP8266_AT_RESPONSE_MQTTRECV_DOWN:
		{
			MQTT_ReceiveDataLen = Mid_WiFi_MQTTRxDataHandler(pData, &DataBuff[0]);
			
			ASCII_Hex_Conversion(&DataBuff[0], MQTT_ReceiveDataLen, &HexDataBuff[0]);
			
			MQTTProtocol_ReceiveDataHandler(PROTOCOL_COMM_TYPE_WIFI, &HexDataBuff[0], MQTT_ReceiveDataLen);
			
		}
		break;

		case ESP8266_AT_RESPONSE_MQTTRECV_SYSTIME:
		{
			MQTT_ReceiveDataLen = Mid_WiFi_MQTTRxDataHandler(pData, &DataBuff[0]);

			Mid_MQTT_SystemTimeProcess(&DataBuff[0], &SystemTime[0]);

			Mid_WiFi_ChangeMQTTState(MQTT_STA_RECV_SYSTIME);
		}
		break;
		
		case ESP8266_AT_RESPONSE_OK:
		{
			if(Mid_WiFi_GetModuleWorkState() == ESP8266_STA_MODULE_DETECT)
			{
				Mid_WiFi_ChangeModuleWorkState(ESP8266_STA_MODULE_INIT);
			}
			else if(Mid_WiFi_GetModuleWorkState() == ESP8266_STA_MODULE_READY)
			{
				switch(Mid_WiFi_GetMQTTState())
				{
					case MQTT_STA_CONFIG:
					{
						Mid_WiFi_ChangeMQTTState(MQTT_STA_CONNECT);
					}
					break;
					
					case MQTT_STA_CONNECT:
					{
						Mid_WiFi_ChangeMQTTState(MQTT_STA_SUB);
					}
					break;
					
					case MQTT_STA_SUB:
					{
						Mid_WiFi_ChangeMQTTState(MQTT_STA_SUB_DATETIME);
					}
					break;

					case MQTT_STA_SUB_DATETIME:
					{
						Mid_WiFi_ChangeMQTTState(MQTT_STA_READY);
					}
					break;
				}
			}
		}
		break;
		
		case ESP8266_AT_RESPONSE_ERROR:
		{
		
		}
		break;
		
	}
}

/**
  * @Brief	Polling function to handle RxData from ESP8266
  * @Param	None
  * @Retval	None
  *	@Note	Comment/Uncomment the WIFI_RX_DEBUG_MODE macro define in mid_wifi.h
			to switch <Direct Communication Mode>/<Debug Mode>
  */
static void Mid_WiFi_RxDataHandler(void)
{
	/* Debug Mode: */
	#ifdef WIFI_RX_DEBUG_MODE
	uint8_t i;
	uint8_t Len;
	uint8_t RxBuff[20];
	
	Len = QueueDataLen(Queue_WiFiRx);
	
	if(Len > 2)	// at least have 2 byte: \r\n
	{
		if(Len >= 20)
		{
			Len = 20;
		}
		
		for(i=0; i<Len; i++)
		{
			QueueDataOut(Queue_WiFiRx, &RxBuff[i]);
		}
		
		Hal_USART_DebugDataQueueIn(RxBuff, Len);
	}
	#endif

	/* Working Mode: */
	#ifndef WIFI_RX_DEBUG_MODE
	
	static uint16_t RxBuffIndex = 0;
	
	uint8_t RxData;
	uint8_t Flag;
	uint8_t StartMatchIndex;
	en_ESP8266_ATResponse_t ATResponseIndex;
	
	while(QueueDataLen(Queue_WiFiRx) > 1)	// Dataframe at least have 2 byte: \r\n
	{
		if(RxBuffIndex >= (WIFI_RX_BUFFER_SIZE - 5))	// roll-over to the head of Queue_WiFiRx
		{
			RxBuffIndex = 0;
			return;
		}
		
		QueueDataOut(Queue_WiFiRx, &RxData);
		WiFi_RxBuffer[RxBuffIndex++] = RxData;	// Queue-out data from Queue_WiFiRx, and store them in the WiFi_RxBuffer
		
		if((RxData == 0x0D) || (RxData == 0x0A))
		{
			if(RxData == 0x0D)
			{
				WiFi_RxBuffer[RxBuffIndex++] = 0x0A;
				QueueDataOut(Queue_WiFiRx, &RxData);
			}
			
			if(RxBuffIndex > 2)	
			{
				Flag = Mid_WiFi_ATResponseIdentitfy(&WiFi_RxBuffer[0],  (uint8_t*)&ATResponseIndex, &StartMatchIndex, RxBuffIndex);
				
				if(Flag == 0)	// identify succeed(matched ATResponse found)
				{
					Mid_WiFi_ATResponseProcess(&WiFi_RxBuffer[0], ATResponseIndex, RxBuffIndex);
				}
			}
			
			// Reset WiFi_RxBuffer[] and RxBuffIndex
			memset(&WiFi_RxBuffer[0], 0, WIFI_RX_BUFFER_SIZE);
			RxBuffIndex = 0;
			
			return;
		}
	}
	
	#endif		
}


/**
  * @Brief	Store the trimmed ATcmd dataframe to WiFi_TxBufferMatrix, 
  *			and queue-in the corresponding WiFi_TxQueueIndex to Queue_WiFiTxSequence
  * @Param	pData: point to the trimmed data
  * @Retval	None
  */
static void Mid_WiFi_TxDataQueueIn(uint8_t *pData)
{
	uint16_t Len;
	uint16_t i;
	
	Len = ((pData[0] << 8) | (pData[1]));
	
	for(i=0; i<Len; i++)
	{
		WiFi_TxBufferMatrix[WiFi_TxQueueIndex][i] = pData[i];
	}
	
	QueueDataIn(Queue_WiFiTxSequence, &WiFi_TxQueueIndex, 1);
	WiFi_TxQueueIndex++;		// point to the next TxQueue
	
	if(WiFi_TxQueueIndex >= WIFI_TX_QUEUE_SUM)
	{
		WiFi_TxQueueIndex = 0;	// roll-over to the position 0
	}
}

/**
  * @Brief	Send out the content of data(get rid of 2byte Length) through WiFi_USART to the ESP8266 module 
  * @Param	pData: point to the datafrmae ready to send
  * @Retval	None
  */
static void Mid_WiFi_TxDataSend(uint8_t *pData)
{
	uint16_t i;
	uint16_t Len;
	uint8_t TxDataBuff[WIFI_TX_BUFFER_SIZE];
	
	Len = ((pData[0] << 8) | (pData[1]));
	
	Len -= 2;	// trim the first 2byte(length indicator)
	
	for(i=0; i<Len; i++)
	{
		TxDataBuff[i] = pData[i+2];
		
		#ifdef DEBUG_WIFI_TX
		QueueDataIn(Queue_DebugTx, &TxDataBuff[i], 1);	
		#endif
	}
	
	Hal_USART_WiFiDataTx(&TxDataBuff[0], Len);
}

/**
  * @Brief	Polling function to handle TxData to ESP8266
  * @Param	None
  * @Retval	None
  */
static void Mid_WiFi_TxDataHandler(void)
{
	static uint32_t AT_IntervalCounter = 0;
	static uint32_t FirmwareCounter = 0;
	static uint32_t WorkCounter = 0;
	static uint16_t ATResendCounter = 0;
	
	uint8_t Index;
	uint8_t Para;
	
	if(QueueDataLen(Queue_WiFiTxSequence))
	{
		AT_IntervalCounter++;
		
		if(AT_IntervalCounter > 10)	// send AT command interval = 100ms
		{
			AT_IntervalCounter = 0;
			
			QueueDataOut(Queue_WiFiTxSequence, &Index);				// get the index of ready-to-send queue
			
			Mid_WiFi_TxDataSend(&WiFi_TxBufferMatrix[Index][0]);	// send out the corresponding AT command through WiFi_USART to ESP8266
		}
	}
	
	/* Debug Mode: */
	#ifdef WIFI_TX_DEBUG_MODE
	DebugCounter++;
	
	if(DebugCounter > 200)
	{
		DebugCounter = 0;
		Para = 0xFF;
		
		Mid_WiFi_ATcmdQueueIn(ESP8266_AT_AT, &Para);
		Mid_WiFi_ATcmdQueueIn(ESP8266_AT_ATE1, &Para);
		Mid_WiFi_ATcmdQueueIn(ESP8266_AT_GETWIFILIST, &Para);
	}
	#endif
	
	/* Working Mode: */
	#ifndef WIFI_TX_DEBUG_MODE
	switch(Mid_WiFi_GetModuleWorkState())
	{
		case ESP8266_STA_MODULE_RESET:
		{
			WorkCounter++;
			
			if(WorkCounter > 200)
			{
				WorkCounter = 0;
				Para = 0xFF;
				
				Mid_WiFi_ATcmdQueueIn(ESP8266_AT_RESET, &Para);
				
				Mid_WiFi_ChangeModuleWorkState(ESP8266_STA_MODULE_DETECT);
			}
		}
		break;
		
		case ESP8266_STA_MODULE_DETECT:
		{
			WorkCounter++;
			
			if(WorkCounter > 200)
			{
				Para = 0xFF;
				WorkCounter = 0;
				
				Mid_WiFi_ATcmdQueueIn(ESP8266_AT_AT, &Para);
			}
		}
		break;
		
		case ESP8266_STA_MODULE_INIT:
		{
			WorkCounter++;
			
			if(WorkCounter > 200)
			{
				Para = 0xFF;
				WorkCounter = 0;
				
				Mid_WiFi_ATcmdQueueIn(ESP8266_AT_ATE1, &Para);
				Mid_WiFi_ATcmdQueueIn(ESP8266_AT_CWMODE, &Para);
				Mid_WiFi_ATcmdQueueIn(ESP8266_AT_CWAUTOCONN, &Para);
				
				Mid_WiFi_ChangeModuleWorkState(ESP8266_STA_MODULE_STATE);
			}
		}
		break;
		
		case ESP8266_STA_MODULE_STATE:
		{
			WorkCounter++;
			
			if(WorkCounter > 200)
			{
				ATResendCounter++;
				
				if(ATResendCounter > 10)	// if no follew-up actions after sending 10 times "AT+CWSTATE?", reset WiFi-Module
				{
					ATResendCounter = 0;
					
					Mid_WiFi_PowerManage(ESP8266_POWER_STATE_RESET);
					
					return;
				}
				
				Para = 0xFF;
				WorkCounter = 0;
				
				Mid_WiFi_ATcmdQueueIn(ESP8266_AT_CWSTATE, &Para);
			}
		}
		break;
		
		case ESP8266_STA_NETPAIR_START:		
		{
			Para = 0xFF;
			
			QueueEmpty(Queue_WiFiTxSequence);
			
			Mid_WiFi_ATcmdQueueIn(ESP8266_AT_CWSTOPSMART, &Para);	// stop SmartConfig to release ESP8266 RAM resource
			Mid_WiFi_ATcmdQueueIn(ESP8266_AT_CWSTARTSMART, &Para);	// start SmartConfig
		
			Mid_WiFi_ChangeModuleWorkState(ESP8266_STA_NETPAIR_IN_PROCESS);
			
			WorkCounter = 0;
			ATResendCounter = 0;
		}
		break;
			
		case ESP8266_STA_NETPAIR_IN_PROCESS:
			
		case ESP8266_STA_GET_SMART_WIFI_INFO:
		{
			WorkCounter++;
			
			if(WorkCounter > 30000)	// Netpairing 5mins no IP obtained, timeout
			{
				WorkCounter = 0;
				ATResendCounter = 0;
				
				Mid_WiFi_ChangeModuleWorkState(ESP8266_STA_GET_IP_TIMEOUT);
			}
		}
		break;
		
		case ESP8266_STA_GET_IP_TIMEOUT:
			
		case ESP8266_STA_GET_IP_SUCCESS:
		{
			WorkCounter++;
			
			if(WorkCounter > 10)	// keep success/fail state at least 100ms for AppLayer capture and further operations
			{
				WorkCounter = 0;
				Para = 0xFF;
				
				QueueEmpty(Queue_WiFiTxSequence);
				
				Mid_WiFi_ATcmdQueueIn(ESP8266_AT_CWSTOPSMART, &Para);		// stop SmartConfig to release ESP8266 RAM resource
				Mid_WiFi_ChangeModuleWorkState(ESP8266_STA_MODULE_DETECT);
			}
		}
		break;
		
		case ESP8266_STA_MODULE_READY:
		{
			WorkCounter++;
			FirmwareCounter++;
			Para = 0xFF;
			
			if(FirmwareCounter == 6000)		// Check New Firmware every 30s
			{
				MQTTProtocol_EventUpQueueIn(TERMINAL_UPEVENT_UPDATE_CHECK, 0);
			
				FirmwareCounter = 0;
			}
			
			if(WorkCounter == 6000)	// Check the WiFi-connection every 60s
			{
				Mid_WiFi_ATcmdQueueIn(ESP8266_AT_CWSTATE, &Para);

				WorkCounter = 0;
			}
			
			if(!Mid_WiFi_MQTT_Pro()) // reset the counter if any MQTT operations
			{
				WorkCounter = 0;
			}
		}
		break;
		
		case ESP8266_STA_PUBLISH_ALARMDATA:
		{
			
		}
		break;
	}
	
	#endif
}


/**
  * @Brief	Manage WiFi-Module power state, turn off the module when AC is disconnected
  * @Param	State: Power-state to set
  * @Retval	0->Module Power <off>, 1->Module Power <on>
  *	@Note	If AC is disconnected, turn off WiFi-Module
  */
static uint8_t Mid_WiFi_PowerManage(en_ESP8266_PowerState_t State)
{
	static uint16_t Timer = 2000;
	
	if(Hal_GPIO_ACStateCheck() == STA_AC_LINK)	// AC connected
	{
		if(State == ESP8266_POWER_STATE_RESET)		// Reset WiFi-Module
		{
			Timer = 0;
			
			Mid_WiFi_ChangeModuleWorkState(ESP8266_STA_MODULE_DETECT);
			Mid_WiFi_ChangeMQTTState(MQTT_STA_IDLE);
		}
		
		if(Timer == 0)
		{
			Mid_WiFi_Init();
			
			Hal_GPIO_WiFiPower_Disable();
		}
		
		if(Timer < 1000)	
		{
			Timer++;
			
			if(Timer > 150)	// Reboot after 1.5s
			{
				Timer = 2000;
				
				Hal_GPIO_WiFiPower_Enable();
			}
			return 0;
		}
		return 1;
	}
	else	// AC is disconnected
	{
		Timer = 0;
		WiFi_WorkState = ESP8266_STA_MODULE_DETECT;
		WiFi_LinkState = ESP8266_LINK_0_NOCONNECTION;
		
		/* Debug Mode: */
		#ifdef WIFI_Module_DEBUG_MODE
		Hal_GPIO_WiFiPower_Enable();
		#endif
		
		/* Working Mode: */
		#ifndef WIFI_Module_DEBUG_MODE
		Hal_GPIO_WiFiPower_Disable();
		#endif
		
		return 0;
	}
}

/**
  * @Brief	According to the current MQTTState, queue-in the corresponding ATcmd, 
  *			and change to the next MQTTState
  * @Param	None
  * @Retval	0-> Response data to process, 0xFF-> idle
  */
static uint8_t Mid_WiFi_MQTT_Pro(void)
{
	uint8_t Index;
	uint8_t i;
	uint8_t MQTTDataBuff[WIFI_MQTT_TX_DATA_SIZE];
	static uint32_t WorkCounter = 0;
	static uint8_t ResendCounter = 0;
	
	memset(&MQTTDataBuff[0], 0, WIFI_MQTT_TX_DATA_SIZE);
	
	switch((uint8_t)WiFi_MQTTState)
	{
		/* "AT+MQTTCLEAN=0" */
		case MQTT_STA_IDLE:
		{
			WorkCounter++;
			
			if(WorkCounter > 200)
			{
				WorkCounter = 0;
				
				MQTTDataBuff[0] = 0xFF;	
				
				Mid_WiFi_ATcmdQueueIn(ESP8266_AT_MQTTCLEAN, &MQTTDataBuff[0]);
				
				Mid_WiFi_ChangeMQTTState(MQTT_STA_CONFIG);
				
				return 0;
			}
		}
		break;
		
		/* "AT+MQTTUSERCFG=0,1,\"" <"client ID">, <"username">, <"password">, <cert_key_ID>, <CA_ID>, <"path"> */
		case MQTT_STA_CONFIG:
		{
			WorkCounter++;
			
			if(WorkCounter > 200)
			{
				WorkCounter = 0;
				Index = 0;
				i = 0;
				
				while(stu_MQTT_ESP8266.ClientID[i])
				{
					MQTTDataBuff[Index++] = stu_MQTT_ESP8266.ClientID[i++];
					
					if(i == MQTT_CLIENT_ID_SIZE)
					{
						break;
					}
				}
				
				/* change <Flag>(0->9) of ClientID to retry connecting */
				stu_MQTT_ESP8266.ClientID[0] += 1;
				
				if(stu_MQTT_ESP8266.ClientID[0] > '9')
				{
					stu_MQTT_ESP8266.ClientID[0] = '0';
				}
				
				
				MQTTDataBuff[Index++] = '\"';
				MQTTDataBuff[Index++] = ',';
				MQTTDataBuff[Index++] = '\"';
				
				i = 0;
				
				while(stu_MQTT_ESP8266.Username[i])
				{
					MQTTDataBuff[Index++] = stu_MQTT_ESP8266.Username[i++];
					
					if(i == MQTT_USERNAME_SIZE)
					{
						break;
					}
				}
				MQTTDataBuff[Index++] = '\"';
				MQTTDataBuff[Index++] = ',';
				MQTTDataBuff[Index++] = '\"';
				
				i = 0;
				
				while(stu_MQTT_ESP8266.Password[i])
				{
					MQTTDataBuff[Index++] = stu_MQTT_ESP8266.Password[i++];
					
					if(i == MQTT_PASSWORD_SIZE)
					{
						break;
					}
				}
				MQTTDataBuff[Index++] = '\"';
				MQTTDataBuff[Index++] = ',';
				MQTTDataBuff[Index++] = '0';
				MQTTDataBuff[Index++] = ',';
				MQTTDataBuff[Index++] = '0';
				MQTTDataBuff[Index++] = ',';
				MQTTDataBuff[Index++] = '\"';
				MQTTDataBuff[Index++] = '\"';
				MQTTDataBuff[Index++] = '\0';
				
				Mid_WiFi_ATcmdQueueIn(ESP8266_AT_MQTTUSERCFG, &MQTTDataBuff[0]);
				
				ResendCounter = 0;
				
				return 0;
			}
		}
		break;
		
		/* "AT+MQTTCONN=0,\"", <"host">, <port>, <reconnect>0->no auto-reconnect, 1->auto-reconnect */
		case MQTT_STA_CONNECT:
		{
			WorkCounter++;
			
			if(WorkCounter > 200)
			{
				WorkCounter = 0;
				ResendCounter++;
				
				if(ResendCounter > 2)
				{
					ResendCounter = 0;
					
					Mid_WiFi_PowerManage(ESP8266_POWER_STATE_RESET);
					
					return 0;
				}
				
				Index = 0;
				i = 0;
				
				while(stu_MQTT_ESP8266.ServerIp[i])
				{
					MQTTDataBuff[Index++] = stu_MQTT_ESP8266.ServerIp[i++];
					
					if(i == MQTT_SERVER_IP_SIZE)
					{
						break;
					}
				}
				MQTTDataBuff[Index++] = '\"';
				MQTTDataBuff[Index++] = ',';
				
				i = 0;
				
				while(stu_MQTT_ESP8266.ServerPort[i])
				{
					MQTTDataBuff[Index++] = stu_MQTT_ESP8266.ServerPort[i++];
				
					if(i == MQTT_SERVER_PORT_SIZE)
					{
						break;
					}
				}
				MQTTDataBuff[Index++] = ',';
				MQTTDataBuff[Index++] = '0';
				MQTTDataBuff[Index++] = '\0';
				
				Mid_WiFi_ATcmdQueueIn(ESP8266_AT_MQTTCONN, &MQTTDataBuff[0]);

				return 0;
			}
		}
		break;
		
		/* "AT+MQTTSUB=0,\"", <"topic">, <qos> */
		case MQTT_STA_SUB:
		{
			WorkCounter++;
			
			if(WorkCounter > 200)
			{
				WorkCounter = 0;
				
				Index = 0;
				i = 0;
				
				/* set SubTopic: */
				while(stu_MQTT_ESP8266.SubTopic[i])
				{
					MQTTDataBuff[Index++] = stu_MQTT_ESP8266.SubTopic[i++];
				
					if(i == MQTT_TOPIC_SIZE)
					{
						break;
					}
				}
				MQTTDataBuff[Index++] = '\"';
				MQTTDataBuff[Index++] = ',';
				MQTTDataBuff[Index++] = '0';
				MQTTDataBuff[Index++] = '\0';
				
				Mid_WiFi_ATcmdQueueIn(ESP8266_AT_MQTTSUB, &MQTTDataBuff[0]);
				
				return 0;
			}
		}

		case MQTT_STA_SUB_DATETIME:
		{
			WorkCounter++;

			if(WorkCounter > 200)
			{
				WorkCounter = 0;

				Index = 0;
				i = 0;
				
				/* set SubTopic_DateTime: */
				while(stu_MQTT_ESP8266.SubTopic_DataTime[i])
				{
					MQTTDataBuff[Index++] = stu_MQTT_ESP8266.SubTopic_DataTime[i++];
					
					if(i == MQTT_TOPIC_SIZE)
					{
						break;
					}
				}
				MQTTDataBuff[Index++] = '\"';
				MQTTDataBuff[Index++] = ',';
				MQTTDataBuff[Index++] = '0';
				MQTTDataBuff[Index++] = '\0';
				
				Mid_WiFi_ATcmdQueueIn(ESP8266_AT_MQTTSUBDATETIME, &MQTTDataBuff[0]);

				return 0;
			}

		}
		break;
		
		case MQTT_STA_PUB:
		{
			WorkCounter++;
			
			if(WorkCounter > 500)
			{
				WorkCounter = 0;
				
				return 0;
			}
		}
		break;
		
		case MQTT_STA_READY:
		{
			WorkCounter++;
			
			if(WorkCounter > 200)
			{
				WorkCounter = 0;
				

				MQTTProtocol_EventUpload_Pro(PROTOCOL_COMM_TYPE_WIFI);
			}
		}
		break;

		case MQTT_STA_RECV_SYSTIME:
		{
			WorkCounter++;
			
			if(WorkCounter > 200)
			{
				WorkCounter = 0;
				Mid_WiFi_ChangeMQTTState(MQTT_STA_READY);

				return 0;
			}
		}
		break;

		case MQTT_STA_RECV_DOWN:
		{
			WorkCounter++;
			
			if(WorkCounter > 200)
			{
				WorkCounter = 0;
				Mid_WiFi_ChangeMQTTState(MQTT_STA_READY);

				return 0;
			}
		}
		break;
			
	}
	
	return 0xFF;
}

/**
  * @Brief	Extract the ReceiveData from received MQTT Data 
  * @Param	pData		: point to MQTTRxData string
  *			pReceiveData: point to the extracted ReceiveData part 
  * @Retval	Length of ReceiveData
  *	@Note	+MQTTSUBRECV:0,"rytwj01wwncy26A2",16,AA00072900123467
  */
static uint8_t Mid_WiFi_MQTTRxDataHandler(uint8_t *pData, uint8_t *pReceiveData)
{
	uint8_t i;
	uint8_t Len;
	uint8_t LenBuff = 0;
	
	while(*pData != '"')
	{
		pData++;
	}
	
	pData++;
	
	while(*pData != '"')
	{
		pData++;
	}
	
	pData += 2;
	
	Len = 0;
	i = 0;
	
	/* capture ReceiveData_Length */
	while(*pData != ',')
	{
		if((*pData >= '0') && (*pData <= '9'))
		{
			Len *= 10;
			Len += *pData - '0';
		}
		
		pData++;
		i++;
		
		if(i > 2)
		{
			break;
		}
	}
	
	pData++;
	LenBuff = Len;
	
	while(Len--)
	{
		*pReceiveData = *pData;
		
		pReceiveData++;
		pData++;
	}
	
	return LenBuff;
}


/*-------------Interrupt Functions Definition--------*/


