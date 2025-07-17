/****************************************************
  * @Name	Mid_Task.c
  * @Brief	Task module for all MDL modules
  ***************************************************/
  
/*-------------Header Files Include-----------------*/
#include "stm32f10x.h"
#include "mid_task.h"
#include "mid_flash.h"
#include "mid_tftlcd.h"
#include "hal_led.h"

/*-------------Module Functions Definition---------*/
/**
  * @Brief	Initialize all Middle-layer modules
  * @Param	None
  * @Retval	None
  */
void Mid_Task_Init(void)
{
	Mid_Flash_Init();
	Mid_TFTLCD_Init();
	
	while(1)
	{
		if(DelayCounter > 5)
		{
			DelayCounter = 0;
			
			break;
		}
	}
}

/**
  * @Brief	Polling functions of Middle-layer modules
  * @Param	None
  * @Retval	None
  */
void Mid_Task_Pro(void)
{

}
