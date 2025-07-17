#ifndef __APP_H_
#define __APP_H_

#include "device.h"

/** Comment this macro to enable <Working Mode> of KeyEvent_Handler
  * Uncomment this macro to enable <Debug Mode> of KeyEvent_Handler	*/
//#define APP_KEYEVENT_DEBUG_MODE

/* Return to the desktop if no action */
#define TIMEOUT_COUNTER_RETURN_DESKTOP	2000
/* Auto sleep if no action */
#define TIMEOUT_COUNTER_SCREEN_SLEEP		3000

/* Menu-EnterPin Coordinates Define: */
#define COOR_MENU_ENTERPIN_TITLE_X			90
#define COOR_MENU_ENTERPIN_TITLE_Y			20

#define COOR_MENU_ENTERPIN_PROMPT_X   	40
#define COOR_MENU_ENTERPIN_PROMPT_Y   	100

/* Menu-Disarm Coordinates Define: */
#define COOR_MENU_DISARM_TITLE_X			60
#define COOR_MENU_DISARM_TITLE_Y			30

#define COOR_MENU_DISARM_PROMPT_X   	40
#define COOR_MENU_DISARM_PROMPT_Y   	100

/* Menu-MainMenu Coordinates Define: */
#define COOR_MENU_MAINMENU_TITLE_X			20
#define COOR_MENU_MAINMENU_TITLE_Y			0

#define COOR_MENU_MAINMENU_CURSOR_X			3

#define COOR_MENU_MAINMENU_OPTION_X			20

/* Menu_SensorPair Coordinates Define: */
#define COOR_MENU_SENSORPAIR_TITLE_X		0
#define COOR_MENU_SENSORPAIR_TITLE_Y		0

#define COOR_MENU_SENSORPAIR_PROMPT_X		60
#define COOR_MENU_SENSORPAIR_PROMPT_Y		120

#define COOR_MENU_SENSORPAIR_TYPE_X			60
#define COOR_MENU_SENSORPAIR_TYPE_Y			70

/* Menu_SensorPara Coordinates Define: */
#define COOR_MENU_SENSORPARA_TITLE_X		0
#define COOR_MENU_SENSORPARA_TITLE_Y		0

#define COOR_MENU_SENSORPARA_PROMPT_X		20
#define COOR_MENU_SENSORPARA_PROMPT_Y		120

#define COOR_MENU_SENSORPARA_CURSOR_X		3

#define COOR_MENU_SENSORPARA_OPTION_X		20

#define COOR_MENU_SENSORPARA_STATE_X		140

/* Menu_SensorPara_Option_Main Coordinates Define: */
#define COOR_MENU_SENSORPARA_OPTION_MAIN_TITLE_X					80
#define COOR_MENU_SENSORPARA_OPTION_MAIN_TITLE_Y					10

#define COOR_MENU_SENSORPARA_OPTION_MAIN_OPTION_X					20

/* Menu_SensorPara_Option_Review Coordinates Define: */
#define COOR_MENU_SENSORPARA_OPTION_REVIEW_TITLE_X				130
#define COOR_MENU_SENSORPARA_OPTION_REVIEW_TITLE_Y				10

#define COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_1_NAME_X	10
#define COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_1_NAME_Y	50
												 
#define COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_2_NAME_X	10
#define COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_2_NAME_Y	80
												 
#define COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_3_NAME_X	10
#define COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_3_NAME_Y	110
												 
#define COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_4_NAME_X	10
#define COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_4_NAME_Y	140
												 
#define COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_5_NAME_X	10
#define COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_5_NAME_Y	170

#define COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_1_DATA_X	180
#define COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_1_DATA_Y	50

#define COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_2_DATA_X	180
#define COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_2_DATA_Y	80

#define COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_3_DATA_X	180
#define COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_3_DATA_Y	110

#define COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_4_DATA_X	180
#define COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_4_DATA_Y	140

#define COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_5_DATA_X	180
#define COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_5_DATA_Y	170


/* Menu_SensorPara_Option_Edit Coordinates Define: */
#define COOR_MENU_SENSORPARA_OPTION_EDIT_TITLE_X					130
#define COOR_MENU_SENSORPARA_OPTION_EDIT_TITLE_Y					10

#define COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_1_NAME_X		10
#define COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_1_NAME_Y		50
												 
#define COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_2_NAME_X		10
#define COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_2_NAME_Y		80
												 
#define COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_3_NAME_X		10
#define COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_3_NAME_Y		110
										 
#define COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_4_NAME_X		10
#define COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_4_NAME_Y		140
										 
#define COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_5_NAME_X		10
#define COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_5_NAME_Y		170
									
#define COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_1_DATA_X		180
#define COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_1_DATA_Y		50
									
#define COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_2_DATA_X		180
#define COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_2_DATA_Y		80
									
#define COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_3_DATA_X		180
#define COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_3_DATA_Y		110
									
#define COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_4_DATA_X		180
#define COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_4_DATA_Y		140
									
#define COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_5_DATA_X		180
#define COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_5_DATA_Y		170

#define COOR_MENU_SENSORPARA_OPTION_EDIT_UPDATE_X					60
#define COOR_MENU_SENSORPARA_OPTION_EDIT_UPDATE_Y					100

/* Menu_SensorPara_Option_Delete Coordinates Define: */
#define COOR_MENU_SENSORPARA_OPTION_DELETE_TITLE_X				110
#define COOR_MENU_SENSORPARA_OPTION_DELETE_TITLE_Y				20

#define COOR_MENU_SENSORPARA_OPTION_DELETE_PROMPT_1_X			70
#define COOR_MENU_SENSORPARA_OPTION_DELETE_PROMPT_1_Y			80

#define COOR_MENU_SENSORPARA_OPTION_DELETE_SENSORNAME_X		170
#define COOR_MENU_SENSORPARA_OPTION_DELETE_SENSORNAME_Y		80

#define COOR_MENU_SENSORPARA_OPTION_DELETE_PROMPT_2_X			90
#define COOR_MENU_SENSORPARA_OPTION_DELETE_PROMPT_2_Y			110

#define COOR_MENU_SENSORPARA_OPTION_DELETE_YES_X					80
#define COOR_MENU_SENSORPARA_OPTION_DELETE_YES_Y					170

#define COOR_MENU_SENSORPARA_OPTION_DELETE_NO_X						200
#define COOR_MENU_SENSORPARA_OPTION_DELETE_NO_Y						170

#define COOR_MENU_SENSORPARA_OPTION_DELETE_UPDATE_X				60
#define COOR_MENU_SENSORPARA_OPTION_DELETE_UPDATE_Y				100

/* Menu_WiFiNetPair Coordinates Define: */
#define COOR_MENU_WIFI_NETPAIR_TITLE_X			0
#define COOR_MENU_WIFI_NETPAIR_TITLE_Y			0
	
#define COOR_MENU_WIFI_NETPAIR_PROMPT_X			50
#define COOR_MENU_WIFI_NETPAIR_PROMPT_Y			60
	
#define COOR_MENU_WIFI_NETPAIR_YES_X				60
#define COOR_MENU_WIFI_NETPAIR_YES_Y				150
	
#define COOR_MENU_WIFI_NETPAIR_NO_X					220
#define COOR_MENU_WIFI_NETPAIR_NO_Y					150

/* Menu_AdminPassword Coordinates Define: */
#define COOR_MENU_ADMIN_PASSWORD_TITLE_X				0
#define COOR_MENU_ADMIN_PASSWORD_TITLE_Y				0

#define COOR_MENU_ADMIN_PASSWORD_PROMPT_1_X			10
#define COOR_MENU_ADMIN_PASSWORD_PROMPT_1_Y			70

#define COOR_MENU_ADMIN_PASSWORD_PROMPT_2_X			10
#define COOR_MENU_ADMIN_PASSWORD_PROMPT_2_Y			110

#define COOR_MENU_ADMIN_PASSWORD_CURSOR_X_BASE	150

#define COOR_MENU_ADMIN_PASSWORD_PROMPT_3_X			50
#define COOR_MENU_ADMIN_PASSWORD_PROMPT_3_Y			160

/* Menu_PhoneNumber Coordinates Define: */
#define COOR_MENU_PHONENUMBER_TITLE_X						0
#define COOR_MENU_PHONENUMBER_TITLE_Y						0
	
/* Menu_TerminalPara Coordinates Define: */	
#define COOR_MENU_TERMINAL_PARA_TITLE_X								0
#define COOR_MENU_TERMINAL_PARA_TITLE_Y								0

#define COOR_MENU_TERMINAL_FIRMWAREVERSION_NAME_X			10
#define COOR_MENU_TERMINAL_FIRMWAREVERSION_NAME_Y			50

#define COOR_MENU_TERMINAL_FIRMWAREVERSION_DATA_X			205
#define COOR_MENU_TERMINAL_FIRMWAREVERSION_DATA_Y			50

#define COOR_MENU_TERMINAL_PROTOCOL_NAME_X						10
#define COOR_MENU_TERMINAL_PROTOCOL_NAME_Y						90

#define COOR_MENU_TERMINAL_PROTOCOL_DATA_X						205
#define COOR_MENU_TERMINAL_PROTOCOL_DATA_Y						90

#define COOR_MENU_TERMINAL_MCU_UID_NAME_X							10
#define COOR_MENU_TERMINAL_MCU_UID_NAME_Y							170

#define COOR_MENU_TERMINAL_MCU_UID_DATA_1_X						140
#define COOR_MENU_TERMINAL_MCU_UID_DATA_1_Y						170

#define COOR_MENU_TERMINAL_MCU_UID_DATA_2_X						140
#define COOR_MENU_TERMINAL_MCU_UID_DATA_2_Y						200

/* Menu_FactoryReset Coordinates Define: */	
#define COOR_MENU_FACTORY_RESET_TITLE_X					0
#define COOR_MENU_FACTORY_RESET_TITLE_Y					0

#define COOR_MENU_FACTORY_RESET_PROMPT_X				70
#define COOR_MENU_FACTORY_RESET_PROMPT_Y				60
				
#define COOR_MENU_FACTORY_RESET_YES_X						60
#define COOR_MENU_FACTORY_RESET_YES_Y						150
				
#define COOR_MENU_FACTORY_RESET_NO_X						220
#define COOR_MENU_FACTORY_RESET_NO_Y						150
	
#define COOR_MENU_FACTORY_RESET_RESETING_X			60
#define COOR_MENU_FACTORY_RESET_RESETING_Y			100

/* Menu_AlarmReport Coordinates Define: */	
#define COOR_MENU_ALARM_REPORT_TITLE_X					0
#define COOR_MENU_ALARM_REPORT_TITLE_Y					0

/* Terminal Work Mode(Desktop display) Coordinates Define: */	
#define COOR_TERMINAL_WORK_MODE_X								70
#define COOR_TERMINAL_WORK_MODE_Y								100

#define COOR_TERMINAL_PROMPT_1_X								60
#define COOR_TERMINAL_PROMPT_1_Y								160

#define COOR_TERMINAL_PROMPT_2_X								170
#define COOR_TERMINAL_PROMPT_2_Y								160

/* Menu-AlarmMenu Coordinates Define: */
#define COOR_MENU_ALARMMENU_PROMPT_1_X							70
#define COOR_MENU_ALARMMENU_PROMPT_1_Y							50

#define COOR_MENU_ALARMMENU_AlARMSOURCE_X_BASE			30
#define COOR_MENU_ALARMMENU_AlARMSOURCE_Y_BASE			120

/* Menu-FirmwareUpdate Coordinates Define: */
#define COOR_MENU_FIRMWARE_UPDATE_TITLE_1_X					35
#define COOR_MENU_FIRMWARE_UPDATE_TITLE_1_Y					20

#define COOR_MENU_FIRMWARE_UPDATE_PROMPT_1_X				30
#define COOR_MENU_FIRMWARE_UPDATE_PROMPT_1_Y				70

#define COOR_MENU_FIRMWARE_UPDATE_PROMPT_2_X				40
#define COOR_MENU_FIRMWARE_UPDATE_PROMPT_2_Y				110

#define COOR_MENU_FIRMWARE_UPDATE_YES_X							80
#define COOR_MENU_FIRMWARE_UPDATE_YES_Y							150

#define COOR_MENU_FIRMWARE_UPDATE_NO_X							200
#define COOR_MENU_FIRMWARE_UPDATE_NO_Y							150

#define COOR_MENU_FIRMWARE_UPDATE_PROMPT_3_X				60
#define COOR_MENU_FIRMWARE_UPDATE_PROMPT_3_Y				160

#define COOR_MENU_FIRMWARE_UPDATE_PERCENTAGE_X			80
#define COOR_MENU_FIRMWARE_UPDATE_PERCENTAGE_Y			60

#define COOR_MENU_FIRMWARE_UPDATE_PROMPT_4_X				60
#define COOR_MENU_FIRMWARE_UPDATE_PROMPT_4_Y				190



/* GeneralMenu Define */
typedef enum
{
	GENERAL_MENU_DESKTOP,
	GENERAL_MENU_PHONECALL,
	GENERAL_MENU_ENTER_PIN,
	GENERAL_MENU_ENTER_DISARM,
	GENERAL_MENU_FIRMWARE_UPDATE,
	
	GENERAL_MENU_SUM,
}en_GeneralMenu_t;

/* ListMenu Define */
typedef enum
{
	LIST_MENU_MAIN,
	LIST_MENU_SENSOR_PAIR,
	LIST_MENU_SENSOR_PARA,
	LIST_MENU_WIFI_NETPAIR,
	LIST_MENU_ADMIN_PASSWORD,
	LIST_MENU_PHONENUMBER,
	LIST_MENU_TERMINAL_PARA,
	LIST_MENU_FACTORY_RESET,
	LIST_MENU_ALARM_REPORT,
	
	LIST_MENU_SUM,
}en_ListMenu_t;

/* SensorParaMenu sensor option Define */
typedef enum
{
	SENSORPARA_MENU_MAIN,
	SENSORPARA_MENU_REVIEW,
	SENSORPARA_MENU_EDIT,
	SENSORPARA_MENU_DELETE,
	
	SENSORPARA_MENU_SUM,
}en_SensorParaMenu_t;

/* SensorPair Menu parameters define */
typedef enum
{
	SENSOR_WORKMODE_APPLY_NET,
	SENSOR_WORKMODE_APPLY_NET_SUCCESS,
	SENSOR_WORKMODE_NORMAL,
	
}en_SensorPairMenu_SensorWorkMode_t;

typedef struct
{
	en_SensorPairMenu_SensorWorkMode_t 	WorkMode;
	en_Sensor_Type_t 					SensorType;
	en_Sensor_PairState_t 				PairState;
	
}stu_SensorPairMenu_SensorPara_t;

/* SensorUpdate Menu CursorPos define */
typedef enum
{
	UPDATE_MENU_CURSOR_POS_YES,
	UPDATE_MENU_CURSOR_POS_NO,
	
	UPDATE_MENU_CURSOR_POS_SUM,
}en_SensorUpdateMenu_CursorPos_t;

/* WiFiNetPair Menu CursorPos define */
typedef enum
{
	WIFI_MENU_CURSOR_POS_YES,
	WIFI_MENU_CURSOR_POS_NO,
	
	WIFI_MENU_CURSOR_POS_SUM,
}en_WiFiNetPairMenu_CursorPos_t;


/* Update AdminPassword State define */
typedef enum
{
	UPDATE_PASSWORD_STATE_IN_PROCESS,
	UPDATE_PASSWORD_STATE_SUCCESS,
	UPDATE_PASSWORD_STATE_FAIL,
	
	UPDATE_PASSWORD_STATE_SUM,
}en_UpdatePasswordState_t;

/* FactoryReset Menu CursorPos define */
typedef enum
{
	FACTORY_RESET_MENU_CURSOR_POS_YES,
	FACTORY_RESET_MENU_CURSOR_POS_NO,

	FACTORY_RESET_MENU_CURSOR_POS_SUM,
}en_FactoryResetMenu_CursorPos_t;

/* Menu-Depth define */
typedef enum
{
	MENU_DESKTOP,
	MENU_SUB_ALARM,
	MENU_SUB_FIRMWARE_UPDATE,
	MENU_SUB_WIFI_NETPAIR,
	MENU_SUB_MENU,
	MENU_SUB_1,
	MENU_SUB_2,
	
}en_Menu_Depth_t;

/* Screen-Command define */
typedef enum
{
	SCREEN_CMD_NULL,			// no action
	SCREEN_CMD_RESET,			// reset screen display
	SCREEN_CMD_RECOVER,		// recover original content display 
	SCREEN_CMD_UPDATE,		// refresh screen display
	
}en_Screen_CMD_t;

/* Menu Structure define */
typedef struct Menu
{
	uint8_t			MenuID;							// MenuID
	void 			(*Action)(void);			// operation function of current Mode
	const uint8_t 	*pMenuTitle;		// point to the MenuTitle string
	en_Menu_Depth_t	MenuDepth;			// store current Menu depth
	en_Screen_CMD_t	ScreenCMD;			// screen command 
	
	uint8_t			ReservedBuff;		// reserved as buffer
	
	uint8_t			KeyIndex;				// store active key's KeyIndex, 0xFF indicates no key action
	uint8_t			KeyEvent;				// store active key's KeyEvent
	
	struct Menu		*pPrevious;		// point to previous menu-option
	struct Menu		*pNext;				// point to next menu-option
	struct Menu		*pParent;			// point to parent menu
	struct Menu		*pChild;			// point to child menu
	
}stu_Menu_t;

/* Terminal work mode define */
typedef enum
{
	TERMINAL_WORK_MODE_AWAYARM,
	TERMINAL_WORK_MODE_HOMEARM,
	TERMINAL_WORK_MODE_ALARMING,
	TERMINAL_WORK_MODE_DISARM,
	
	Terminal_WORK_MODE_SUM,
}en_Terminal_WorkMode_t;

/* Terminal operation CMD source define */
typedef enum
{
	TERMINAL_CMD_SOURCE_KEY,
	TERMINAL_CMD_SOURCE_REMOTE,
	TERMINAL_CMD_SOURCE_SERVER,
	
	TERMINAL_CMD_SOURCE_SUM,
}en_Terminal_CMDSource_t;

/* Terminal operation structure define */
typedef struct 
{
	en_Terminal_WorkMode_t 	WorkMode;			// current terminal work mode
	en_Screen_CMD_t			ScreenCMD;				// screen command
	uint8_t 				KeyIndex;							// pressed key index, 0xFF indicate no key pressed
	void					(*Action)(void);				// operation function of current mode
	
}stu_TerminalMode_t;

extern stu_TerminalMode_t *pTerminalMode;

/* Terminal Alarming workmode state define */
typedef enum
{
	ALARM_WORKMODE_STATE_ALARM,
	ALARM_WORKMODE_STATE_DISARM,
	
	ALARM_WORKMODE_STATE_SUM,
}en_AlarmWorkMode_state_t;


void App_Init(void);
void App_Pro(void);
void App_Terminal_ModeChange(uint8_t Zone, en_Terminal_WorkMode_t WorkMode, en_Terminal_CMDSource_t CMDSource);

void App_SensorOffline_Init(void);
void App_SensorOffline_Handler(void);


#endif
