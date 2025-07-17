/****************************************************
  * @Name	main.c
  * @Brief	Main function
  ***************************************************/

/*-------------Header Files Include----------------*/
#include "stm32f10x.h"  

#include "os_system.h"
#include "os_cpu.h"
#include "nvic_setting.h"

#include "hal_task.h"
#include "mid_task.h"
#include "app.h"
#include "device.h"

int main(void)
{
	NVIC_SetVectorTable(0x8000000, 0xC800);	// Divide the Flash into two parts: 1. (50KB)BootLoader->start from 0x8000000, 2. (206KB)AppPart->start from 0x800C800
	__enable_irq();
	
	/* -----------Module Initialize---------- */
	OS_CPU_Init();
	OS_TaskInit();		// System Init
	
	NVIC_Setting();		// NVIC Group Setting
	
	Hal_Task_Init();	// HAL Init
	
	Device_Init();		// Device_Init fuction reads data from EEPROM, which needs HAL module being initialized 
										// the MCU_UID will be used by Mid Layer, invoke before Mid_Task_Init function
	
	Mid_Task_Init();	// Middle-Layer Init
	
	App_Init();			// Application-Layer Init

	
	/* ----------OS_Task Creat-------------- */
	/* Period Timebase = 10ms */
	OS_CreatTask(OS_TASK1, Hal_Task_Pro, 1, OS_RUN);	// HAL operation
	
	OS_CreatTask(OS_TASK2, Mid_Task_Pro, 1, OS_RUN);	// Middle layer operation
	
	OS_CreatTask(OS_TASK3, App_Pro, 1, OS_RUN);			// Application operation

	/* ----------Start Scheduler------------- */
	OS_Start();
}
