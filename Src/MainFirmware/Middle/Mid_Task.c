/****************************************************
  * @Name	Mid_Task.c
  * @Brief	Task module for all MDL modules
  ***************************************************/
  
/*-------------Header Files Include-----------------*/
#include "stm32f10x.h"
#include "mid_task.h"
#include "mid_flash.h"
#include "mid_tftlcd.h"
#include "mid_lora.h"
#include "mid_wifi.h"
#include "mid_mqtt.h"
#include "mid_eeprom.h"
#include "mid_powermanage.h"
#include "mqtt_protocol.h"

/*-------------Module Functions Definition---------*/
/**
  * @Brief	Initialize all Middle-layer modules
  * @Param	None
  * @Retval	None
  */
void Mid_Task_Init(void)
{
	Mid_Flash_Init();
	Mid_TFTLCD_Init();
	Mid_Lora_Init();
	Mid_WiFi_Init();
	Mid_MQTT_Init();
	Mid_PowerManage_Init();
	MQTTProtocol_Init();
}

/**
  * @Brief	Polling functions of Middle-layer modules
  * @Param	None
  * @Retval	None
  */
void Mid_Task_Pro(void)
{
	Mid_Lora_Pro();
	Mid_WiFi_Pro();
	Mid_PowerManage_Pro();
}
