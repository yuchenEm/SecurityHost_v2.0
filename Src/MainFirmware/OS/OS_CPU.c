/****************************************************
  * @Name	OS_CPU.c
  * @Brief	CPU handler
  * @Note	Modify header files and Core functions for different MCU
  ***************************************************/

/*-------------Header Files Include----------------*/
#include "stm32f10x.h"                  
#include "os_system.h"
#include "os_cpu.h"


/*-------------Internal Functions Declaration-------*/
static void OS_CoreClock_Init(void);
static unsigned char OS_CPU_GetInterruptState(void);
static void OS_CPU_CriticalControl(CPU_EA_TYPEDEF cmd, unsigned char *pSta);


/*-------------Module Variables Declaration---------*/



/*-------------Module Functions Definition----------*/
/**
  * @Brief	Initialize CPU module 
  * @Param	None 
  * @Retval	None
  */
void OS_CPU_Init(void)
{
	OS_CoreClock_Init();
	OS_CPUInterruptCBSRegister(OS_CPU_CriticalControl);
}


/*-------------Internal Functions Definition--------*/
/**
  * @Brief	Systick setting (Reload Value)
  * @Param	None 
  * @Retval	None
  */
static void OS_CoreClock_Init(void)
{
	SysTick_Config(SystemCoreClock / 100); // 10ms
}

/**************************************************************************
	@Name		: OS_CPU_GetInterruptState
	@Function	: get CPU interrupt status
	@Return		: 0: interrupt is off; 1: interrupt is on
***************************************************************************/
static unsigned char OS_CPU_GetInterruptState(void)
{
	return(!__get_PRIMASK());
	/*
	__get_PRIMASK() will return (__regPriMask)
	__regPriMask: Bit[0] of priority mask register(Exception Mask Registers)
	Bit[0]=1: any interrupts priority>0 will be masked(only Reset, NMI, Hardfault can respond)
	
	Exception Mask Registers: PRIMASK, FAULTMASK, BASEPRI
	*/
}


/**************************************************************************
	@Name		: OS_CPU_CriticalControl
	@Function	: CPU eadge condition handler
		@cmd		: control command
		@*psta: interrupt condition
***************************************************************************/
static void OS_CPU_CriticalControl(CPU_EA_TYPEDEF cmd, unsigned char *pSta)
{
	if(cmd == CPU_ENTER_CRITICAL)
	{
		*pSta = OS_CPU_GetInterruptState();	// save interrupt status
		__disable_irq();		// turn off interrupt
	}
	else if(cmd == CPU_EXIT_CRITICAL)
	{
		if(*pSta)
		{
			__enable_irq();		// turn on interrupt
		}
		else 
		{
			__disable_irq();	// turn off interrupt
		}
	}
}

/*-------------Interrupt Functions Definition-------*/
/*-----------------------------------------------------
	@Name		: SysTick_Handler()
	@Function	: Call OS Task scheduler function
-------------------------------------------------------*/
void SysTick_Handler(void)
{
	OS_ClockInterruptHandle();
}
