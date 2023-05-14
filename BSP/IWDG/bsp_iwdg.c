/*******************************************
	*文件名 ：  bsp_iwdg.c
	*作   者：  罗成
	*修改时间： 2022.03.13
	*版   本：  v1.0
  *说   明：  IWDG文件
*******************************************/

#include "bsp_iwdg.h"

void WWDG_Config(uint8_t prer,uint16_t rlr)
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(prer);
	IWDG_SetReload(rlr);
	IWDG_ReloadCounter();
	IWDG_Enable();
}
