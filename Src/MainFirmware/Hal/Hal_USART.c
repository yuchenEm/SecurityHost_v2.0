/****************************************************
  * @Name	Hal_USART.c
  * @Brief	USART1 --> Debug USART
  *			UART5  --> Lora module communication
  *			USART3 --> WiFi module communication
  ***************************************************/
  
/*-------------Header Files Include-----------------*/
#include "stm32f10x.h" 
#include "os_system.h"
#include "hal_usart.h"
#include "hal_gpio.h"
#include "hal_dma.h"
#include "string.h"

/*-------------Internal Functions Declaration------*/
static void Hal_USART_Config(void);
static void Hal_USART1_Config(void);	// Debug Port
static void Hal_UART5_Config(void);		// Lora Port
static void Hal_USART3_Config(void);	// WiFi Port
static void Hal_USART2_Config(void);	

static void Hal_USART1_DMA_SendData(void);

static void Hal_USART_LoraDebug(void);
static void Hal_USART_WiFiDebug(void);
static void Hal_USART_GSMDebug(void);

/*-------------Module Variables Declaration--------*/
uint8_t Buffer_DebugTx[BUFFER_DEBUG_TX_SIZE];

volatile uint8_t DebugBusyFlag;		// 0 -> idle, 1 -> busy

Queue512 Queue_DebugTx;						// 512 bytes DebugTx queue buffer

/*---Module Call-Back function pointer Definition---*/
Lora_USART_RxCBF_t Lora_USART_RxCBF;
WiFi_USART_RxCBF_t WiFi_USART_RxCBF;
GSM_USART_RxCBF_t GSM_USART_RxCBF;

/*-------------Module Functions Definition---------*/
/**
  * @Brief	Initialize USART module
  * @Param	None
  * @Retval	None
  */
void Hal_USART_Init(void)
{
	Hal_USART_Config();
	
	DebugBusyFlag = 0;
	QueueEmpty(Queue_DebugTx);
	
	Lora_USART_RxCBF = 0;
	WiFi_USART_RxCBF = 0;
	GSM_USART_RxCBF = 0;
	
	Hal_USART_DebugStringQueueIn("Connection Succeed\r\n");
}

/**
  * @Brief	Polling in sequence for USART module
  * @Param	None
  * @Retval	None
  */
void Hal_USART_Pro(void)
{
	Hal_USART1_DMA_SendData();
	
}

/**
  * @Brief	USART Lora_Rx call-back function register
  * @Param	pCBF: function pointer to the uplayer call-back function
  * @Retval	None
  */
void Hal_USART_LoraRxCBFRegister(Lora_USART_RxCBF_t pCBF)
{
	if(Lora_USART_RxCBF == 0)
	{
		Lora_USART_RxCBF = pCBF;
	}
}

/**
  * @Brief	USART WiFi_Rx call-back function register
  * @Param	pCBF: function pointer to the uplayer call-back function
  * @Retval	None
  */
void Hal_USART_WiFiRxCBFRegister(WiFi_USART_RxCBF_t pCBF)
{
	if(WiFi_USART_RxCBF == 0)
	{
		WiFi_USART_RxCBF = pCBF;
	}
}

/**
  * @Brief	USART GSM call-back function register
  * @Param	pCBF: function pointer to the uplayer call-back function
  * @Retval	None
  */
void Hal_USART_GSMRxCBFRegister(GSM_USART_RxCBF_t pCBF)
{
	if(GSM_USART_RxCBF == 0)
	{
		GSM_USART_RxCBF = pCBF;
	}
}

/**
  * @Brief	Queue-in debug String data to queue Queue_DebugTx
  * @Param	pData: pointer to the String address
  * @Retval	None
  */
void Hal_USART_DebugStringQueueIn(const char pData[])
{
	uint8_t Len;
	
	Len = (uint8_t)(strlen(pData));
	
	if((Len + QueueDataLen(Queue_DebugTx)) < Queue512_Length)
	{
		QueueDataIn(Queue_DebugTx, (uint8_t *)pData, Len);
	}
}

/**
  * @Brief	Queue-in debug data to queue Queue_DebugTx
  * @Param	pData: pointer to the Data address
			Len	 : data length
  * @Retval	None
  */
void Hal_USART_DebugDataQueueIn(uint8_t pData[], uint16_t Len)
{
	if((Len + QueueDataLen(Queue_DebugTx)) < Queue512_Length)	// overflow part will be ignored
	{
		QueueDataIn(Queue_DebugTx, (uint8_t *)pData, Len);
	}
}

/**
  * @Brief	Send data through Lora_USART_Tx to the Lora-module
  * @Param	pData: pointer to the Data address
			Len	 : data length(0-255)
  * @Retval	None
  */
void Hal_USART_LoraDataTx(uint8_t *pData, uint8_t Len)
{
	while(Len)
	{
		USART_SendData(LORA_USART_PORT, *pData);
		
		while(USART_GetFlagStatus(LORA_USART_PORT, USART_FLAG_TC) == RESET)
		{
			
		}
		pData++;	// point to the next byte data
		Len--;
	}
}

/**
  * @Brief	Send data through WiFi_USART_Tx(USART3) to the esp8266 WiFi-module 
  * @Param	pData: pointer to the Data address
			Len	 : data length(0-255)
  * @Retval	None
  */
void Hal_USART_WiFiDataTx(uint8_t *pData, uint8_t Len)
{
	while(Len)
	{
		USART_SendData(WIFI_USART_PORT, *pData);
		
		while(USART_GetFlagStatus(WIFI_USART_PORT, USART_FLAG_TC) == RESET)
		{
			
		}
		pData++;
		Len--;
	}
}

/**
  * @Brief	Send data through GSM_USART_Tx(USART2) to the EC200 GSM-module 
  * @Param	pData: pointer to the Data address
			Len	 : data length(0-255)
  * @Retval	None
  */
void Hal_USART_GSMDataTx(uint8_t *pData, uint8_t Len)
{
	while(Len)
	{
		USART_SendData(GSM_USART_PORT, *pData);
		
		while(USART_GetFlagStatus(GSM_USART_PORT, USART_FLAG_TC) == RESET)
		{
			
		}
		pData++;
		Len--;
	}
}

/**
  * @Brief	Send String data through GSM_USART_Tx(USART2) to the EC200 GSM-module 
  * @Param	pData: pointer to the String data address
  * @Retval	None
  */
void Hal_USART_GSMStringTx(uint8_t *pData)
{
	while(*pData)
	{
		USART_SendData(GSM_USART_PORT, *pData);
		
		while(USART_GetFlagStatus(GSM_USART_PORT, USART_FLAG_TC) == RESET)
		{
			
		}
		pData++;
	}
}


/*-------------Internal Functions Definition--------*/
/**
  * @Brief	Config USART module pin and parameters
  * @Param	None
  * @Retval	None
  */
static void Hal_USART_Config(void)
{
	Hal_USART1_Config();
	Hal_UART5_Config();
	Hal_USART3_Config();
	Hal_USART2_Config();
}

/**	!!! For USART: better to set GPIO and USART parameters in the same function, avoid sending error byte(0xE0)
  * @Brief	Config USART_1 as Debug_USART
  * @Param	None
  * @Retval	None
  */
static void Hal_USART1_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	// USART1_TX -> PA9 		
	GPIO_InitStructure.GPIO_Pin = DEBUG_TX_PIN;	         
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DEBUG_TX_PORT, &GPIO_InitStructure);	
	
	// USART1_RX -> PA10
	GPIO_InitStructure.GPIO_Pin = DEBUG_RX_PIN;	        
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
	GPIO_Init(DEBUG_RX_PORT, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(DEBUG_USART_PORT, &USART_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_ITConfig(DEBUG_USART_PORT, USART_IT_RXNE, ENABLE);
	USART_ITConfig(DEBUG_USART_PORT, USART_IT_TXE, DISABLE);	// disable Tx
	USART_Cmd(DEBUG_USART_PORT, ENABLE);
	
	// USART1_Tx use DMA1_Channel4,  USART1_Rx use DMA1_Channel5
	USART_DMACmd(DEBUG_USART_PORT, USART_DMAReq_Tx, ENABLE);	// enable USART DMA_Tx transfer
}

/**	
  * @Brief	Config UART_5 as Lora_USART
  * @Param	None
  * @Retval	None
  */
static void Hal_UART5_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
	
	// UART5_TX -> PC12
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = LORA_TX_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LORA_TX_PORT, &GPIO_InitStructure);
	
	// UART5_RX ->	PD2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = LORA_RX_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LORA_RX_PORT, &GPIO_InitStructure);
	
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(LORA_USART_PORT, &USART_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 6;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_ITConfig(LORA_USART_PORT, USART_IT_RXNE, ENABLE);
	
	USART_Cmd(LORA_USART_PORT, ENABLE);
}

/**	
  * @Brief	Config UART_3 as WiFi_USART
  * @Param	None
  * @Retval	None
  */
static void Hal_USART3_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	
	// USART3_TX -> PB10  		
	GPIO_InitStructure.GPIO_Pin = WIFI_TX_PIN;	         
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(WIFI_TX_PORT, &GPIO_InitStructure);	
	
	// USART3_RX -> PB11
	GPIO_InitStructure.GPIO_Pin = WIFI_RX_PIN;	        
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
	GPIO_Init(WIFI_RX_PORT, &GPIO_InitStructure);
	
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(WIFI_USART_PORT, &USART_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_ITConfig(WIFI_USART_PORT, USART_IT_RXNE, ENABLE);
	
	USART_Cmd(WIFI_USART_PORT, ENABLE);
}

/**	
  * @Brief	Config UART_2 as GSM_USART
  * @Param	None
  * @Retval	None
  */
static void Hal_USART2_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	
	// USART2_TX -> PA2 ,		
	GPIO_InitStructure.GPIO_Pin = GSM_TX_PIN;	         
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GSM_TX_PORT, &GPIO_InitStructure);		
	
	// USART2_RX ->	PA3
	GPIO_InitStructure.GPIO_Pin = GSM_RX_PIN;	        
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
	GPIO_Init(GSM_RX_PORT, &GPIO_InitStructure);
	
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(GSM_USART_PORT, &USART_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= 4 ;	
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;					
	NVIC_Init(&NVIC_InitStructure);	
	
	USART_ITConfig(GSM_USART_PORT, USART_IT_RXNE, ENABLE);
	
	USART_Cmd(GSM_USART_PORT, ENABLE);
}

/**
  * @Brief	DMA automatically transfer data from Queue_DebugTx to the USART1 port and transmit
  * @Param	None
  * @Retval	None
  *	@Note	USART1 combined DMA1_Channel4 work as the DebugDataTx Channel
  *			any valid data in the Queue_DebugTx will be print out through seriel port
  */
static void Hal_USART1_DMA_SendData(void)
{
	uint16_t i;
	uint16_t Len;
	
	if(DebugBusyFlag)	// wait for DMA idle
	{
		return;
	}
	
	Len = QueueDataLen(Queue_DebugTx);	// get the datalength ready to send
	
	for(i=0; i<Len; i++)
	{
		QueueDataOut(Queue_DebugTx, &Buffer_DebugTx[i]);	// queue out data to DebugTx buffer
	}
	
	if(Len)
	{
		/* !!! use DMA stdLib function will lead to potential issue: !!!	 */
		/* (1) when WiFi module reset, the DMA stop transfer data to USART	 */
		
		/* DMA_Register operation */
		DMA1_Channel4->CCR &= ~(1<<0);
		DMA1_Channel4->CNDTR = Len;
		DMA1_Channel4->CMAR = (uint32_t)(&Buffer_DebugTx[0]);
		DMA1_Channel4->CCR |= 1<<0;
		
		DebugBusyFlag = 1;	// DMA busy
	}
}

/**
  * @Brief	Lora_USART debug function
  * @Param	None
  * @Retval	None
  */
static void Hal_USART_LoraDebug(void)
{
	static uint16_t LoraTx_Counter = 0;
	
	LoraTx_Counter++;
	
	if(LoraTx_Counter > 200)	// trigger every 200 cycles(2s)
	{
		LoraTx_Counter = 0;
		Hal_USART_LoraDataTx("Lora USART Test\r\n", 17);
	}
}

/**
  * @Brief	WiFi_USART debug function
  * @Param	None
  * @Retval	None
  */
static void Hal_USART_WiFiDebug(void)
{
	uint16_t Len;
	uint16_t i;
	
	static uint16_t WiFiTx_Counter = 0;
	
	WiFiTx_Counter++;
	
	if(WiFiTx_Counter > 300)
	{
		WiFiTx_Counter = 0;
		Hal_USART_DebugDataQueueIn("Test\r\n", 6);
	}
	
	/* DMA problem? Test */
	Len = QueueDataLen(Queue_DebugTx);	// get the datalength ready to send
	
	for(i=0; i<Len; i++)
	{
		QueueDataOut(Queue_DebugTx, &Buffer_DebugTx[i]);	// queue out data to DebugTx buffer
	
		USART_SendData(USART1, Buffer_DebugTx[i]);
		
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
		{
		
		}
	}
	
}

/**
  * @Brief	GSM_USART debug function
  * @Param	None
  * @Retval	None
  */
static void Hal_USART_GSMDebug(void)
{
	static uint16_t GSMTx_Counter = 0;
	
	GSMTx_Counter++;
	
	if(GSMTx_Counter > 200)
	{
		GSMTx_Counter = 0;
		Hal_USART_GSMDataTx("ATI\r\n", 5);
	}
}

/*-------------Interrupt Functions Definition--------*/
/**
  * @Brief	DMA1_Channel4 IRQ handler
  * @Param	None
  * @Retval	None
  */
void DMA1_Channel4_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_IT_TC4) != RESET)	// wait DMA1_channel4 transfer finish
	{
		DMA_ClearITPendingBit(DMA1_IT_TC4);
		DMA_Cmd(DMA1_Channel4, DISABLE);
		DebugBusyFlag = 0;	// DMA1_channel4 is idle
	}
}

/**
  * @Brief	USART1 IRQ handler(Debug)
  * @Param	None
  * @Retval	None
  */
void USART1_IRQHandler(void)
{
	uint8_t RxData;
	
	if(USART_GetITStatus(DEBUG_USART_PORT, USART_IT_RXNE) != RESET)
	{							
		RxData = USART_ReceiveData(DEBUG_USART_PORT);
		USART_ClearITPendingBit(DEBUG_USART_PORT, USART_IT_RXNE);
		 
		QueueDataIn(Queue_DebugTx, &RxData, 1);		
	}
}

/**
  * @Brief	UART5 IRQ handler(Lora)
  *			Use USART5 RXNE interrupt to receive data
  * @Param	None
  * @Retval	None
  */
void UART5_IRQHandler(void)
{
	uint8_t RxData;
	
	if(USART_GetITStatus(LORA_USART_PORT, USART_IT_RXNE) != RESET)
	{
		RxData = USART_ReceiveData(LORA_USART_PORT);
		USART_ClearITPendingBit(LORA_USART_PORT, USART_IT_RXNE);
		
		if(Lora_USART_RxCBF)
		{
			Lora_USART_RxCBF(RxData);
		}
	}
}

/**
  * @Brief	UART3 IRQ handler(WiFi)
  *			Use USART3 RXNE interrupt to receive data
  * @Param	None
  * @Retval	None
  */
void USART3_IRQHandler(void)
{
	uint8_t RxData;
	
	if(USART_GetITStatus(WIFI_USART_PORT, USART_IT_RXNE) != RESET)
	{
		RxData = USART_ReceiveData(WIFI_USART_PORT);
		USART_ClearITPendingBit(WIFI_USART_PORT, USART_IT_RXNE);
		
		if(WiFi_USART_RxCBF)
		{
			WiFi_USART_RxCBF(RxData);
		}
		
		Hal_USART_DebugDataQueueIn(&RxData, 1);	// Queue-in received data to Queue_DebugTx
	}
}

/**
  * @Brief	UART2 IRQ handler(GSM)
  *			Use USART2 RXNE interrupt to receive data
  * @Param	None
  * @Retval	None
  */
void USART2_IRQHandler(void)
{
	uint8_t RxData;
	
	if(USART_GetITStatus(GSM_USART_PORT, USART_IT_RXNE) != RESET)
	{
		RxData = USART_ReceiveData(GSM_USART_PORT);
		USART_ClearITPendingBit(GSM_USART_PORT, USART_IT_RXNE);
		
		if(GSM_USART_RxCBF)
		{
			GSM_USART_RxCBF(RxData);
		}
		
		Hal_USART_DebugDataQueueIn(&RxData, 1);
	}
}

