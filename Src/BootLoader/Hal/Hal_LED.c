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
static void Hal_LED_DebugHandler(void);

/*-------------Module Variables Declaration--------*/
volatile uint8_t DelayCounter = 0;


/*-------------Module Functions Definition---------*/
/**
  * @Brief	Initialize LED module 
  * @Param	None
  * @Retval	None
  */
void Hal_LED_Init(void)
{	
	Hal_Timer_Creat(T_LED, Hal_LED_DebugHandler, 20000, T_STATE_START);
	
	GPIO_SetBits(LED7_PORT, LED7_PIN);
	GPIO_ResetBits(LED8_PORT, LED8_PIN);
}

/**
  * @Brief	Polling in sequence for LED module
  * @Param	None
  * @Retval	None
  */
void Hal_LED_Pro(void)
{

}

/*-------------Internal Functions Definition--------*/
/**
  * @Brief	Handler for debug
  * @Param	None
  * @Retval	None
  */
static void Hal_LED_DebugHandler(void)
{
	Hal_LED7_Toggle();
	Hal_LED8_Toggle();
	
	Hal_Timer_Reset(T_LED, T_STATE_START);
	
	DelayCounter++;
}
