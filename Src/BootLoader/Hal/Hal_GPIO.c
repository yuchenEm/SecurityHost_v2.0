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

static void Hal_GPIO_SPI2Config(void);
static void Hal_GPIO_TFTLCDConfig(void);


/*-------------Module Variables Declaration--------*/



/*-------------Module Functions Definition---------*/
/**
  * @Brief	Initialize GPIO module
  * @Param	None
  * @Retval	None
  */
void Hal_GPIO_Init(void)
{		
	Hal_GPIO_LEDConfig();

	Hal_GPIO_SPI2Config();
	Hal_GPIO_TFTLCDConfig();


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
	
	GPIO_SetBits(LED7_PORT, LED7_PIN);
	GPIO_ResetBits(LED8_PORT, LED8_PIN);
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
	
	// TFTLCD_CMD(SPI3_MISO)
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
	
	// TFTLCD_RES(SPI3_NSS)
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

