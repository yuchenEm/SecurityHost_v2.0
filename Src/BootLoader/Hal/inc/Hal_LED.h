#ifndef __HAL_LED_H_
#define __HAL_LED_H_

/* LED Toggle function macro define */
#define Hal_LED7_Toggle()	GPIO_WriteBit(LED7_PORT, LED7_PIN, (BitAction)(1 - GPIO_ReadOutputDataBit(LED7_PORT, LED7_PIN)))
#define Hal_LED8_Toggle()	GPIO_WriteBit(LED8_PORT, LED8_PIN, (BitAction)(1 - GPIO_ReadOutputDataBit(LED8_PORT, LED8_PIN)))

extern volatile uint8_t DelayCounter;

/*-------------Module Functions Declaration---------*/
void Hal_LED_Init(void);
void Hal_LED_Pro(void);

#endif
