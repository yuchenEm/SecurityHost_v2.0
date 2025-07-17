/****************************************************
  * @Name	Hal_Timer.c
  * @Brief	config timer
  *		TIM4 --> Timer-Matrix
  *		TIM3 --> AL6630 TH_module data capture
  ***************************************************/

/*-------------Header Files Include-----------------*/
#include "stm32f10x.h" 
#include "hal_timer.h"
#include "hal_led.h"

/*-------------Internal Functions Declaration------*/
static void Hal_Timer_Config(void);
static void Hal_Timer4_Config(uint16_t Arr, uint16_t Psc);
static void Hal_Timer3_Config(uint16_t Arr, uint16_t Psc);

static void Hal_Timer_ITHandler(void);

/*-------------Module Variables Declaration--------*/
volatile stu_Timer_t Stu_Timer[T_SUM];

/*-------------Module Functions Definition---------*/
/**
  * @Brief	Initialize Timer module
  * @Param	None
  * @Retval	None
  */
void Hal_Timer_Init(void)
{
	unsigned char i;
	
	Hal_Timer_Config();
	
	for(i=0; i<T_SUM; i++)
	{
		Stu_Timer[i].state = T_STATE_STOP;
		Stu_Timer[i].CurrentCount = 0;
		Stu_Timer[i].func = 0;
		Stu_Timer[i].Period = 0;
	}
}

/**
  * @Brief	Creat timer
  * @Param	ID		: timer ID
  *			proc	: pointer to the polling function
  *			Period	: counting number of timebase
  *			State	: timer status
  * @Retval	None
  */
void Hal_Timer_Creat(en_Timer_ID_t ID, void (*proc)(void), unsigned short Period, en_Timer_State_t State)
{
	Stu_Timer[ID].state = State;
	Stu_Timer[ID].CurrentCount = 0;
	Stu_Timer[ID].func = proc;
	Stu_Timer[ID].Period = Period;
}

/**
  * @Brief	Reset timer and change timer status
  * @Param	ID		: timer ID
  *			State	: timer status
  * @Retval	T_SUCCESS / T_FAIL
  */
en_Timer_Result_t Hal_Timer_Reset(en_Timer_ID_t ID, en_Timer_State_t State)
{
	if(Stu_Timer[ID].func)
	{
		Stu_Timer[ID].state = State;
		Stu_Timer[ID].CurrentCount = 0;
		return T_SUCCESS;
	}
	else
	{
		return T_FAIL;
	}
}

/**
  * @Brief	Delete timer
  * @Param	ID		: timer ID
  * @Retval	T_SUCCESS / T_FAIL
  */
en_Timer_Result_t Hal_Timer_Delete(en_Timer_ID_t ID)
{
	if(Stu_Timer[ID].func)
	{
		Stu_Timer[ID].state = T_STATE_STOP;
		Stu_Timer[ID].CurrentCount = 0;
		Stu_Timer[ID].func = 0;
		return T_SUCCESS;
	}
	else
	{
		return T_FAIL;
	}
}

/**
  * @Brief	Change timer status
  * @Param	ID		: timer ID
  *			State	: timer status
  * @Retval	T_SUCCESS / T_FAIL
  */
en_Timer_Result_t Hal_Timer_StateControl(en_Timer_ID_t ID, en_Timer_State_t State)
{
	if(Stu_Timer[ID].func)
	{
		Stu_Timer[ID].state = State;
		return T_SUCCESS;
	}
	else
	{
		return T_FAIL;
	}
}

/**
  * @Brief	Get timer status
  * @Param	ID		: timer ID
  * @Retval	T_SUCCESS / T_FAIL
  */
en_Timer_State_t Hal_Timer_GetState(en_Timer_ID_t ID)
{
	if(Stu_Timer[ID].func)
	{
		return Stu_Timer[ID].state;
	 
	}
	else
	{
		return T_STATE_INVALID;
	}
}

/*-------------Internal Functions Definition--------*/
/**
  * @Brief	Config Timer
  * @Param	None
  * @Retval	None
  * @TimBase: Based on [TIMER_PRESCALER_72]
  *		TIMEBASE_1us	1
  * 	TIMEBASE_5us	5
  * 	TIMEBASE_10us	10
  * 	TIMEBASE_20us	20
  * 	TIMEBASE_50us	50
  * 	TIMEBASE_100us	100
  * 	TIMEBASE_500us	500
  * 	TIMEBASE_1ms	1000
  * 	TIMEBASE_5ms	5000
  * 	TIMEBASE_10ms	10000
  * 	TIMEBASE_20ms	20000
  * 	TIMEBASE_50ms	50000
  */
static void Hal_Timer_Config(void)
{
	Hal_Timer4_Config(TIMEBASE_50us, TIMER_PRESCALER_72);	// period = 50us
	Hal_Timer3_Config(0xFFFF, TIMER_PRESCALER_72);				// ARR set maxium to avoid counter overflow during input-capture
}

/**
  * @Brief	Config Timer4 parameters (TIM4 for Timer-Matrix)
  * @Param	None
  * @Retval	None
  */
static void Hal_Timer4_Config(uint16_t Arr, uint16_t Psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	
	TIM_DeInit(TIM4);
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = Arr - 1;	
	TIM_TimeBaseStructure.TIM_Prescaler = Psc - 1; 
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	
	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM4, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

/**
  * @Brief	Config Timer3 parameters (TIM3 for AL6630 TH_module data capture)
  * @Param	Arr: TIM3 auto reload value
  *			Psc: TIM3 prescaler value
  * @Retval	None
  */
static void Hal_Timer3_Config(uint16_t Arr, uint16_t Psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_ICInitTypeDef TIM_ICInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = Arr - 1;
	TIM_TimeBaseStructure.TIM_Prescaler = Psc - 1;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_2; 								// PA7-->TIM3_CH2
	TIM_ICInitStructure.TIM_ICFilter = 0x00;		 										// no filter
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; // TIM Input 2 connect to IC2
	TIM_ICInit(TIM3, &TIM_ICInitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	TIM_ITConfig(TIM3, TIM_IT_CC2, ENABLE);		// channel2 capture-compare enable
	
	TIM_Cmd(TIM3, DISABLE);		// DISABLE defalut
}

/**
  * @Brief	Handler for timer interrupt
  * @Param	None
  * @Retval	None
  */
static void Hal_Timer_ITHandler(void)
{
	unsigned char i;
	for(i=0; i<T_SUM; i++)
	{
		if((Stu_Timer[i].func) && (Stu_Timer[i].state == T_STATE_START))
		{
			Stu_Timer[i].CurrentCount++;
			
			if(Stu_Timer[i].CurrentCount >= Stu_Timer[i].Period)
			{
				Stu_Timer[i].state = T_STATE_STOP;
				Stu_Timer[i].CurrentCount = Stu_Timer[i].CurrentCount; // 0?
				Stu_Timer[i].func(); // handler call-back functions
			}
		}
	}
}

/*-------------Interrupt Functions Definition--------*/
/**
  * @Brief	TIM4_IRQHandler
  * @Param	None
  * @Retval	None
  */
void TIM4_IRQHandler(void)
{
	TIM_ClearFlag(TIM4, TIM_FLAG_Update);
	Hal_Timer_ITHandler();
}
