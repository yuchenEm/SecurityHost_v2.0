/****************************************************
  * @Name	Mid_PowerManage.c
  * @Brief	Detect the AC-charger connection condition  
  *			and BatteryVoltageLevel condotion
  ***************************************************/
  
/*-------------Header Files Include-----------------*/
#include "stm32f10x.h"
#include "mid_powermanage.h"
#include "hal_gpio.h"
#include "hal_adc.h"
#include "mqtt_protocol.h"


/*-------------Internal Functions Declaration------*/
static void Mid_PowerManage_ACStateCheckHandler(void);
static void Mid_PowerManage_BatteryStateCheckHandler(void);


/*-------------Module Variables Declaration--------*/
en_ACLinkSta_t 	ACLinkState;
uint8_t 		BatteryVoltageLevel;

/*---Module Call-Back function pointer Definition---*/


/*-------------Module Functions Definition---------*/
/**
  * @Brief	Initialize PowerManage module
  * @Param	None
  * @Retval	None
  */
void Mid_PowerManage_Init(void)
{
	ACLinkState = Hal_GPIO_ACStateCheck();
	BatteryVoltageLevel = Hal_ADC_GetBatteryVoltageLevel();
}

/**
  * @Brief	Polling function
  * @Param	None
  * @Retval	None
  */
void Mid_PowerManage_Pro(void)
{
	Mid_PowerManage_ACStateCheckHandler();
	Mid_PowerManage_BatteryStateCheckHandler();
}

/*-------------Internal Functions Definition--------*/
/**
  * @Brief	Check AC state handler
  * @Param	None
  * @Retval	None
  */
static void Mid_PowerManage_ACStateCheckHandler(void)
{
	if(ACLinkState != Hal_GPIO_ACStateCheck())
	{
		ACLinkState = Hal_GPIO_ACStateCheck();
		
		/* AC charger connected */
		if(ACLinkState == STA_AC_LINK)
		{
			/* queue-in EventUpload MQTT message */
			MQTTProtocol_EventUpQueueIn(TERMINAL_UPEVENT_HOST_AC_CONNECT, 0xFF);
		}
		/* AC charger disconnected */
		else
		{
			/* queue-in EventUpload MQTT message */
			MQTTProtocol_EventUpQueueIn(TERMINAL_UPEVENT_HOST_AC_DISCONN, 0xFF);
		}
	}
}

/**
  * @Brief	Check battery state handler
  * @Param	None
  * @Retval	None
  */
static void Mid_PowerManage_BatteryStateCheckHandler(void)
{
	/* battery supply mode */
	if(ACLinkState == STA_AC_BREAK)
	{
		if(BatteryVoltageLevel != Hal_ADC_GetBatteryVoltageLevel())
		{
			BatteryVoltageLevel = Hal_ADC_GetBatteryVoltageLevel();
			
			/* battery low */
			if(BatteryVoltageLevel == LEVEL_LOW)
			{
				/* queue-in EventUpload MQTT message */
				MQTTProtocol_EventUpQueueIn(TERMINAL_UPEVENT_HOST_BATLOW, 0xFF);
			}
		}
	}
}


/*-------------Interrupt Functions Definition--------*/


