/****************************************************
  * @Name	Hal_GPIO.c
  * @Brief	Config LED pins and parameters
			Config AC_charger pins and parameters
			Config SPI2 pins and parameters
			Config TFTLCD pins and parameters
			Config AL6630(TemperatureHumidity) pins and parameters
			Config Key pins and parameters
			Config WiFi-Powermanage pins and parameters
			Config GSM-Powermanage pins and parameters
			Config VoltageLevel-Detect pins and parameters
			Config I2C-EEPROM pins and parameters
			
			Detect outsource power(USB_5v)
  ***************************************************/

/*-------------Header Files Include-----------------*/
#include "stm32f10x.h"                  
#include "hal_gpio.h"

/*-------------Internal Functions Declaration------*/
static void Hal_GPIO_LEDConfig(void);
static void Hal_GPIO_ACStateConfig(void);
static void Hal_GPIO_SPI2Config(void);
static void Hal_GPIO_TFTLCDConfig(void);
static void Hal_GPIO_AL6630Config(void);
static void Hal_GPIO_KeyConfig(void);
static void Hal_GPIO_WiFiPowerConfig(void);
static void Hal_GPIO_GSMPowerConfig(void);
static void Hal_GPIO_VolageLevelConfig(void);
static void Hal_GPIO_I2CConfig(void);

static uint8_t 	Hal_GPIO_GetACState(void);
static void 	Hal_GPIO_DelayTest(uint32_t i);

/*-------------Module Variables Declaration--------*/

static en_ACLinkSta_t AcState;	// state of USB_5v pin

/*-------------Module Functions Definition---------*/
/**
  * @Brief	Initialize GPIO module
  * @Param	None
  * @Retval	None
  */
void Hal_GPIO_Init(void)
{		
	Hal_GPIO_LEDConfig();
	Hal_GPIO_ACStateConfig();
	Hal_GPIO_SPI2Config();
	Hal_GPIO_TFTLCDConfig();
	Hal_GPIO_AL6630Config();
	Hal_GPIO_KeyConfig();
	Hal_GPIO_WiFiPowerConfig();
	Hal_GPIO_GSMPowerConfig();
	Hal_GPIO_VolageLevelConfig();
	Hal_GPIO_I2CConfig();

	AcState = (en_ACLinkSta_t)Hal_GPIO_GetACState();
	
	Hal_GPIO_WiFiPower_Enable();
}

/**
  * @Brief	Get AC_Check pin state
  * @Param	None
  * @Retval	AC_Check State
  * 	1-> USB_5v connected
  *		0-> USB_5v disconnected
  */
en_ACLinkSta_t Hal_GPIO_ACStateCheck(void)
{
	en_ACLinkSta_t state;
	
	static uint8_t timer = 0;
	
	state = (en_ACLinkSta_t)Hal_GPIO_GetACState();
	
	if(state == AcState)
	{
		timer = 0;
	}
	else if(state != AcState)
	{
		timer++;
		
		if(timer > 20)
		{
			timer = 0;
			AcState = state;
		}
	}
	
	return AcState;
}

/**
  * @Brief	Set AL6630 SDA Pin mode
  * @Param	Mode: 
  *		INPUT
  *		OUTPUT
  * @Retval	None
  */
void Hal_GPIO_AL6630PinModeSet(en_AL6630PinMode_t Mode)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	if(Mode == OUTPUT)
	{
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Pin = AL6630_SDA_PIN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(AL6630_SDA_PORT, &GPIO_InitStructure);
		
		GPIO_ResetBits(AL6630_SDA_PORT, AL6630_SDA_PIN);	
	}
	else if(Mode == INPUT)
	{
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
		GPIO_InitStructure.GPIO_Pin = AL6630_SDA_PIN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(AL6630_SDA_PORT, &GPIO_InitStructure);
		
		GPIO_ResetBits(AL6630_SDA_PORT, AL6630_SDA_PIN);	// set Low
	}
}

/**
  * @Brief	Turn on Power of WiFi module
  * @Param	None
  * @Retval	None
  */
void Hal_GPIO_WiFiPower_Enable(void)
{
	GPIO_SetBits(WIFI_POWEREN_PORT, WIFI_POWEREN_PIN);
}

/**
  * @Brief	Turn off Power of WiFi module
  * @Param	None
  * @Retval	None
  */
void Hal_GPIO_WiFiPower_Disable(void)
{
	GPIO_ResetBits(WIFI_POWEREN_PORT, WIFI_POWEREN_PIN);
}


/*-------------Internal Functions Definition--------*/
/**
  * @Brief	Config GPIO (LED) 
  * @Param	None
  * @Retval	None
  */
static void Hal_GPIO_LEDConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = LED7_PIN | LED8_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(LED7_PORT, &GPIO_InitStructure);
	
	GPIO_ResetBits(LED7_PORT, LED7_PIN);
	GPIO_ResetBits(LED8_PORT, LED8_PIN);
}

/**
  * @Brief	Config GPIO (CHECK_ACSTATE)
  * @Param	None
  * @Retval	None
  */
static void Hal_GPIO_ACStateConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = CHECK_ACSTATE_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(CHECK_ACSTATE_PORT, &GPIO_InitStructure);
}

/**
  * @Brief	Config GPIO (SPI2)
  * @Param	None
  * @Retval	None
  */
static void Hal_GPIO_SPI2Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
	// SPI2 pins: SCK, MISO, MOSI
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = SPI2_SCK_PIN | SPI2_MISO_PIN | SPI2_MOSI_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SPI2_SCK_PORT, &GPIO_InitStructure);
	
	// SPI2 pins: NSS
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = SPI2_NSS_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SPI2_NSS_PORT, &GPIO_InitStructure);
	
	GPIO_SetBits(SPI2_NSS_PORT, SPI2_NSS_PIN);
}

/**
  * @Brief	Config GPIO (TFTLCD/SPI3)
  * @Param	None
  * @Retval	None
  */
static void Hal_GPIO_TFTLCDConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	
	// TFTLCD_CS
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = TFTLCD_CS_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(TFTLCD_CS_PORT, &GPIO_InitStructure);
	
	GPIO_SetBits(TFTLCD_CS_PORT, TFTLCD_CS_PIN);
	
	// TFTLCD_DIO(SPI3_MOSI)
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = TFTLCD_DIO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(TFTLCD_DIO_PORT, &GPIO_InitStructure);
	
	// TFTLCD_CMD
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = TFTLCD_CMD_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(TFTLCD_CMD_PORT, &GPIO_InitStructure);
	
	GPIO_SetBits(TFTLCD_CMD_PORT, TFTLCD_CMD_PIN);
	
	// TFTLCD_CLK(SPI3_SCK)
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = TFTLCD_CLK_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(TFTLCD_CLK_PORT, &GPIO_InitStructure);
	
	// TFTLCD_RES
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = TFTLCD_RES_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(TFTLCD_RES_PORT, &GPIO_InitStructure);
	
	GPIO_SetBits(TFTLCD_RES_PORT, TFTLCD_RES_PIN);
	
	// TFTLCD_LEDA_EN
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = TFTLCD_LEDA_EN_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(TFTLCD_LEDA_EN_PORT, &GPIO_InitStructure);
	
	GPIO_ResetBits(TFTLCD_LEDA_EN_PORT, TFTLCD_LEDA_EN_PIN);
	
}

/**
  * @Brief	Config GPIO(AL6630)
  * @Param	None
  * @Retval	None
  */
static void Hal_GPIO_AL6630Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = AL6630_SDA_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(AL6630_SDA_PORT, &GPIO_InitStructure);
	
	GPIO_SetBits(AL6630_SDA_PORT, AL6630_SDA_PIN);
}

/**
  * @Brief	Config key pins and parameters
  * @Param	None
  * @Retval	None
  */
static void Hal_GPIO_KeyConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = KEY_DB4_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(KEY_DB4_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = KEY_DB0_PIN | KEY_DB1_PIN | KEY_DB2_PIN | KEY_DB3_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(KEY_DB0_PORT, &GPIO_InitStructure);
}

/**
  * @Brief	Config WiFi-Module powermanage pins and parameters
  * @Param	None
  * @Retval	None
  */
static void Hal_GPIO_WiFiPowerConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = WIFI_POWEREN_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(WIFI_POWEREN_PORT, &GPIO_InitStructure);
	
}

/**
  * @Brief	Config BatteryLevel-Detect pins and parameters
  * @Param	None
  * @Retval	None
  */
static void Hal_GPIO_VolageLevelConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
	// PB0-Battery Voltage Level Detect
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;	// analog input
	GPIO_InitStructure.GPIO_Pin = BATTERY_LEVEL_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(BATTERY_LEVEL_PORT, &GPIO_InitStructure);
	
	// PC0-Reference Voltage(2.5v) Detect
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;	
	GPIO_InitStructure.GPIO_Pin = REFERENCE_2_5V_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(REFERENCE_2_5V_PORT, &GPIO_InitStructure);
}

/**
  * @Brief	Config I2C-EEPROM pins and parameters
  * @Param	None
  * @Retval	None
  */
static void Hal_GPIO_I2CConfig(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure; 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	/* Configure I2C2 pins: PB8->SCL and PB9->SDA */
	GPIO_InitStructure.GPIO_Pin =  I2C_SCL_PIN | I2C_SDA_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;  
	GPIO_Init(I2C_SCL_PORT, &GPIO_InitStructure);
  
	GPIO_SetBits(I2C_SDA_PORT, I2C_SDA_PIN);
	GPIO_SetBits(I2C_SCL_PORT, I2C_SCL_PIN);
}


/**
  * @Brief	Get pin-state of AC_Check
  * @Param	None
  * @Retval	PinState
  * 	1-> High
  *		0-> Low
  */
static uint8_t Hal_GPIO_GetACState(void)
{
	return (GPIO_ReadInputDataBit(CHECK_ACSTATE_PORT, CHECK_ACSTATE_PIN));
}

/**
  * @Brief	Delay function for test
  * @Param	i: the number of delay cycles
  *		uint16_t: can be 0 - 0xFFFF FFFF
  * @Retval	None
  */
static void Hal_GPIO_DelayTest(uint32_t i)
{
	while(i--);
}
