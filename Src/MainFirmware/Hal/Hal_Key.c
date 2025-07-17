/****************************************************
  * @Name	Hal_Key.c
  * @Brief	ADA20A 16_key capacitor key pad driver (5DB, 17states encoder, Low effective)
  ***************************************************/
  
/*-------------Header Files Include-----------------*/
#include "stm32f10x.h"
#include "hal_key.h"
#include "hal_gpio.h"
#include "string.h"


/*-------------Internal Functions Declaration------*/
static void Hal_Key_KeyPressDecoder(uint8_t *pKeyFunction);

static uint8_t Hal_Key_GetState_DB0(void);
static uint8_t Hal_Key_GetState_DB1(void);
static uint8_t Hal_Key_GetState_DB2(void);
static uint8_t Hal_Key_GetState_DB3(void);
static uint8_t Hal_Key_GetState_DB4(void);

/*-------------Module Variables Declaration--------*/
uint8_t  KeyState[KEY_SUM];								// Key State
uint16_t KeyScanTimer[KEY_SUM];						// Timer of dithering elimination
uint16_t KeyLongPressTimer[KEY_SUM];			// Timer of long-press
uint16_t KeyContinuePressTimer[KEY_SUM];	// Timer of continue-press


/* GetKeyValue: the array of function pointers */
uint8_t (*GetKeyValue[DB_SUM])() = 
{
	Hal_Key_GetState_DB4,
	Hal_Key_GetState_DB3,
	Hal_Key_GetState_DB2,
	Hal_Key_GetState_DB1,
	Hal_Key_GetState_DB0
};

/* key scan call-back function pointer */
KeyEvent_CallBack_t KeyScanCBF;

/*-------------Module Functions Definition---------*/
/**
  * @Brief	Initialize Key module
  * @Param	None
  * @Retval	None
  */
void Hal_Key_Init(void)
{
	uint8_t i;
	
	KeyScanCBF = 0;
	
	for(i=0; i<KEY_SUM; i++)
	{
		KeyState[i] = KEYSCAN_STEP_IDLE;
		KeyScanTimer[i] = KEY_SCAN_TIME;
		KeyLongPressTimer[i] = KEY_LONGPRESS_TIME;
		KeyContinuePressTimer[i] = KEY_CONTINUEPRESS_TIME;
	}
}

/**
  * @Brief	Polling in sequence for Key module
  * @Param	None
  * @Retval	None
  */
void Hal_Key_Pro(void)
{
	uint8_t i;	
	uint8_t KeyIndex;	// Index 0-16 corresponding to 16-tpyes Keyname
	uint8_t KeyEvent;
	uint8_t KeyPressFlag[KEY_SUM];
	
	memset(KeyPressFlag, 0, KEY_SUM);	// reset KeyPressFlag[KEY_SUM] to all 0
	
	Hal_Key_KeyPressDecoder(KeyPressFlag);	// record pressed key
	
	for(i=0; i<KEY_SUM; i++)
	{
		KeyIndex = 0xFF;	// reset KeyIndex=0xFF indicate no action
		KeyEvent = 0;
		
		switch(KeyState[i])
		{
			case KEYSCAN_STEP_IDLE:
			{
				if(KeyPressFlag[i] == 1)
				{
					KeyState[i] = KEYSCAN_STEP_CLICK;
				}
			}
			break;
			
			case KEYSCAN_STEP_CLICK:
			{
				if(KeyPressFlag[i] == 1)
				{
					if(!(--KeyScanTimer[i]))
					{
						KeyScanTimer[i] = KEY_SCAN_TIME;
						KeyState[i] = KEYSCAN_STEP_LONG_PRESS;
						
						KeyIndex = i;
						KeyEvent = KEY_CLICK;
					}
				}
				else
				{
					KeyScanTimer[i] = KEY_SCAN_TIME;
					KeyState[i] = KEYSCAN_STEP_IDLE;
				}
			}
			break;
			
			case KEYSCAN_STEP_LONG_PRESS:
			{
				if(KeyPressFlag[i] == 1)
				{
					if(!(--KeyLongPressTimer[i]))
					{
						KeyLongPressTimer[i] = KEY_LONGPRESS_TIME;
						KeyState[i] = KEYSCAN_STEP_CONTINUE_PRESS;
						
						KeyIndex = i;
						KeyEvent = KEY_LONG_PRESS;
					}
				}
				else
				{
					KeyLongPressTimer[i] = KEY_LONGPRESS_TIME;
					KeyState[i] = KEYSCAN_STEP_RELEASE;
					
					KeyIndex = i;
					KeyEvent = KEY_CLICK_RELEASE;
				}
			}
			break;
			
			case KEYSCAN_STEP_CONTINUE_PRESS:
			{
				if(KeyPressFlag[i] == 1)
				{
					if(!(--KeyContinuePressTimer[i]))
					{
						KeyContinuePressTimer[i] = KEY_CONTINUEPRESS_TIME;
						KeyState[i] = KEYSCAN_STEP_CONTINUE_PRESS;
						
						KeyIndex = i;
						KeyEvent = KEY_LONG_PRESS_CONTINUE;
					}
				}
				else
				{
					KeyContinuePressTimer[i] = KEY_CONTINUEPRESS_TIME;
					KeyState[i] = KEYSCAN_STEP_RELEASE;
					
					KeyIndex = i;
					KeyEvent = KEY_LONG_PRESS_RELEASE;
				}
			}
			break;
			
			case KEYSCAN_STEP_RELEASE:
			{
				if(KeyPressFlag[i] == 0)
				{
					KeyState[i] = KEYSCAN_STEP_IDLE;
				}
			}
			break;
		}
		
		if(KeyIndex != 0xFF)
		{
			if(KeyScanCBF)
			{
				KeyScanCBF((en_KeyType_t)KeyIndex, (en_KeyEvent_t)KeyEvent);
			}
		}
	}
}

/**
  * @Brief	Register call-back function(API for upper layer)
  * @Param	pCBF: function pointer from upper layer
  * @Retval	None
  */
void Hal_Key_KeyScanCBFRegister(KeyEvent_CallBack_t pCBF)
{
	if(KeyScanCBF == 0)
	{
		KeyScanCBF = pCBF;
	}
}


/*-------------Internal Functions Definition--------*/
/**
  * @Brief	Decode from 5 DB data to resolve pressed key, and set corresponding flags
  * @Param	pKeyPressFlag: pointer to the flags corresponding to the pressed key
  * @Retval	None
  */
static void Hal_Key_KeyPressDecoder(uint8_t *pKeyPressFlag)
{
	uint8_t i;
	uint8_t KeyValue = 0;
	
	// Read 5bits DB data
	// KeyValue = DB4 DB3 DB2 DB1 DB0
	for(i=0; i<DB_SUM; i++)	
	{
		KeyValue <<= 1;
		
		if((*GetKeyValue[i])())
		{
			KeyValue |= 0x01;
		}
	}
	
	switch(KeyValue)
	{
		case 0x01:
		{
			pKeyPressFlag[KEY_MENU_CONFIRM] = 1;
		}
		break;
		
		case 0x02:
		{
			pKeyPressFlag[KEY_DISARM] = 1;
		}
		break;
		
		case 0x03:
		{
			pKeyPressFlag[KEY_HOMEARM] = 1;
		}
		break;
		
		case 0x04:
		{
			pKeyPressFlag[KEY_AWAYARM] = 1;
		}
		break;
		
		case 0x05:
		{
			pKeyPressFlag[KEY_CALL_CANCEL] = 1;
		}
		break;
		
		case 0x06:
		{
			pKeyPressFlag[KEY_9] = 1;
		}
		break;
		
		case 0x07:
		{
			pKeyPressFlag[KEY_6_RIGHT] = 1;
		}
		break;
		
		case 0x08:
		{
			pKeyPressFlag[KEY_3] = 1;
		}
		break;
		
		case 0x09:
		{
			pKeyPressFlag[KEY_0] = 1;
		}
		break;
		
		case 0x0A:
		{
			pKeyPressFlag[KEY_8_DOWN] = 1;
		}
		break;
		
		case 0x0B:
		{
			pKeyPressFlag[KEY_5] = 1;
		}
		break;
		
		case 0x0C:
		{
			pKeyPressFlag[KEY_2_UP] = 1;
		}
		break;
		
		case 0x0D:
		{
			pKeyPressFlag[KEY_SOS_DELET] = 1;
		}
		break;
		
		case 0x0E:
		{
			pKeyPressFlag[KEY_7] = 1;
		}
		break;
		
		case 0x0F:
		{
			pKeyPressFlag[KEY_4_LEFT] = 1;
		}
		break;
		
		case 0x10:
		{
			pKeyPressFlag[KEY_1] = 1;
		}
		break;
	}
}


/**
  * @Brief	Get key status function
  * @Param	None
  * @Retval	Key Pins status
  */
static uint8_t Hal_Key_GetState_DB0(void)
{
	return GPIO_ReadInputDataBit(KEY_DB0_PORT, KEY_DB0_PIN);
}

static uint8_t Hal_Key_GetState_DB1(void)
{
	return GPIO_ReadInputDataBit(KEY_DB1_PORT, KEY_DB1_PIN);
}

static uint8_t Hal_Key_GetState_DB2(void)
{
	return GPIO_ReadInputDataBit(KEY_DB2_PORT, KEY_DB2_PIN);
}

static uint8_t Hal_Key_GetState_DB3(void)
{
	return GPIO_ReadInputDataBit(KEY_DB3_PORT, KEY_DB3_PIN);
}

static uint8_t Hal_Key_GetState_DB4(void)
{
	return GPIO_ReadInputDataBit(KEY_DB4_PORT, KEY_DB4_PIN);
}


/*-------------Interrupt Functions Definition--------*/


