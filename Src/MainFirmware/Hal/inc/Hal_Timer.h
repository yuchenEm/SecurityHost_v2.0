#ifndef __HAL_TIMER_H_
#define __HAL_TIMER_H_

/* Timer Prescaler define */
#define TIMER_PRESCALER_72	72

/* TimeBase define */
#define	TIMEBASE_1us	1
#define TIMEBASE_5us	5
#define TIMEBASE_10us	10
#define TIMEBASE_20us	20
#define TIMEBASE_50us	50
#define TIMEBASE_100us	100
#define TIMEBASE_500us	500
#define TIMEBASE_1ms	1000
#define TIMEBASE_5ms	5000
#define TIMEBASE_10ms	10000
#define TIMEBASE_20ms	20000
#define TIMEBASE_50ms	50000

/* Timer ID Setting */
typedef enum
{
	T_LED,	
	T_SENSOR_OFFLINE,
	
	T_SUM,
}en_Timer_ID_t;

/* Timer Function Return Value */
typedef enum
{
	T_SUCCESS,
	T_FAIL,	
}en_Timer_Result_t;

/* Timer Status */
typedef enum
{
	T_STATE_INVALID, //0
	T_STATE_STOP,	
	T_STATE_START,	
}en_Timer_State_t;

/* Timer Structure Define */
typedef struct
{
	en_Timer_State_t state; 		// INVALID: failed; STOP: timer idle; START: timer run
	unsigned char CompleteFlag; 	// 0: not complete; 1: complete
	unsigned short CurrentCount; 
	unsigned short Period; 
	void (*func)(void); 
}stu_Timer_t;

void Hal_Timer_Init(void);
void Hal_Timer_Creat(en_Timer_ID_t ID, void (*proc)(void), unsigned short Period, en_Timer_State_t State);
en_Timer_Result_t Hal_Timer_Reset(en_Timer_ID_t ID, en_Timer_State_t State);
en_Timer_Result_t Hal_Timer_Delete(en_Timer_ID_t ID);
en_Timer_Result_t Hal_Timer_StateControl(en_Timer_ID_t ID, en_Timer_State_t State);
en_Timer_State_t Hal_Timer_GetState(en_Timer_ID_t ID);


#endif
