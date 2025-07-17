/****************************************************
  * @Name	Hal_JumpToApp.c
  * @Brief	
  ***************************************************/
  
/*-------------Header Files Include-----------------*/
#include "stm32f10x.h"
#include "hal_jumptoapp.h"

/*-------------Internal Functions Declaration------*/
static void Hal_JumpToApp_PeripheralDeInit(void);

/*-------------Module Variables Declaration--------*/
pIAP_Function_t JumpToAppFunction;

/*---Module Call-Back function pointer Definition---*/


/*-------------Module Functions Definition---------*/
/**
  * @brief  Set the Main Stack Pointer
  * @param  MainStackPointer: address of the main-stack-top
  * @Retval	None  
  */
__asm void MSR_MSP(uint32_t MainStackPointer)
{
	MSR 	MSP, r0		// Set Main Stack pointer value to Address
	BX 		r14
}

/**
  * @Brief	Jump to the Flash address of AppPart, and execute the code
  * @Param	None
  * @Retval	None
  */
void Hal_JumpToApp_Jump(void)
{
	uint32_t AppStackTopAddress;			// the address of Stack-Top of the AppPart(first word of VectorTable)
	uint32_t AppResetHandlerAddress;	// the address of Reset_Handler of the AppPart

	
	AppStackTopAddress = *(volatile uint32_t *)EMBEDDED_FLASH_Address_APP_BASE;						// 0x0800C800
	AppResetHandlerAddress = *(volatile uint32_t *)(EMBEDDED_FLASH_Address_APP_BASE + 4);	// 0x0800C804
	
	/* Check if valid stack address(RAM address), then jump to AppPart */
	/* according to the target setting, the end of RAM: 0x200010000 
	   effective RAM address:0x20000000 -> 0x2000FFFF 
	*/
	if((AppStackTopAddress & 0x2FFF0000) == 0x20000000)
	{
		Hal_JumpToApp_PeripheralDeInit();
		
		/* initialize AppPart stack-pointer */
		MSR_MSP(AppStackTopAddress);
		//__set_MSP(AppStackTopAddress);
		
		/* Jump to the Reset_Handler of the AppPart to finish jump operation */
		JumpToAppFunction = (pIAP_Function_t)AppResetHandlerAddress;
		JumpToAppFunction();
	}
}

/*-------------Internal Functions Definition--------*/
/**
  * @Brief	DeInit all used Peripheral and disable Interrupt
  * @Param	None
  * @Retval	None
  */
static void Hal_JumpToApp_PeripheralDeInit(void)
{
	TIM_DeInit(TIM4);  
	TIM_Cmd(TIM4, DISABLE);

	USART_DeInit(USART1);  
	USART_DeInit(USART2);
	USART_DeInit(USART3);
	USART_DeInit(UART4);
	USART_DeInit(UART5);
	
	USART_Cmd(USART1,DISABLE);  
	USART_Cmd(USART2,DISABLE);
	USART_Cmd(USART3,DISABLE);
	USART_Cmd(UART4,DISABLE);
	USART_Cmd(UART5,DISABLE);

	SPI_I2S_DeInit(SPI2);
	SPI_I2S_DeInit(SPI3);

	SPI_Cmd(SPI2, DISABLE); 	
	SPI_Cmd(SPI3, DISABLE);

	RCC_RTCCLKCmd(DISABLE); 
	
	__disable_irq();
}


/*-------------Interrupt Functions Definition--------*/


