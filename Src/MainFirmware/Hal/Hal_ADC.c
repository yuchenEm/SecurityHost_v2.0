/****************************************************
  * @Name	Hal_ADC.c
  * @Brief	
  ***************************************************/
  
/*-------------Header Files Include-----------------*/
#include "stm32f10x.h"
#include "hal_adc.h"
#include "hal_gpio.h"

/*-------------Internal Functions Declaration------*/
static void Hal_ADC_Config(void);

static uint16_t 		 Hal_ADC_GetADC1Value(uint8_t Channel);
static en_VoltageLevel_t Hal_ADC_CaptureBatteryLevel(void);

/*-------------Module Variables Declaration--------*/
stu_BatVoltLevelDetect_t sBatVoltLevelDetect;

/*---Module Call-Back function pointer Definition---*/


/*-------------Module Functions Definition---------*/
/**
  * @Brief	Initialize ADC module(capture Battery-VolatgeLevel)
  * @Param	None
  * @Retval	None
  */
void Hal_ADC_Init(void)
{
	Hal_ADC_Config();
	
	/* BattVoltLevelDetect parameters initial */
	sBatVoltLevelDetect.Step = STEP_IDLE;
	sBatVoltLevelDetect.ADC_CaptureCounter = 0;
	sBatVoltLevelDetect.Sum_BatVoltCaptured = 0;
	sBatVoltLevelDetect.Sum_ReferVoltCaptured = 0;
	sBatVoltLevelDetect.BatVoltLevel = LEVEL_FULL;
	sBatVoltLevelDetect.BatChargeState = STA_BAT_CHARGING;
}

/**
  * @Brief	Polling in sequence for ADC module
  * @Param	None
  * @Retval	None
  */
void Hal_ADC_Pro(void)
{
	static uint16_t ADC_IntervalCounter = 0;
	static uint8_t counter = 0;
	
	en_VoltageLevel_t BatVoltLevel_Updated;
	
	// Update the BatVoltLevel with function Hal_ADC_CaptureBatteryLevel:
	BatVoltLevel_Updated = Hal_ADC_CaptureBatteryLevel();
	
	if(BatVoltLevel_Updated == LEVEL_IDLE)
	{
		ADC_IntervalCounter++;
		
		if(ADC_IntervalCounter > ADC_CAPTURE_INTERVAL)
		{
			ADC_IntervalCounter = 0;
			sBatVoltLevelDetect.Step = STEP_CAPTURE_START;
		}
	}
	
	else if(BatVoltLevel_Updated == LEVEL_NODATA)
	{
		
	}
	
	else
	{
		if(BatVoltLevel_Updated != sBatVoltLevelDetect.BatVoltLevel)
		{
			counter++;
			
			if(counter > 3)
			{
				counter = 0;
				
				// Update the atVoltLevelDetect structure data
				sBatVoltLevelDetect.BatVoltLevel = BatVoltLevel_Updated;
			}
		}
		else
		{
			counter = 0;
		}
	}
}

/**
  * @Brief	Get current Battery VoltageLevel
  * @Param	None
  * @Retval	Battery VoltageLevel
  */
en_VoltageLevel_t Hal_ADC_GetBatteryVoltageLevel(void)
{
	return sBatVoltLevelDetect.BatVoltLevel;
}

/*-------------Internal Functions Definition--------*/
/**
  * @Brief	Config ADC module
  * @Param	None
  * @Retval	None
  */
static void Hal_ADC_Config(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);	// ADC_CLK = 72MHz / 8 = 9MHz
	
	/* ADC1 */
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; // conversion is performed in single mode
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // software trigger
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_NbrOfChannel = 2;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;		// one channel mode
	ADC_Init(ADC1, &ADC_InitStructure);
	
	ADC_Cmd(ADC1, ENABLE);
	
	// Calibration ADC1
	ADC_ResetCalibration(ADC1);						// Reset calibration
	while(ADC_GetCalibrationStatus(ADC1) == SET);	// wait for calibration finish
	ADC_StartCalibration(ADC1);						// Start calibration
	while(ADC_GetCalibrationStatus(ADC1) == SET);	// wait for calibration finish
	
}

/**
  * @Brief	Get ADC1 value of specified channel
  * @Param	Channel: target channel
  *		@arg BATTERY_LEVEL_ADC_CHANNEL
  *		@arg AC_CHARGER_LEVEL_ADC_CHANNEL
  *		@arg REFERENCE_2_5V_ADC_CHANNEL
  *
  * @Retval	Captured ADC value
  */
static uint16_t Hal_ADC_GetADC1Value(uint8_t Channel)
{
	ADC_RegularChannelConfig(ADC1, Channel, 1, ADC_SampleTime_239Cycles5);
	
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);	// Start ADC1 conversion
	
	while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET); // Wait for the conversion finish
	
	return ADC_GetConversionValue(ADC1);
}

/**
  * @Brief	Capture Battery-VolatgeLevel with ADC1
  * @Param	None
  * @Retval	Battery VoltageLevel
  */
static en_VoltageLevel_t Hal_ADC_CaptureBatteryLevel(void)
{
	uint32_t BatVoltage;
	
	switch((uint8_t)sBatVoltLevelDetect.Step)
	{
		case STEP_IDLE:
		{
			sBatVoltLevelDetect.ADC_CaptureCounter = 0;
			sBatVoltLevelDetect.Sum_BatVoltCaptured = 0;
			sBatVoltLevelDetect.Sum_ReferVoltCaptured = 0;
			
			return LEVEL_IDLE;
		}
		
		case STEP_CAPTURE_START:
		{
			sBatVoltLevelDetect.Sum_BatVoltCaptured += Hal_ADC_GetADC1Value(BATTERY_LEVEL_ADC_CHANNEL);
			sBatVoltLevelDetect.Sum_ReferVoltCaptured += Hal_ADC_GetADC1Value(REFERENCE_2_5V_ADC_CHANNEL);
			
			sBatVoltLevelDetect.ADC_CaptureCounter++;
			
			if(sBatVoltLevelDetect.ADC_CaptureCounter >= ADC_CAPTURE_COUNTS)
			{
				sBatVoltLevelDetect.ADC_CaptureCounter = 0;
				
				sBatVoltLevelDetect.Step = STEP_CAPTURE_FINISH;
			}
		}
		break;
		
		case STEP_CAPTURE_FINISH:
		{
			// Use the average VoltageValue as the result:
			sBatVoltLevelDetect.ADC_ReferenceValue = sBatVoltLevelDetect.Sum_ReferVoltCaptured / ADC_CAPTURE_COUNTS;
			sBatVoltLevelDetect.ADC_BatteryValue = sBatVoltLevelDetect.Sum_BatVoltCaptured / ADC_CAPTURE_COUNTS;
			
			sBatVoltLevelDetect.Step = STEP_DATA_PROCESS;
		}
		break;
		
		case STEP_DATA_PROCESS:
		{
			BatVoltage = VOLTAGE_DIVIDER_FACTOR * ((REFERENCE_VOLTAGE * MULTI_FACTOR * sBatVoltLevelDetect.ADC_BatteryValue) / sBatVoltLevelDetect.ADC_ReferenceValue);
		
			if(BatVoltage > BATTERY_VOLT_LEVEL4)
			{
				return LEVEL_FULL;
			}
			
			else if(BatVoltage > BATTERY_VOLT_LEVEL3)
			{
				return LEVEL_VOLT_4;
			}
			
			else if(BatVoltage > BATTERY_VOLT_LEVEL2)
			{
				return LEVEL_VOLT_3;
			}
			
			else if(BatVoltage > BATTERY_VOLT_LEVEL1)
			{
				return LEVEL_VOLT_2;
			}
			
			else if(BatVoltage > BATTERY_VOLT_LEVEL0)
			{
				return LEVEL_VOLT_1;
			}
			
			else
			{
				return LEVEL_LOW;
			}
		}
	}
	
	return LEVEL_NODATA;
}


/*-------------Interrupt Functions Definition--------*/


