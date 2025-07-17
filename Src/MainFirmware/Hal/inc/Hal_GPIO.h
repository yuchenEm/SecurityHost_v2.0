#ifndef __HAL_GPIO_H_
#define __HAL_GPIO_H_

/* LED Pin */
#define LED7_PORT	GPIOC
#define LED7_PIN	GPIO_Pin_15

#define LED8_PORT	GPIOC
#define LED8_PIN	GPIO_Pin_14

/* AC-Check Pin */
#define CHECK_ACSTATE_PORT	GPIOB
#define CHECK_ACSTATE_PIN	GPIO_Pin_1

/* SPI2 Pin */
#define SPI2_SCK_PORT       GPIOB
#define SPI2_SCK_PIN        GPIO_Pin_13

#define SPI2_MOSI_PORT      GPIOB
#define SPI2_MOSI_PIN       GPIO_Pin_15

#define SPI2_MISO_PORT      GPIOB
#define SPI2_MISO_PIN       GPIO_Pin_14

#define SPI2_NSS_PORT       GPIOB
#define SPI2_NSS_PIN        GPIO_Pin_12

/* TFT_LCD Pin */
#define TFTLCD_CS_PORT			GPIOB
#define TFTLCD_CS_PIN			GPIO_Pin_6

#define TFTLCD_DIO_PORT			GPIOB
#define TFTLCD_DIO_PIN			GPIO_Pin_5		// PB5-SPI3_MOSI

#define TFTLCD_CMD_PORT			GPIOB
#define TFTLCD_CMD_PIN			GPIO_Pin_4		// PB4-SPI3_MISO, NJRST remap

#define TFTLCD_CLK_PORT			GPIOB
#define TFTLCD_CLK_PIN			GPIO_Pin_3		// PB3-SPI3_SCK, JTDO remap

#define TFTLCD_RES_PORT			GPIOA
#define TFTLCD_RES_PIN			GPIO_Pin_15		// PA15-SPI3_NSS, JTDI remap

#define TFTLCD_LEDA_EN_PORT		GPIOC
#define TFTLCD_LEDA_EN_PIN		GPIO_Pin_10	

/* AL6630 TH_Module SDA Pin */
#define AL6630_SDA_PORT			GPIOA
#define AL6630_SDA_PIN			GPIO_Pin_7

/* Key Pin */
#define KEY_DB0_PORT   	GPIOC
#define KEY_DB0_PIN    	GPIO_Pin_6

#define KEY_DB1_PORT   	GPIOC
#define KEY_DB1_PIN    	GPIO_Pin_7

#define KEY_DB2_PORT   	GPIOC
#define KEY_DB2_PIN    	GPIO_Pin_8

#define KEY_DB3_PORT   	GPIOC
#define KEY_DB3_PIN    	GPIO_Pin_9

#define KEY_DB4_PORT   	GPIOA
#define KEY_DB4_PIN    	GPIO_Pin_8

#define KEY_INT_PORT	GPIOC
#define KEY_INT_PIN		GPIO_Pin_5

/* Debug USART Pin */
#define DEBUG_TX_PORT		GPIOA
#define DEBUG_TX_PIN		GPIO_Pin_9

#define DEBUG_RX_PORT		GPIOA
#define DEBUG_RX_PIN		GPIO_Pin_10

#define DEBUG_USART_PORT	USART1

/* Lora USART Pin */
#define LORA_TX_PORT		GPIOC
#define LORA_TX_PIN			GPIO_Pin_12

#define LORA_RX_PORT		GPIOD
#define LORA_RX_PIN			GPIO_Pin_2

#define LORA_USART_PORT		UART5

/* WiFi USART Pin */
#define WIFI_TX_PORT		GPIOB
#define WIFI_TX_PIN			GPIO_Pin_10

#define WIFI_RX_PORT		GPIOB
#define WIFI_RX_PIN			GPIO_Pin_11

#define WIFI_USART_PORT		USART3

/* WiFi Module PowerManage Pin */
#define WIFI_POWEREN_PORT	GPIOC
#define WIFI_POWEREN_PIN 	GPIO_Pin_4	// ME2617 CE_pin: Low -> disable, High -> enable

/* VoltageLevel Detect Pin */
#define BATTERY_LEVEL_PORT				GPIOB
#define	BATTERY_LEVEL_PIN				GPIO_Pin_0	
#define BATTERY_LEVEL_ADC_CHANNEL		ADC_Channel_8	// ADC_1_2_Channel_8

#define AC_CHARGER_LEVEL_PORT			GPIOB
#define	AC_CHARGER_LEVEL_PIN			GPIO_Pin_1	
#define AC_CHARGER_LEVEL_ADC_CHANNEL	ADC_Channel_9	// ADC_1_2_Channel_9

#define REFERENCE_2_5V_PORT				GPIOC
#define REFERENCE_2_5V_PIN				GPIO_Pin_0	
#define REFERENCE_2_5V_ADC_CHANNEL		ADC_Channel_10	// ADC_1_2_Channel_10

/* I2C-EEPROM Pin */
#define I2C_SCL_PORT	GPIOB
#define I2C_SCL_PIN		GPIO_Pin_8

#define I2C_SDA_PORT	GPIOB
#define I2C_SDA_PIN		GPIO_Pin_9


/* AC charger link status */
typedef enum
{
	STA_AC_BREAK = 0,
	STA_AC_LINK,
}en_ACLinkSta_t;

/* TemHum sensor AL6630 Pin mode type */
typedef enum
{
	INPUT = 0,
	OUTPUT,
}en_AL6630PinMode_t;

/*-------------Module Functions Declaration---------*/
void Hal_GPIO_Init(void);

en_ACLinkSta_t Hal_GPIO_ACStateCheck(void);
void Hal_GPIO_AL6630PinModeSet(en_AL6630PinMode_t Mode);

void Hal_GPIO_WiFiPower_Enable(void);
void Hal_GPIO_WiFiPower_Disable(void);

#endif
