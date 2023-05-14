/*******************************************
	*文件名 ：  bsp_pwr.c
	*作   者：  罗成
	*修改时间： 2022.06.10
	*版   本：  v1.0
*******************************************/

#include "bsp_pwr.h"


/*睡眠模式*/
void Sleep_mode(void)
{
	__WFI();
}

/*停止模式*/
void Stop_mode(void)
{
//	DBGMCU_Config(DBGMCU_IWDG_STOP,ENABLE);      /*无效*/
	PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
}

void SYSCLKConfig_STOP(void)
{
	  SystemInit();
}

void Stop_interface(void)
{
	Stop_mode();
	SYSCLKConfig_STOP();
}

/* 待机模式 */
void Standby_mode(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR , ENABLE);
	/*清除WU状态位*/
	PWR_ClearFlag(PWR_FLAG_WU);
//	/*使能WKUP引脚的唤醒功能，使能PA0*/
//	PWR_WakeUpPinCmd(ENABLE);
  /* 进入待机模式 */	
	PWR_EnterSTANDBYMode();
}


