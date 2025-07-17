#ifndef __NVIC_SETTING_H_
#define __NVIC_SETTING_H_

/* NVIC Priority Setting Table */
/**--------------------------------------------
  * 		PreemptionPriority	SubPriority
  *	USART1			3				3
  *	USART2			4				3
  *	USART3			5				3
  *	UART5			5				6
  *	TIM4			0				3
  *	TIM3			2				0
  *	DMA2			/				/
  *	DMA1			6				0
  *--------------------------------------------
  */

void NVIC_Setting(void);

#endif
