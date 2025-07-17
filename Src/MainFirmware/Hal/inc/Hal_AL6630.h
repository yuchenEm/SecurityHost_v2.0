#ifndef __HAL_AL6630_H_
#define __HAL_AL6630_H_

/* TH capture interval time */
#define TIME_INTERVAL_TH_CAPTURE  300

/* AL6630 TH_Module SDA Pin control function */
#define AL6630_SDA_SetHigh()	GPIO_SetBits(AL6630_SDA_PORT, AL6630_SDA_PIN)
#define AL6630_SDA_SetLow()		GPIO_ResetBits(AL6630_SDA_PORT, AL6630_SDA_PIN)

typedef enum
{
	TH_STEP_IDLE,
	TH_STEP_START,
	TH_STEP_WAIT_ACK,
	TH_STEP_DATA_CAPTURE,	
	TH_STEP_DATA_DECODE,
	TH_STEP_OVER,	
}en_TH_Step_t;

typedef struct
{
	unsigned short TempData;		// Temperature value
	unsigned short HumData;			// Humidity value
	unsigned char Step;   			// protcess step
	
	unsigned char Capfalg;			// Data capture flag
	unsigned char Len;				// Data length
	unsigned int CapCount;			// count of data capture
	
	unsigned char TemHumBuffer[5];	// buffer of 5 bytes data captured
}stu_TH_t;

void Hal_AL6630_Init(void);
void Hal_AL6630_Pro(void);
uint16_t Hal_AL6630_GetTemperatureValue(void);
uint16_t Hal_AL6630_GetHumidityValue(void);

#endif
