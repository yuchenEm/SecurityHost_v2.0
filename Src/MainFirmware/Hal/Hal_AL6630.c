/****************************************************
  * @Name	Hal_AL6630.c
  * @Brief	AL6630 - Temperature and Humidity module
  ***************************************************/
  
/*-------------Header Files Include-----------------*/
#include "stm32f10x.h" 
#include "hal_al6630.h"
#include "hal_gpio.h"
#include "string.h"
#include "hal_timer.h"

/*-------------Internal Functions Declaration------*/
static void Hal_AL6630_CaptureReset(void);

/*-------------Module Variables Declaration--------*/
stu_TH_t Stu_TempHum;

FlagStatus Timer_Counter_1ms;
FlagStatus Timer_TriggerFlag_1ms;

/*-------------Module Functions Definition---------*/
/**
  * @Brief	Initialize TH structure, prepare for capture data
  * @Param	None
  * @Retval	None
  */
void Hal_AL6630_Init(void)
{
	Stu_TempHum.TempData = 0;
	Stu_TempHum.HumData = 0;
	Stu_TempHum.Step = TH_STEP_IDLE;
	
	Timer_Counter_1ms = RESET;
	Timer_TriggerFlag_1ms = RESET;
}

/**
  * @Brief	Polling in sequence for AL6630 module (get temperature and humidity)
  * @Param	None
  * @Retval	None
*	@Note	Protocal: Start signal, ACK, 5 byte data
  *		DataFrame: 0x00 	0x00 	0x00 	0x00 	0x00
  *				   HumHigh	HumLow	TemHigh	TemLow	Sumcheck
  */
void Hal_AL6630_Pro(void)
{
	uint8_t i;
	uint8_t SumCheck = 0;
	
	static uint32_t CaptureInterval_Timer = 0;
	
	CaptureInterval_Timer++;
	
	// need at least 2s after boot to let module get ready
	if(CaptureInterval_Timer > TIME_INTERVAL_TH_CAPTURE) // 10ms * 300 cycles = 3s 
	{
		CaptureInterval_Timer = 0;
		Stu_TempHum.Step = TH_STEP_START;
	}
	
	switch(Stu_TempHum.Step)
	{
		case TH_STEP_IDLE:
		{
			
		}
		break;
		
		case TH_STEP_START:		
		{
			Hal_AL6630_CaptureReset();
			Stu_TempHum.Step = TH_STEP_WAIT_ACK;
			
			Hal_GPIO_AL6630PinModeSet(OUTPUT);	// SDA set low after mdoe change
		}
		break;
		
		case TH_STEP_WAIT_ACK:	
		{
			AL6630_SDA_SetHigh();				// release bus(at least 10us)
			
			Stu_TempHum.Step = TH_STEP_DATA_CAPTURE;
			TIM_Cmd(TIM3, ENABLE);				// start timer
			TIM_SetCounter(TIM3, 0);			// reset counter, start counting coming pulse width
			
			Hal_GPIO_AL6630PinModeSet(INPUT);	// SDA set low after mode change
		}
		break;
		
		case TH_STEP_DATA_CAPTURE:
		{
			if(Stu_TempHum.Capfalg == 1)	// 5 byte data captured
			{
				Stu_TempHum.Capfalg = 0;
				Stu_TempHum.Step = TH_STEP_IDLE;
				
				for(i=0; i<4; i++)
				{
					SumCheck += Stu_TempHum.TemHumBuffer[i];
				}
				
				if(SumCheck == Stu_TempHum.TemHumBuffer[4])	// Sumcheck successful
				{
					Stu_TempHum.HumData = Stu_TempHum.TemHumBuffer[0] * 256 + Stu_TempHum.TemHumBuffer[1];
					Stu_TempHum.TempData = Stu_TempHum.TemHumBuffer[2] * 256 + Stu_TempHum.TemHumBuffer[3];
				}	
			}
		}
		break;
		
		case TH_STEP_DATA_DECODE:
		{
		
		}
		break;
		
		case TH_STEP_OVER:
		{
		
		}
		break;
	}
}

/**
  * @Brief	Get and return temperature data
  * @Param	None
  * @Retval	Temperature data
  */
uint16_t Hal_AL6630_GetTemperatureValue(void)
{
	return Stu_TempHum.TempData;
}

/**
  * @Brief	Get and return humidity data
  * @Param	None
  * @Retval	Humidity data
  */
uint16_t Hal_AL6630_GetHumidityValue(void)
{
	return Stu_TempHum.HumData;
}

/*-------------Internal Functions Definition--------*/
/**
  * @Brief	Reset TH structure capture parameters
  * @Param	None
  * @Retval	None
  */
static void Hal_AL6630_CaptureReset(void)
{
	Stu_TempHum.Capfalg = 0;
	Stu_TempHum.Len = 0;
	Stu_TempHum.CapCount = 0;
	memset(Stu_TempHum.TemHumBuffer, 0, 5);
}

/*-------------Interrupt Functions Definition--------*/
/**
  * @Brief	TIM3_IRQHandler
  * @Param	None
  * @Retval	None
  */
void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)	
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	}
	
	if(TIM_GetITStatus(TIM3, TIM_IT_CC2) != RESET)		// input capture interrupt (falling eadge)
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_CC2);
		
		Stu_TempHum.CapCount = TIM_GetCapture2(TIM3);
		
		if((Stu_TempHum.CapCount > 140) && (Stu_TempHum.CapCount < 180))	// ACK -> 160us
		{
			Stu_TempHum.Len = 0;
		}
		else
		{
			if((Stu_TempHum.CapCount > 60) && (Stu_TempHum.CapCount < 90))	// '0' -> 76us
			{
			
			}
			else if((Stu_TempHum.CapCount > 100) && (Stu_TempHum.CapCount < 135))	// '1' -> 120us
			{
				Stu_TempHum.TemHumBuffer[Stu_TempHum.Len / 8] |= (0x80 >> (Stu_TempHum.Len % 8));
			}
			Stu_TempHum.Len++;
		}
		
		if(Stu_TempHum.Len == 40)	// data capture finish
		{
			TIM_Cmd(TIM3, DISABLE);
			
			Stu_TempHum.Capfalg = 1;
		}
		
		TIM_SetCounter(TIM3,0);		// after capture each bit pulse width, reset counter
	}	
}

