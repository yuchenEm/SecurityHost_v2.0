/****************************************************
  * @Name	Mid_Lora.c
  * @Brief	
  *	
  *	@Protocal: 
	(1) Sensor-->Terminal (Join Network Request)
  	***************************************************
    * DataFrame:
    *   head:                   FE
    *   datelength:             0x10 (16 byte before checksum)
    *   function code:   		1 byte
    *   allocated NodeNo.(CRC): 2 byte
    *   sensor MAC address:     12 byte
    *   sensor type:            1 byte
    *
    *   checksum:               1 byte
    ***************************************************
	
	(2) Sensor-->Terminal (Function Command)
	***************************************************
    * DataFrame:
    *   head:                   FE
    *   datelength:             0x03 (3 byte before checksum)
    *   function code:   		1 byte
    *   allocated NodeNo.(CRC): 2 byte
    *
    *   checksum:               1 byte
    ***************************************************
	
	(3) Terminal-->Sensor (Join Network Request Reply)
  	***************************************************
    * DataFrame:
    *   head:                   FE
    *   datelength:             0x10 (16 byte before checksum)
    *   return function code:   0x81 (1 byte)
    *   allocated NodeNo.(CRC): 2 byte
    *   sensor MAC address:     12 byte
    *   sensor type:            1 byte
    *
    *   checksum:               1 byte
    ***************************************************
  ***************************************************/
  
/*-------------Header Files Include-----------------*/
#include "stm32f10x.h"
#include "mid_lora.h"
#include "hal_usart.h"
#include "os_system.h"
#include "crc16.h"

/*-------------Internal Functions Declaration------*/
static void Mid_Lora_RxDataQueueIn(uint8_t Data);
static void Mid_Lora_TxDataQueueIn(uint8_t *pData, uint8_t Len);

static void Mid_Lora_UART5_SendData(void);
static void Mid_Lora_UART5_ReceiveData(void);

static void Mid_Lora_RxDataHandler(uint8_t *pData, uint8_t Len);
static void Mid_Lora_TxDataHandler(en_Lora_FunctionCode_t FunctionCode, stu_Lora_Sensor_DataFrame_t *pDataFrame);

/*-------------Module Variables Declaration--------*/
volatile Queue256 Queue_LoraRx;	// Lora receive buffer
Queue256 Queue_LoraTx;					// Lora transmit buffer


/*---Module Call-Back function pointer Definition---*/
Lora_ApplyNetReq_HandlerCBF_t Lora_ApplyNetReq_HandlerCBF;
Lora_FunctionCMD_HandlerCBF_t Lora_FunctionCMD_HandlerCBF;


/*-------------Module Functions Definition---------*/
/**
  * @Brief	Initialize Lora mid-layer module
  * @Param	None
  * @Retval	None
  */
void Mid_Lora_Init(void)
{
	QueueEmpty(Queue_LoraRx);
	QueueEmpty(Queue_LoraTx);
	
	Lora_ApplyNetReq_HandlerCBF = 0;
	
	Hal_USART_LoraRxCBFRegister(Mid_Lora_RxDataQueueIn);
}

/**
  * @Brief	Polling in sequence for Lora mid-layer module
  * @Param	None
  * @Retval	None
  */
void Mid_Lora_Pro(void)
{
	Mid_Lora_UART5_SendData();
	Mid_Lora_UART5_ReceiveData();
}

/**
  * @Brief	Register sensor ApplyNet handler CBF from uplayer(API)
  * @Param	pCBF: function pointer from uplayer
  * @Retval	None
  */
void Mid_Lora_ApplyNetReq_HandlerCBFRegister(Lora_ApplyNetReq_HandlerCBF_t pCBF)
{
	if(Lora_ApplyNetReq_HandlerCBF == 0)
	{
		Lora_ApplyNetReq_HandlerCBF = pCBF;
	}
}

/**
  * @Brief	Register sensor functional command handler CBF from uplayer(API)
  * @Param	pCBF: function pointer from uplayer
  * @Retval	None
  */
void Mid_Lora_FunctionCMD_HandlerCBFregister(Lora_FunctionCMD_HandlerCBF_t pCBF)
{
	if(Lora_FunctionCMD_HandlerCBF == 0)
	{
		Lora_FunctionCMD_HandlerCBF = pCBF;
	}
}


/*-------------Internal Functions Definition--------*/
/**
  * @Brief	Queue-in the data to Queue_LoraRx(handler of Lora_USART_RxCBF)
  * @Param	Data: byte data ready for queue-in
  * @Retval	None
  */
static void Mid_Lora_RxDataQueueIn(uint8_t Data)
{
	if(QueueDataLen(Queue_LoraRx) < Queue256_Length)
	{
		QueueDataIn(Queue_LoraRx, &Data, 1);
	}
}

/**
  * @Brief	Queue-in the data to Queue_LoraTx
  * @Param	pData: pointer to the address of data ready to queue-in
  *			Len	 : data length
  * @Retval	None
  * @Note	Overflowing part will be ignored
  */
static void Mid_Lora_TxDataQueueIn(uint8_t *pData, uint8_t Len)
{
	if((QueueDataLen(Queue_LoraTx) + Len) < Queue256_Length)
	{
		QueueDataIn(Queue_LoraTx, pData, Len);
	}
}

/**
  * @Brief	Check Queue_LoraTx buffer, if there is data in the queue, 
  *			send it through UART5 to the Lora module
  * @Param	None
  * @Retval	None
  */
static void Mid_Lora_UART5_SendData(void)
{
	uint8_t i;
	uint8_t Len;
	uint8_t LoraTxBuff[256];
	
	Len = QueueDataLen(Queue_LoraTx);
	
	if(Len)
	{
		for(i=0; i<Len; i++)
		{
			QueueDataOut(Queue_LoraTx, &LoraTxBuff[i]);
		}
		
		Hal_USART_LoraDataTx(&LoraTxBuff[0], Len);
	}
}

/**
  * @Brief	Check the Queue_LoraRx if there is data in the queue 
  *			according to the protocal, sumcheck the data and handle it
  * @Param	None
  * @Retval	None
  * @Note	Routine:
		1. Check Framehead(0xFE) and DataLength
		2. Obtain the effective data and CRC value
		3. Compare Sumcheck value
  */
static void Mid_Lora_UART5_ReceiveData(void)
{
	static uint8_t Len = 0;
	static uint8_t TimeoutCounter = 0;
	static uint8_t LoraRxbuff[100];
	
	uint8_t i;
	uint8_t SumCheck;
	uint8_t DataBuff;
	
	if(Len == 0)
	{
		if(QueueDataLen(Queue_LoraRx) > 1)	// Check if there is data(Framehead and DataLength) in the Queue_LoraRx
		{
			QueueDataOut(Queue_LoraRx, &DataBuff);
			
			if(DataBuff == 0xFE)	// check Framehead(0xFE)
			{
				QueueDataOut(Queue_LoraRx, &Len);	// get DataLength
				
				TimeoutCounter = 0;
			}
		}
	}
	
	if(Len > 0)
	{
		if(QueueDataLen(Queue_LoraRx) > Len)	// check if the whole dataframe is collected
		{
			if(Len > 99)	// error datalength, abondon
			{
				QueueEmpty(Queue_LoraRx);
				Len = 0;
				return;
			}
			
			SumCheck = 0;
			
			for(i=0; i<Len; i++)
			{
				QueueDataOut(Queue_LoraRx, &LoraRxbuff[i]);
				SumCheck += LoraRxbuff[i];
			}
			
			QueueDataOut(Queue_LoraRx, &DataBuff);	// get sumcheck value
			
			if(SumCheck == DataBuff)				// Sumcheck succeed, dataframe valid
			{
				Mid_Lora_RxDataHandler(&LoraRxbuff[0], Len);			// Handle the effective dataframe with specified protocal
				Hal_USART_DebugDataQueueIn(&LoraRxbuff[0], Len);	// Debug_USART print received data
			}
			
			Len = 0;
		}
		else
		{
			TimeoutCounter++;
			
			if(TimeoutCounter >= 10)	// 100ms timeout, abondon this dataframe
			{
				TimeoutCounter = 0;
				Len = 0;
			}
		}
	}
}

/**
  * @Brief	Handle the received data with specified protocal		
  * @Param	pData: the pointer to the starting address of valid data
  *			Len	 : data length
  * @Retval	None
  *	@Note	DataFrame: 
			(1) Sensor-->Terminal (Join Network Request)
			(2) Sensor-->Terminal (Function Command)
  */
static void Mid_Lora_RxDataHandler(uint8_t *pData, uint8_t Len)
{
	uint8_t i;
	uint16_t CRC16Value;
	uint8_t DataParseResult;	// store the parse result of Lora-DataFrame from Sensor
	
	stu_Lora_Sensor_DataFrame_t SensorDataFrame;
	stu_Lora_ApplyNet_SensorPara_t ApplyNetSensorPara;
	
	if(Len > 2)	// 3 byte: 1byte function code, 2byte NodeNo
	{
		SensorDataFrame.FunctionCode = pData[0];
		SensorDataFrame.NodeNo[0] = pData[1];	// CRC Lowbyte
		SensorDataFrame.NodeNo[1] = pData[2];	// CRC Highbyte
		
		switch((uint8_t)SensorDataFrame.FunctionCode)
		{
			/* Join Network request from sensor: */
			case LORA_COM_APPLY_NET:	
			{
				if(Len == STU_APPLYNET_DATAFRAME_SIZE)
				{
					for(i=0; i<SENSOR_MAC_ADDRESS_SIZE; i++)	// obtain sensor MACAddress from dataframe
					{
						SensorDataFrame.MACAddress[i] = pData[i+3];
					}
					
					SensorDataFrame.SensorType = pData[SENSOR_MAC_ADDRESS_SIZE+3];	// obtain sensor type
					
					CRC16Value = Mid_CRC16_Modbus(&SensorDataFrame.MACAddress[0], SENSOR_MAC_ADDRESS_SIZE);	// obtain CRC16 value
					
					if(CRC16Value == ((SensorDataFrame.NodeNo[0]) | SensorDataFrame.NodeNo[1] << 8))	// CRC16check succeed, dataframe is valid
					{
						if(Lora_ApplyNetReq_HandlerCBF)	// handle ApplyNetReq command
						{
							ApplyNetSensorPara = Lora_ApplyNetReq_HandlerCBF(LORA_COM_APPLY_NET, SensorDataFrame);
							
							if(ApplyNetSensorPara.State == LORA_APPLYNET_SUCCESSFUL)	// Pair successful
							{
								Mid_Lora_TxDataHandler(LORA_COM_APPLY_NET, &SensorDataFrame);
							}
						}
					}
				}
			}
			break;
			
			/* Functional command from sensor: */
			case LORA_COM_HEART:
			case LORA_COM_BAT_LOW:
			case LORA_COM_ALARM:
			case LORA_COM_TAMPER:			
			case LORA_COM_DISARM:
			case LORA_COM_AWAYARM:
			case LORA_COM_HOMEARM:
			case LORA_COM_DOORCLOSE:
			{
				if(Len == 3)	
				{
					if(Lora_FunctionCMD_HandlerCBF)
					{
						DataParseResult = Lora_FunctionCMD_HandlerCBF((en_Lora_FunctionCode_t)SensorDataFrame.FunctionCode, SensorDataFrame);
					
						if(DataParseResult == 0)	// not find any matching NodeNo(CRC16), need retry join the Network
						{

						}
						else
						{
							Mid_Lora_TxDataHandler((en_Lora_FunctionCode_t)SensorDataFrame.FunctionCode, &SensorDataFrame);
						}
					}
				}
			}
			break;
		}
	}
}

/**
  * @Brief	Handle the return dataframe for sensor with specified protocal
  * @Param	FunctionCode: obtained functioncode
  *			pDataFrame  : point to the address of dataframe
  * @Retval	None
  */
static void Mid_Lora_TxDataHandler(en_Lora_FunctionCode_t FunctionCode, stu_Lora_Sensor_DataFrame_t *pDataFrame)
{
	uint8_t i;
	
	uint8_t LoraTxBuff[20];
	uint8_t Index;
	uint8_t SumCheck = 0;
	
	LoraTxBuff[0] = 0xFE;	// Dataframe head
	Index = 1;
	
	switch((uint8_t)FunctionCode)
	{
		case LORA_COM_APPLY_NET:
		{
			LoraTxBuff[Index++] = 0x10;					 					// Datalength(16 byte)
			LoraTxBuff[Index++] = FunctionCode + 0x80;	 	// Return functioncode
			LoraTxBuff[Index++] = pDataFrame->NodeNo[0];
			LoraTxBuff[Index++] = pDataFrame->NodeNo[1]; 	// Sensor NodeNo
			
			for(i=0; i<SENSOR_MAC_ADDRESS_SIZE; i++)	 		// Sensor MACAddress
			{
				LoraTxBuff[Index++] = pDataFrame->MACAddress[i];	
			}
			
			LoraTxBuff[Index++] = pDataFrame->SensorType; // Sensor type
			
			for(i=2; i<Index; i++)					  	  				// SumCheck
			{
				SumCheck += LoraTxBuff[i];
			}	
			LoraTxBuff[Index] = SumCheck;
			
			Mid_Lora_TxDataQueueIn(LoraTxBuff, Index+1);  // Queue-in LoraTxBuff
		}
		break;
		
		case LORA_COM_HEART:
		case LORA_COM_BAT_LOW:
		case LORA_COM_ALARM:
		case LORA_COM_TAMPER:
		case LORA_COM_DISARM:
		case LORA_COM_AWAYARM:
		case LORA_COM_HOMEARM:
		case LORA_COM_DOORCLOSE:
		case LORA_COM_RESPONSE_NONODE:
		{
			LoraTxBuff[Index++] = 0x03;		
			
			if(FunctionCode == LORA_COM_RESPONSE_NONODE)
			{
				LoraTxBuff[Index++] = LORA_COM_RESPONSE_NONODE;
			}
			else
			{
				LoraTxBuff[Index++] = FunctionCode + 0x80;
			}
			
			LoraTxBuff[Index++] = pDataFrame->NodeNo[0];
			LoraTxBuff[Index++] = pDataFrame->NodeNo[1];
			
			for(i=2; i<Index; i++)
			{
				SumCheck += LoraTxBuff[i];
			}
			LoraTxBuff[Index] = SumCheck;
			
			Mid_Lora_TxDataQueueIn(LoraTxBuff, Index+1);
		}
		break;
	}
}

/*-------------Interrupt Functions Definition--------*/


