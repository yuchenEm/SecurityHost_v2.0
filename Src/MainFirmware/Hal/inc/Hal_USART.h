#ifndef __HAL_USART_H_
#define __HAL_USART_H_

/* Lora_USART_Rx call-back function typedef */
typedef void (*Lora_USART_RxCBF_t)(uint8_t RxData);

/* WiFi_USART_Rx call-back function typedef */
typedef void (*WiFi_USART_RxCBF_t)(uint8_t RxData);

/* GSM_USART_Rx call-back function typedef */
typedef void (*GSM_USART_RxCBF_t)(uint8_t RxData);

void Hal_USART_Init(void);
void Hal_USART_Pro(void);

void Hal_USART_LoraRxCBFRegister(Lora_USART_RxCBF_t pCBF);
void Hal_USART_WiFiRxCBFRegister(WiFi_USART_RxCBF_t pCBF);
void Hal_USART_GSMRxCBFRegister(GSM_USART_RxCBF_t pCBF);

void Hal_USART_DebugStringQueueIn(const char pData[]);
void Hal_USART_DebugDataQueueIn(uint8_t *pData, uint16_t Len);

void Hal_USART_LoraDataTx(uint8_t *pData, uint8_t Len);
void Hal_USART_WiFiDataTx(uint8_t *pData, uint8_t Len);
void Hal_USART_GSMDataTx(uint8_t *pData, uint8_t Len);
void Hal_USART_GSMStringTx(uint8_t *pData);

#endif
