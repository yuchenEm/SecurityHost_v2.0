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

int main(void)
{
	NVIC_SetVectorTable(0x8000000, 0x0000);
	
	/* -----------Module Initialize---------- */
	OS_CPU_Init();
	OS_TaskInit();		// System Init
	
	NVIC_Setting();		// NVIC Group Setting
	
	Hal_Task_Init();	// HAL Init

	Mid_Task_Init();	// Middle-Layer Init
	
	App_Init();
	
	/* ----------OS_Task Creat-------------- */
	/* Period Timebase = 10ms */	
	OS_CreatTask(OS_TASK1, App_Pro, 1, OS_RUN);	
	


	/* ----------Start Scheduler------------- */
	OS_Start();
}
