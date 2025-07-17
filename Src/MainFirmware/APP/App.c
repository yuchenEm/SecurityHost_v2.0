/****************************************************
  * @Name	App.c
  * @Brief	
  
	DataStructure of Menu Design:
	** Circular Doubly Linked List **

	Design Stage:
		Enter(Return) Menu --> check ScreenCMD(Reset/Recover)
		Menu KeyScan
		Refresh Menu Display

	Menu Layer:
	1.General Menu
		1.1.Desktop
			1.1.1.System Disarm(Enter Pin)
			1.1.2.Enter MainMenu(Enter Pin)
			1.1.3.Phone Call
			1.1.4.Firmware Update
	2.List Menu
		2.1.MainMenu
			2.1.1.Sensor Pair
			2.1.2.Sensor Parameter
				> Paired Sensors List
					> View
					> Edit
					> Delete
			2.1.3.WiFi NetPair
			2.1.4.Admin Password
			2.1.5.PhoneNumber
			2.1.6.Host Parameter
			2.1.7.Factory Reset
			2.1.8.Alarm Report

  ***************************************************/
  
/*-------------Header Files Include-----------------*/
#include "stm32f10x.h"        
#include "app.h"
#include "hal_key.h"
#include "hal_tftlcd.h"
#include "hal_timer.h"
#include "mid_tftlcd.h"
#include "mid_lora.h"
#include "mid_wifi.h"
#include "mid_eeprom.h"
#include "mid_firmware.h"
#include "tftlcd_icon.h"
#include "device.h"
#include "stringprocess.h"
#include "os_system.h"
#include "mqtt_protocol.h"

/*-------------Internal Functions Declaration------*/
static void App_Menu_Init(void);

static void 	App_GeneralMenu_DesktopHandler(void);
static void 	App_GeneralMenu_PhoneCallHandler(void);
static void 	App_GeneralMenu_EnterPinHandler(void);
static void 	App_GeneralMenu_DisarmHandler(void);
static void 	App_GeneralMenu_FirmwareUpdateHandler(void);

static void 	App_ListMenu_MainMenuHandler(void);
static void	App_ListMenu_SensorPairHandler(void);
static void 	App_ListMenu_SensorParaHandler(void);
static void 	App_ListMenu_WiFiNetPairHandler(void);
static void 	App_ListMenu_AdminPasswordHandler(void);
static void 	App_ListMenu_PhoneNumberHandler(void);
static void 	App_ListMenu_TerminalParaHandler(void);
static void 	App_ListMenu_FactoryResetHandler(void);
static void 	App_ListMenu_AlarmReportHandler(void);

static void 	App_SensorParaMenu_MainHandler(void);
static void 	App_SensorParaMenu_ReviewHandler(void);
static void 	App_SensorParaMenu_EditHandler(void);
static void 	App_SensorParaMenu_DeleteHandler(void);

static void 	App_AlarmMenu_AlarmHandler(void);
static void 	App_AlarmMenu_DisarmHandler(void);

static void 	App_KeyEvent_Handler(en_KeyType_t KeyIndex, en_KeyEvent_t KeyEvent);

static stu_Lora_ApplyNet_SensorPara_t 	App_Lora_ApplyNetReq_Handler(en_Lora_FunctionCode_t FunctionCode, stu_Lora_Sensor_DataFrame_t stu_DataFrame);
static stu_Sensor_PairPara_t 						App_Lora_ApplyNet_AddSensor(stu_Sensor_t *pSensorPara);
static unsigned char 										App_Lora_FunctionCMD_Handler(en_Lora_FunctionCode_t FunctionCode, stu_Lora_Sensor_DataFrame_t stu_DataFrame);
static void 														App_Lora_RxDataHandler(uint8_t *pData);


static void 	App_ScreenControl(uint8_t CMD);
static void 	App_UpdateSensorPara(uint8_t SensorIndex, stu_Sensor_t *pSensorPara);

static void 	App_TerminalMode_AwayArmHandler(void);
static void 	App_TerminalMode_HomeArmHandler(void);
static void 	App_TerminalMode_AlarmingHandler(void);
static void 	App_TerminalMode_DisarmHandler(void);
	

/*-------------Module Variables Declaration--------*/
uint16_t TimeoutCounter_ReturnDesktop;
uint16_t TimeoutCounter_ScreenSleep;
uint8_t ScreenState;	// 0->screen sleep; 1->screen on

/* General-Menu structure collections: */
stu_Menu_t GeneralMenu[GENERAL_MENU_SUM] = {
	{GENERAL_MENU_DESKTOP, 			App_GeneralMenu_DesktopHandler, 		"Desktop", 			MENU_DESKTOP, 	SCREEN_CMD_RESET, 0, 0xFF, 0, 0, 0, 0, 0},
	{GENERAL_MENU_PHONECALL, 		App_GeneralMenu_PhoneCallHandler, 		"PhoneCall", 		MENU_SUB_ALARM, SCREEN_CMD_RESET, 0, 0xFF, 0, 0, 0, 0, 0},
	{GENERAL_MENU_ENTER_PIN, 		App_GeneralMenu_EnterPinHandler, 		"EnterPin", 		MENU_SUB_MENU, 	SCREEN_CMD_RESET, 0, 0xFF, 0, 0, 0, 0, 0},
	{GENERAL_MENU_ENTER_DISARM, 	App_GeneralMenu_DisarmHandler, 			"EnterPin", 		MENU_SUB_MENU, 	SCREEN_CMD_RESET, 0, 0xFF, 0, 0, 0, 0, 0},
	{GENERAL_MENU_FIRMWARE_UPDATE, 	App_GeneralMenu_FirmwareUpdateHandler, 	"FirmwareUpdate", 	MENU_SUB_FIRMWARE_UPDATE, SCREEN_CMD_RESET, 0, 0xFF, 0, 0, 0, 0, 0},

};

/* List-Menu structure collections: */
stu_Menu_t ListMenu[LIST_MENU_SUM] = {
	{LIST_MENU_MAIN, 			App_ListMenu_MainMenuHandler, 		"    Main Menu    ",  MENU_SUB_MENU, 	SCREEN_CMD_RESET, 0, 0xFF, 0, 0, 0, 0, 0},
	{LIST_MENU_SENSOR_PAIR, 	App_ListMenu_SensorPairHandler, 	"1.Sensor Pair     ", MENU_SUB_1, 		SCREEN_CMD_RESET, 0, 0xFF, 0, 0, 0, 0, 0},
	{LIST_MENU_SENSOR_PARA, 	App_ListMenu_SensorParaHandler, 	"2.Sensor Parameter", MENU_SUB_1, 		SCREEN_CMD_RESET, 0, 0xFF, 0, 0, 0, 0, 0},
	{LIST_MENU_WIFI_NETPAIR, 	App_ListMenu_WiFiNetPairHandler, 	"3.WiFi NetPair    ", MENU_SUB_WIFI_NETPAIR, SCREEN_CMD_RESET, 0, 0xFF, 0, 0, 0, 0, 0},
	{LIST_MENU_ADMIN_PASSWORD, 	App_ListMenu_AdminPasswordHandler, 	"4.Admin Password  ", MENU_SUB_1, 		SCREEN_CMD_RESET, 0, 0xFF, 0, 0, 0, 0, 0},
	{LIST_MENU_PHONENUMBER, 	App_ListMenu_PhoneNumberHandler, 	"5.Phone Number    ", MENU_SUB_1, 		SCREEN_CMD_RESET, 0, 0xFF, 0, 0, 0, 0, 0},
	{LIST_MENU_TERMINAL_PARA, 	App_ListMenu_TerminalParaHandler, 	"6.Host Parameter  ", MENU_SUB_1, 		SCREEN_CMD_RESET, 0, 0xFF, 0, 0, 0, 0, 0},
	{LIST_MENU_FACTORY_RESET, 	App_ListMenu_FactoryResetHandler, 	"7.Factory Reset   ", MENU_SUB_1, 		SCREEN_CMD_RESET, 0, 0xFF, 0, 0, 0, 0, 0},
	{LIST_MENU_ALARM_REPORT, 	App_ListMenu_AlarmReportHandler, 	"8.Alarm Report    ", MENU_SUB_1, 		SCREEN_CMD_RESET, 0, 0xFF, 0, 0, 0, 0, 0},
};

/* SensorPara SubMenu structure collections: */
stu_Menu_t SensorParaMenu[SENSORPARA_MENU_SUM] = {
	{SENSORPARA_MENU_MAIN,		App_SensorParaMenu_MainHandler,		"View",		MENU_SUB_1,		SCREEN_CMD_RESET, 0, 0xFF, 0, 0, 0, 0, 0},
	{SENSORPARA_MENU_REVIEW,	App_SensorParaMenu_ReviewHandler,	"View",		MENU_SUB_2,		SCREEN_CMD_RESET, 0, 0xFF, 0, 0, 0, 0, 0},
	{SENSORPARA_MENU_EDIT,		App_SensorParaMenu_EditHandler,		"Edit",		MENU_SUB_2,		SCREEN_CMD_RESET, 0, 0xFF, 0, 0, 0, 0, 0},
	{SENSORPARA_MENU_DELETE,	App_SensorParaMenu_DeleteHandler,	"Delete",	MENU_SUB_2,		SCREEN_CMD_RESET, 0, 0xFF, 0, 0, 0, 0, 0},
};

/* Terminal AlarmMenu structure collection: */
stu_Menu_t TerminalAlarmMenu[ALARM_WORKMODE_STATE_SUM] = {
	{ALARM_WORKMODE_STATE_ALARM, 	App_AlarmMenu_AlarmHandler, 	"System Alarming", MENU_SUB_ALARM, SCREEN_CMD_RESET, 0, 0xFF, 0, 0, 0, 0, 0},
	{ALARM_WORKMODE_STATE_DISARM, 	App_AlarmMenu_DisarmHandler, 	"System Disarm  ", MENU_SUB_ALARM, SCREEN_CMD_RESET, 0, 0xFF, 0, 0, 0, 0, 0},
};

/* Current Menu pointer: */
stu_Menu_t *pMenu;

/* Pairing Sensor parameters structure: */
stu_SensorPairMenu_SensorPara_t SensorPairMenu_SensorPara;

/* Terminal operation structure collection: */
stu_TerminalMode_t TerminalMode[Terminal_WORK_MODE_SUM] = {
	{TERMINAL_WORK_MODE_AWAYARM, 	SCREEN_CMD_RESET, 0xFF, App_TerminalMode_AwayArmHandler},
	{TERMINAL_WORK_MODE_HOMEARM, 	SCREEN_CMD_RESET, 0xFF, App_TerminalMode_HomeArmHandler},
	{TERMINAL_WORK_MODE_ALARMING, 	SCREEN_CMD_RESET, 0xFF, App_TerminalMode_AlarmingHandler},
	{TERMINAL_WORK_MODE_DISARM, 	SCREEN_CMD_RESET, 0xFF, App_TerminalMode_DisarmHandler},
};

/* Current Terminal operation pointer: */
stu_TerminalMode_t *pTerminalMode;

/* Application-Layer Lora Receive buffer */
Queue16 Queue_AppLoraRx;

/* Triggered Sensor ID buffer */
Queue8 Queue_TriggerSensorID;

/*-------------Module Functions Definition---------*/
/**
  * @Brief	Initialize App module
  * @Param	None
  * @Retval	None
  */
void App_Init(void)
{
	/* empty LoraRx buffer */
	QueueEmpty(Queue_AppLoraRx);
	
	/* empty Triggered Sensor ID buffer */
	QueueEmpty(Queue_TriggerSensorID);
	
	/* set Terminal WorkMode to AwayArm */
	pTerminalMode = &TerminalMode[TERMINAL_WORK_MODE_AWAYARM];
	
	/* initial Menu parameters */
	App_Menu_Init();
	
	/* Low-Layer call-back-function register */
	Hal_Key_KeyScanCBFRegister(App_KeyEvent_Handler);
	
	Mid_Lora_ApplyNetReq_HandlerCBFRegister(App_Lora_ApplyNetReq_Handler);
	Mid_Lora_FunctionCMD_HandlerCBFregister(App_Lora_FunctionCMD_Handler);
	
	TimeoutCounter_ReturnDesktop = 0;
	TimeoutCounter_ScreenSleep = 0;
	
	App_ScreenControl(1);	// turn on the Screen
	
}
  
/**
  * @Brief	Polling in sequence for Application layer
  * @Param	None
  * @Retval	None
  */
void App_Pro(void)
{
	/* Idle 20s, return to the Desktop Menu: */
	if((pMenu->MenuDepth == MENU_SUB_MENU) || (pMenu->MenuDepth == MENU_SUB_1) || (pMenu->MenuDepth == MENU_SUB_2))
	{
		TimeoutCounter_ReturnDesktop++;
		
		if(TimeoutCounter_ReturnDesktop > TIMEOUT_COUNTER_RETURN_DESKTOP)
		{
			TimeoutCounter_ReturnDesktop = 0;
			
			pMenu = &GeneralMenu[GENERAL_MENU_DESKTOP];
			pMenu->ScreenCMD = SCREEN_CMD_RESET;
		}
	}
	else
	{
	
	}
	
	/* Idle 30s, turn off the screen backlight: */
	if((pMenu->MenuDepth != MENU_SUB_ALARM) && (pMenu->MenuDepth != MENU_SUB_FIRMWARE_UPDATE) && (pMenu->MenuDepth != MENU_SUB_WIFI_NETPAIR))
	{
		TimeoutCounter_ScreenSleep++;
		
		if(TimeoutCounter_ScreenSleep > TIMEOUT_COUNTER_SCREEN_SLEEP)
		{
			TimeoutCounter_ScreenSleep = 0;
			
			App_ScreenControl(0);
		}
	}
	
	/* cast current Menu operation: */
	pMenu->Action();
	
}

/**
  * @Brief	According to the provided info, change corresponding Terminal work mode
  * @Param	Zone	 : Sensor defense zone(armtype) 0xFF->Host, 0-20->Sensor
  *			WorkMode : terminal work mode
  *			CMDSource: command source(key, remote, server)
  * @Retval	None
  */
void App_Terminal_ModeChange(uint8_t Zone, en_Terminal_WorkMode_t WorkMode, en_Terminal_CMDSource_t CMDSource)
{
	/* according <WorkMode> update pMenu */
	if(WorkMode == TERMINAL_WORK_MODE_ALARMING)
	{
		if(pMenu != &TerminalAlarmMenu[ALARM_WORKMODE_STATE_ALARM])
		{
			pMenu = &TerminalAlarmMenu[ALARM_WORKMODE_STATE_ALARM];
			pMenu->ScreenCMD = SCREEN_CMD_RESET;
			pTerminalMode = &TerminalMode[WorkMode];
			
			App_ScreenControl(1);
		}
		
		/* Host send alarm command */
		if(Zone == 0xFF)
		{
			MQTTProtocol_EventUpQueueIn(TERMINAL_UPEVENT_HOST_ALARM_SOS, 0xFF);
		}
		/* Detector(Sensor) send alarm command */
		else
		{
			MQTTProtocol_EventUpQueueIn(TERMINAL_UPEVENT_DETECTOR_ALARM, Zone);
		}
	}
	else if(WorkMode == TERMINAL_WORK_MODE_DISARM)
	{
		//if(pMenu == &GeneralMenu[GENERAL_MENU_ENTER_DISARM])
		if( (pMenu == &TerminalAlarmMenu[ALARM_WORKMODE_STATE_ALARM])  ||
			(pMenu == &TerminalAlarmMenu[ALARM_WORKMODE_STATE_DISARM]) ||
			(pMenu == &GeneralMenu[GENERAL_MENU_ENTER_DISARM])				)
		{
			pMenu = &GeneralMenu[GENERAL_MENU_DESKTOP];
			pMenu->ScreenCMD = SCREEN_CMD_RESET;
		}
		
		/* upload the updated Terminal WorkMode to MQTT server */
		switch((uint8_t)CMDSource)
		{
			case TERMINAL_CMD_SOURCE_KEY:
			{
				MQTTProtocol_EventUpQueueIn(TERMINAL_UPEVENT_DISARM_BY_HOST, Zone);
			}
			break;
			
			case TERMINAL_CMD_SOURCE_REMOTE:
			{
				MQTTProtocol_EventUpQueueIn(TERMINAL_UPEVENT_DISARM_BY_REMOTE, Zone);
			}
			break;
			
			case TERMINAL_CMD_SOURCE_SERVER:
			{
				MQTTProtocol_EventUpQueueIn(TERMINAL_UPEVENT_DISARM_BY_SERVER, Zone);
			}
			break;
			
		}
	}
	else if(WorkMode == TERMINAL_WORK_MODE_HOMEARM)
	{
		/* upload the updated Terminal WorkMode to MQTT server */
		switch((uint8_t)CMDSource)
		{
			case TERMINAL_CMD_SOURCE_KEY:
			{
				MQTTProtocol_EventUpQueueIn(TERMINAL_UPEVENT_HOMEARM_BY_HOST, Zone);
			}
			break;
			
			case TERMINAL_CMD_SOURCE_REMOTE:
			{
				MQTTProtocol_EventUpQueueIn(TERMINAL_UPEVENT_HOMEARM_BY_REMOTE, Zone);
			}
			break;
			
			case TERMINAL_CMD_SOURCE_SERVER:
			{
				MQTTProtocol_EventUpQueueIn(TERMINAL_UPEVENT_HOMEARM_BY_SERVER, Zone);
			}
			break;
			
		}
	}
	else if(WorkMode == TERMINAL_WORK_MODE_AWAYARM)
	{
		/* upload the updated Terminal WorkMode to MQTT server */
		switch((uint8_t)CMDSource)
		{
			case TERMINAL_CMD_SOURCE_KEY:
			{
				MQTTProtocol_EventUpQueueIn(TERMINAL_UPEVENT_AWAYARM_BY_HOST, Zone);
			}
			break;
			
			case TERMINAL_CMD_SOURCE_REMOTE:
			{
				MQTTProtocol_EventUpQueueIn(TERMINAL_UPEVENT_AWAYARM_BY_REMOTE, Zone);
			}
			break;
			
			case TERMINAL_CMD_SOURCE_SERVER:
			{
				MQTTProtocol_EventUpQueueIn(TERMINAL_UPEVENT_AWAYARM_BY_SERVER, Zone);
			}
			break;
			
		}
	}
	
	/* wake up screen */
	App_ScreenControl(1);
	
	TimeoutCounter_ReturnDesktop = 0;
	TimeoutCounter_ScreenSleep = 0;
	
	/* update current terminal work mode */
	if(pTerminalMode != &TerminalMode[WorkMode])
	{
		pTerminalMode = &TerminalMode[WorkMode];
		pTerminalMode->ScreenCMD = SCREEN_CMD_RESET;
	}
	
}

/**
  * @Brief	Creat Timer(1s) for SensorOffline_Handler
  * @Param	None
  * @Retval	None
  */
void App_SensorOffline_Init(void)
{
	Hal_Timer_Creat(T_SENSOR_OFFLINE, App_SensorOffline_Handler, 20000, T_STATE_START);
}

/**
  * @Brief	Check the Sleeptime of all paired Sensors, Queue-in Sensor-offline event if offline-counter overflow
  * @Param	None
  * @Retval	None
  */
void App_SensorOffline_Handler(void)
{
	uint8_t i;
	
	for(i=0; i<SENSOR_NUMBER_MAX; i++)
	{
		if(Device_Get_SensorPara_SleepTime(i))
		{
			if(Device_Get_SensorPara_SleepTime(i) < SENSOR_OFFLINE_COUNT)
			{
				Device_Set_SensorPara_SleepTime(i, (Device_Get_SensorPara_SleepTime(i) + 1));
			}
			else if(Device_Get_SensorPara_SleepTime(i) == SENSOR_OFFLINE_COUNT)
			{
				Device_Set_SensorPara_SleepTime(i, (SENSOR_OFFLINE_COUNT + 1));
				
				if(Device_Get_SensorPara_Sensor_Type(i) != SENSOR_TYPE_REMOTE)
				{
					MQTTProtocol_EventUpQueueIn(TERMINAL_UPEVENT_DETECTOR_OFFLINE, i);
				}
			}
		}
	}
}


/*-------------Internal Functions Definition--------*/
/**
  * @Brief	Menu initial
  * @Param	None
  * @Retval	None
  */
static void App_Menu_Init(void)
{
	uint8_t i;
	
	/* initial ListMenu members */
	ListMenu[1].pPrevious = &ListMenu[LIST_MENU_SUM - 1];
	ListMenu[1].pNext = &ListMenu[2];
	ListMenu[1].pParent = &ListMenu[LIST_MENU_MAIN];
	
	for(i=2; i<LIST_MENU_SUM - 1; i++)
	{
		ListMenu[i].pPrevious = &ListMenu[i - 1];
		ListMenu[i].pNext = &ListMenu[i + 1];
		ListMenu[i].pParent = &ListMenu[LIST_MENU_MAIN];
	}
	
	ListMenu[LIST_MENU_SUM - 1].pPrevious = &ListMenu[LIST_MENU_SUM - 2];
	ListMenu[LIST_MENU_SUM - 1].pNext = &ListMenu[1];
	ListMenu[LIST_MENU_SUM - 1].pParent = &ListMenu[LIST_MENU_MAIN];
	
	/* initial SensorParaMenu members(Review-Edit-Delete) */
	SensorParaMenu[SENSORPARA_MENU_REVIEW].pPrevious = &SensorParaMenu[SENSORPARA_MENU_SUM - 1];
	SensorParaMenu[SENSORPARA_MENU_REVIEW].pNext = &SensorParaMenu[SENSORPARA_MENU_EDIT];
	SensorParaMenu[SENSORPARA_MENU_REVIEW].pParent = &ListMenu[LIST_MENU_SENSOR_PARA];
	
	for(i=2; i<SENSORPARA_MENU_SUM-1; i++)
	{
		SensorParaMenu[i].pPrevious = &SensorParaMenu[i - 1];
		SensorParaMenu[i].pNext = &SensorParaMenu[i + 1];
		SensorParaMenu[i].pParent = &ListMenu[LIST_MENU_SENSOR_PARA];
	}
	
	SensorParaMenu[SENSORPARA_MENU_SUM - 1].pPrevious = &SensorParaMenu[i - 1];
	SensorParaMenu[SENSORPARA_MENU_SUM - 1].pNext = &SensorParaMenu[SENSORPARA_MENU_REVIEW];
	SensorParaMenu[SENSORPARA_MENU_SUM - 1].pParent = &ListMenu[LIST_MENU_SENSOR_PARA];
	
	/* initial AlarmMenu members */
	TerminalAlarmMenu[ALARM_WORKMODE_STATE_ALARM].pChild = &TerminalAlarmMenu[ALARM_WORKMODE_STATE_DISARM];
	TerminalAlarmMenu[ALARM_WORKMODE_STATE_DISARM].pParent = &TerminalAlarmMenu[ALARM_WORKMODE_STATE_ALARM];
	
	GeneralMenu[GENERAL_MENU_ENTER_DISARM].pParent = &GeneralMenu[GENERAL_MENU_DESKTOP];
		
	/* begin with Desktop-Menu */
	pMenu = &GeneralMenu[GENERAL_MENU_DESKTOP];
}

/**
  * @Brief	Desktop Menu Handler
  * @Param	None
  * @Retval	None
  */
static void App_GeneralMenu_DesktopHandler(void)
{
	en_KeyType_t  KeyPress;
	en_KeyEvent_t KeyEvent;
	
	/* DesktopMenu Screen Display: */
	if((pMenu->ScreenCMD == SCREEN_CMD_RESET) || (pMenu->ScreenCMD == SCREEN_CMD_RECOVER))
	{
		TimeoutCounter_ReturnDesktop = 0;
		TimeoutCounter_ScreenSleep = 0;
		
		pMenu->ScreenCMD = SCREEN_CMD_NULL;
		pMenu->KeyIndex = 0xFF;
		
		Mid_TFTLCD_ScreenClear();
		Mid_TFTLCD_DisplayTH(COOR_ICON_TEMP_X, COOR_ICON_TEMP_Y, COOR_ICON_HUM_X, COOR_ICON_HUM_Y);
		Mid_TFTLCD_DisplayPowerState(COOR_ICON_AC_X, COOR_ICON_AC_Y, COOR_ICON_BAT_X, COOR_ICON_BAT_Y);
		Mid_TFTLCD_ShowSystemTime(&SystemTime[0]);
		Mid_TFTLCD_DisplayWiFiSignal();
		Mid_TFTLCD_DisplayMQTTConnection();
			
		pTerminalMode->ScreenCMD = SCREEN_CMD_RESET;
	}
	
	Mid_TFTLCD_DisplayTH(COOR_ICON_TEMP_X, COOR_ICON_TEMP_Y, COOR_ICON_HUM_X, COOR_ICON_HUM_Y);
	Mid_TFTLCD_DisplayPowerState(COOR_ICON_AC_X, COOR_ICON_AC_Y, COOR_ICON_BAT_X, COOR_ICON_BAT_Y);
	Mid_TFTLCD_ShowSystemTime(&SystemTime[0]);
	Mid_TFTLCD_DisplayWiFiSignal();
	Mid_TFTLCD_DisplayMQTTConnection();
	
	
	/* New Firmware is available: */
	if(Mid_Firmware_GetUpdateState() == FIRMWARE_UPDATE_STA_NEW_VERSION)
	{
		TimeoutCounter_ReturnDesktop = 0;
		TimeoutCounter_ScreenSleep = 0;
		
		App_ScreenControl(1);
		
		pMenu = &GeneralMenu[GENERAL_MENU_FIRMWARE_UPDATE];
		pMenu->pParent = &GeneralMenu[GENERAL_MENU_DESKTOP];
		pMenu->ScreenCMD = SCREEN_CMD_RESET;
	}
	
	/* DesktopMenu KeyScan: */
	if(pMenu->KeyIndex != 0xFF)
	{
		KeyPress = (en_KeyType_t)pMenu->KeyIndex;
		KeyEvent = (en_KeyEvent_t)pMenu->KeyEvent;
		
		pMenu->KeyIndex = 0xFF;	// reset the KeyValue, ready for receive next key operation
		
		if(KeyEvent == KEY_CLICK)
		{
			switch((uint8_t)KeyPress)
			{
				case KEY_MENU_CONFIRM:
				{
					pMenu = &GeneralMenu[GENERAL_MENU_ENTER_PIN];
					pMenu->pParent = &GeneralMenu[GENERAL_MENU_DESKTOP];
					pMenu->ScreenCMD = SCREEN_CMD_RESET;
				}
				break;
				
				case KEY_DISARM:
				{
					pMenu = &GeneralMenu[GENERAL_MENU_ENTER_DISARM];
					pMenu->pParent = &GeneralMenu[GENERAL_MENU_DESKTOP];
					pMenu->ScreenCMD = SCREEN_CMD_RESET;
					
				}
				break;
				
				case KEY_HOMEARM:
				{	
					App_Terminal_ModeChange(0xFF, TERMINAL_WORK_MODE_HOMEARM, TERMINAL_CMD_SOURCE_KEY);
				}
				break;
				
				case KEY_AWAYARM:
				{
					App_Terminal_ModeChange(0xFF, TERMINAL_WORK_MODE_AWAYARM, TERMINAL_CMD_SOURCE_KEY);
				}
				break;
			}
		}
		else if(KeyEvent == KEY_LONG_PRESS)
		{	
			if(KeyPress == KEY_SOS_DELET)
			{
				uint8_t TerminalCode = 0xFF;
				
				App_Terminal_ModeChange(0xFF, TERMINAL_WORK_MODE_ALARMING, TERMINAL_CMD_SOURCE_KEY);
				
				QueueDataIn(Queue_TriggerSensorID, &TerminalCode, 1);
			}
		}
	}
	
	pTerminalMode->Action();
}

/**
  * @Brief	Phone Call Menu Handler
  * @Param	None
  * @Retval	None
  */
static void App_GeneralMenu_PhoneCallHandler(void)
{

}

/**
  * @Brief	Enter Pin Menu Handler
  * @Param	None
  * @Retval	None
  */
static void App_GeneralMenu_EnterPinHandler(void)
{
	en_KeyType_t KeyPress;			// indicate the index of pressed key
	en_KeyEvent_t KeyEvent;

	uint8_t KeyPressValue = 0;
	uint8_t KeyPressFlag = 0;		// 0->no effective key pressed, 1->effective key pressed
	
	static uint8_t CursorPos = 0;
	static uint16_t CursorBlinkCounter = 0;
	static uint8_t BlinkFlag = 0;
	static uint8_t Password[4];
	static uint8_t PasswordFillupFlag = 0;
	static uint8_t PasswordCorrectFlag = 0;
	static uint16_t Counter = 0;
	
	/* Reset Screen */
	if(pMenu->ScreenCMD == SCREEN_CMD_RESET)
	{
		TimeoutCounter_ReturnDesktop = 0;
		TimeoutCounter_ScreenSleep = 0;
		
		pMenu->ScreenCMD = SCREEN_CMD_NULL;
		
		Mid_TFTLCD_ScreenClear();
		Mid_TFTLCD_ShowString(COOR_MENU_ENTERPIN_TITLE_X, COOR_MENU_ENTERPIN_TITLE_Y, "Enter Pin", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
		
		CursorPos = 0;
		BlinkFlag = 0;
		CursorBlinkCounter = 0;
		PasswordFillupFlag = 0;
		PasswordCorrectFlag = 0;
		Counter = 0;
		
		Password[0] = ' ';
		Password[1] = ' ';
		Password[2] = ' ';
		Password[3] = ' ';
	}
	
	/* enter PIN not complete: */
	if(!PasswordFillupFlag)
	{
		CursorBlinkCounter++;
		
		if(CursorBlinkCounter > 30)
		{
			CursorBlinkCounter = 0;
			BlinkFlag = !BlinkFlag;
			
			if(CursorPos < 4)
			{
				if(BlinkFlag)
				{
					Mid_TFTLCD_ShowString(75 + 48*CursorPos, 100, " ", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
				}
				else
				{
					Mid_TFTLCD_ShowString(75 + 48*CursorPos, 100, "_", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
				}
			}
		}
	
		/* EnterPinMenu KeyScan: */
		if((pMenu->KeyIndex != 0xFF) && (!PasswordFillupFlag))
		{
			TimeoutCounter_ReturnDesktop = 0;
			TimeoutCounter_ScreenSleep = 0;
			
			KeyPress = (en_KeyType_t)pMenu->KeyIndex;
			KeyEvent = (en_KeyEvent_t)pMenu->KeyEvent;
			
			pMenu->KeyIndex = 0xFF;
			
			if(KeyEvent == KEY_CLICK)
			{
				switch((uint8_t)KeyPress)
				{
					case KEY_CALL_CANCEL:
					{
						pMenu = pMenu->pParent;
						pMenu->ScreenCMD = SCREEN_CMD_RESET;
					}
					break;
					
					case KEY_0:
					{
						KeyPressValue = '0';
						KeyPressFlag = 1;
					}
					break;
					
					case KEY_1:
					{
						KeyPressValue = '1';
						KeyPressFlag = 1;
					}
					break;
					
					case KEY_2_UP:
					{
						KeyPressValue = '2';
						KeyPressFlag = 1;
					}
					break;
					
					case KEY_3:
					{
						KeyPressValue = '3';
						KeyPressFlag = 1;
					}
					break;
					
					case KEY_4_LEFT:
					{
						KeyPressValue = '4';
						KeyPressFlag = 1;
					}
					break;
					
					case KEY_5:
					{
						KeyPressValue = '5';
						KeyPressFlag = 1;
					}
					break;
					
					case KEY_6_RIGHT:
					{
						KeyPressValue = '6';
						KeyPressFlag = 1;
					}
					break;
					
					case KEY_7:
					{
						KeyPressValue = '7';
						KeyPressFlag = 1;
					}
					break;
					
					case KEY_8_DOWN:
					{
						KeyPressValue = '8';
						KeyPressFlag = 1;
					}
					break;
					
					case KEY_9:
					{
						KeyPressValue = '9';
						KeyPressFlag = 1;
					}
					break;
					
					case KEY_MENU_CONFIRM:
					{
						if(( Password[0] == Device_Get_SystemPara_AdminPassword(0) && 
							 Password[1] == Device_Get_SystemPara_AdminPassword(1) && 
							 Password[2] == Device_Get_SystemPara_AdminPassword(2) && 
							 Password[3] == Device_Get_SystemPara_AdminPassword(3)		) || 
						   /* Super adminpassword: 0000 */
						   ( Password[0] == 0 && 
							 Password[1] == 0 && 
							 Password[2] == 0 && 
							 Password[3] == 0 											))
						{
							PasswordFillupFlag = 1;
							PasswordCorrectFlag = 1;
							
							Counter = 0;
						}
						else
						{
							PasswordFillupFlag = 1;
							
							Counter = 0;
						}
						
						/* password correct, jump to the next level menu */
						if(PasswordCorrectFlag == 1)
						{
							Mid_TFTLCD_ShowString(COOR_MENU_ENTERPIN_PROMPT_X, COOR_MENU_ENTERPIN_PROMPT_Y, "Password Correct", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
						}
						/* password wrong, show "Wrong Password" for 2s then back to the Desktop Menu */
						else
						{
							Mid_TFTLCD_ShowString(COOR_MENU_ENTERPIN_PROMPT_X, COOR_MENU_ENTERPIN_PROMPT_Y, "Wrong Password", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
						}
						
					}
					break;
					
					case KEY_SOS_DELET:
					{
						if(CursorPos > 0)
						{
							if(CursorPos < 4)
							{
								Password[CursorPos] = ' ';
							}
							else if(CursorPos == 4)
							{
								Password[3] = ' ';
							}
							
							Mid_TFTLCD_ShowString(75 + 48*CursorPos, 100, " ", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
							
							CursorPos--;
						}
					}
					break;
				}
			}
		}
		
		if(CursorPos < 4)
		{
			if(KeyPressFlag)
			{
				KeyPressFlag = 0;
				
				Password[CursorPos] = KeyPressValue - '0';
				
				if(CursorPos == 3)
				{
					Mid_TFTLCD_ShowString(75 + 48*CursorPos, 100, "*", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
				
					CursorPos = 4;
				}
				else
				{
					Mid_TFTLCD_ShowString(75 + 48*CursorPos, 100, "*", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
					
					CursorPos++;
				}
				
				CursorBlinkCounter = 0;
			}
		}
		else
		{
			CursorPos = 4;
		}
	}
	
	/* enter PIN success: */
	if(PasswordFillupFlag)
	{
		Counter++;
		
		if(Counter > 200)
		{
			Counter = 0;
			PasswordFillupFlag = 0;
			
			if(PasswordCorrectFlag == 1)
			{
				PasswordCorrectFlag = 0;
				
				pMenu = &ListMenu[LIST_MENU_MAIN];
				pMenu->ScreenCMD = SCREEN_CMD_RESET;
			}
			else
			{
				pMenu = pMenu->pParent;
				pMenu->ScreenCMD = SCREEN_CMD_RESET;
			}
		}
	}
}

/**
  * @Brief	Disarm Menu Handler
  * @Param	None
  * @Retval	None
  */
static void App_GeneralMenu_DisarmHandler(void)
{
	en_KeyType_t KeyPress;			// indicate the index of pressed key
	en_KeyEvent_t KeyEvent;

	uint8_t KeyPressValue = 0;
	uint8_t KeyPressFlag = 0;		// 0->no effective key pressed, 1->effective key pressed
	
	static uint8_t CursorPos = 0;
	static uint16_t CursorBlinkCounter = 0;
	static uint8_t BlinkFlag = 0;
	static uint8_t Password[4];
	static uint8_t PasswordFillupFlag = 0;
	static uint8_t PasswordCorrectFlag = 0;
	static uint16_t Counter = 0;
	
	/* Reset Screen */
	if(pMenu->ScreenCMD == SCREEN_CMD_RESET)
	{
		TimeoutCounter_ReturnDesktop = 0;
		TimeoutCounter_ScreenSleep = 0;
		
		pMenu->ScreenCMD = SCREEN_CMD_NULL;
		
		Mid_TFTLCD_ScreenClear();
		Mid_TFTLCD_ShowString(COOR_MENU_DISARM_TITLE_X, COOR_MENU_DISARM_TITLE_Y, "System Disarm", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
		
		CursorPos = 0;
		BlinkFlag = 0;
		CursorBlinkCounter = 0;
		PasswordFillupFlag = 0;
		PasswordCorrectFlag = 0;
		Counter = 0;
		
		Password[0] = ' ';
		Password[1] = ' ';
		Password[2] = ' ';
		Password[3] = ' ';
	}
	
	/* enter PIN not complete: */
	if(!PasswordFillupFlag)
	{
		CursorBlinkCounter++;
		
		if(CursorBlinkCounter > 30)
		{
			CursorBlinkCounter = 0;
			BlinkFlag = !BlinkFlag;
			
			if(CursorPos < 4)
			{
				if(BlinkFlag)
				{
					Mid_TFTLCD_ShowString(75 + 48*CursorPos, 100, " ", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
				}
				else
				{
					Mid_TFTLCD_ShowString(75 + 48*CursorPos, 100, "_", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
				}
			}		
		}
		
		/* DisarmMenu KeyScan: */
		if((pMenu->KeyIndex != 0xFF) && (!PasswordFillupFlag))
		{
			TimeoutCounter_ReturnDesktop = 0;
			TimeoutCounter_ScreenSleep = 0;
			
			KeyPress = (en_KeyType_t)pMenu->KeyIndex;
			KeyEvent = (en_KeyEvent_t)pMenu->KeyEvent;
			
			pMenu->KeyIndex = 0xFF;
			
			if(KeyEvent == KEY_CLICK)
			{
				switch((uint8_t)KeyPress)
				{
					case KEY_CALL_CANCEL:
					{
						pMenu = pMenu->pParent;
						pMenu->ScreenCMD = SCREEN_CMD_RESET;
					}
					break;
					
					case KEY_0:
					{
						KeyPressValue = '0';
						KeyPressFlag = 1;
					}
					break;
					
					case KEY_1:
					{
						KeyPressValue = '1';
						KeyPressFlag = 1;
					}
					break;
					
					case KEY_2_UP:
					{
						KeyPressValue = '2';
						KeyPressFlag = 1;
					}
					break;
					
					case KEY_3:
					{
						KeyPressValue = '3';
						KeyPressFlag = 1;
					}
					break;
					
					case KEY_4_LEFT:
					{
						KeyPressValue = '4';
						KeyPressFlag = 1;
					}
					break;
					
					case KEY_5:
					{
						KeyPressValue = '5';
						KeyPressFlag = 1;
					}
					break;
					
					case KEY_6_RIGHT:
					{
						KeyPressValue = '6';
						KeyPressFlag = 1;
					}
					break;
					
					case KEY_7:
					{
						KeyPressValue = '7';
						KeyPressFlag = 1;
					}
					break;
					
					case KEY_8_DOWN:
					{
						KeyPressValue = '8';
						KeyPressFlag = 1;
					}
					break;
					
					case KEY_9:
					{
						KeyPressValue = '9';
						KeyPressFlag = 1;
					}
					break;
					
					case KEY_MENU_CONFIRM:
					{
						if(( Password[0] == Device_Get_SystemPara_AdminPassword(0) && 
							 Password[1] == Device_Get_SystemPara_AdminPassword(1) && 
							 Password[2] == Device_Get_SystemPara_AdminPassword(2) && 
							 Password[3] == Device_Get_SystemPara_AdminPassword(3)	  )	|| 
						   /* Super adminpassword: 0000 */
						   ( Password[0] == 0 && 
							 Password[1] == 0 && 
							 Password[2] == 0 && 
							 Password[3] == 0 										  ))
						{
							PasswordFillupFlag = 1;
							PasswordCorrectFlag = 1;
							
							Counter = 0;
						}
						else
						{
							PasswordFillupFlag = 1;
							
							Counter = 0;
						}
						
						if(PasswordCorrectFlag == 1)
						{
							Mid_TFTLCD_ShowString(COOR_MENU_DISARM_PROMPT_X, COOR_MENU_DISARM_PROMPT_Y, "Disarm Success", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
						}
						else
						{
							Mid_TFTLCD_ShowString(COOR_MENU_DISARM_PROMPT_X, COOR_MENU_DISARM_PROMPT_Y, "Wrong Password", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
						}
						
					}
					break;
					
					case KEY_SOS_DELET:
					{
						if(CursorPos > 0)
						{
							if(CursorPos < 4)
							{
								Password[CursorPos] = ' ';
							}
							else if(CursorPos == 4)
							{
								Password[3] = ' ';
							}
							
							Mid_TFTLCD_ShowString(75 + 48*CursorPos, 100, " ", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
							
							CursorPos--;
						}
					}
					break;
				}
			}
		}
		
		if(CursorPos < 4)
		{
			if(KeyPressFlag)
			{
				KeyPressFlag = 0;
				
				Password[CursorPos] = KeyPressValue - '0';
				
				if(CursorPos == 3)
				{
					Mid_TFTLCD_ShowString(75 + 48*CursorPos, 100, "*", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
				
					CursorPos = 4;
				}
				else
				{
					Mid_TFTLCD_ShowString(75 + 48*CursorPos, 100, "*", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
					
					CursorPos++;
				}
				
				CursorBlinkCounter = 0;
			}
		}
		else
		{
			CursorPos = 4;
		}
	}
	
	/* enter PIN complete: */
	if(PasswordFillupFlag)
	{
		Counter++;
		
		if(Counter > 200)
		{
			Counter = 0;
			PasswordFillupFlag = 0;
			
			if(PasswordCorrectFlag)
			{
				PasswordCorrectFlag = 0;
				
				App_Terminal_ModeChange(0xFF, TERMINAL_WORK_MODE_DISARM, TERMINAL_CMD_SOURCE_KEY);
			}
			else
			{
				pMenu = pMenu->pParent;
				pMenu->ScreenCMD = SCREEN_CMD_RESET;
			}
		}
	}
}

/**
  * @Brief	FirmwareUpdate Menu Handler
  * @Param	None
  * @Retval	None
  */
static void App_GeneralMenu_FirmwareUpdateHandler(void)
{
	en_KeyType_t KeyPress;		// indicate the index of pressed key
	en_KeyEvent_t KeyEvent;
	
	uint8_t CommType;
	
	static stu_Sensor_t SensorParaBuff;
	
	static uint8_t CursorPos = 0;
	static uint16_t Counter = 0;
	
	uint8_t		   UpdateState;					// Firmware Update State 
	static uint8_t UpdateFlag = 0;		// 0->no update ongoing, 1->updating
	static uint8_t UpdateEndFlag = 0;	// 1-> end update process
	
	uint16_t 		PercentageBuff = 0;
	uint8_t 		PercentageCharBuff[7];
	static uint16_t Percentage = 0;
	
	/* Reset Screen */
	if(pMenu->ScreenCMD == SCREEN_CMD_RESET)
	{
		pMenu->ScreenCMD = SCREEN_CMD_NULL;
		
		if(Device_CheckSensorExistence(pMenu->ReservedBuff))
		{
			Device_GetSensorPara(&SensorParaBuff, pMenu->ReservedBuff);
		}
		
		Mid_TFTLCD_ScreenClear();
		
		Mid_TFTLCD_ShowString(COOR_MENU_FIRMWARE_UPDATE_TITLE_1_X, COOR_MENU_FIRMWARE_UPDATE_TITLE_1_Y, pMenu->pMenuTitle, LCD_FONT_COLOR, LCD_FONT_BACK_COLOR, 32, 0);
		Mid_TFTLCD_ShowString(COOR_MENU_FIRMWARE_UPDATE_PROMPT_1_X, COOR_MENU_FIRMWARE_UPDATE_PROMPT_1_Y, "Have New Firmware", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		Mid_TFTLCD_ShowString(COOR_MENU_FIRMWARE_UPDATE_PROMPT_2_X, COOR_MENU_FIRMWARE_UPDATE_PROMPT_2_Y, "Update or Not?", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		Mid_TFTLCD_ShowString(COOR_MENU_FIRMWARE_UPDATE_YES_X, COOR_MENU_FIRMWARE_UPDATE_YES_Y, "Yes", LCD_FONT_COLOR, LCD_FONT_BACK_COLOR, 24, 0);
		Mid_TFTLCD_ShowString(COOR_MENU_FIRMWARE_UPDATE_NO_X, COOR_MENU_FIRMWARE_UPDATE_NO_Y, "No", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		
		CursorPos = 1;
		Counter = 0;
		UpdateFlag = 0;
		UpdateEndFlag = 0;
	}
	
	/* FirmwareUpdate KeyScan: */
	if(pMenu->KeyIndex != 0xFF)
	{
		KeyPress = (en_KeyType_t)pMenu->KeyIndex;
		KeyEvent = (en_KeyEvent_t)pMenu->KeyEvent;
			
		pMenu->KeyIndex = 0xFF;
		
		if(!UpdateFlag)
		{
			if(KeyEvent == KEY_CLICK)
			{
				switch((uint8_t)KeyPress)
				{
					case KEY_CALL_CANCEL:
					{
						Mid_Firmware_SetUpdateState(FIRMWARE_UPDATE_STA_IDLE);
						
						pMenu = pMenu->pParent;
						pMenu->ScreenCMD = SCREEN_CMD_RECOVER;
					}
					break;
				
					case KEY_MENU_CONFIRM:
					{
						/* Press "Yes" to start downloading Firmware */
						if(CursorPos == 1)
						{
							Mid_TFTLCD_ScreenClear();
							Mid_TFTLCD_ShowString(COOR_MENU_FIRMWARE_UPDATE_PROMPT_3_X, COOR_MENU_FIRMWARE_UPDATE_PROMPT_3_Y, "Downloading...", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
							Mid_TFTLCD_ShowString(COOR_MENU_FIRMWARE_UPDATE_PERCENTAGE_X, COOR_MENU_FIRMWARE_UPDATE_PERCENTAGE_Y, "000.0%", LCD_FONT_COLOR, LCD_BACK_COLOR, 48, 0);
							
							/* Start downloading Firmware */
							Mid_Firmware_StartDownload();
							
							UpdateFlag = 1;
							Percentage = 0;
							
						}
						/* Press "No" not to update Firmware */
						else
						{
							pMenu = pMenu->pParent;
							pMenu->ScreenCMD = SCREEN_CMD_RECOVER;
							Mid_Firmware_SetUpdateState(FIRMWARE_UPDATE_STA_IDLE);
						}
					}
					break;
				
					case KEY_4_LEFT:
					case KEY_6_RIGHT:
					{
						if(CursorPos == 1)
						{
							CursorPos = 0;
							Mid_TFTLCD_ShowString(COOR_MENU_FIRMWARE_UPDATE_YES_X, COOR_MENU_FIRMWARE_UPDATE_YES_Y, "Yes", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
							Mid_TFTLCD_ShowString(COOR_MENU_FIRMWARE_UPDATE_NO_X, COOR_MENU_FIRMWARE_UPDATE_NO_Y, "No", LCD_FONT_COLOR, LCD_FONT_BACK_COLOR, 24, 0);
						}
						else if(CursorPos == 0)
						{
							CursorPos = 1;
							Mid_TFTLCD_ShowString(COOR_MENU_FIRMWARE_UPDATE_YES_X, COOR_MENU_FIRMWARE_UPDATE_YES_Y, "Yes", LCD_FONT_COLOR, LCD_FONT_BACK_COLOR, 24, 0);
							Mid_TFTLCD_ShowString(COOR_MENU_FIRMWARE_UPDATE_NO_X, COOR_MENU_FIRMWARE_UPDATE_NO_Y, "No", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
						}
					}
					break;
				
				}
			}
		}	
	}
	
	/* Firmware is downloading: */
	if(UpdateFlag == 1)
	{		
		UpdateState = (en_FirmwareUpdateState_t)Mid_Firmware_GetUpdateState();
		
		if(UpdateState == FIRMWARE_UPDATE_STA_DOWNLOAD_START)
		{
			if(Mid_WiFi_GetMQTTState() == MQTT_STA_READY)
			{
				CommType = (uint8_t)PROTOCOL_COMM_TYPE_WIFI;
				
				PercentageBuff = Mid_Firmware_DownloadProgress_Pro(CommType, &MQTTProtocol_GetNewFirmware_DataPack);
			}
			/* Update and display the Percentage */
			if(Percentage != PercentageBuff)
			{
				Percentage = PercentageBuff;
				
				/* update fail */
				if(PercentageBuff == 0xFFFF)
				{					
					Mid_Firmware_SetUpdateState(FIRMWARE_UPDATE_STA_DOWNLOAD_FAIL);
					
					Counter = 0;
					UpdateEndFlag = 1;
				}
				/* update continue */
				else
				{
					PercentageCharBuff[0] = Percentage / 1000 + '0';
					PercentageCharBuff[1] = Percentage % 1000 / 100 + '0';
					PercentageCharBuff[2] = Percentage % 100 / 10 + '0';
					PercentageCharBuff[3] = '.';
					PercentageCharBuff[4] = Percentage % 10 + '0';
					PercentageCharBuff[5] = '%';
					PercentageCharBuff[6] = 0;
					
					Mid_TFTLCD_ShowString(COOR_MENU_FIRMWARE_UPDATE_PERCENTAGE_X, COOR_MENU_FIRMWARE_UPDATE_PERCENTAGE_Y, &PercentageCharBuff[0], LCD_FONT_COLOR, LCD_BACK_COLOR, 48, 0);
				
					/* download finish */
					if(Percentage == 1000)
					{
						
					}
				}
			}
		}
		else if(UpdateState == FIRMWARE_UPDATE_STA_DOWNLOAD_FAIL)
		{
			Mid_TFTLCD_ScreenClear();
			Mid_TFTLCD_ShowString(COOR_MENU_FIRMWARE_UPDATE_PROMPT_3_X, COOR_MENU_FIRMWARE_UPDATE_PROMPT_3_Y, "Download Failed", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		
			Mid_Firmware_SetUpdateState(FIRMWARE_UPDATE_STA_IDLE);
		}
		else if(UpdateState == FIRMWARE_UPDATE_STA_SUCCESS)
		{
			Mid_TFTLCD_ScreenClear();
			
			PercentageCharBuff[0] = '1';
			PercentageCharBuff[1] = '0';
			PercentageCharBuff[2] = '0';
			PercentageCharBuff[3] = '.';
			PercentageCharBuff[4] = '0';
			PercentageCharBuff[5] = '%';
			PercentageCharBuff[6] = 0;
			
			Mid_TFTLCD_ShowString(COOR_MENU_FIRMWARE_UPDATE_PERCENTAGE_X, COOR_MENU_FIRMWARE_UPDATE_PERCENTAGE_Y, &PercentageCharBuff[0], LCD_FONT_COLOR, LCD_BACK_COLOR, 48, 0);
			Mid_TFTLCD_ShowString(COOR_MENU_FIRMWARE_UPDATE_PROMPT_3_X, COOR_MENU_FIRMWARE_UPDATE_PROMPT_3_Y, "Download Finish", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
			Mid_TFTLCD_ShowString(COOR_MENU_FIRMWARE_UPDATE_PROMPT_4_X, COOR_MENU_FIRMWARE_UPDATE_PROMPT_4_Y, "Please Restart", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		
			Device_Set_SystemPara_FirmwareVersion(0, stu_Firmware.NewVersion.Version[0]);
			Device_Set_SystemPara_FirmwareVersion(1, stu_Firmware.NewVersion.Version[1]);
			
			Mid_EEPROM_PageWrite(EEPROM_ADDRESS_SYSTEMPARA_OFFSET, (uint8_t *)(&stu_SystemPara), sizeof(stu_SystemPara));
			
			Counter = 0;
			UpdateEndFlag = 1;
		}
	}
	
	if(UpdateEndFlag)
	{
		Counter++;
		
		if(Counter > 200)
		{
			Counter = 0;
			UpdateEndFlag = 0;
			
			pMenu = pMenu->pParent;
			pMenu->ScreenCMD = SCREEN_CMD_RECOVER;
			
			Mid_Firmware_SetUpdateState(FIRMWARE_UPDATE_STA_IDLE);
		}
	}
	
}

/**
  * @Brief	ListMenu-Main Menu Handler
  * @Param	None
  * @Retval	None
  * @Note	pListMenu				pointer of selected option
  * 		pListMenuBackup			temporary pointer buffer
  * 		pListMenuFirstOption	pointer of the option displayed on the firstline
  * 		ListMenuCursorPos		indicate the Cursor(>) position ahead of option
  * Enter Stage:
  * 	if ScreenCMD = RESET, reset all variables
  * 	if ScreenCMD = Recover, keep the data of pListMenu, pListMenuFirstOption, ListMenuCursorPos
  * KeyScan Stage:
  * 	according to the eadge condition(ListMenuCursorPos = 1 / ListMenuCursorPos = 6), decide whether the screen need to be refreshed(roll up/down)
  * 	pListMenu->pListMenu.pPrevious / pListMenu->pListMenu.pNext, change current selected option
  * 	pListMenuFirstOption->pListMenuFirstOption.pPrevious / pListMenuFirstOption->pListMenuFirstOption.pNext, accordingly change Firstline option
  * 	if Confirm botton pressed, pMenu = pListMenu and jump to the submenu
  * Display Stage:
  * 	check value of pListMenuBackup and pListMenu, if pListMenu changed update pListMenuBackup = pListMenu
  * 	use the pListMenuFirstOption as the firstline option to display all list options of the menu
  * 	according to the ListMenuCursorPos display the Cursor(>) ahead of selected option
  */
static void App_ListMenu_MainMenuHandler(void)
{
	uint8_t KeyPress;		// indicate the index of pressed key
	uint8_t KeyEvent;

	uint8_t i;
	uint8_t ScreenRefreshFlag;
	
	static stu_Menu_t *pListMenu;									// current selsected ListMenuOption
	static stu_Menu_t *pListMenuBackup = 0;				// back up current pMenu
	static stu_Menu_t *pListMenuFirstOption = 0;	// point to the first display option
	static uint8_t ListMenuCursorPos;
	
	/* enter MainMenu from EnterPin menu */
	if(pMenu->ScreenCMD == SCREEN_CMD_RESET)
	{
		TimeoutCounter_ReturnDesktop = 0;
		TimeoutCounter_ScreenSleep = 0;
		
		pMenu->ScreenCMD = SCREEN_CMD_NULL;
		pListMenu = &ListMenu[LIST_MENU_MAIN];
		
		Mid_TFTLCD_ScreenClear();
		Mid_TFTLCD_ShowString(COOR_MENU_MAINMENU_TITLE_X, COOR_MENU_MAINMENU_TITLE_Y, ListMenu[0].pMenuTitle, LCD_FONT_COLOR, LCD_FONT_BACK_COLOR, 32, 0);
	
		pListMenu = &ListMenu[LIST_MENU_SENSOR_PAIR];
		pListMenuBackup = 0;
		pListMenuFirstOption = pListMenu;
		
		ScreenRefreshFlag = 1;
		ListMenuCursorPos = 1;
		
		KeyPress = 0xFF;
	}
	/* return Mainmenu from submenu */
	else if(pMenu->ScreenCMD == SCREEN_CMD_RECOVER)
	{
		TimeoutCounter_ReturnDesktop = 0;
		TimeoutCounter_ScreenSleep = 0;
		
		pMenu->ScreenCMD = SCREEN_CMD_NULL;
		
		Mid_TFTLCD_ScreenClear();
		Mid_TFTLCD_ShowString(COOR_MENU_MAINMENU_TITLE_X, COOR_MENU_MAINMENU_TITLE_Y, ListMenu[0].pMenuTitle, LCD_FONT_COLOR, LCD_FONT_BACK_COLOR, 32, 0);

		ScreenRefreshFlag = 1;
		pListMenuBackup = 0;
		
		KeyPress = 0xFF;
	}
	
	/* MainMenu KeyScan: */
	if(pMenu->KeyIndex != 0xFF)
	{
		KeyPress = pMenu->KeyIndex;
		pMenu->KeyIndex = 0xFF;
		KeyEvent = pMenu->KeyEvent;
		
		if(KeyEvent == KEY_CLICK)
		{
			switch((uint8_t)KeyPress)
			{
				case KEY_2_UP:
				{
					if(ListMenuCursorPos == 1)
					{
						pListMenuFirstOption = pListMenuFirstOption->pPrevious;
						pListMenu = pListMenu->pPrevious;
						
						ListMenuCursorPos = 1;
						ScreenRefreshFlag = 1;
					}
					else
					{
						Mid_TFTLCD_ShowString(COOR_MENU_MAINMENU_CURSOR_X, 32*ListMenuCursorPos, " ", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
					
						pListMenu = pListMenu->pPrevious;
						ListMenuCursorPos--;
					}
				}
				break;
				
				case KEY_8_DOWN:
				{
					if(ListMenuCursorPos == 6)
					{
						pListMenuFirstOption = pListMenuFirstOption->pNext;
						pListMenu = pListMenu->pNext;
						
						ListMenuCursorPos = 6;
						ScreenRefreshFlag = 1;
					}
					else
					{
						Mid_TFTLCD_ShowString(COOR_MENU_MAINMENU_CURSOR_X, 32*ListMenuCursorPos, " ", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
					
						pListMenu = pListMenu->pNext;
						ListMenuCursorPos++;
					}
				}
				break;
				
				case KEY_CALL_CANCEL:
				{
					pMenu = &GeneralMenu[GENERAL_MENU_DESKTOP];
					pMenu->ScreenCMD = SCREEN_CMD_RESET;
				}
				break;
				
				case KEY_MENU_CONFIRM:
				{
					pMenu = pListMenu;
					pMenu->ScreenCMD = SCREEN_CMD_RESET;
				}
				break;
				
			}
		}
	}
	
	/* update and refresh the ListMenu display */
	if(pListMenuBackup != pListMenu)
	{
		pListMenuBackup = pListMenu;
		
		if(ScreenRefreshFlag == 1)
		{
			ScreenRefreshFlag = 0;
			pListMenu = pListMenuFirstOption;
			
			for(i=1; i<7; i++)
			{
				Mid_TFTLCD_ShowString(COOR_MENU_MAINMENU_OPTION_X, (32 * i), pListMenu->pMenuTitle, LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
				
				pListMenu = pListMenu->pNext;
			}
			
			pListMenu = pListMenuBackup;
			
			Mid_TFTLCD_ShowString(COOR_MENU_MAINMENU_CURSOR_X, (32 * ListMenuCursorPos), ">", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
		}
		else
		{
			Mid_TFTLCD_ShowString(COOR_MENU_MAINMENU_CURSOR_X, (32 * ListMenuCursorPos), ">", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
		}
		
		TimeoutCounter_ReturnDesktop = 0;
		TimeoutCounter_ScreenSleep = 0;
	}
}

/**
  * @Brief	ListMenu-SensorPair Menu Handler
  * @Param	None
  * @Retval	None
  * @Note	
  * 	Data architecture: 
  * 		1. SensorPairMenu_SensorPara: (global Menu variable)
  * 									  store the parameters of Sensor which is pairing with the Terminal
  * 			WorkMode 	-> indicates pairing sensor WorkMode, changed by App_Lora_ApplyNetReq_Handler function
  * 			SensorType 	-> Door/Remote/... 
  * 			PairState 	-> the pairing sensor PairState, incdicates whether this sensor's info has been saved in Terminal or not
  * 		
  * 		2. Lora_ApplyNet_SensorPara: (return structure data of App_Lora_ApplyNetReq_Handler)
  * 									 store the return info of sensor which send the ApplyNet request to the Terminal
  * 			State		-> the ApplyNet Sensor's pairing state, indicates the situation(result) of the pairing process
  * 			SensorIndex	-> index of the ApplyNet sensor, indicates the sequence number(1-20) of the sensors saved by the Termianl
  * 
  * 		3. Sensor_PairPara: (local variable of App_Lora_ApplyNetReq_Handler, work as buffer)
  * 							store the return info of App_Lora_ApplyNet_AddSensor
  * 			PairState	-> the ApplyNet Sensor's pairing state
  * 			SensorIndex	-> index of the ApplyNet sensor
  * 
  * 		4. Sensor_Buff: (local variable of App_Lora_ApplyNetReq_Handler, work as buffer)
  * 						store the data of DataFrame from ApplyNet Sensor
  * 
  * 	SensorPair process:
  * 		1. Enter the SensorPairMenu, change the SensorPairMenu_SensorPara WorkMode, start waiting for Sensor ApllyNet Request
  * 		2. The Lora module receive datapackage from Sensor, provide FunctionCode and corresponding DataFrame to App_Lora_ApplyNetReq_Handler
  * 		3. If received FunctionCode = LORA_COM_APPLY_NET, update SensorPairMenu_SensorPara WorkMode = SUCCESS and SensorType
  * 		4. Use Sensor_Buff to store the provided DataFrame from Sensor, then send to App_Lora_ApplyNet_AddSensor to decide whether save to the Terminal/already paired
  * 		5. Use Sensor_PairPara to receive the return value(PairState and SensorIndex) of App_Lora_ApplyNet_AddSensor, and update the PairState of SensorPairMenu_SensorPara
  * 		6. According to the obtined PairState of SensorPairMenu_SensorPara, update the return value Lora_ApplyNet_SensorPara for Lora-Module to prepare the response to the Sensor
  * 		7. According to the PairState of SensorPairMenu_SensorPara, display corresponding Menu Message on TFTLCD 
  */
static void	App_ListMenu_SensorPairHandler(void)
{
	uint8_t KeyPress;		// indicate the index of pressed key
	uint8_t KeyEvent;
	
	static uint16_t Counter = 0;
	static uint8_t PairCompleteFlag = 0;
	
	/* enter SensorPair from MainMenu */
	if(pMenu->ScreenCMD == SCREEN_CMD_RESET)
	{
		TimeoutCounter_ReturnDesktop = 0;
		TimeoutCounter_ScreenSleep = 0;
		
		pMenu->ScreenCMD = SCREEN_CMD_NULL;
		
		Mid_TFTLCD_ScreenClear();
		Mid_TFTLCD_ShowString(COOR_MENU_SENSORPAIR_TITLE_X, COOR_MENU_SENSORPAIR_TITLE_Y, "    Pairing Sensor    ", LCD_FONT_COLOR, LCD_FONT_BACK_COLOR, 32, 0);
		Mid_TFTLCD_ShowString(COOR_MENU_SENSORPAIR_PROMPT_X, COOR_MENU_SENSORPAIR_PROMPT_Y, " Pairing...  ", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
		
		KeyPress = 0xFF;
		
		SensorPairMenu_SensorPara.WorkMode = SENSOR_WORKMODE_APPLY_NET;
		Counter = 0;
		PairCompleteFlag = 0;
	}
	
	/* SensorPair Menu KeyScan */
	if(pMenu->KeyIndex != 0xFF)
	{
		KeyPress = pMenu->KeyIndex;
		pMenu->KeyIndex = 0xFF;
		KeyEvent = pMenu->KeyEvent;
		
		if(KeyEvent == KEY_CLICK)
		{
			switch((uint8_t)KeyPress)
			{
				case KEY_CALL_CANCEL:
				{
					pMenu = &ListMenu[LIST_MENU_MAIN];
					pMenu->ScreenCMD = SCREEN_CMD_RECOVER;
				}
				break;
				
			}
		}
	}
	
	/* check whether sensor pairing succeed */
	if((SensorPairMenu_SensorPara.WorkMode == SENSOR_WORKMODE_APPLY_NET_SUCCESS) && (!PairCompleteFlag))
	{
		SensorPairMenu_SensorPara.WorkMode = SENSOR_WORKMODE_NORMAL;
		Counter = 0;
		PairCompleteFlag = 1;
		
		switch((uint8_t)SensorPairMenu_SensorPara.PairState)
		{
			case SENSOR_PAIRSTATE_FAIL:
			{
				Mid_TFTLCD_ShowString(COOR_MENU_SENSORPAIR_PROMPT_X, COOR_MENU_SENSORPAIR_PROMPT_Y, "Join Net Fail", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
			}
			break;
			
			case SENSOR_PAIRSTATE_UNPAIR:
			{
				Mid_TFTLCD_ShowString(COOR_MENU_SENSORPAIR_PROMPT_X, COOR_MENU_SENSORPAIR_PROMPT_Y, "Pair Success ", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
				
				switch((uint8_t)SensorPairMenu_SensorPara.SensorType)
				{
					case SENSOR_TYPE_DOOR:
					{
						Mid_TFTLCD_ShowString(COOR_MENU_SENSORPAIR_TYPE_X, COOR_MENU_SENSORPAIR_TYPE_Y, "Door Sensor", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
					}
					break;
					
					case SENSOR_TYPE_REMOTE:
					{
						Mid_TFTLCD_ShowString(COOR_MENU_SENSORPAIR_TYPE_X, COOR_MENU_SENSORPAIR_TYPE_Y, "   Remote  ", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
					}
					break;
					
				}
			}
			break;
			
			case SENSOR_PAIRSTATE_PAIRED:
			{
				Mid_TFTLCD_ShowString(COOR_MENU_SENSORPAIR_PROMPT_X, COOR_MENU_SENSORPAIR_PROMPT_Y, "Sensor Exist ", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
			}
			break;
			
		}
	}
	
	/* back to the main menu after 2s */
	if(PairCompleteFlag)
	{
		Counter++;
		
		if(Counter > 200)
		{
			Counter = 0;
			PairCompleteFlag = 0;
			
			pMenu = pMenu->pParent;
			pMenu->ScreenCMD = SCREEN_CMD_RESET;
			
			SensorPairMenu_SensorPara.WorkMode = SENSOR_WORKMODE_NORMAL;
		}

		TimeoutCounter_ReturnDesktop = 0;
		TimeoutCounter_ScreenSleep = 0;
	}
}

/**
  * @Brief	ListMenu-SensorPara Menu Handler
  * @Param	None
  * @Retval	None
  */
static void App_ListMenu_SensorParaHandler(void)
{
	uint8_t i, j;
	
	uint8_t KeyPress;		// indicate the index of pressed key
	uint8_t KeyEvent;
	
	stu_Sensor_t SensorParaBuff;
	
	static stu_Sensor_t SensorBuff[SENSOR_NUMBER_MAX];					// buffer temporary store Sensor parameters 
	static stu_Menu_t 	SensorPara_ListMenu[SENSOR_NUMBER_MAX];	// SensorPara List Menu linked-list structure
	
	static stu_Menu_t *pSensorPara_ListMenu = 0;								// pointer of SensorPara List option 
	static stu_Menu_t *pSensorPara_ListMenuBackup = 0;					// pointer of backup the previous selected option
	static stu_Menu_t *pSensorPara_ListMenuFirstOption = 0;			// pointer of the first option display
	
	static uint8_t SensorIndex = 0;
	static uint8_t ListMenuCursorPos = 0;
	
	uint8_t ScreenRefreshFlag;
	
	uint8_t Timer_H;
	uint8_t Timer_L;
	
	/* enter SensorPara from MainMenu */
	if(pMenu->ScreenCMD == SCREEN_CMD_RESET)
	{
		TimeoutCounter_ReturnDesktop = 0;
		TimeoutCounter_ScreenSleep = 0;
		
		pMenu->ScreenCMD = SCREEN_CMD_NULL;
		
		Mid_TFTLCD_ScreenClear();
		Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_TITLE_X, COOR_MENU_SENSORPARA_TITLE_Y, "    Sensor List    ", LCD_FONT_COLOR, LCD_FONT_BACK_COLOR, 32, 0);
		
		KeyPress = 0xFF;
		
		ScreenRefreshFlag = 1;
		SensorIndex = 0;
		pSensorPara_ListMenu = &SensorPara_ListMenu[0];
		pSensorPara_ListMenuBackup = 0;
		pSensorPara_ListMenuFirstOption = pSensorPara_ListMenu;
		ListMenuCursorPos = 1;
		
		for(i=0; i<SENSOR_NUMBER_MAX; i++)
		{
			if(Device_CheckSensorExistence(i))
			{
				Device_GetSensorPara(&SensorParaBuff, i);
				
				(pSensorPara_ListMenu + SensorIndex)->MenuID = SensorIndex;
				(pSensorPara_ListMenu + SensorIndex)->MenuDepth = MENU_SUB_2;
				(pSensorPara_ListMenu + SensorIndex)->ReservedBuff = SensorParaBuff.ID - 1;
				
				SensorBuff[SensorIndex].ID = SensorParaBuff.ID - 1;
				
				for(j=0; j<26; j++)
				{
					if(SensorParaBuff.SensorName[j] == 0)
					{
						SensorBuff[SensorIndex].SensorName[j] = ' ';
					}
					else
					{
						SensorBuff[SensorIndex].SensorName[j] = SensorParaBuff.SensorName[j];
					}
				}
				
				if(SensorParaBuff.Sensor_Type != SENSOR_TYPE_REMOTE)
				{
					Timer_H = SensorParaBuff.SleepTime / 3600;			// hour
					Timer_L = (SensorParaBuff.SleepTime % 3600) / 60;	// minute
					
					SensorBuff[SensorIndex].SensorName[11] = (Timer_H / 10) + '0';
					SensorBuff[SensorIndex].SensorName[12] = (Timer_H % 10) + '0';
					SensorBuff[SensorIndex].SensorName[13] = ':';
					SensorBuff[SensorIndex].SensorName[14] = (Timer_L / 10) + '0';
					SensorBuff[SensorIndex].SensorName[15] = (Timer_L % 10) + '0';
					SensorBuff[SensorIndex].SensorName[16] = ' ';
					
					if(SensorParaBuff.SleepTime > SENSOR_OFFLINE_COUNT)
					{
						SensorBuff[SensorIndex].SensorName[17] = 'O';
						SensorBuff[SensorIndex].SensorName[18] = 'f';
						SensorBuff[SensorIndex].SensorName[19] = 'f';
						SensorBuff[SensorIndex].SensorName[20] = 'l';
						SensorBuff[SensorIndex].SensorName[21] = 'i';
						SensorBuff[SensorIndex].SensorName[22] = 'n';
						SensorBuff[SensorIndex].SensorName[23] = 'e';
						SensorBuff[SensorIndex].SensorName[24] = ' ';
						SensorBuff[SensorIndex].SensorName[25] = 0;
					}
					else
					{
						SensorBuff[SensorIndex].SensorName[17] = 'O';
						SensorBuff[SensorIndex].SensorName[18] = 'n';
						SensorBuff[SensorIndex].SensorName[19] = 'l';
						SensorBuff[SensorIndex].SensorName[20] = 'i';
						SensorBuff[SensorIndex].SensorName[21] = 'n';
						SensorBuff[SensorIndex].SensorName[22] = 'e';
						SensorBuff[SensorIndex].SensorName[23] = ' ';
						SensorBuff[SensorIndex].SensorName[24] = ' ';
						SensorBuff[SensorIndex].SensorName[25] = 0;
					}
				}
				else
				{
					SensorBuff[SensorIndex].SensorName[16] = ' ';
					SensorBuff[SensorIndex].SensorName[17] = ' ';
					SensorBuff[SensorIndex].SensorName[18] = ' ';
					SensorBuff[SensorIndex].SensorName[19] = ' ';
					SensorBuff[SensorIndex].SensorName[20] = ' ';
					SensorBuff[SensorIndex].SensorName[21] = ' ';
					SensorBuff[SensorIndex].SensorName[22] = ' ';
					SensorBuff[SensorIndex].SensorName[23] = ' ';
					SensorBuff[SensorIndex].SensorName[24] = 0;
				}
				
				(pSensorPara_ListMenu + SensorIndex)->pMenuTitle = SensorBuff[SensorIndex].SensorName;
				
				SensorIndex++;
			}
		}
		
		/* at least 1 Sensor exist, set up the linked-list structure */
		if(SensorIndex != 0)
		{
			if(SensorIndex > 1)
			{
				pSensorPara_ListMenu->pPrevious = pSensorPara_ListMenu + (SensorIndex - 1);
				pSensorPara_ListMenu->pNext = pSensorPara_ListMenu + 1;
				
				pSensorPara_ListMenu->pParent = &ListMenu[LIST_MENU_MAIN];
				
				for(i=1; i<SensorIndex-1; i++)
				{
					(pSensorPara_ListMenu + i)->pPrevious = pSensorPara_ListMenu + (i - 1);
					(pSensorPara_ListMenu + i)->pNext = pSensorPara_ListMenu + (i + 1);
					(pSensorPara_ListMenu + i)->pParent = &ListMenu[LIST_MENU_MAIN];
				}
				
				(pSensorPara_ListMenu + (SensorIndex - 1))->pPrevious = pSensorPara_ListMenu + (SensorIndex - 2);
				(pSensorPara_ListMenu + (SensorIndex - 1))->pNext = pSensorPara_ListMenu;
				(pSensorPara_ListMenu + (SensorIndex - 1))->pParent = &ListMenu[LIST_MENU_MAIN];
			}
			else if(SensorIndex == 1)
			{
				pSensorPara_ListMenu->pPrevious = pSensorPara_ListMenu;
				pSensorPara_ListMenu->pNext = pSensorPara_ListMenu;
				pSensorPara_ListMenu->pParent = &ListMenu[LIST_MENU_MAIN];
			}
		}
		/* no Sensor exist */
		else
		{
			pSensorPara_ListMenuBackup = pSensorPara_ListMenu;
			
			Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_PROMPT_X, COOR_MENU_SENSORPARA_PROMPT_Y, " No Sensor ", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
		}
		
		pSensorPara_ListMenuFirstOption = pSensorPara_ListMenu;
	}
	
	/* return to the SensorPara Menu from submenu */
	else if(pMenu->ScreenCMD == SCREEN_CMD_RECOVER)
	{
		TimeoutCounter_ReturnDesktop = 0;
		TimeoutCounter_ScreenSleep = 0;
		
		pMenu->ScreenCMD = SCREEN_CMD_NULL;
		
		Mid_TFTLCD_ScreenClear();
		Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_TITLE_X, COOR_MENU_SENSORPARA_TITLE_Y, "    Sensor List    ", LCD_FONT_COLOR, LCD_FONT_BACK_COLOR, 32, 0);
		
		KeyPress = 0xFF;
		ScreenRefreshFlag = 1;
		pSensorPara_ListMenuBackup = 0;
	}
	
	/* SensorPara Menu KeyScan */
	if(pMenu->KeyIndex != 0xFF)
	{
		KeyPress = pMenu->KeyIndex;
		pMenu->KeyIndex = 0xFF;
		KeyEvent = pMenu->KeyEvent;
		
		if(KeyEvent == KEY_CLICK)
		{
			switch((uint8_t)KeyPress)
			{
				case KEY_MENU_CONFIRM:
				{
					if(SensorIndex > 0)
					{
						pMenu = &SensorParaMenu[SENSORPARA_MENU_MAIN];
						pMenu->ReservedBuff = pSensorPara_ListMenu->ReservedBuff;
						pMenu->ScreenCMD = SCREEN_CMD_RESET;
					}
				}
				break;
				
				case KEY_CALL_CANCEL:
				{
					pMenu = &ListMenu[LIST_MENU_MAIN];
					pMenu->ScreenCMD = SCREEN_CMD_RECOVER;
				}
				break;
				
				case KEY_2_UP:
				{
					if(SensorIndex < 2)
					{
					
					}
					else if(SensorIndex < 7)	// one page
					{
						if(ListMenuCursorPos == 1)
						{
							Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_CURSOR_X, (32 * ListMenuCursorPos), " ", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
							
							pSensorPara_ListMenu = pSensorPara_ListMenu->pPrevious;
							ListMenuCursorPos = SensorIndex;
						}
						else
						{
							Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_CURSOR_X, (32 * ListMenuCursorPos), " ", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
							
							pSensorPara_ListMenu = pSensorPara_ListMenu->pPrevious;
							ListMenuCursorPos--;
						}
					}
					else	// more than 1 page
					{
						if(ListMenuCursorPos == 1)
						{
							pSensorPara_ListMenuFirstOption = pSensorPara_ListMenuFirstOption->pPrevious;
							pSensorPara_ListMenu = pSensorPara_ListMenu->pPrevious;
							
							ListMenuCursorPos = 1;
							ScreenRefreshFlag = 1;
						}
						else
						{
							Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_CURSOR_X, (32 * ListMenuCursorPos), " ", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
							
							pSensorPara_ListMenu = pSensorPara_ListMenu->pPrevious;
							ListMenuCursorPos--;
						}
					}
				}
				break;
				
				case KEY_8_DOWN:
				{
					if(SensorIndex < 2)
					{
					
					}
					else if(SensorIndex < 7)	// one page
					{
						if(ListMenuCursorPos == SensorIndex)	// when cursor point to the last sensor option, roll over to the first option
						{
							Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_CURSOR_X, (32 * ListMenuCursorPos), " ", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
							
							pSensorPara_ListMenu = pSensorPara_ListMenu->pNext;
							ListMenuCursorPos = 1;
						}
						else
						{
							Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_CURSOR_X, (32 * ListMenuCursorPos), " ", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
							
							pSensorPara_ListMenu = pSensorPara_ListMenu->pNext;
							ListMenuCursorPos++;
						}
					}
					else	// more than 1 page
					{
						if(ListMenuCursorPos == 6)
						{
							pSensorPara_ListMenuFirstOption = pSensorPara_ListMenuFirstOption->pNext;
							pSensorPara_ListMenu = pSensorPara_ListMenu->pNext;
							
							ListMenuCursorPos = 6;
							ScreenRefreshFlag = 1;
						}
						else
						{
							Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_CURSOR_X, (32 * ListMenuCursorPos), " ", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
							
							pSensorPara_ListMenu = pSensorPara_ListMenu->pNext;
							ListMenuCursorPos++;
						}
					}
				}
				break;
				
			}
		}
	}
	
	/* refresh screen display */
	if(pSensorPara_ListMenuBackup != pSensorPara_ListMenu)
	{
		pSensorPara_ListMenuBackup = pSensorPara_ListMenu;
		
		if(ScreenRefreshFlag)
		{
			ScreenRefreshFlag = 0;
			pSensorPara_ListMenu = pSensorPara_ListMenuFirstOption;
		
			if(SensorIndex < 7)	// 1 page
			{
				for(i=0; i<SensorIndex; i++)
				{
					Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_X, (32 * (i + 1)), pSensorPara_ListMenu->pMenuTitle, LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
					
					pSensorPara_ListMenu = pSensorPara_ListMenu->pNext;
				}
			}
			else	// more than 1 page
			{
				for(i=1; i<7; i++)
				{
					Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_X, (32 * i), pSensorPara_ListMenu->pMenuTitle, LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
					
					pSensorPara_ListMenu = pSensorPara_ListMenu->pNext;
				}
			}
			
			pSensorPara_ListMenu = pSensorPara_ListMenuBackup;
			
			Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_CURSOR_X, (32 * ListMenuCursorPos), ">", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		}
		else
		{
			Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_CURSOR_X, (32 * ListMenuCursorPos), ">", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		}

		TimeoutCounter_ReturnDesktop = 0;
		TimeoutCounter_ScreenSleep = 0;
	}
}

/**
  * @Brief	SensorParaMenu MainHandler for call-back function
  * @Param	None
  * @Retval	None
  */
static void App_SensorParaMenu_MainHandler(void)
{
	uint8_t i;
	
	uint8_t KeyPress;		// indicate the index of pressed key
	uint8_t KeyEvent;
	
	stu_Sensor_t SelectSensor;	// the parameters of selected sensor
	
	static stu_Menu_t *pSensorPara_OptionMainMenu = 0;
	static stu_Menu_t *pSensorPara_OptionMainMenuBackup = 0;
	static stu_Menu_t *pSensorPara_OptionMainMenuFirstOption = 0;
	
	static uint8_t OptionMainMenuCursorPos = 0;
	
	uint8_t ScreenRefreshFlag;
	
	/* enter SensorPara_OptionMainMeu from SensorPara_ListMenu */
	if(pMenu->ScreenCMD == SCREEN_CMD_RESET)
	{
		TimeoutCounter_ReturnDesktop = 0;
		TimeoutCounter_ScreenSleep = 0;
		
		if(Device_CheckSensorExistence(pMenu->ReservedBuff))	// ReservedBuff stores the SensorIndex
		{
			Device_GetSensorPara(&SelectSensor, pMenu->ReservedBuff);
		}
		
		pMenu->ScreenCMD = SCREEN_CMD_NULL;
		
		Mid_TFTLCD_ScreenClear();
		Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_MAIN_TITLE_X, COOR_MENU_SENSORPARA_OPTION_MAIN_TITLE_Y, SelectSensor.SensorName, LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
		
		KeyPress = 0xFF;
		
		pSensorPara_OptionMainMenu = &SensorParaMenu[SENSORPARA_MENU_REVIEW];	// point to the Review option by default
		pSensorPara_OptionMainMenuFirstOption = pSensorPara_OptionMainMenu;
		pSensorPara_OptionMainMenuBackup = 0;
		ScreenRefreshFlag = 1;
		OptionMainMenuCursorPos = 1;
	
	}
	/* return SensorPara_OptionMainMeu from submenu */
	else if(pMenu->ScreenCMD == SCREEN_CMD_RECOVER)
	{
		if(Device_CheckSensorExistence(pMenu->ReservedBuff))	// ReservedBuff stores the SensorIndex
		{
			Device_GetSensorPara(&SelectSensor, pMenu->ReservedBuff);
		}
		
		pMenu->ScreenCMD = SCREEN_CMD_NULL;
		
		Mid_TFTLCD_ScreenClear();
		Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_MAIN_TITLE_X, COOR_MENU_SENSORPARA_OPTION_MAIN_TITLE_Y, SelectSensor.SensorName, LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
		
		KeyPress = 0xFF;
		
		pSensorPara_OptionMainMenuBackup = 0;
		ScreenRefreshFlag = 1;
	}
	
	/*  SensorPara_OptionMainMeu KeyScan */
	if(pMenu->KeyIndex != 0xFF)
	{
		KeyPress = pMenu->KeyIndex;
		pMenu->KeyIndex = 0xFF;
		KeyEvent = pMenu->KeyEvent;
		
		if(KeyEvent == KEY_CLICK)
		{
			switch((uint8_t)KeyPress)
			{
				case KEY_MENU_CONFIRM:
				{
					pSensorPara_OptionMainMenu->ReservedBuff = pMenu->ReservedBuff;
					pMenu = pSensorPara_OptionMainMenu;
					pMenu->ScreenCMD = SCREEN_CMD_RESET;
					pMenu->pParent = &SensorParaMenu[SENSORPARA_MENU_MAIN];
				}
				break;
				
				case KEY_CALL_CANCEL:
				{
					pMenu = &ListMenu[LIST_MENU_SENSOR_PARA];
					pMenu->ScreenCMD = SCREEN_CMD_RECOVER;
				}
				break;
				
				case KEY_2_UP:
				{
					if(OptionMainMenuCursorPos == 1)
					{
						/* cancel selection of current option */
						Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_MAIN_OPTION_X, (20 + 32 * OptionMainMenuCursorPos), pSensorPara_OptionMainMenu->pMenuTitle, LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
						pSensorPara_OptionMainMenu = pSensorPara_OptionMainMenu->pPrevious;

						OptionMainMenuCursorPos = SENSORPARA_MENU_SUM - 1;
					}
					else
					{
						/* cancel selection of current option */
						Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_MAIN_OPTION_X, (20 + 32 * OptionMainMenuCursorPos), pSensorPara_OptionMainMenu->pMenuTitle, LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
						pSensorPara_OptionMainMenu = pSensorPara_OptionMainMenu->pPrevious;
						OptionMainMenuCursorPos--;
					}
				}
				break;
				
				case KEY_8_DOWN:
				{
					if(OptionMainMenuCursorPos == (SENSORPARA_MENU_SUM - 1))
					{
						/* cancel selection of current option */
						Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_MAIN_OPTION_X, (20 + 32 * OptionMainMenuCursorPos), pSensorPara_OptionMainMenu->pMenuTitle, LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
						pSensorPara_OptionMainMenu = pSensorPara_OptionMainMenu->pNext;

						OptionMainMenuCursorPos = 1;
					}
					else
					{
						/* cancel selection of current option */
						Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_MAIN_OPTION_X, (20 + 32 * OptionMainMenuCursorPos), pSensorPara_OptionMainMenu->pMenuTitle, LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
						pSensorPara_OptionMainMenu = pSensorPara_OptionMainMenu->pNext;
						OptionMainMenuCursorPos++;
					
					}
				}
				break;
				
			}
		}
	}
	
	/* update screen display */
	if(pSensorPara_OptionMainMenuBackup != pSensorPara_OptionMainMenu)
	{
		pSensorPara_OptionMainMenuBackup = pSensorPara_OptionMainMenu;
		
		if(ScreenRefreshFlag)
		{
			ScreenRefreshFlag = 0;
			
			pSensorPara_OptionMainMenu = pSensorPara_OptionMainMenuFirstOption;
			
			for(i=0; i<SENSORPARA_MENU_SUM-1; i++)	// display all options 
			{
				Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_MAIN_OPTION_X, (20 + 32 * (i + 1)), pSensorPara_OptionMainMenu->pMenuTitle, LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
			
				pSensorPara_OptionMainMenu = pSensorPara_OptionMainMenu->pNext;
			}
			
			pSensorPara_OptionMainMenu = pSensorPara_OptionMainMenuBackup;
			
			/* highlight the selected option */
			Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_MAIN_OPTION_X, (20 + 32 * OptionMainMenuCursorPos), pSensorPara_OptionMainMenu->pMenuTitle, LCD_FONT_COLOR, LCD_FONT_BACK_COLOR, 24, 0);
		}
		else
		{
			Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_MAIN_OPTION_X, (20 + 32 * OptionMainMenuCursorPos), pSensorPara_OptionMainMenu->pMenuTitle, LCD_FONT_COLOR, LCD_FONT_BACK_COLOR, 24, 0);
		}
		
		TimeoutCounter_ReturnDesktop = 0;
		TimeoutCounter_ScreenSleep = 0;
	}
}

/**
  * @Brief	SensorParaMenu ReviewHandler for call-back function
  * @Param	None
  * @Retval	None
  */
static void App_SensorParaMenu_ReviewHandler(void)
{
	uint8_t KeyPress;		// indicate the index of pressed key
	uint8_t KeyEvent;
	
	uint8_t TempBuff[25];
	
	stu_Sensor_t SelectSensor;
	
	/* enter SensorPara_OptionReviewMeu from SensorPara_OptionMainMeu */
	if(pMenu->ScreenCMD == SCREEN_CMD_RESET)
	{
		TimeoutCounter_ReturnDesktop = 0;
		TimeoutCounter_ScreenSleep = 0;
		
		if(Device_CheckSensorExistence(pMenu->ReservedBuff))	// ReservedBuff stores the SensorIndex
		{
			Device_GetSensorPara(&SelectSensor, pMenu->ReservedBuff);
		}
		
		pMenu->ScreenCMD = SCREEN_CMD_NULL;
		
		KeyPress = 0xFF;
		
		Mid_TFTLCD_ScreenClear();
		
		Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_REVIEW_TITLE_X, COOR_MENU_SENSORPARA_OPTION_REVIEW_TITLE_Y, pMenu->pMenuTitle, LCD_FONT_COLOR, LCD_FONT_BACK_COLOR, 32, 0);
		Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_1_NAME_X, COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_1_NAME_Y, "<Name>      : ", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_2_NAME_X, COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_2_NAME_Y, "<SensorType>: ", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_3_NAME_X, COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_3_NAME_Y, "<ArmType>   : ", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_4_NAME_X, COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_4_NAME_Y, "<Node ID>   : ", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_5_NAME_X, COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_5_NAME_Y, "<LastOnline>: ", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		
		Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_1_DATA_X, COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_1_DATA_Y, SelectSensor.SensorName, LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		
		switch((uint8_t)SelectSensor.Sensor_Type)
		{
			case SENSOR_TYPE_DOOR:
			{
				Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_2_DATA_X, COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_2_DATA_Y, "Door Sensor", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
			}
			break;
			
			case SENSOR_TYPE_REMOTE:
			{
				Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_2_DATA_X, COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_2_DATA_Y, "Remote", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
			}
			break;
		}
		
		switch((uint8_t)SelectSensor.Sensor_ArmType)
		{
			case SENSOR_ARMTYPE_0_24HOURS:
			{
				Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_3_DATA_X, COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_3_DATA_Y, "24hr_ARM", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
			}
			break;
			
			case SENSOR_ARMTYPE_1_AWAYARM:
			{
				Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_3_DATA_X, COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_3_DATA_Y, "AWAY_ARM", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
			}
			break;
			
			case SENSOR_ARMTYPE_2_HOMEARM:
			{
				Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_3_DATA_X, COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_3_DATA_Y, "HOME_ARM", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
			}
			break;
		}
		
		/* get the NodeID in ASCII */
		Hex_ASCII_Conversion(&SelectSensor.Node[0], &TempBuff[0], 2);
		TempBuff[4] = 0; // add '\0' as the terminator of string
		
		Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_4_DATA_X, COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_4_DATA_Y, &TempBuff[0], LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		
		/* get the sleeptime */
		TempBuff[0] = SelectSensor.SleepTime / 3600;		// hour
		TempBuff[1] = SelectSensor.SleepTime % 3600 / 60;	// minute
		
		TempBuff[2] = TempBuff[0] / 10 + '0';
		TempBuff[3] = TempBuff[0] % 10 + '0';
		TempBuff[4] = ':';
		TempBuff[5] = TempBuff[1] / 10 + '0';
		TempBuff[6] = TempBuff[1] % 10 + '0';
		TempBuff[7] = 0;
		
		Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_5_DATA_X, COOR_MENU_SENSORPARA_OPTION_REVIEW_PARA_5_DATA_Y, &TempBuff[2], LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
	}
	
	/* SensorPara_OptionReviewMeu KeyScan */
	if(pMenu->KeyIndex != 0xFF)
	{
		TimeoutCounter_ReturnDesktop = 0;
		TimeoutCounter_ScreenSleep = 0;
		
		KeyPress = pMenu->KeyIndex;
		pMenu->KeyIndex = 0xFF;
		KeyEvent = pMenu->KeyEvent;
		
		if(KeyEvent == KEY_CLICK)
		{
			switch((uint8_t)KeyPress)
			{
				case KEY_CALL_CANCEL:
				{
					pMenu = pMenu->pParent;
					pMenu->ScreenCMD = SCREEN_CMD_RECOVER;
				}
				break;
				
			}
		}
	}
}

/**
  * @Brief	SensorParaMenu EditHandler for call-back function
  * @Param	None
  * @Retval	None
  */
static void App_SensorParaMenu_EditHandler(void)
{
	uint8_t KeyPress;		// indicate the index of pressed key
	uint8_t KeyEvent;
	
	uint8_t TempBuff[25];
	
	static stu_Sensor_t SelectSensor;
	static uint8_t SetValueBuff = 0;
	static uint8_t EditCompleteFlag = 0;
	static uint16_t Counter = 0;
	static uint8_t *pArmTypeValue[SENSOR_ARMTYPE_SUM] = {
		"24hr_ARM", 
		"AWAY_ARM", 
		"HOME_ARM"
	};
	
	/* enter SensorPara_OptionEditMeu from SensorPara_OptionMainMeu */
	if(pMenu->ScreenCMD == SCREEN_CMD_RESET)
	{
		TimeoutCounter_ReturnDesktop = 0;
		TimeoutCounter_ScreenSleep = 0;
		
		if(Device_CheckSensorExistence(pMenu->ReservedBuff))				// ReservedBuff stores the SensorIndex
		{
			Device_GetSensorPara(&SelectSensor, pMenu->ReservedBuff);
		}
		
		pMenu->ScreenCMD = SCREEN_CMD_NULL;
		
		KeyPress = 0xFF;
		
		Mid_TFTLCD_ScreenClear();
		
		Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_EDIT_TITLE_X, COOR_MENU_SENSORPARA_OPTION_EDIT_TITLE_Y, pMenu->pMenuTitle, LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
		Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_1_NAME_X, COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_1_NAME_Y, "<Name>      : ", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_2_NAME_X, COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_2_NAME_Y, "<SensorType>: ", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_3_NAME_X, COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_3_NAME_Y, "<ArmType>   : ", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_4_NAME_X, COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_4_NAME_Y, "<Node ID>   : ", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_5_NAME_X, COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_5_NAME_Y, "<LastOnline>: ", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
														 
		Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_1_DATA_X, COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_1_DATA_Y, SelectSensor.SensorName, LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		
		switch((uint8_t)SelectSensor.Sensor_Type)
		{
			case SENSOR_TYPE_DOOR:
			{
				Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_2_DATA_X, COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_2_DATA_Y, "Door Sensor", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
			}
			break;
			
			case SENSOR_TYPE_REMOTE:
			{
				Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_2_DATA_X, COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_2_DATA_Y, "Remote", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
			}
			break;
		}
		
		switch((uint8_t)SelectSensor.Sensor_ArmType)
		{
			case SENSOR_ARMTYPE_0_24HOURS:
			{
				Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_3_DATA_X, COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_3_DATA_Y, "24H_ARM", LCD_FONT_COLOR, LCD_FONT_BACK_COLOR, 24, 0);
			}
			break;
			
			case SENSOR_ARMTYPE_1_AWAYARM:
			{
				Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_3_DATA_X, COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_3_DATA_Y, "AWAY_ARM", LCD_FONT_COLOR, LCD_FONT_BACK_COLOR, 24, 0);
			}
			break;
			
			case SENSOR_ARMTYPE_2_HOMEARM:
			{
				Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_3_DATA_X, COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_3_DATA_Y, "HOME_ARM", LCD_FONT_COLOR, LCD_FONT_BACK_COLOR, 24, 0);
			}
			break;
		}
		
		/* get the NodeID in ASCII */
		Hex_ASCII_Conversion(&SelectSensor.Node[0], &TempBuff[0], 2);
		TempBuff[4] = 0; // add '\0' as the terminator of string
		
		Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_4_DATA_X, COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_4_DATA_Y, &TempBuff[0], LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		
		/* get the sleeptime */
		TempBuff[0] = SelectSensor.SleepTime / 3600;		// hour
		TempBuff[1] = SelectSensor.SleepTime % 3600 / 60;	// minute
		
		TempBuff[2] = TempBuff[0] / 10 + '0';
		TempBuff[3] = TempBuff[0] % 10 + '0';
		TempBuff[4] = ':';
		TempBuff[5] = TempBuff[1] / 10 + '0';
		TempBuff[6] = TempBuff[1] % 10 + '0';
		TempBuff[7] = 0;
		
		Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_5_DATA_X, COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_5_DATA_Y, &TempBuff[2], LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		
		SetValueBuff = SelectSensor.Sensor_ArmType;
		EditCompleteFlag = 0;
		Counter = 0;
	}
	
	/* SensorPara_OptionEditMeu KeyScan */
	if(pMenu->KeyIndex != 0xFF)
	{
		TimeoutCounter_ReturnDesktop = 0;
		TimeoutCounter_ScreenSleep = 0;
		
		KeyPress = pMenu->KeyIndex;
		pMenu->KeyIndex = 0xFF;
		KeyEvent = pMenu->KeyEvent;
		
		if(KeyEvent == KEY_CLICK)
		{
			switch((uint8_t)KeyPress)
			{
				case KEY_MENU_CONFIRM:
				{
					Counter = 0;
					EditCompleteFlag = 1;
					
					App_UpdateSensorPara(SelectSensor.ID-1, &SelectSensor);
					
					Mid_TFTLCD_ScreenClear();
					Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_EDIT_UPDATE_X, COOR_MENU_SENSORPARA_OPTION_EDIT_UPDATE_Y, "Updating...", LCD_FONT_COLOR, LCD_BACK_COLOR, 32,0);
				}
				break;
				
				case KEY_CALL_CANCEL:
				{
					pMenu = pMenu->pParent;
					pMenu->ScreenCMD = SCREEN_CMD_RECOVER;
				}
				break;
				
				case KEY_4_LEFT:
				{
					if(SetValueBuff == (SENSOR_ARMTYPE_0_24HOURS))
					{
						SetValueBuff = (SENSOR_ARMTYPE_SUM - 1);
					}
					else
					{
						SetValueBuff--;
					}
					
					SelectSensor.Sensor_ArmType = (en_Sensor_ArmType_t)SetValueBuff;
					Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_3_DATA_X, COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_3_DATA_Y, pArmTypeValue[SetValueBuff], LCD_FONT_COLOR, LCD_FONT_BACK_COLOR, 24, 0);
				}
				break;
				
				case KEY_6_RIGHT:
				{
					if(SetValueBuff == (SENSOR_ARMTYPE_SUM - 1))
					{
						SetValueBuff = SENSOR_ARMTYPE_0_24HOURS;
					}
					else
					{
						SetValueBuff++;
					}
					
					SelectSensor.Sensor_ArmType = (en_Sensor_ArmType_t)SetValueBuff;
					Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_3_DATA_X, COOR_MENU_SENSORPARA_OPTION_EDIT_PARA_3_DATA_Y, pArmTypeValue[SetValueBuff], LCD_FONT_COLOR, LCD_FONT_BACK_COLOR, 24, 0);
				}
				break;
			}
		}
	}
	
	if(EditCompleteFlag)
	{
		Counter++;
		
		if(Counter > 150)				// display "Updating..." for 1.5s
		{
			Counter = 0;
			EditCompleteFlag = 0;
			
			pMenu = pMenu->pParent;
			pMenu->ScreenCMD = SCREEN_CMD_RECOVER;
		}
	}
}

/**
  * @Brief	SensorParaMenu DeleteHandler for call-back function
  * @Param	None
  * @Retval	None
  */
static void App_SensorParaMenu_DeleteHandler(void)
{
	uint8_t KeyPress;		// indicate the index of pressed key
	uint8_t KeyEvent;
	
	static uint16_t Counter;
	static uint8_t DeleteCompleteFlag = 0;
	static uint8_t SensorDeleteMenuCursorPos = 0;
	static stu_Sensor_t SelectSensor;
	
	/* enter SensorPara_OptionDeleteMeu from SensorPara_OptionMainMeu */
	if(pMenu->ScreenCMD == SCREEN_CMD_RESET)
	{
		TimeoutCounter_ReturnDesktop = 0;
		TimeoutCounter_ScreenSleep = 0;
		
		if(Device_CheckSensorExistence(pMenu->ReservedBuff))				// ReservedBuff stores the SensorIndex
		{
			Device_GetSensorPara(&SelectSensor, pMenu->ReservedBuff);
		}
		
		pMenu->ScreenCMD = SCREEN_CMD_NULL;
		
		KeyPress = 0xFF;
		
		Mid_TFTLCD_ScreenClear();
		Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_DELETE_TITLE_X, COOR_MENU_SENSORPARA_OPTION_DELETE_TITLE_Y, pMenu->pMenuTitle, LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);	
		Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_DELETE_PROMPT_1_X, COOR_MENU_SENSORPARA_OPTION_DELETE_PROMPT_1_Y, "Delete ", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_DELETE_SENSORNAME_X, COOR_MENU_SENSORPARA_OPTION_DELETE_SENSORNAME_Y, SelectSensor.SensorName, LCD_FONT_COLOR, LCD_PARA_BACK_COLOR, 24, 0);
		Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_DELETE_PROMPT_2_X, COOR_MENU_SENSORPARA_OPTION_DELETE_PROMPT_2_Y, "Are You Sure?", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_DELETE_YES_X, COOR_MENU_SENSORPARA_OPTION_DELETE_YES_Y, "YES", LCD_FONT_COLOR, LCD_FONT_BACK_COLOR, 24, 0);
		Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_DELETE_NO_X, COOR_MENU_SENSORPARA_OPTION_DELETE_NO_Y, "NO", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		
		SensorDeleteMenuCursorPos = UPDATE_MENU_CURSOR_POS_YES;
		Counter = 0;
		DeleteCompleteFlag = 0;
	}
	
	/* SensorPara_OptionDeleteMeu KeyScan */
	if(pMenu->KeyIndex != 0xFF)
	{
		TimeoutCounter_ReturnDesktop = 0;
		TimeoutCounter_ScreenSleep = 0;
		
		KeyPress = pMenu->KeyIndex;
		pMenu->KeyIndex = 0xFF;
		KeyEvent = pMenu->KeyEvent;
		
		if(!DeleteCompleteFlag)	
		{
			if(KeyEvent == KEY_CLICK)
			{
				switch((uint8_t)KeyPress)
				{
					case KEY_CALL_CANCEL:
					{
						pMenu = pMenu->pParent;
						pMenu->ScreenCMD = SCREEN_CMD_RECOVER;
					}
					break;
					
					case KEY_4_LEFT:
						
					case KEY_6_RIGHT:
					{
						if(SensorDeleteMenuCursorPos == UPDATE_MENU_CURSOR_POS_YES)
						{
							SensorDeleteMenuCursorPos = UPDATE_MENU_CURSOR_POS_NO;
							
							Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_DELETE_YES_X, COOR_MENU_SENSORPARA_OPTION_DELETE_YES_Y, "YES", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
							Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_DELETE_NO_X, COOR_MENU_SENSORPARA_OPTION_DELETE_NO_Y, "NO", LCD_FONT_COLOR, LCD_FONT_BACK_COLOR, 24, 0);
						}
						else if(SensorDeleteMenuCursorPos == UPDATE_MENU_CURSOR_POS_NO)
						{
							SensorDeleteMenuCursorPos = UPDATE_MENU_CURSOR_POS_YES;
							
							Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_DELETE_YES_X, COOR_MENU_SENSORPARA_OPTION_DELETE_YES_Y, "YES", LCD_FONT_COLOR, LCD_FONT_BACK_COLOR, 24, 0);
							Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_DELETE_NO_X, COOR_MENU_SENSORPARA_OPTION_DELETE_NO_Y, "NO", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
						}
					}
					break;
					
					case KEY_MENU_CONFIRM:
					{
						if(SensorDeleteMenuCursorPos == UPDATE_MENU_CURSOR_POS_YES)
						{
							DeleteCompleteFlag = 1;
							Counter = 0;
							SelectSensor.PairFlag = 0;
							
							App_UpdateSensorPara(SelectSensor.ID-1, &SelectSensor);
							
							Mid_TFTLCD_ScreenClear();
							Mid_TFTLCD_ShowString(COOR_MENU_SENSORPARA_OPTION_DELETE_UPDATE_X, COOR_MENU_SENSORPARA_OPTION_DELETE_UPDATE_Y, "Updating...", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
						}
						else if(SensorDeleteMenuCursorPos == UPDATE_MENU_CURSOR_POS_NO)
						{
							pMenu = pMenu->pParent;
							pMenu->ScreenCMD = SCREEN_CMD_RECOVER;
						}
					}
					break;
					
				}
			}
		}				
	}
	
	/* update screen display */
	if(DeleteCompleteFlag)
	{
		Counter++;
		
		if(Counter > 150)
		{
			Counter = 0;
			DeleteCompleteFlag = 0;
			
			pMenu = &ListMenu[LIST_MENU_SENSOR_PARA];
			pMenu->ScreenCMD = SCREEN_CMD_RESET;
		}
	}
}

/**
  * @Brief	ListMenu-WiFiNetPair Menu Handler
  * @Param	None
  * @Retval	None
  */
static void App_ListMenu_WiFiNetPairHandler(void)
{
	uint8_t KeyPress;		// indicate the index of pressed key
	uint8_t KeyEvent;
	
	static uint8_t WiFiNetPairCursorPos = 0;
	
	/* enter WiFiNetPair from MainMenu */
	if(pMenu->ScreenCMD == SCREEN_CMD_RESET)
	{
		TimeoutCounter_ReturnDesktop = 0;
		TimeoutCounter_ScreenSleep = 0;
		
		pMenu->ScreenCMD = SCREEN_CMD_NULL;
		
		Mid_TFTLCD_ScreenClear();
		Mid_TFTLCD_ShowString(COOR_MENU_WIFI_NETPAIR_TITLE_X, COOR_MENU_WIFI_NETPAIR_TITLE_Y, 	"    WiFi NetPair    ", LCD_FONT_COLOR, LCD_FONT_BACK_COLOR, 	32, 0);
		Mid_TFTLCD_ShowString(COOR_MENU_WIFI_NETPAIR_PROMPT_X, COOR_MENU_WIFI_NETPAIR_PROMPT_Y, "Start WiFi NetPair?", 	LCD_FONT_COLOR, LCD_BACK_COLOR, 		24, 0);
		Mid_TFTLCD_ShowString(COOR_MENU_WIFI_NETPAIR_YES_X, COOR_MENU_WIFI_NETPAIR_YES_Y, 		"YES", 					LCD_FONT_COLOR, LCD_FONT_BACK_COLOR, 	24, 0);
		Mid_TFTLCD_ShowString(COOR_MENU_WIFI_NETPAIR_NO_X, COOR_MENU_WIFI_NETPAIR_NO_Y, 		"NO", 					LCD_FONT_COLOR, LCD_BACK_COLOR, 		24, 0);
		
		KeyPress = 0xFF;
		WiFiNetPairCursorPos = WIFI_MENU_CURSOR_POS_YES;
	}
	
	/* WiFiNetPair Menu KeyScan */
	if(pMenu->KeyIndex != 0xFF)
	{
		TimeoutCounter_ReturnDesktop = 0;
		TimeoutCounter_ScreenSleep = 0;
		
		KeyPress = pMenu->KeyIndex;
		pMenu->KeyIndex = 0xFF;
		KeyEvent = pMenu->KeyEvent;
		
		if(KeyEvent == KEY_CLICK)
		{
			switch((uint8_t)KeyPress)
			{
				case KEY_MENU_CONFIRM:
				{
				
				}
				break;
				
				case KEY_CALL_CANCEL:
				{
					pMenu = pMenu->pParent;
					pMenu->ScreenCMD = SCREEN_CMD_RECOVER;
				}
				break;
				
				case KEY_4_LEFT:
					
				case KEY_6_RIGHT:
				{
					if(WiFiNetPairCursorPos == WIFI_MENU_CURSOR_POS_YES)
					{
						WiFiNetPairCursorPos = WIFI_MENU_CURSOR_POS_NO;
						
						Mid_TFTLCD_ShowString(COOR_MENU_WIFI_NETPAIR_YES_X, COOR_MENU_WIFI_NETPAIR_YES_Y, "YES", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
						Mid_TFTLCD_ShowString(COOR_MENU_WIFI_NETPAIR_NO_X, COOR_MENU_WIFI_NETPAIR_NO_Y, "NO", LCD_FONT_COLOR, LCD_FONT_BACK_COLOR, 24, 0);
					}
					else if(WiFiNetPairCursorPos == WIFI_MENU_CURSOR_POS_NO)
					{
						WiFiNetPairCursorPos = WIFI_MENU_CURSOR_POS_YES;
						
						Mid_TFTLCD_ShowString(COOR_MENU_WIFI_NETPAIR_YES_X, COOR_MENU_WIFI_NETPAIR_YES_Y, "YES", LCD_FONT_COLOR, LCD_FONT_BACK_COLOR, 24, 0);
						Mid_TFTLCD_ShowString(COOR_MENU_WIFI_NETPAIR_NO_X, COOR_MENU_WIFI_NETPAIR_NO_Y, "NO", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
					}
				}
				break;
				
			}
		}
	}
	
}

/**
  * @Brief	ListMenu-AdminPassword Menu Handler
  * @Param	None
  * @Retval	None
  */
static void App_ListMenu_AdminPasswordHandler(void)
{
	uint8_t i;
	
	uint8_t KeyPress;		// indicate the index of pressed key
	uint8_t KeyEvent;
	
	uint8_t KeyPressValue = 0;	// store the numberic value of pressed key
	uint8_t KeyPressFlag = 0;		// 0->no valid key pressed, 1->valid key pressed
	
	static uint8_t PasswordCursorPos_X = 0;
	static uint8_t PasswordCursorPos_Y = 0;
	
	static uint16_t BlinkCounter = 0;
	static uint8_t BlinkFlag = 0;
	
	static uint16_t Counter = 0;
	static uint8_t UpdatePasswordState = UPDATE_PASSWORD_STATE_IN_PROCESS;
	static uint8_t Password_New[4];
	static uint8_t Password_Confirm[4];
	
	/* enter AdminPassword from MainMenu */
	if(pMenu->ScreenCMD == SCREEN_CMD_RESET)
	{
		TimeoutCounter_ReturnDesktop = 0;
		TimeoutCounter_ScreenSleep = 0;
		
		pMenu->ScreenCMD = SCREEN_CMD_NULL;
		
		KeyPress = 0xFF;
		
		Mid_TFTLCD_ScreenClear();
		Mid_TFTLCD_ShowString(COOR_MENU_ADMIN_PASSWORD_TITLE_X, COOR_MENU_ADMIN_PASSWORD_TITLE_Y, "    Password    ", LCD_FONT_COLOR, LCD_FONT_BACK_COLOR, 32, 0);
		Mid_TFTLCD_ShowString(COOR_MENU_ADMIN_PASSWORD_PROMPT_1_X, COOR_MENU_ADMIN_PASSWORD_PROMPT_1_Y, "New Pin: ", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
		Mid_TFTLCD_ShowString(COOR_MENU_ADMIN_PASSWORD_PROMPT_2_X, COOR_MENU_ADMIN_PASSWORD_PROMPT_2_Y, "Confirm: ", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);

		BlinkCounter = 0;
		BlinkFlag = 0;
		
		PasswordCursorPos_X = 0;
		PasswordCursorPos_Y = COOR_MENU_ADMIN_PASSWORD_PROMPT_1_Y;
		
		UpdatePasswordState = UPDATE_PASSWORD_STATE_IN_PROCESS;
		
		Password_New[0] = ' ';
		Password_New[1] = ' ';
		Password_New[2] = ' ';
		Password_New[3] = ' ';
		
		Password_Confirm[0] = ' ';
		Password_Confirm[1] = ' ';
		Password_Confirm[2] = ' ';
		Password_Confirm[3] = ' ';
		
		Counter = 0;
	}
	
	/* Enter updating password */
	if(UpdatePasswordState == UPDATE_PASSWORD_STATE_IN_PROCESS)
	{
		BlinkCounter++;
		
		if(BlinkCounter > 30)
		{
			BlinkCounter = 0;
			BlinkFlag = !BlinkFlag;
			
			if(PasswordCursorPos_X < 4)
			{
				if(BlinkFlag)
				{
					Mid_TFTLCD_ShowString(COOR_MENU_ADMIN_PASSWORD_CURSOR_X_BASE + (32 * PasswordCursorPos_X), PasswordCursorPos_Y, " ", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
				}
				else
				{
					Mid_TFTLCD_ShowString(COOR_MENU_ADMIN_PASSWORD_CURSOR_X_BASE + (32 * PasswordCursorPos_X), PasswordCursorPos_Y, "_", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
				}
			}	
		}
	}
	
	/* AdminPassword Menu KeyScan */
	if(pMenu->KeyIndex != 0xFF)
	{
		TimeoutCounter_ReturnDesktop = 0;
		TimeoutCounter_ScreenSleep = 0;
		
		KeyPress = pMenu->KeyIndex;
		pMenu->KeyIndex = 0xFF;
		KeyEvent = pMenu->KeyEvent;
		
		if(KeyEvent == KEY_CLICK)
		{
			switch((uint8_t)KeyPress)
			{
				case KEY_MENU_CONFIRM:
				{
					if(PasswordCursorPos_Y == COOR_MENU_ADMIN_PASSWORD_PROMPT_1_Y)
					{
						if(PasswordCursorPos_X == 4)
						{
							Mid_TFTLCD_ShowString(COOR_MENU_ADMIN_PASSWORD_CURSOR_X_BASE + (32 * PasswordCursorPos_X), PasswordCursorPos_Y, " ", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
							
							PasswordCursorPos_X = 0;
							PasswordCursorPos_Y = COOR_MENU_ADMIN_PASSWORD_PROMPT_2_Y;
							
						}
					}
					else if(PasswordCursorPos_Y == COOR_MENU_ADMIN_PASSWORD_PROMPT_2_Y)
					{
						if(PasswordCursorPos_X == 4)
						{
							if( (Password_New[0] == Password_Confirm[0]) &&
								(Password_New[1] == Password_Confirm[1]) &&
								(Password_New[2] == Password_Confirm[2]) &&
								(Password_New[3] == Password_Confirm[3])	)
							{
								Counter = 0;
								UpdatePasswordState = UPDATE_PASSWORD_STATE_SUCCESS;
								
								Mid_TFTLCD_ShowString(COOR_MENU_ADMIN_PASSWORD_PROMPT_3_X, COOR_MENU_ADMIN_PASSWORD_PROMPT_3_Y, " Updating... ", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
							}
							else
							{
								Counter = 0;
								UpdatePasswordState = UPDATE_PASSWORD_STATE_FAIL;
							
								Mid_TFTLCD_ShowString(COOR_MENU_ADMIN_PASSWORD_PROMPT_3_X, COOR_MENU_ADMIN_PASSWORD_PROMPT_3_Y, " Invalid Pin ", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
							}
						}
					}
				}
				break;
				
				case KEY_CALL_CANCEL:
				{
					pMenu = &ListMenu[LIST_MENU_MAIN];
					pMenu->ScreenCMD = SCREEN_CMD_RECOVER;
				}
				break;
				
				case KEY_SOS_DELET:
				{
					if(PasswordCursorPos_X > 0)
					{
						Mid_TFTLCD_ShowString(COOR_MENU_ADMIN_PASSWORD_CURSOR_X_BASE + (32 * PasswordCursorPos_X), PasswordCursorPos_Y, " ", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
						
						if(PasswordCursorPos_Y == COOR_MENU_ADMIN_PASSWORD_PROMPT_1_Y)
						{
							Password_New[PasswordCursorPos_X] = ' ';
						}
						else if(PasswordCursorPos_Y == COOR_MENU_ADMIN_PASSWORD_PROMPT_2_Y)
						{
							Password_Confirm[PasswordCursorPos_X] = ' ';
						}
						
						PasswordCursorPos_X--;
					}
				}
				break;
				
				case KEY_0:
				{
					KeyPressValue = '0';
					KeyPressFlag = 1;
				}
				break;
				
				case KEY_1:
				{
					KeyPressValue = '1';
					KeyPressFlag = 1;
				}
				break;
				
				case KEY_2_UP:
				{
					KeyPressValue = '2';
					KeyPressFlag = 1;
				}
				break;
				
				case KEY_3:
				{
					KeyPressValue = '3';
					KeyPressFlag = 1;
				}
				break;
				
				case KEY_4_LEFT:
				{
					KeyPressValue = '4';
					KeyPressFlag = 1;
				}
				break;
				
				case KEY_5:
				{
					KeyPressValue = '5';
					KeyPressFlag = 1;
				}
				break;
				
				case KEY_6_RIGHT:
				{
					KeyPressValue = '6';
					KeyPressFlag = 1;
				}
				break;
				
				case KEY_7:
				{
					KeyPressValue = '7';
					KeyPressFlag = 1;
				}
				break;
				
				case KEY_8_DOWN:
				{
					KeyPressValue = '8';
					KeyPressFlag = 1;
				}
				break;
				
				case KEY_9:
				{
					KeyPressValue = '9';
					KeyPressFlag = 1;
				}
				break;
				
				
			}
		}
	}
	
	/* password enter not complete, save updated Pin and move cursor forward */
	if(PasswordCursorPos_X < 4)
	{
		if(KeyPressFlag)
		{
			KeyPressFlag = 0;
			BlinkCounter = 0;
			
			Mid_TFTLCD_ShowString(COOR_MENU_ADMIN_PASSWORD_CURSOR_X_BASE + (32 * PasswordCursorPos_X), PasswordCursorPos_Y, "*", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
			
			if(PasswordCursorPos_Y == COOR_MENU_ADMIN_PASSWORD_PROMPT_1_Y)
			{
				Password_New[PasswordCursorPos_X] = KeyPressValue;
				PasswordCursorPos_X++;
			}
			else if(PasswordCursorPos_Y == COOR_MENU_ADMIN_PASSWORD_PROMPT_2_Y)
			{
				Password_Confirm[PasswordCursorPos_X] = KeyPressValue;
				PasswordCursorPos_X++;
			}	
		}
	}
	/* Password fill up, stop cursor move forward */
	else
	{
		KeyPressFlag = 0;
	}
	
	/* update screen display */
	Counter++;
	
	if(Counter > 150)
	{
		Counter = 0;
		
		if(UpdatePasswordState == UPDATE_PASSWORD_STATE_SUCCESS)
		{
			UpdatePasswordState = UPDATE_PASSWORD_STATE_IN_PROCESS;
			
			for(i=0; i<4; i++)
			{
				Device_Set_SystemPara_AdminPassword(i, (Password_New[i] - '0'));
			}
			
			/* update the new Password into EEPROM */
			Mid_EEPROM_PageWrite(EEPROM_ADDRESS_SYSTEMPARA_OFFSET, (uint8_t *)(&stu_SystemPara), sizeof(stu_SystemPara));
			
			pMenu = pMenu->pParent;
			pMenu->ScreenCMD = SCREEN_CMD_RECOVER;
		}
		else if(UpdatePasswordState == UPDATE_PASSWORD_STATE_FAIL)
		{
			UpdatePasswordState = UPDATE_PASSWORD_STATE_IN_PROCESS;
			
			pMenu->ScreenCMD = SCREEN_CMD_RESET;
		}
	}
}

/**
  * @Brief	ListMenu-Phonenumber Menu Handler
  * @Param	None
  * @Retval	None
  */
static void App_ListMenu_PhoneNumberHandler(void)
{
	uint8_t KeyPress;		// indicate the index of pressed key
	uint8_t KeyEvent;
	
	/* enter PhoneNumber menu from MainMenu */
	if(pMenu->ScreenCMD == SCREEN_CMD_RESET)
	{
		TimeoutCounter_ReturnDesktop = 0;
		TimeoutCounter_ScreenSleep = 0;
		
		pMenu->ScreenCMD = SCREEN_CMD_NULL;
		
		Mid_TFTLCD_ScreenClear();
		Mid_TFTLCD_ShowString(COOR_MENU_PHONENUMBER_TITLE_X, COOR_MENU_PHONENUMBER_TITLE_Y, ListMenu[5].pMenuTitle, LCD_FONT_COLOR, LCD_FONT_BACK_COLOR, 32, 0);
		
		KeyPress = 0xFF;
	}
	
	/* PhoneNumber Menu KeyScan */
	if(pMenu->KeyIndex != 0xFF)
	{
		TimeoutCounter_ReturnDesktop = 0;
		TimeoutCounter_ScreenSleep = 0;
		
		KeyPress = pMenu->KeyIndex;
		pMenu->KeyIndex = 0xFF;
		KeyEvent = pMenu->KeyEvent;
		
		if(KeyEvent == KEY_CLICK)
		{
			switch((uint8_t)KeyPress)
			{
				case KEY_CALL_CANCEL:
				{
					pMenu = &ListMenu[LIST_MENU_MAIN];
					pMenu->ScreenCMD = SCREEN_CMD_RECOVER;
				}
				break;
				
			}
		}
	}
	
}

/**
  * @Brief	ListMenu-TerminalPara Menu Handler, display Termial info(MCU_UID, FirmwareVersion)
  * @Param	None
  * @Retval	None
  */
static void App_ListMenu_TerminalParaHandler(void)
{
	uint8_t i;
	
	uint8_t KeyPress;		// indicate the index of pressed key
	uint8_t KeyEvent;
	
	uint8_t TempBuff[13];
	uint16_t FirmwareVersion;
	uint8_t UID_Buff;
	
	/* enter TerminalPara menu from MainMenu */
	if(pMenu->ScreenCMD == SCREEN_CMD_RESET)
	{
		TimeoutCounter_ReturnDesktop = 0;
		TimeoutCounter_ScreenSleep = 0;
		
		pMenu->ScreenCMD = SCREEN_CMD_NULL;
		
		KeyPress = 0xFF;
		
		Mid_TFTLCD_ScreenClear();
		Mid_TFTLCD_ShowString(COOR_MENU_TERMINAL_PARA_TITLE_X, COOR_MENU_TERMINAL_PARA_TITLE_Y, "    Terminal Info    ", LCD_FONT_COLOR, LCD_FONT_BACK_COLOR, 32, 0);
		Mid_TFTLCD_ShowString(COOR_MENU_TERMINAL_FIRMWAREVERSION_NAME_X, COOR_MENU_TERMINAL_FIRMWAREVERSION_NAME_Y, "<FirmwareVer>: V", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		
		FirmwareVersion = (Device_Get_SystemPara_FirmwareVersion(0) << 8) + Device_Get_SystemPara_FirmwareVersion(1);
		i = 0;
		
		if(FirmwareVersion != 0x00 || FirmwareVersion != 0xFF)
		{
			TempBuff[i++] = (FirmwareVersion % 10000) / 1000 + '0';
			TempBuff[i++] = (FirmwareVersion % 1000) / 100 + '0';
			TempBuff[i++] = '.';
			TempBuff[i++] = (FirmwareVersion % 100) / 10 + '0';
			TempBuff[i++] = (FirmwareVersion % 10) + '0';
			TempBuff[i++] = '\0';
		}
		
		Mid_TFTLCD_ShowString(COOR_MENU_TERMINAL_FIRMWAREVERSION_DATA_X, COOR_MENU_TERMINAL_FIRMWAREVERSION_DATA_Y, TempBuff, LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		Mid_TFTLCD_ShowString(COOR_MENU_TERMINAL_PROTOCOL_NAME_X, COOR_MENU_TERMINAL_PROTOCOL_NAME_Y, "<Protocol ID>: V", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		
		i = 0;
		
		Mid_TFTLCD_ShowString(COOR_MENU_TERMINAL_MCU_UID_NAME_X, COOR_MENU_TERMINAL_MCU_UID_NAME_Y, "<MCU UID>: ", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		
		/* high 6 bytes of MCU_UID */
		for(i=0; i<6; i++)
		{
			UID_Buff = ((Device_Get_MCU_UID(i) >> 4) & 0x0F);	// high 4bits
			
			if(UID_Buff < 10)
			{
				TempBuff[2 * i] = UID_Buff + '0';
			}
			else
			{
				TempBuff[2 * i] = UID_Buff - 10 + 'A';
			}
			
			UID_Buff = (Device_Get_MCU_UID(i) & 0x0F);	// low 4bits
			
			if(UID_Buff < 10)
			{
				TempBuff[(2 * i) + 1] = UID_Buff + '0';
			}
			else
			{
				TempBuff[(2 * i) + 1] = UID_Buff - 10 + 'A';
			}
		}
		
		TempBuff[12] = '\0';
		
		Mid_TFTLCD_ShowString(COOR_MENU_TERMINAL_MCU_UID_DATA_1_X, COOR_MENU_TERMINAL_MCU_UID_DATA_1_Y, TempBuff, LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		
		/* low 6 bytes of MCU_UID */
		for(i=0; i<6; i++)
		{
			UID_Buff = ((Device_Get_MCU_UID(i + 6) >> 4) & 0x0F);	// high 4bits
			
			if(UID_Buff < 10)
			{
				TempBuff[2 * i] = UID_Buff + '0';
			}
			else
			{
				TempBuff[2 * i] = UID_Buff - 10 + 'A';
			}
			
			UID_Buff = (Device_Get_MCU_UID(i + 6) & 0x0F);	// low 4bits
			
			if(UID_Buff < 10)
			{
				TempBuff[(2 * i) + 1] = UID_Buff + '0';
			}
			else
			{
				TempBuff[(2 * i) + 1] = UID_Buff - 10 + 'A';
			}
		}
		
		TempBuff[12] = '\0';
		
		Mid_TFTLCD_ShowString(COOR_MENU_TERMINAL_MCU_UID_DATA_2_X, COOR_MENU_TERMINAL_MCU_UID_DATA_2_Y, TempBuff, LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);

	}
	
	/* TerminalPara Menu KeyScan */
	if(pMenu->KeyIndex != 0xFF)
	{
		TimeoutCounter_ReturnDesktop = 0;
		TimeoutCounter_ScreenSleep = 0;
		
		KeyPress = pMenu->KeyIndex;
		pMenu->KeyIndex = 0xFF;
		KeyEvent = pMenu->KeyEvent;
		
		if(KeyEvent == KEY_CLICK)
		{
			switch((uint8_t)KeyPress)
			{
				case KEY_CALL_CANCEL:
				{
					pMenu = pMenu->pParent;
					pMenu->ScreenCMD = SCREEN_CMD_RECOVER;
				}
				break;
				
			}
		}
	}
	
}

/**
  * @Brief	ListMenu-FactoryReset Menu Handler
  * @Param	None
  * @Retval	None
  */
static void App_ListMenu_FactoryResetHandler(void)
{
	uint8_t KeyPress;		// indicate the index of pressed key
	uint8_t KeyEvent;
	
	static uint8_t CursorPos = 0;
	static uint8_t ResetConfirmFlag = 0;
	static uint16_t Counter = 0;
	
	/* enter FactoryReset menu from MainMenu */
	if(pMenu->ScreenCMD == SCREEN_CMD_RESET)
	{
		TimeoutCounter_ReturnDesktop = 0;
		TimeoutCounter_ScreenSleep = 0;
		
		pMenu->ScreenCMD = SCREEN_CMD_NULL;
		
		KeyPress = 0xFF;
		
		Mid_TFTLCD_ScreenClear();
		Mid_TFTLCD_ShowString(COOR_MENU_FACTORY_RESET_TITLE_X, COOR_MENU_FACTORY_RESET_TITLE_Y, "    FactoryReset    ", LCD_FONT_COLOR, LCD_FONT_BACK_COLOR, 32, 0);
		Mid_TFTLCD_ShowString(COOR_MENU_FACTORY_RESET_PROMPT_X, COOR_MENU_FACTORY_RESET_PROMPT_Y, "Reset Terminal?", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		Mid_TFTLCD_ShowString(COOR_MENU_FACTORY_RESET_YES_X, COOR_MENU_FACTORY_RESET_YES_Y, "YES", LCD_FONT_COLOR, LCD_FONT_BACK_COLOR, 24, 0);
		Mid_TFTLCD_ShowString(COOR_MENU_FACTORY_RESET_NO_X, COOR_MENU_FACTORY_RESET_NO_Y, "NO", LCD_FONT_COLOR, LCD_BACK_COLOR, 24 , 0);

		CursorPos = FACTORY_RESET_MENU_CURSOR_POS_YES;
		ResetConfirmFlag = 0;
		Counter = 0;
	}
	
	/* FactoryReset Menu KeyScan */
	if(pMenu->KeyIndex != 0xFF)
	{
		TimeoutCounter_ReturnDesktop = 0;
		TimeoutCounter_ScreenSleep = 0;
		
		KeyPress = pMenu->KeyIndex;
		pMenu->KeyIndex = 0xFF;
		KeyEvent = pMenu->KeyEvent;
		
		if(KeyEvent == KEY_CLICK)
		{
			switch((uint8_t)KeyPress)
			{
				case KEY_MENU_CONFIRM:
				{
					if(CursorPos == FACTORY_RESET_MENU_CURSOR_POS_YES)
					{
						ResetConfirmFlag = 1;
						
						Mid_TFTLCD_ScreenClear();
						Mid_TFTLCD_ShowString(COOR_MENU_FACTORY_RESET_RESETING_X, COOR_MENU_FACTORY_RESET_RESETING_Y, "Resetting...", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
						
						Device_ParaReset();		// Reset Sensor and Termianl info
					}
					else if(CursorPos == FACTORY_RESET_MENU_CURSOR_POS_NO)
					{
						pMenu = pMenu->pParent;
						pMenu->ScreenCMD = SCREEN_CMD_RECOVER;
					}
				}
				break;
				
				case KEY_CALL_CANCEL:
				{
					pMenu = pMenu->pParent;
					pMenu->ScreenCMD = SCREEN_CMD_RECOVER;
				}
				break;
				
				case KEY_4_LEFT:
					
				case KEY_6_RIGHT:
				{
					if(CursorPos == FACTORY_RESET_MENU_CURSOR_POS_YES)
					{
						CursorPos = FACTORY_RESET_MENU_CURSOR_POS_NO;
						
						Mid_TFTLCD_ShowString(COOR_MENU_FACTORY_RESET_YES_X, COOR_MENU_FACTORY_RESET_YES_Y, "YES", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
						Mid_TFTLCD_ShowString(COOR_MENU_FACTORY_RESET_NO_X, COOR_MENU_FACTORY_RESET_NO_Y, "NO", LCD_FONT_COLOR, LCD_FONT_BACK_COLOR, 24, 0);
					}
					else if(CursorPos == WIFI_MENU_CURSOR_POS_NO)
					{
						CursorPos = FACTORY_RESET_MENU_CURSOR_POS_YES;
						
						Mid_TFTLCD_ShowString(COOR_MENU_FACTORY_RESET_YES_X, COOR_MENU_FACTORY_RESET_YES_Y, "YES", LCD_FONT_COLOR, LCD_FONT_BACK_COLOR, 24, 0);
						Mid_TFTLCD_ShowString(COOR_MENU_FACTORY_RESET_NO_X, COOR_MENU_FACTORY_RESET_NO_Y, "NO", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
					}
				}
				break;
			}
		}
	}
	
	/* update screen display */
	if(ResetConfirmFlag)
	{
		Counter++;
		
		if(Counter > 150)
		{
			Counter = 0;
			
			pMenu = &GeneralMenu[GENERAL_MENU_DESKTOP];
			pMenu->ScreenCMD = SCREEN_CMD_RESET;
		}
	}
}

/**
  * @Brief	ListMenu-AlarmReport Menu Handler
  * @Param	None
  * @Retval	None
  */
static void App_ListMenu_AlarmReportHandler(void)
{
	uint8_t KeyPress;		// indicate the index of pressed key
	uint8_t KeyEvent;
	
	/* enter AlarmReport menu from MainMenu */
	if(pMenu->ScreenCMD == SCREEN_CMD_RESET)
	{
		TimeoutCounter_ReturnDesktop = 0;
		TimeoutCounter_ScreenSleep = 0;
		
		pMenu->ScreenCMD = SCREEN_CMD_NULL;
		
		Mid_TFTLCD_ScreenClear();
		Mid_TFTLCD_ShowString(COOR_MENU_ALARM_REPORT_TITLE_X, COOR_MENU_ALARM_REPORT_TITLE_Y, ListMenu[8].pMenuTitle, LCD_FONT_COLOR, LCD_FONT_BACK_COLOR, 32, 0);
		
		KeyPress = 0xFF;
	}
	
	/* AlarmReport Menu KeyScan */
	if(pMenu->KeyIndex != 0xFF)
	{
		TimeoutCounter_ReturnDesktop = 0;
		TimeoutCounter_ScreenSleep = 0;
		
		KeyPress = pMenu->KeyIndex;
		pMenu->KeyIndex = 0xFF;
		KeyEvent = pMenu->KeyEvent;
		
		if(KeyEvent == KEY_CLICK)
		{
			switch((uint8_t)KeyPress)
			{
				case KEY_CALL_CANCEL:
				{
					pMenu = &ListMenu[LIST_MENU_MAIN];
					pMenu->ScreenCMD = SCREEN_CMD_RECOVER;
				}
				break;
				
			}
		}
	}
	
}



/**
  * @Brief	Keyevent handler for call-back function
  * @Param	KeyIndex: Key index
  *			KeyEvent: Key event
  * @Retval	None
  */
static void App_KeyEvent_Handler(en_KeyType_t KeyIndex, en_KeyEvent_t KeyEvent)
{
	
	#ifdef APP_KEYEVENT_DEBUG_MODE
	uint8_t KeyChar[3];
	
	KeyChar[0] = KeyIndex / 10 + 0x30;	// 0x30 -> '0'
	KeyChar[1] = KeyIndex % 10 + 0x30;
	KeyChar[2] = '\0';
	
	Mid_TFTLCD_ShowString(0, 40, "KEYDBC:", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
	Mid_TFTLCD_ShowString(80, 40, KeyChar, LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
	
	Mid_TFTLCD_ShowString(120, 40, "-KEY-", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
	
	switch((uint8_t)KeyIndex)
	{
		case KEY_0:
		{
			Mid_TFTLCD_ShowString(200, 40, "KEY0", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		}
		break;
		
		case KEY_1:
		{
			Mid_TFTLCD_ShowString(200, 40, "KEY1", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		}
		break;
		
		case KEY_2_UP:
		{
			Mid_TFTLCD_ShowString(200, 40, "KEY2", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		}
		break;
		
		case KEY_3:
		{
			Mid_TFTLCD_ShowString(200, 40, "KEY3", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		}
		break;
		
		case KEY_4_LEFT:
		{
			Mid_TFTLCD_ShowString(200, 40, "KEY4", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		}
		break;
		
		case KEY_5:
		{
			Mid_TFTLCD_ShowString(200, 40, "KEY5", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		}
		break;
		
		case KEY_6_RIGHT:
		{
			Mid_TFTLCD_ShowString(200, 40, "KEY6", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		}
		break;
		
		case KEY_7:
		{
			Mid_TFTLCD_ShowString(200, 40, "KEY7", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		}
		break;
		
		case KEY_8_DOWN:
		{
			Mid_TFTLCD_ShowString(200, 40, "KEY8", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
			
			Mid_WiFi_ChangeModuleWorkState(ESP8266_STA_NETPAIR_START);
		}
		break;
		
		case KEY_9:
		{
			Mid_TFTLCD_ShowString(200, 40, "KEY9", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		}
		break;
		
		case KEY_DISARM:
		{
			Mid_TFTLCD_ShowString(200, 40, "DISA", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		}
		break;
		
		case KEY_HOMEARM:
		{
			Mid_TFTLCD_ShowString(200, 40, "HOME", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		}
		break;
		
		case KEY_AWAYARM:
		{
			Mid_TFTLCD_ShowString(200, 40, "AWAY", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		}
		break;
		
		case KEY_CALL_CANCEL:
		{
			Mid_TFTLCD_ShowString(200, 40, "CANC", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		}
		break;
		
		case KEY_SOS_DELET:
		{
			Mid_TFTLCD_ShowString(200, 40, "DELE", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		}
		break;
		
		case KEY_MENU:
		{
			Mid_TFTLCD_ShowString(200, 40, "MENU", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		}
		break;
	}
	
	KeyChar[0] = KeyEvent / 10 + 0x30;
	KeyChar[1] = KeyEvent % 10 + 0x30;
	KeyChar[2] = '\0';
	
	Mid_TFTLCD_ShowString(0, 65, "KEYSTA:", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
	Mid_TFTLCD_ShowString(80, 65, KeyChar, LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
	#endif

	#ifndef APP_KEYEVENT_DEBUG_MODE
	/* turn on Screen if any key operation: */
	if(!ScreenState)
	{
		App_ScreenControl(1);
	}
	else
	{
		if(pMenu)
		{
			pMenu->KeyIndex = KeyIndex;
			pMenu->KeyEvent = KeyEvent;
			
			TimeoutCounter_ReturnDesktop = 0;
			TimeoutCounter_ScreenSleep = 0;
		}
	}

	
	
	#endif
	
}

/**
  * @Brief	Sensor ApplyNetwork request through Lora-datapackage handler
  * @Param	FunctionCode : the functioncode from Sensor
  *			stu_DataFrame: the Dataframe(FunctionCode, NodeNo, MACAddress, SensorType) from Sensor
  * @Retval	stu_Lora_ApplyNet_SensorPara: 
  *				State 		-> the ApplyNetState(pairing, success, fail)
  *				SensorIndex -> the index of sensor(change maxium number of sensor: SENSOR_NUMBER_MAX in devic.h)
  */
static stu_Lora_ApplyNet_SensorPara_t App_Lora_ApplyNetReq_Handler(en_Lora_FunctionCode_t FunctionCode, stu_Lora_Sensor_DataFrame_t DataFrame)
{
	uint8_t i;
	
	stu_Lora_ApplyNet_SensorPara_t 	Lora_ApplyNet_SensorPara;	// store the ApplyNet sensors parameter, as the return value to the LoraSensor
	stu_Sensor_PairPara_t 			Sensor_PairPara;			// store the Pairing parameters of Sensors(saved in Terminal)
	stu_Sensor_t 					Sensor_Buff;				// temporary buff of Sensor parameters
	
	Lora_ApplyNet_SensorPara.State = LORA_APPLYNET_FAIL;
	
	/* receive the ApplyNet Request from LoRa-Sensors */
	if(FunctionCode == LORA_COM_APPLY_NET)
	{
		/* SensorPairMenu is waiting for Sensor ApplyNet: */
		if(SensorPairMenu_SensorPara.WorkMode == SENSOR_WORKMODE_APPLY_NET)
		{
			SensorPairMenu_SensorPara.WorkMode = SENSOR_WORKMODE_APPLY_NET_SUCCESS;
			SensorPairMenu_SensorPara.SensorType = (en_Sensor_Type_t)DataFrame.SensorType;
			
			/* grab the LoraSensor parameters from the DataFrame received, save in the Sensor_Buff */
			Sensor_Buff.Sensor_Type = (en_Sensor_Type_t)DataFrame.SensorType;
			Sensor_Buff.Sensor_ArmType = SENSOR_ARMTYPE_1_AWAYARM;
			
			for(i=0; i<12; i++)
			{
				Sensor_Buff.MACAddress[i] = DataFrame.MACAddress[i];
			}
			
			Sensor_Buff.Node[0] = DataFrame.NodeNo[0];
			Sensor_Buff.Node[1] = DataFrame.NodeNo[1];
			
			Sensor_Buff.SleepTime = 0;
			
			/* according to the info grab from DataFrame, add sensor parameters to the Terminal and update the Sensor Pairing parameters */
			Sensor_PairPara = App_Lora_ApplyNet_AddSensor(&Sensor_Buff);
			
			/* update the Sensor parameters in SensorPairMenu */
			SensorPairMenu_SensorPara.PairState = Sensor_PairPara.PairState;
			
			/* according to the Sensor PairState in SensorPairMenu, change the ApplyNet State and SensorIndex for return value */
			if(SensorPairMenu_SensorPara.PairState == SENSOR_PAIRSTATE_UNPAIR)
			{
				Lora_ApplyNet_SensorPara.State = LORA_APPLYNET_SUCCESSFUL;
				Lora_ApplyNet_SensorPara.SensorIndex = Sensor_PairPara.SensorIndex;
			}
			else if(SensorPairMenu_SensorPara.PairState == SENSOR_PAIRSTATE_PAIRED)
			{
				Lora_ApplyNet_SensorPara.State = LORA_APPLYNET_FAIL;
				Lora_ApplyNet_SensorPara.SensorIndex = Sensor_PairPara.SensorIndex;
			}
		}
	}
	
	return Lora_ApplyNet_SensorPara;
}

/**
  * @Brief	According to the provided Sensor parameters, save sensor parameters into EEPROM
  * @Param	pSensorPara: 
  * @Retval	Sensor Pairing parameters(PairState, Code)
  * @Note 	if the sensor already saved in Terminal, retutn PairState = PAIRED and correspondiong SensorIndex
  * 		if the sensor not saved in Terminal, collect info of provided Sensor parameters and flsh into the EEPROM
  * 		if all Terminal Sensor slots have been paired, return PairState = FAIL
  */
static stu_Sensor_PairPara_t App_Lora_ApplyNet_AddSensor(stu_Sensor_t *pSensorPara)
{
	uint8_t i, j, k, Offset;
	uint8_t Index = 0;
	
	stu_Sensor_t 			SensorBuff;
	stu_Sensor_PairPara_t 	SensorPairPara;
	
	/* according to the provided info, check if the Sensor has paired with the Terminal */
	/* if the info indicate the Sensor already Paired with Terminal, return the saved Sensor Pairing parameters */
	for(i=0; i<SENSOR_NUMBER_MAX; i++)
	{
		if( (stu_Sensor[i].PairFlag) &&
			
			(stu_Sensor[i].MACAddress[0] == pSensorPara->MACAddress[0]) &&
			(stu_Sensor[i].MACAddress[1] == pSensorPara->MACAddress[1]) &&
			(stu_Sensor[i].MACAddress[2] == pSensorPara->MACAddress[2]) &&
			(stu_Sensor[i].MACAddress[3] == pSensorPara->MACAddress[3]) &&
			(stu_Sensor[i].MACAddress[4] == pSensorPara->MACAddress[4]) &&
			(stu_Sensor[i].MACAddress[5] == pSensorPara->MACAddress[5]) &&
			(stu_Sensor[i].MACAddress[6] == pSensorPara->MACAddress[6]) &&
			(stu_Sensor[i].MACAddress[7] == pSensorPara->MACAddress[7]) &&
			(stu_Sensor[i].MACAddress[8] == pSensorPara->MACAddress[8]) &&
			(stu_Sensor[i].MACAddress[9] == pSensorPara->MACAddress[9]) &&
			(stu_Sensor[i].MACAddress[10] == pSensorPara->MACAddress[10]) &&
			(stu_Sensor[i].MACAddress[11] == pSensorPara->MACAddress[11]) &&
		
			(stu_Sensor[i].Node[0] == pSensorPara->Node[0]) &&
			(stu_Sensor[i].Node[1] == pSensorPara->Node[1]) 
		)
		{
			SensorPairPara.PairState = SENSOR_PAIRSTATE_PAIRED;
			SensorPairPara.SensorIndex = i;
			
			return SensorPairPara;
		}
	}
	
	/* if the ApplyNet Sensor is unpaired with the Terminal, grab the info and save to the Terminal's EEPROM */
	for(i=0; i<SENSOR_NUMBER_MAX; i++)
	{
		if(!stu_Sensor[i].PairFlag)
		{
			SensorBuff.SensorName[0] = 'D';
			SensorBuff.SensorName[1] = 'e';
			SensorBuff.SensorName[2] = 'v';
			SensorBuff.SensorName[3] = 'i';
			SensorBuff.SensorName[4] = 'c';
			SensorBuff.SensorName[5] = 'e';
			SensorBuff.SensorName[6] = '-';
			
			Index = 7;
			Offset = i + 1;
			
			SensorBuff.SensorName[Index++] = '0' + (Offset / 100);
			SensorBuff.SensorName[Index++] = '0' + ((Offset % 100) / 10);
			SensorBuff.SensorName[Index++] = '0' + ((Offset % 100) % 10);
			
			for(j=Index; j<26; j++)
			{
				SensorBuff.SensorName[j] = 0;	// clear the rest of SensorName(unused) to \0
			}
			
			SensorBuff.ID = i + 1;
			SensorBuff.PairFlag = 1;
			SensorBuff.SensorNo = Offset;
			SensorBuff.Sensor_Type = pSensorPara->Sensor_Type;
			SensorBuff.Sensor_ArmType = pSensorPara->Sensor_ArmType;
			
			for(k=0; k<12; k++)
			{
				SensorBuff.MACAddress[k] = pSensorPara->MACAddress[k];
			}
			
			SensorBuff.Node[0] = pSensorPara->Node[0];
			SensorBuff.Node[1] = pSensorPara->Node[1];
			SensorBuff.SleepTime = 0;
			
			/* update the sensor info stored in Terminal */
			Mid_EEPROM_PageWrite(EEPROM_ADDRESS_SENSORPARA_OFFSET + (i * STU_SENSOR_SIZE), (uint8_t *)(&SensorBuff), sizeof(SensorBuff));
			Mid_EEPROM_SequentialRead(EEPROM_ADDRESS_SENSORPARA_OFFSET + (i * STU_SENSOR_SIZE), (uint8_t *)(&stu_Sensor[i]), STU_SENSOR_SIZE);
		
			SensorPairPara.PairState = SENSOR_PAIRSTATE_UNPAIR;
			SensorPairPara.SensorIndex = i;
			
			return SensorPairPara;
		}
	}
	
	/* Pairing failed */
	SensorPairPara.PairState = SENSOR_PAIRSTATE_FAIL;
	SensorPairPara.SensorIndex = i;
	
	return SensorPairPara;
}


/**
  * @Brief	Lora received data handler, queue-in "#XX" dataframe
  * @Param	pData: pointer of the come-in data
  * @Retval	None
  */
static void App_Lora_RxDataHandler(uint8_t *pData)
{
	uint8_t TempBuff;
	
	TempBuff = '#';
	
	QueueDataIn(Queue_AppLoraRx, &TempBuff, 1);
	QueueDataIn(Queue_AppLoraRx, &pData[0], 2);
}

/**
  * @Brief	Process received Lora dataframe, Lora_FunctionCMD handler
  * @Param	FunctionCode: Command(FunctionCode) from Sensor
  *			DataFrame	: DataFrame from Sensor
  * @Retval	Result:
  *				0 -> unpair sensor's data, invalid
  *				1 -> valid data, extract (SensorIndex, Sensor-Command) wrap with '#'and queue-in
  */
static uint8_t App_Lora_FunctionCMD_Handler(en_Lora_FunctionCode_t FunctionCode, stu_Lora_Sensor_DataFrame_t DataFrame)
{
	uint8_t EventBuff;
	uint8_t SensorID;
	uint8_t RxDataBuff[2];
	
	stu_Sensor_t SensorParaBuff;
	
	SensorParaBuff.Node[0] = DataFrame.NodeNo[0];
	SensorParaBuff.Node[1] = DataFrame.NodeNo[1];
	
	EventBuff = (uint8_t)DataFrame.FunctionCode;
	SensorID = Device_SensorNodeMatch(&SensorParaBuff.Node[0]);
	
	if(SensorID == 0xFF)
	{
		return 0;
	}
	else
	{
		RxDataBuff[0] = SensorID - 1;	// Sensor defense zone
		RxDataBuff[1] = EventBuff;		// Sensor Command
		
		App_Lora_RxDataHandler(&RxDataBuff[0]);
	}
	
	return 1;
}

/**
  * @Brief	Control the backlight of Screen
  * @Param	CMD: 0->turn off backlight, 1->turn on backlight
  * @Retval	None
  */
static void App_ScreenControl(uint8_t CMD)
{
	if(CMD == 1)
	{
		if(!ScreenState)
		{
			ScreenState = 1;
			Hal_TFTLCD_Display_On();
			TimeoutCounter_ScreenSleep = 0;
		}
	}
	else
	{
		if(ScreenState)
		{
			ScreenState = 0;
			Hal_TFTLCD_Display_Off();
			TimeoutCounter_ScreenSleep = 0;
		}
	}
}

/**
  * @Brief	Update the Sensor parameters and save to EEPROM
  * @Param	SensorIndex: selected Sensor index
  *			pSensorPara: updated sensor parameters
  * @Retval	None
  */
static void App_UpdateSensorPara(uint8_t SensorIndex, stu_Sensor_t *pSensorPara)
{
	uint8_t i;
	
	if(SensorIndex >= SENSOR_NUMBER_MAX)
	{
		return;
	}
	
	stu_Sensor[SensorIndex].ID = pSensorPara->ID;
	stu_Sensor[SensorIndex].PairFlag = pSensorPara->PairFlag;
	stu_Sensor[SensorIndex].SensorNo = pSensorPara->SensorNo;
	
	for(i=0; i<16; i++)
	{
		stu_Sensor[SensorIndex].SensorName[i] = pSensorPara->SensorName[i];
	}
	
	stu_Sensor[SensorIndex].Sensor_Type = pSensorPara->Sensor_Type;
	stu_Sensor[SensorIndex].Sensor_ArmType = pSensorPara->Sensor_ArmType;
	
	for(i=0; i<12; i++)
	{
		stu_Sensor[SensorIndex].MACAddress[i] = pSensorPara->MACAddress[i];
	}
	
	stu_Sensor[SensorIndex].Node[0] = pSensorPara->Node[0];
	stu_Sensor[SensorIndex].Node[1] = pSensorPara->Node[1];
	
	Mid_EEPROM_PageWrite(EEPROM_ADDRESS_SENSORPARA_OFFSET + (SensorIndex * STU_SENSOR_SIZE), (uint8_t *)pSensorPara, STU_SENSOR_SIZE);
	Mid_EEPROM_SequentialRead(EEPROM_ADDRESS_SENSORPARA_OFFSET + (SensorIndex * STU_SENSOR_SIZE), (uint8_t *)&stu_Sensor[SensorIndex], STU_SENSOR_SIZE);
}


/**
  * @Brief	Terminal AwayArm mode handler
  * @Param	None
  * @Retval	None
  */
static void App_TerminalMode_AwayArmHandler(void)
{
	uint8_t DataBuff;
	uint8_t SensorIndex;		
	uint8_t FunctionCode;
	uint8_t PromptDisplayFlag;
	
	stu_Sensor_t SensorParaBuff;
	
	static uint16_t Counter = 0;
	
	if(pTerminalMode->ScreenCMD == SCREEN_CMD_RESET)
	{
		pTerminalMode->ScreenCMD = SCREEN_CMD_NULL;
		pTerminalMode->KeyIndex = 0xFF;
		
		Mid_TFTLCD_ShowString(COOR_TERMINAL_WORK_MODE_X, COOR_TERMINAL_WORK_MODE_Y, "AWAY ARM", LCD_FONT_COLOR, LCD_BACK_COLOR, 48, 0);
		
		PromptDisplayFlag = 1;
	}
	
	/* Queue_AppLoraRx has data ready for process */
	if(QueueDataLen(Queue_AppLoraRx))
	{
		QueueDataOut(Queue_AppLoraRx, &DataBuff);
		
		if(DataBuff == '#')
		{
			if(QueueDataLen(Queue_AppLoraRx) >= 2)
			{
				QueueDataOut(Queue_AppLoraRx, &SensorIndex);
				QueueDataOut(Queue_AppLoraRx, &FunctionCode);
				
				Device_GetSensorPara(&SensorParaBuff, SensorIndex);
				
				if(SensorParaBuff.Sensor_Type == SENSOR_TYPE_REMOTE)
				{
					if(FunctionCode == LORA_COM_DISARM)
					{
						App_Terminal_ModeChange(SensorParaBuff.ID, TERMINAL_WORK_MODE_DISARM, TERMINAL_CMD_SOURCE_REMOTE);
					}
					else if(FunctionCode == LORA_COM_AWAYARM)
					{
						App_Terminal_ModeChange(SensorParaBuff.ID, TERMINAL_WORK_MODE_AWAYARM, TERMINAL_CMD_SOURCE_REMOTE);
					}
					else if(FunctionCode == LORA_COM_HOMEARM)
					{
						App_Terminal_ModeChange(SensorParaBuff.ID, TERMINAL_WORK_MODE_HOMEARM, TERMINAL_CMD_SOURCE_REMOTE);
					}
					else if(FunctionCode == LORA_COM_ALARM)
					{
						App_Terminal_ModeChange(SensorParaBuff.ID, TERMINAL_WORK_MODE_ALARMING, TERMINAL_CMD_SOURCE_REMOTE);
						
						SensorIndex += 1;	// SensorID = SensorIndex + 1
						QueueDataIn(Queue_TriggerSensorID, &SensorIndex, 1);
					}
					else if(FunctionCode == LORA_COM_BAT_LOW)
					{
						App_ScreenControl(1);
						
						Mid_TFTLCD_ShowString(COOR_TERMINAL_PROMPT_1_X, COOR_TERMINAL_PROMPT_1_Y, SensorParaBuff.SensorName, LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
						Mid_TFTLCD_ShowString(COOR_TERMINAL_PROMPT_2_X, COOR_TERMINAL_PROMPT_2_Y, " Volt Low ", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
						
						PromptDisplayFlag = 1;
					}

				}
				else if(SensorParaBuff.Sensor_Type == SENSOR_TYPE_DOOR)
				{
					if(FunctionCode == LORA_COM_ALARM)
					{
						if( (SensorParaBuff.Sensor_ArmType == SENSOR_ARMTYPE_0_24HOURS) || 
							(SensorParaBuff.Sensor_ArmType == SENSOR_ARMTYPE_1_AWAYARM)		)
						{
							App_Terminal_ModeChange(SensorParaBuff.ID, TERMINAL_WORK_MODE_ALARMING, TERMINAL_CMD_SOURCE_REMOTE);
						
							SensorIndex += 1;	// SensorID = SensorIndex + 1
							QueueDataIn(Queue_TriggerSensorID, &SensorIndex, 1);
						}
						else
						{
							App_ScreenControl(1);
							
							Mid_TFTLCD_ShowString(COOR_TERMINAL_PROMPT_1_X, COOR_TERMINAL_PROMPT_1_Y, SensorParaBuff.SensorName, LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
							Mid_TFTLCD_ShowString(COOR_TERMINAL_PROMPT_2_X, COOR_TERMINAL_PROMPT_2_Y, "Door Open ", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
							
							PromptDisplayFlag = 1;
							
						}
						
						/* queue-in EventUpload MQTT message */
						MQTTProtocol_EventUpQueueIn(TERMINAL_UPEVENT_DOOR_OPEN, SensorIndex);
					}
					else if(FunctionCode == LORA_COM_DOORCLOSE)
					{
						App_ScreenControl(1);
						
						Mid_TFTLCD_ShowString(COOR_TERMINAL_PROMPT_1_X, COOR_TERMINAL_PROMPT_1_Y, SensorParaBuff.SensorName, LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
						Mid_TFTLCD_ShowString(COOR_TERMINAL_PROMPT_2_X, COOR_TERMINAL_PROMPT_2_Y, "Door Close", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
						
						PromptDisplayFlag = 1;
						
						/* queue-in EventUpload MQTT message */
						MQTTProtocol_EventUpQueueIn(TERMINAL_UPEVENT_DOOR_CLOSE, SensorIndex);
					}
					else if(FunctionCode == LORA_COM_BAT_LOW)
					{
						App_ScreenControl(1);
						
						Mid_TFTLCD_ShowString(COOR_TERMINAL_PROMPT_1_X, COOR_TERMINAL_PROMPT_1_Y, SensorParaBuff.SensorName, LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
						Mid_TFTLCD_ShowString(COOR_TERMINAL_PROMPT_2_X, COOR_TERMINAL_PROMPT_2_Y, " Volt Low ", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
						
						PromptDisplayFlag = 1;
						
						/* queue-in EventUpload MQTT message */
						MQTTProtocol_EventUpQueueIn(TERMINAL_UPEVENT_DETECTOR_BATLOW, SensorIndex);
					}
				}
			}
		}
	}
	
	/* display the prompt flah effect */
	if(PromptDisplayFlag)
	{
		Counter++;
		
		if(Counter > 200)
		{
			PromptDisplayFlag = 0;
			Counter = 0;
			
			Mid_TFTLCD_ShowString(0, COOR_TERMINAL_PROMPT_1_Y, "                         ", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		}
	}
}

/**
  * @Brief	Terminal HomeArm mode handler
  * @Param	None
  * @Retval	None
  */
static void App_TerminalMode_HomeArmHandler(void)
{
	uint8_t DataBuff;
	uint8_t SensorIndex;		
	uint8_t FunctionCode;
	uint8_t PromptDisplayFlag;
	
	stu_Sensor_t SensorParaBuff;
	
	static uint16_t Counter = 0;
	
	if(pTerminalMode->ScreenCMD == SCREEN_CMD_RESET)
	{
		pTerminalMode->ScreenCMD = SCREEN_CMD_NULL;
		pTerminalMode->KeyIndex = 0xFF;
		
		Mid_TFTLCD_ShowString(COOR_TERMINAL_WORK_MODE_X, COOR_TERMINAL_WORK_MODE_Y, "HOME ARM", LCD_FONT_COLOR, LCD_BACK_COLOR, 48, 0);
		
		PromptDisplayFlag = 1;
	}
	
	/* Queue_AppLoraRx has data ready for process */
	if(QueueDataLen(Queue_AppLoraRx))
	{
		QueueDataOut(Queue_AppLoraRx, &DataBuff);
		
		if(DataBuff == '#')
		{
			if(QueueDataLen(Queue_AppLoraRx) >= 2)
			{
				QueueDataOut(Queue_AppLoraRx, &SensorIndex);
				QueueDataOut(Queue_AppLoraRx, &FunctionCode);
				
				Device_GetSensorPara(&SensorParaBuff, SensorIndex);
				
				if(SensorParaBuff.Sensor_Type == SENSOR_TYPE_REMOTE)
				{
					if(FunctionCode == LORA_COM_DISARM)
					{
						App_Terminal_ModeChange(SensorParaBuff.ID, TERMINAL_WORK_MODE_DISARM, TERMINAL_CMD_SOURCE_REMOTE);
					}
					else if(FunctionCode == LORA_COM_AWAYARM)
					{
						App_Terminal_ModeChange(SensorParaBuff.ID, TERMINAL_WORK_MODE_AWAYARM, TERMINAL_CMD_SOURCE_REMOTE);
					}
					else if(FunctionCode == LORA_COM_HOMEARM)
					{
						App_Terminal_ModeChange(SensorParaBuff.ID, TERMINAL_WORK_MODE_HOMEARM, TERMINAL_CMD_SOURCE_REMOTE);
					}
					else if(FunctionCode == LORA_COM_ALARM)
					{
						App_Terminal_ModeChange(SensorParaBuff.ID, TERMINAL_WORK_MODE_ALARMING, TERMINAL_CMD_SOURCE_REMOTE);
						
						SensorIndex += 1;	// SensorID = SensorIndex + 1
						QueueDataIn(Queue_TriggerSensorID, &SensorIndex, 1);
					}
					else if(FunctionCode == LORA_COM_BAT_LOW)
					{
						App_ScreenControl(1);
						
						Mid_TFTLCD_ShowString(COOR_TERMINAL_PROMPT_1_X, COOR_TERMINAL_PROMPT_1_Y, SensorParaBuff.SensorName, LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
						Mid_TFTLCD_ShowString(COOR_TERMINAL_PROMPT_2_X, COOR_TERMINAL_PROMPT_2_Y, " Volt Low ", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
						
						PromptDisplayFlag = 1;
					}

				}
				else if(SensorParaBuff.Sensor_Type == SENSOR_TYPE_DOOR)
				{
					if(FunctionCode == LORA_COM_ALARM)
					{
						if( (SensorParaBuff.Sensor_ArmType == SENSOR_ARMTYPE_0_24HOURS) ||
							(SensorParaBuff.Sensor_ArmType == SENSOR_ARMTYPE_2_HOMEARM)		)
						{
							App_Terminal_ModeChange(SensorParaBuff.ID, TERMINAL_WORK_MODE_ALARMING, TERMINAL_CMD_SOURCE_REMOTE);
						
							SensorIndex += 1;	// SensorID = SensorIndex + 1
							QueueDataIn(Queue_TriggerSensorID, &SensorIndex, 1);
						}
						else
						{
							App_ScreenControl(1);
							
							Mid_TFTLCD_ShowString(COOR_TERMINAL_PROMPT_1_X, COOR_TERMINAL_PROMPT_1_Y, SensorParaBuff.SensorName, LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
							Mid_TFTLCD_ShowString(COOR_TERMINAL_PROMPT_2_X, COOR_TERMINAL_PROMPT_2_Y, "Door Open ", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
							
							PromptDisplayFlag = 1;
						}
						
						/* queue-in EventUpload MQTT message */
						MQTTProtocol_EventUpQueueIn(TERMINAL_UPEVENT_DOOR_OPEN, SensorIndex);
					}
					else if(FunctionCode == LORA_COM_DOORCLOSE)
					{
						App_ScreenControl(1);
						
						Mid_TFTLCD_ShowString(COOR_TERMINAL_PROMPT_1_X, COOR_TERMINAL_PROMPT_1_Y, SensorParaBuff.SensorName, LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
						Mid_TFTLCD_ShowString(COOR_TERMINAL_PROMPT_2_X, COOR_TERMINAL_PROMPT_2_Y, "Door Close", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
						
						PromptDisplayFlag = 1;
						
						/* queue-in EventUpload MQTT message */
						MQTTProtocol_EventUpQueueIn(TERMINAL_UPEVENT_DOOR_CLOSE, SensorIndex);
					}
					else if(FunctionCode == LORA_COM_BAT_LOW)
					{
						App_ScreenControl(1);
						
						Mid_TFTLCD_ShowString(COOR_TERMINAL_PROMPT_1_X, COOR_TERMINAL_PROMPT_1_Y, SensorParaBuff.SensorName, LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
						Mid_TFTLCD_ShowString(COOR_TERMINAL_PROMPT_2_X, COOR_TERMINAL_PROMPT_2_Y, " Volt Low ", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
						
						PromptDisplayFlag = 1;
						
						/* queue-in EventUpload MQTT message */
						MQTTProtocol_EventUpQueueIn(TERMINAL_UPEVENT_DETECTOR_BATLOW, SensorIndex);
					}
				}
			}
		}
	}
	
	/* display the prompt flah effect */
	if(PromptDisplayFlag)
	{
		Counter++;
		
		if(Counter > 200)
		{
			PromptDisplayFlag = 0;
			Counter = 0;
			
			Mid_TFTLCD_ShowString(0, COOR_TERMINAL_PROMPT_1_Y, "                         ", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		}
	}
}

/**
  * @Brief	Terminal Alarming mode handler 
  * @Param	None
  * @Retval	None
  */
static void App_TerminalMode_AlarmingHandler(void)
{
	uint8_t DataBuff;
	uint8_t SensorIndex;		
	uint8_t FunctionCode;
	
	stu_Sensor_t SensorParaBuff;
	
	
	/* Queue_AppLoraRx has data ready for process */
	if(QueueDataLen(Queue_AppLoraRx))
	{
		QueueDataOut(Queue_AppLoraRx, &DataBuff);
		
		if(DataBuff == '#')
		{
			if(QueueDataLen(Queue_AppLoraRx) >= 2)
			{
				QueueDataOut(Queue_AppLoraRx, &SensorIndex);
				QueueDataOut(Queue_AppLoraRx, &FunctionCode);
				
				Device_GetSensorPara(&SensorParaBuff, SensorIndex);
				
				if(SensorParaBuff.Sensor_Type == SENSOR_TYPE_REMOTE)
				{
					if(FunctionCode == LORA_COM_DISARM)
					{
						App_Terminal_ModeChange(SensorParaBuff.ID, TERMINAL_WORK_MODE_DISARM, TERMINAL_CMD_SOURCE_REMOTE);
					}
					else if(FunctionCode == LORA_COM_ALARM)
					{
						App_Terminal_ModeChange(SensorParaBuff.ID, TERMINAL_WORK_MODE_ALARMING, TERMINAL_CMD_SOURCE_REMOTE);
						
						SensorIndex += 1;	// SensorID = SensorIndex +1
						QueueDataIn(Queue_TriggerSensorID, &SensorIndex, 1);
					}

				}
				else if(SensorParaBuff.Sensor_Type == SENSOR_TYPE_DOOR)
				{
					if(FunctionCode == LORA_COM_ALARM)
					{
						if( (SensorParaBuff.Sensor_ArmType == SENSOR_ARMTYPE_0_24HOURS) ||
							(SensorParaBuff.Sensor_ArmType == SENSOR_ARMTYPE_1_AWAYARM)		)
						{
							App_Terminal_ModeChange(SensorParaBuff.ID, TERMINAL_WORK_MODE_ALARMING, TERMINAL_CMD_SOURCE_REMOTE);
						
							SensorIndex += 1;	// SensorID = SensorIndex +1
							QueueDataIn(Queue_TriggerSensorID, &SensorIndex, 1);
						}

						/* queue-in EventUpload MQTT message */
						MQTTProtocol_EventUpQueueIn(TERMINAL_UPEVENT_DOOR_OPEN, SensorIndex);
					}
				}
			}
		}
	}
}

/**
  * @Brief	Terminal Disarm mode handler
  * @Param	None
  * @Retval	None
  */
static void App_TerminalMode_DisarmHandler(void)
{
	uint8_t DataBuff;
	uint8_t SensorIndex;		
	uint8_t FunctionCode;
	uint8_t PromptDisplayFlag;
	
	stu_Sensor_t SensorParaBuff;
	
	static uint16_t Counter = 0;
	
	if(pTerminalMode->ScreenCMD == SCREEN_CMD_RESET)
	{
		pTerminalMode->ScreenCMD = SCREEN_CMD_NULL;
		pTerminalMode->KeyIndex = 0xFF;
		
		Mid_TFTLCD_ShowString(COOR_TERMINAL_WORK_MODE_X, COOR_TERMINAL_WORK_MODE_Y, " DISARM ", LCD_FONT_COLOR, LCD_BACK_COLOR, 48, 0);
		
		PromptDisplayFlag = 1;
	}
	
	/* Queue_AppLoraRx has data ready for process */
	if(QueueDataLen(Queue_AppLoraRx))
	{
		QueueDataOut(Queue_AppLoraRx, &DataBuff);
		
		if(DataBuff == '#')
		{
			if(QueueDataLen(Queue_AppLoraRx) >= 2)
			{
				QueueDataOut(Queue_AppLoraRx, &SensorIndex);
				QueueDataOut(Queue_AppLoraRx, &FunctionCode);
				
				Device_GetSensorPara(&SensorParaBuff, SensorIndex);
				
				if(SensorParaBuff.Sensor_Type == SENSOR_TYPE_REMOTE)
				{
					if(FunctionCode == LORA_COM_DISARM)
					{
						App_Terminal_ModeChange(SensorParaBuff.ID, TERMINAL_WORK_MODE_DISARM, TERMINAL_CMD_SOURCE_REMOTE);
					}
					else if(FunctionCode == LORA_COM_AWAYARM)
					{
						App_Terminal_ModeChange(SensorParaBuff.ID, TERMINAL_WORK_MODE_AWAYARM, TERMINAL_CMD_SOURCE_REMOTE);
					}
					else if(FunctionCode == LORA_COM_HOMEARM)
					{
						App_Terminal_ModeChange(SensorParaBuff.ID, TERMINAL_WORK_MODE_HOMEARM, TERMINAL_CMD_SOURCE_REMOTE);
					}
					else if(FunctionCode == LORA_COM_ALARM)
					{

					}
					else if(FunctionCode == LORA_COM_BAT_LOW)
					{
						App_ScreenControl(1);
						
						Mid_TFTLCD_ShowString(COOR_TERMINAL_PROMPT_1_X, COOR_TERMINAL_PROMPT_1_Y, SensorParaBuff.SensorName, LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
						Mid_TFTLCD_ShowString(COOR_TERMINAL_PROMPT_2_X, COOR_TERMINAL_PROMPT_2_Y, " Volt Low ", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
						
						PromptDisplayFlag = 1;
					}

				}
				else if(SensorParaBuff.Sensor_Type == SENSOR_TYPE_DOOR)
				{
					if(FunctionCode == LORA_COM_ALARM)
					{
						if(SensorParaBuff.Sensor_ArmType == SENSOR_ARMTYPE_0_24HOURS)
						{
							App_Terminal_ModeChange(SensorParaBuff.ID, TERMINAL_WORK_MODE_ALARMING, TERMINAL_CMD_SOURCE_REMOTE);
						
							SensorIndex += 1;	// SensorID = SensorIndex + 1
							QueueDataIn(Queue_TriggerSensorID, &SensorIndex, 1);
						}
						else
						{
							App_ScreenControl(1);
							
							Mid_TFTLCD_ShowString(COOR_TERMINAL_PROMPT_1_X, COOR_TERMINAL_PROMPT_1_Y, SensorParaBuff.SensorName, LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
							Mid_TFTLCD_ShowString(COOR_TERMINAL_PROMPT_2_X, COOR_TERMINAL_PROMPT_2_Y, "Door Open ", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
							
							PromptDisplayFlag = 1;
						}
						
						/* queue-in EventUpload MQTT message */
						MQTTProtocol_EventUpQueueIn(TERMINAL_UPEVENT_DOOR_OPEN, SensorIndex);
					}
					else if(FunctionCode == LORA_COM_DOORCLOSE)
					{
						App_ScreenControl(1);
						
						Mid_TFTLCD_ShowString(COOR_TERMINAL_PROMPT_1_X, COOR_TERMINAL_PROMPT_1_Y, SensorParaBuff.SensorName, LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
						Mid_TFTLCD_ShowString(COOR_TERMINAL_PROMPT_2_X, COOR_TERMINAL_PROMPT_2_Y, "Door Close", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
						
						PromptDisplayFlag = 1;
						
						/* queue-in EventUpload MQTT message */
						MQTTProtocol_EventUpQueueIn(TERMINAL_UPEVENT_DOOR_CLOSE, SensorIndex);
					}
					else if(FunctionCode == LORA_COM_BAT_LOW)
					{
						App_ScreenControl(1);
						
						Mid_TFTLCD_ShowString(COOR_TERMINAL_PROMPT_1_X, COOR_TERMINAL_PROMPT_1_Y, SensorParaBuff.SensorName, LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
						Mid_TFTLCD_ShowString(COOR_TERMINAL_PROMPT_2_X, COOR_TERMINAL_PROMPT_2_Y, " Volt Low ", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
						
						PromptDisplayFlag = 1;
						
						/* queue-in EventUpload MQTT message */
						MQTTProtocol_EventUpQueueIn(TERMINAL_UPEVENT_DETECTOR_BATLOW, SensorIndex);
					}
				}
			}
		}
	}
	
	/* display the prompt flah effect */
	if(PromptDisplayFlag)
	{
		Counter++;
		
		if(Counter > 200)
		{
			PromptDisplayFlag = 0;
			Counter = 0;
			
			Mid_TFTLCD_ShowString(0, COOR_TERMINAL_PROMPT_1_Y, "                         ", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		}
	}
}

/**
  * @Brief	Alarm process handler for AlarmMenu
  * @Param	None
  * @Retval	None
  */
static void App_AlarmMenu_AlarmHandler(void)
{
	uint8_t i;
	
	uint8_t KeyPress;	// index
	uint8_t KeyEvent;
	
	uint8_t TriggerSensorID;
	
	static uint16_t BlinkCounter = 0;
	static uint8_t BlinkFlag = 1;		// 1->blink, 0->no blink
	
	static uint8_t AlarmNum = 0;		// alarming sensor number
	static uint8_t AlarmFlag = 0;		// 1->alarming, 0-> no alarm
	static uint8_t AlarmFullFlag = 0;	// alarm info full flag
	static uint8_t AlarmInfo[3];		// buffer store the alarming sensor ID
	
	stu_Sensor_t SensorParaBuff;
	
	
	/* enter AlarmMenu: */
	if(pMenu->ScreenCMD == SCREEN_CMD_RESET)
	{
		TimeoutCounter_ReturnDesktop = 0;
		TimeoutCounter_ScreenSleep = 0;
		
		Mid_TFTLCD_ScreenClear();
		
		pMenu->ScreenCMD = SCREEN_CMD_NULL;
		pMenu->KeyIndex = 0xFF;
		
		Mid_TFTLCD_ShowString(COOR_MENU_ALARMMENU_PROMPT_1_X, COOR_MENU_ALARMMENU_PROMPT_1_Y, "ALARMING", LCD_FONT_COLOR, LCD_BACK_COLOR, 48, 0);
	
		AlarmNum = 0;
		AlarmFullFlag = 0;
		AlarmFlag = 0;;
		BlinkCounter = 0;
		BlinkFlag = 0;
		
		
	}
	else if(pMenu->ScreenCMD == SCREEN_CMD_RECOVER)
	{
		Mid_TFTLCD_ScreenClear();
		
		pMenu->ScreenCMD = SCREEN_CMD_NULL;
		pMenu->KeyIndex = 0xFF;
		AlarmFlag = 1;
		
		Mid_TFTLCD_ShowString(COOR_MENU_ALARMMENU_PROMPT_1_X, COOR_MENU_ALARMMENU_PROMPT_1_Y, "ALARMING", LCD_FONT_COLOR, LCD_BACK_COLOR, 48, 0);
	}
	
	/* AlarmMenu KeyScan: */
	if(pMenu->KeyIndex != 0xFF)
	{
		KeyPress = pMenu->KeyIndex;
		pMenu->KeyIndex = 0xFF;
		KeyEvent = pMenu->KeyEvent;
		
		if(KeyEvent == KEY_CLICK)
		{
			switch(KeyPress)
			{
				case KEY_DISARM:
				{
					pMenu = pMenu->pChild;	// enter pin-disarm menu
					pMenu->ScreenCMD = SCREEN_CMD_RESET;
				}
				break;
							
			}
		}
	}
	
	/* check and store triggered sensor ID to buffer */
	if(QueueDataLen(Queue_TriggerSensorID))
	{
		QueueDataOut(Queue_TriggerSensorID, &TriggerSensorID);
		
		if(TriggerSensorID > 0)
		{
			AlarmFlag = 1;
			AlarmInfo[AlarmNum++] = TriggerSensorID;
			
			if(AlarmNum == 3)
			{
				AlarmNum = 0;
				AlarmFullFlag = 1;
			}
		}
	}
	
	/* display the alarm info */
	if(AlarmFlag)
	{
		AlarmFlag = 0;
		
		if(AlarmFullFlag == 1)
		{
			for(i=0; i<3; i++)
			{
				if(AlarmInfo[i] == 0xFF)
				{
					Mid_TFTLCD_ShowString(COOR_MENU_ALARMMENU_AlARMSOURCE_X_BASE, (COOR_MENU_ALARMMENU_AlARMSOURCE_Y_BASE + 30 * i), "Host KeyBoard SOS  ", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
				}
				else
				{
					if((AlarmInfo[i] - 1) < SENSOR_NUMBER_MAX)
					{
						Device_GetSensorPara(&SensorParaBuff, (AlarmInfo[i] - 1));
					
						switch(SensorParaBuff.Sensor_Type)
						{
							case SENSOR_TYPE_REMOTE:
							{
								Mid_TFTLCD_ShowString(COOR_MENU_ALARMMENU_AlARMSOURCE_X_BASE, (COOR_MENU_ALARMMENU_AlARMSOURCE_Y_BASE + 30 * i), "Remote: SOS        ", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
							}
							break;
						
							case SENSOR_TYPE_DOOR:
							{
								Mid_TFTLCD_ShowString(COOR_MENU_ALARMMENU_AlARMSOURCE_X_BASE, (COOR_MENU_ALARMMENU_AlARMSOURCE_Y_BASE + 30 * i), "Door:              ", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
								Mid_TFTLCD_ShowString(COOR_MENU_ALARMMENU_AlARMSOURCE_X_BASE + 80, (COOR_MENU_ALARMMENU_AlARMSOURCE_Y_BASE + 30 * i), SensorParaBuff.SensorName, LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
							}
							break;
						
						}
					}
				}
			}
		}
		else
		{
			for(i=0; i<AlarmNum; i++)
			{
				if(AlarmInfo[i] == 0xFF)
				{
					Mid_TFTLCD_ShowString(COOR_MENU_ALARMMENU_AlARMSOURCE_X_BASE, (COOR_MENU_ALARMMENU_AlARMSOURCE_Y_BASE + 30 * i), "Host KeyBoard SOS  ", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
				}
				else
				{
					if((AlarmInfo[i] - 1) < SENSOR_NUMBER_MAX)
					{
						Device_GetSensorPara(&SensorParaBuff, (AlarmInfo[i] - 1));
					
						switch(SensorParaBuff.Sensor_Type)
						{
							case SENSOR_TYPE_REMOTE:
							{
								Mid_TFTLCD_ShowString(COOR_MENU_ALARMMENU_AlARMSOURCE_X_BASE, (COOR_MENU_ALARMMENU_AlARMSOURCE_Y_BASE + 30 * i), "Remote: SOS        ", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
							}
							break;
						
							case SENSOR_TYPE_DOOR:
							{
								Mid_TFTLCD_ShowString(COOR_MENU_ALARMMENU_AlARMSOURCE_X_BASE, (COOR_MENU_ALARMMENU_AlARMSOURCE_Y_BASE + 30 * i), "Door:              ", LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
								Mid_TFTLCD_ShowString(COOR_MENU_ALARMMENU_AlARMSOURCE_X_BASE + 80, (COOR_MENU_ALARMMENU_AlARMSOURCE_Y_BASE + 30 * i), SensorParaBuff.SensorName, LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
							}
							break;
						
						}
					}
				}
			}
		}
	}
	
	/* blink "ALARMING" prompt display */
	BlinkCounter++;
	if(BlinkCounter > 150)
	{
		BlinkCounter = 0;
		BlinkFlag = !BlinkFlag;
		
		if(BlinkFlag)
		{
			Mid_TFTLCD_ShowString(COOR_MENU_ALARMMENU_PROMPT_1_X, COOR_MENU_ALARMMENU_PROMPT_1_Y, "ALARMING", LCD_FONT_COLOR, LCD_BACK_COLOR, 48, 0);
		}
		else
		{
			Mid_TFTLCD_ShowString(COOR_MENU_ALARMMENU_PROMPT_1_X, COOR_MENU_ALARMMENU_PROMPT_1_Y, "        ", LCD_FONT_COLOR, LCD_BACK_COLOR, 48, 0);
		}
	}
	
	/* update current Terminal WorkMode */
	pTerminalMode->Action();
}
	
/**
  * @Brief	Disarm process handler for AlarmMenu
  * @Param	None
  * @Retval	None
  */
static void App_AlarmMenu_DisarmHandler(void)
{
	en_KeyType_t KeyPress;		// indicate the index of pressed key
	en_KeyEvent_t KeyEvent;

	uint8_t KeyPressValue = 0;
	uint8_t KeyPressFlag = 0;	// 0->no effective key pressed, 1->effective key pressed
	
	static uint8_t CursorPos = 0;
	static uint16_t CursorBlinkCounter = 0;
	static uint8_t BlinkFlag = 0;
	static uint8_t Password[4];
	static uint8_t PasswordFillupFlag = 0;
	static uint8_t PasswordCorrectFlag = 0;
	static uint16_t Counter = 0;
	
	/* Reset Screen */
	if(pMenu->ScreenCMD == SCREEN_CMD_RESET)
	{
		TimeoutCounter_ReturnDesktop = 0;
		TimeoutCounter_ScreenSleep = 0;
		
		pMenu->ScreenCMD = SCREEN_CMD_NULL;
		
		Mid_TFTLCD_ScreenClear();
		Mid_TFTLCD_ShowString(COOR_MENU_DISARM_TITLE_X, COOR_MENU_DISARM_TITLE_Y, "System Disarm", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
		
		CursorPos = 0;
		BlinkFlag = 0;
		CursorBlinkCounter = 0;
		PasswordFillupFlag = 0;
		PasswordCorrectFlag = 0;
		Counter = 0;
		
		Password[0] = ' ';
		Password[1] = ' ';
		Password[2] = ' ';
		Password[3] = ' ';
	}
	
	/* enter PIN not complete: */
	if(!PasswordFillupFlag)
	{
		CursorBlinkCounter++;
		
		if(CursorBlinkCounter > 30)
		{
			CursorBlinkCounter = 0;
			BlinkFlag = !BlinkFlag;
			
			if(CursorPos < 4)
			{
				if(BlinkFlag)
				{
					Mid_TFTLCD_ShowString(75 + 48*CursorPos, 100, " ", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
				}
				else
				{
					Mid_TFTLCD_ShowString(75 + 48*CursorPos, 100, "_", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
				}
			}		
		}
		
		/* DisarmMenu KeyScan: */
		if((pMenu->KeyIndex != 0xFF) && (!PasswordFillupFlag))
		{
			TimeoutCounter_ReturnDesktop = 0;
			TimeoutCounter_ScreenSleep = 0;
			
			KeyPress = (en_KeyType_t)pMenu->KeyIndex;
			KeyEvent = (en_KeyEvent_t)pMenu->KeyEvent;
			
			pMenu->KeyIndex = 0xFF;
			
			if(KeyEvent == KEY_CLICK)
			{
				switch((uint8_t)KeyPress)
				{
					case KEY_CALL_CANCEL:
					{
						pMenu = pMenu->pParent;
						pMenu->ScreenCMD = SCREEN_CMD_RECOVER;
					}
					break;
					
					case KEY_0:
					{
						KeyPressValue = '0';
						KeyPressFlag = 1;
					}
					break;
					
					case KEY_1:
					{
						KeyPressValue = '1';
						KeyPressFlag = 1;
					}
					break;
					
					case KEY_2_UP:
					{
						KeyPressValue = '2';
						KeyPressFlag = 1;
					}
					break;
					
					case KEY_3:
					{
						KeyPressValue = '3';
						KeyPressFlag = 1;
					}
					break;
					
					case KEY_4_LEFT:
					{
						KeyPressValue = '4';
						KeyPressFlag = 1;
					}
					break;
					
					case KEY_5:
					{
						KeyPressValue = '5';
						KeyPressFlag = 1;
					}
					break;
					
					case KEY_6_RIGHT:
					{
						KeyPressValue = '6';
						KeyPressFlag = 1;
					}
					break;
					
					case KEY_7:
					{
						KeyPressValue = '7';
						KeyPressFlag = 1;
					}
					break;
					
					case KEY_8_DOWN:
					{
						KeyPressValue = '8';
						KeyPressFlag = 1;
					}
					break;
					
					case KEY_9:
					{
						KeyPressValue = '9';
						KeyPressFlag = 1;
					}
					break;
					
					case KEY_MENU_CONFIRM:
					{
						if(( Password[0] == Device_Get_SystemPara_AdminPassword(0) && 
							 Password[1] == Device_Get_SystemPara_AdminPassword(1) && 
							 Password[2] == Device_Get_SystemPara_AdminPassword(2) && 
							 Password[3] == Device_Get_SystemPara_AdminPassword(3)	  )	|| 
						   /* Super adminpassword: 0000 */
						   ( Password[0] == 0 && 
							 Password[1] == 0 && 
							 Password[2] == 0 && 
							 Password[3] == 0 										  ))
						{
							PasswordFillupFlag = 1;
							PasswordCorrectFlag = 1;
							
							Counter = 0;
						}
						else
						{
							PasswordFillupFlag = 1;
							
							Counter = 0;
						}
						
						if(PasswordCorrectFlag == 1)
						{
							Mid_TFTLCD_ShowString(COOR_MENU_DISARM_PROMPT_X, COOR_MENU_DISARM_PROMPT_Y, "Disarm Success", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
						}
						else
						{
							Mid_TFTLCD_ShowString(COOR_MENU_DISARM_PROMPT_X, COOR_MENU_DISARM_PROMPT_Y, "Wrong Password", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
						}
						
					}
					break;
					
					case KEY_SOS_DELET:
					{
						if(CursorPos > 0)
						{
							if(CursorPos < 4)
							{
								Password[CursorPos] = ' ';
							}
							else if(CursorPos == 4)
							{
								Password[3] = ' ';
							}
							
							Mid_TFTLCD_ShowString(75 + 48*CursorPos, 100, " ", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
							
							CursorPos--;
						}
					}
					break;
				}
			}
		}
		
		if(CursorPos < 4)
		{
			if(KeyPressFlag)
			{
				KeyPressFlag = 0;
				
				Password[CursorPos] = KeyPressValue - '0';
				
				if(CursorPos == 3)
				{
					Mid_TFTLCD_ShowString(75 + 48*CursorPos, 100, "*", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
				
					CursorPos = 4;
				}
				else
				{
					Mid_TFTLCD_ShowString(75 + 48*CursorPos, 100, "*", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
					
					CursorPos++;
				}
				
				CursorBlinkCounter = 0;
			}
		}
		else
		{
			CursorPos = 4;
		}
	}
	
	/* enter PIN complete: */
	if(PasswordFillupFlag)
	{
		Counter++;
		
		if(Counter > 200)
		{
			Counter = 0;
			PasswordFillupFlag = 0;
			
			if(PasswordCorrectFlag)
			{
				PasswordCorrectFlag = 0;
				
				App_Terminal_ModeChange(0xFF, TERMINAL_WORK_MODE_DISARM, TERMINAL_CMD_SOURCE_KEY);
			}
			else
			{
				pMenu = pMenu->pParent;
				pMenu->ScreenCMD = SCREEN_CMD_RECOVER;
			}
		}
	}
}

/*-------------Interrupt Functions Definition--------*/


