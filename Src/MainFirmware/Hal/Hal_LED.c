/****************************************************
  * @Name	Hal_LED.c
  * @Brief	LED config and handle
  ***************************************************/

/*-------------Header Files Include-----------------*/
#include "stm32f10x.h"                  
#include "hal_led.h"
#include "hal_gpio.h"
#include "hal_timer.h"

/*-------------Internal Functions Declaration------*/
static void Hal_LED_DelayTest(uint32_t i);
static void Hal_LED_DebugHandler(void);

/*-------------Module Variables Declaration--------*/



/*-------------Module Functions Definition---------*/
/**
  * @Brief	Initialize LED module 
  * @Param	None
  * @Retval	None
  */
void Hal_LED_Init(void)
{	
	Hal_Timer_Creat(T_LED, Hal_LED_DebugHandler, 40000, T_STATE_START);
}

/**
  * @Brief	Polling in sequence for LED module
  * @Param	None
  * @Retval	None
  */
void Hal_LED_Pro(void)
{
	static uint16_t count = 0;
	
	count++;
	
	if(count > 99)
	{
		count = 0;
		Hal_LED8_Toggle();
	}
}

/*-------------Internal Functions Definition--------*/
/**
  * @Brief	Delay function for test
  * @Param	i: the number of delay cycles
  *		uint16_t: can be 0 - 0xFFFF FFFF
  * @Retval	None
  */
static void Hal_LED_DelayTest(uint32_t i)
{
	while(i--);
}

/**
  * @Brief	Handler for debug
  * @Param	None
  * @Retval	None
  */
static void Hal_LED_DebugHandler(void)
{	
	Hal_LED7_Toggle();
	Hal_Timer_Reset(T_LED, T_STATE_START);
}
