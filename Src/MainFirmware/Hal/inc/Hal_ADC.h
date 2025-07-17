#ifndef __HAL_ADC_H_
#define __HAL_ADC_H_

#define ADC_CAPTURE_COUNTS   	10 // Capture counts each round
#define ADC_CAPTURE_INTERVAL  	50 // Capture interval(cycles) between two round

/* Batter voltage value macro define: */
/* For convience, multiply VoltageValue with MULTI_FACTOR(100) */
#define BATTERY_VOLT_LEVEL0		360	
#define BATTERY_VOLT_LEVEL1		370	
#define BATTERY_VOLT_LEVEL2		380
#define BATTERY_VOLT_LEVEL3		390
#define BATTERY_VOLT_LEVEL4		400	

#define REFERENCE_VOLTAGE		2.5	// Reference voltage and Voltage divider factor depends on Circuits design
#define VOLTAGE_DIVIDER_FACTOR	2	

#define MULTI_FACTOR			100


/* VoltageValue capture Steps: */
typedef enum
{
	STEP_IDLE,   			
	STEP_CAPTURE_START,  	
	STEP_CAPTURE_FINISH,  	
	STEP_DATA_PROCESS,      	

}en_VoltageCapture_Step_t;

/* VoltageLevel: */
typedef enum
{
	LEVEL_LOW,
	LEVEL_VOLT_1,
	LEVEL_VOLT_2,
	LEVEL_VOLT_3,
	LEVEL_VOLT_4,
	LEVEL_FULL,
	LEVEL_NODATA,
	LEVEL_IDLE,
}en_VoltageLevel_t;

/* Battery charging status: */
typedef enum
{
	STA_BAT_CHARGING,
	STA_BAT_FULL,
}en_BatteryChargeState_t;

/* */
typedef struct
{
	en_VoltageCapture_Step_t 	Step;
	en_BatteryChargeState_t 	BatChargeState;
	en_VoltageLevel_t 			BatVoltLevel;
	
	unsigned char ADC_CaptureCounter;
	
	unsigned int Sum_BatVoltCaptured;
	unsigned int Sum_ReferVoltCaptured;
	
	unsigned short ADC_ReferenceValue;	// ADC value of reference
	unsigned short ADC_BatteryValue;	// ADC value of batteryVoltage
	
}stu_BatVoltLevelDetect_t;

void Hal_ADC_Init(void);
void Hal_ADC_Pro(void);
en_VoltageLevel_t Hal_ADC_GetBatteryVoltageLevel(void);

#endif
