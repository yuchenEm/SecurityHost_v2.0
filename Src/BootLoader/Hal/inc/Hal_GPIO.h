#ifndef __HAL_GPIO_H_
#define __HAL_GPIO_H_

/* LED Pin */
#define LED7_PORT	GPIOC
#define LED7_PIN	GPIO_Pin_15

#define LED8_PORT	GPIOC
#define LED8_PIN	GPIO_Pin_14

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


/*-------------Module Functions Declaration---------*/
void Hal_GPIO_Init(void);

#endif
