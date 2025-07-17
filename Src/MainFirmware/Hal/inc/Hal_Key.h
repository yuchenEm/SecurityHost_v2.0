#ifndef __HAL_KEY_H_
#define __HAL_KEY_H_

/* Key operation time-delay define (timebase: 10ms) */
#define KEY_SCAN_TIME			3	// 30ms
#define KEY_LONGPRESS_TIME		200	// 2s
#define KEY_CONTINUEPRESS_TIME	150 // 1.5s

/* 16key BCD encoder */
/*----------------------------------------------------------------
KEY			INT		DB4		DB3		DB2		DB1		DB0
Null		1		0		0		0		0		0		NUll
Touch0		0		0		0		0		0		1		KEY16
Touch1		0		0		0		0		1		0		KEY15
Touch2		0		0		0		0		1		1		KEY14
Touch3		0		0		0		1		0		0		KEY13
Touch4		0		0		0		1		0		1		KEY12
Touch5		0		0		0		1		1		0		KEY11
Touch6		0		0		0		1		1		1		KEY10
Touch7		0		0		1		0		0		0		KEY9
Touch8		0		0		1		0		0		1		KEY8
Touch9		0		0		1		0		1		0		KEY7
Touch10		0		0		1		0		1		1		KEY6
Touch11		0		0		1		1		0		0		KEY5
Touch12		0		0		1		1		0		1		KEY4
Touch13		0		0		1		1		1		0		KEY3
Touch14		0		0		1		1		1		1		KEy2
Touch15		0		1		0		0		0		0		KEY1
------------------------------------------------------------------*/

/* Key pad number table */
/*----------------------------------------------------------------
KeyNo			Function code
Key1	--> 	1
Key2	-->		4/Left
Key3	-->		7
Key4	--> 	SOS/Delete
Key5	-->		2/Up
Key6	-->		5
Key7	-->		8/Down
Key8	-->		0
Key9	-->		3
Key10	-->		6/Right
Key11	-->		9
Key12	-->		Call/Cancel
Key13	-->		AwayArm
Key14	-->		HomeArm
Key15	-->		Disarm
Key16	-->		Menu
------------------------------------------------------------------*/


/* DataBits pin */
enum
{
	DB_0,
	DB_1,
	DB_2,
	DB_3,
	DB_4,
	
	DB_SUM,
};

/* Key pad correspondig Keyname */
typedef enum
{	
	KEY_MENU_CONFIRM,			
	KEY_DISARM,			
	KEY_HOMEARM,		
	KEY_AWAYARM,		
	
	KEY_CALL_CANCEL,	
	KEY_9,				
	KEY_6_RIGHT,		
	KEY_3,				
	
	KEY_0,				
	KEY_8_DOWN,			
	KEY_5,				
    KEY_2_UP,			
	
	KEY_SOS_DELET,		
	KEY_7,				
	KEY_4_LEFT,			
	KEY_1,				
	
	KEY_SUM
}en_KeyType_t;

/* Key scan steps */
typedef enum
{
	KEYSCAN_STEP_IDLE,
	KEYSCAN_STEP_CLICK,
	KEYSCAN_STEP_LONG_PRESS,
	KEYSCAN_STEP_CONTINUE_PRESS,
	KEYSCAN_STEP_RELEASE
	
}en_KeyScanStep_t;

/* Key Event */
typedef enum
{
	KEY_IDLE,       	 	
	KEY_CLICK,          	
	KEY_CLICK_RELEASE,          
	KEY_LONG_PRESS,			   
	KEY_LONG_PRESS_CONTINUE,	
	KEY_LONG_PRESS_RELEASE
	
}en_KeyEvent_t;

/* */
typedef struct
{
	unsigned char Key;
	unsigned char State;
	
}stu_Key_t;

/* Call-Back function declaration */
typedef void (*KeyEvent_CallBack_t)(en_KeyType_t KeyNum, en_KeyEvent_t KeyEvent);

void Hal_Key_Init(void);
void Hal_Key_Pro(void);
void Hal_Key_KeyScanCBFRegister(KeyEvent_CallBack_t pCBF);


#endif
