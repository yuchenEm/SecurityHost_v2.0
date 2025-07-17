/****************************************************
  * @Name	Hal_Task.c
  * @Brief	Task module for all HAL modules
  ***************************************************/
  
/*-------------Header Files Include-----------------*/
#include "stm32f10x.h"
#include "hal_task.h"

#include "hal_gpio.h"
#include "hal_led.h"
#include "hal_timer.h"
#include "hal_spi.h"
#include "hal_dma.h"
#include "hal_tftlcd.h"


/*-------------Module Functions Definition---------*/
/**
  * @Brief	Initialize all Hal modules
  * @Param	None
  * @Retval	None
  */
void Hal_Task_Init(void)
{
	Hal_GPIO_Init();
	Hal_Timer_Init();
	Hal_LED_Init();
	Hal_SPI_Init();
	Hal_DMA_Init();
	
}

/**
  * @Brief	Polling functions of Hal modules
  * @Param	None
  * @Retval	None
  */
void Hal_Task_Pro(void)
{
	Hal_LED_Pro();
}
