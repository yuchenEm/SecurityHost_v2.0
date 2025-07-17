#ifndef  __MID_LORA_H_
#define  __MID_LORA_H_

#define STU_APPLYNET_DATAFRAME_SIZE			0x10	// 16 byte 
#define SENSOR_MAC_ADDRESS_SIZE				0x0C	// 12

/* Sensor-->Terminal / Terminal-->Sensor Lora functioncode(event) */
typedef enum
{
	LORA_COM_APPLY_NET = 1,    
    LORA_COM_HEART,            
    LORA_COM_BAT_LOW,          
    LORA_COM_ALARM, 
	LORA_COM_TAMPER,
	LORA_COM_DISARM,
	LORA_COM_AWAYARM,
	LORA_COM_HOMEARM,
    LORA_COM_DOORCLOSE = 9,    
    
    LORA_COM_RESPONSE_APPLY_NET = 0x81,
    LORA_COM_RESPONSE_HEART,     
    LORA_COM_RESPONSE_BAT_LOW,
    LORA_COM_RESPONSE_ALARM,
    LORA_COM_RESPONSE_TAMPER,
    LORA_COM_RESPONSE_DISARM,
    LORA_COM_RESPONSE_AWAYARM,
    LORA_COM_RESPONSE_HOMEARM,
    LORA_COM_RESPONSE_DOORCLOSE,
  
    LORA_COM_RESPONSE_NONODE = 0xFF,
	
}en_Lora_FunctionCode_t;

/* Sensor-->Terminal (Join Network Request) pairing status */
typedef enum
{
	LORA_APPLYNET_PAIRING,
	LORA_APPLYNET_SUCCESSFUL,
	LORA_APPLYNET_FAIL,
}en_Lora_ApplyNet_State_t;

/* Sensor-->Terminal (Join Network Request) sensor parameters */
typedef struct
{
	en_Lora_ApplyNet_State_t State;
	unsigned char SensorIndex;
}stu_Lora_ApplyNet_SensorPara_t;

/* Sensor-->Terminal (Join Network Request) DataFrame structure */
typedef struct
{
	unsigned char FunctionCode;
	unsigned char NodeNo[2];		// Use CRC16Value as sensor's NodeNo
	unsigned char MACAddress[12];
	unsigned char SensorType;
}stu_Lora_Sensor_DataFrame_t;


/* Sensor's ApplyNet functioncode handler Call-Back Function typedef: */
typedef stu_Lora_ApplyNet_SensorPara_t (*Lora_ApplyNetReq_HandlerCBF_t)(en_Lora_FunctionCode_t FunctionCode, stu_Lora_Sensor_DataFrame_t stu_DataFrame);

/* Sensor's functional command handler Call-Back Function typedef: */
typedef unsigned char (*Lora_FunctionCMD_HandlerCBF_t)(en_Lora_FunctionCode_t FunctionCode, stu_Lora_Sensor_DataFrame_t stu_DataFrame);



void Mid_Lora_Init(void);
void Mid_Lora_Pro(void);

void Mid_Lora_ApplyNetReq_HandlerCBFRegister(Lora_ApplyNetReq_HandlerCBF_t pCBF);
void Mid_Lora_FunctionCMD_HandlerCBFregister(Lora_FunctionCMD_HandlerCBF_t pCBF);

#endif
