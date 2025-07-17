#include "stm32f10x.h"             
#include "nvic_setting.h"

void NVIC_Setting(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}
