#ifndef __HAL_JUMPTOAPP_H_
#define __HAL_JUMPTOAPP_H_

/* Base-address of the App part in embedded Flash */
#define EMBEDDED_FLASH_Address_APP_BASE		0x0800C800


/* Function pointer of jump function define */
typedef void (*pIAP_Function_t)(void);


void Hal_JumpToApp_Jump(void);

#endif
