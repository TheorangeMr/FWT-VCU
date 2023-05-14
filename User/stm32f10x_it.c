/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "FreeRTOS.h"
#include "task.h"
#include "bsp_delay.h"
#include "bsp_timer.h"
#include "usart.h"
#include "event_groups.h"
#include "bsp_sdio_sdcard.h"
#include "bsp_pwr.h"

/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
	printf("eeror!!\r\n");
	__set_FAULTMASK(1);
	NVIC_SystemReset();
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
//void SVC_Handler(void)
//{
//}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
//void PendSV_Handler(void)
//{
//}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */

extern void xPortSysTickHandler(void);
//systick中断服务函数
void SysTick_Handler(void)
{	
#if (INCLUDE_xTaskGetSchedulerState  == 1 )
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
  {
#endif  /* INCLUDE_xTaskGetSchedulerState */  
    
    xPortSysTickHandler();
    
#if (INCLUDE_xTaskGetSchedulerState  == 1 )
  }
#endif  /* INCLUDE_xTaskGetSchedulerState */
}

extern vu8 RTC_Time_Flag;   
extern CanRxMsg RxMessage;

/* CAN中断服务函数 */
void CAN_RX_IRQHandler(void)
{
//	printf("Interrupt OK!\r\n");
	u8 i = 0;
	
	/* 初始化CAN接收器 */
	RxMessage.StdId=0x00;
  RxMessage.ExtId=0x00;
  RxMessage.IDE=0;
  RxMessage.DLC=0;
  RxMessage.FMI=0;
	
	for(i = 0; i < 8; i++)
	{
		RxMessage.Data[i]=0x00;
	}

	/* 接收数据 */
  CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);

	/* 判断是否为目标方发送,并对数据进行校验 */
  if((RxMessage.ExtId==0x2001) && (RxMessage.IDE==CAN_ID_EXT) && (RxMessage.DLC==3)
		 && (RxMessage.Data[0]==0x02) && (RxMessage.Data[2] == 0x18))
  {
		 RTC_Time_Flag = RxMessage.Data[1];			
  }
}



#define EVENTBIT_0	(1<<0)                          //测速结束事件
#define EVENTBIT_1	(1<<1)                          //串口打印事件
#define EVENTBIT_2	(1<<2)                          //里程计数事件

vu8 Actual_Endflag = 0;                              //实际结束标志
u8 Startup_Flag = 0;                                //开始采集标志
vu8 Endup_Flag = 0;                                  //结束标志
u32 Pulse_Count = 1;                                //传感器脉冲计数
vu32 Total_Time_M1 = 0;                               //M1  频率
vu32 Total_Time_M2 = 0;                               //M2
vu32 Overflow_Count = 0;                             
vu8  Sampling_Flag = 0;
extern EventGroupHandle_t EventGroupHandler;



/* 定时器1捕获/比较中断服务函数 */
void ADVANCE_TIM_CCx_IRQHandler(void)
{
	BaseType_t pxHigherPriorityTaskWoken,xResult;
	if (TIM_GetITStatus(ADVANCE_TIMEX, ADVANCE_TIM_IT_CCx) != 0) //通道1捕获
	{
		if(Startup_Flag == 1)
		{
			Pulse_Count++;                                                            //M2脉冲计数
		}
		if(((GPIOA->IDR & GPIO_Pin_8) >> 8) == 1&&Sampling_Flag == 0)               //确认引脚为高电平
		{
			if(Endup_Flag != 0)                                                       //采样周期结束捕获第二次上升沿
			{
				TIM_Cmd(ADVANCE_TIMEX,DISABLE);                                         //关闭定时器1计数
				TIM_ITConfig(ADVANCE_TIMEX, TIM_IT_CC1, DISABLE);		                    //禁止捕获/比较1中断
				Total_Time_M1 = Pulse_Count;                                            //获取采样周期内的M1
				Total_Time_M2 = TIM_GetCounter(ADVANCE_TIMEX);									        //获取高频时钟周期M2
				Pulse_Count = 1;                                                        //脉冲置1
				Startup_Flag = 0;                                                       //首次标志清零
				Actual_Endflag = 1;                                                     //实际采样结束标志EVENTBIT_2
				Endup_Flag = 0;
				/*事件置位*/
				pxHigherPriorityTaskWoken = pdFALSE;
				xResult = xEventGroupSetBitsFromISR(EventGroupHandler,EVENTBIT_0|EVENTBIT_2,&pxHigherPriorityTaskWoken);
				if(xResult != pdFAIL)
				{
					portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
				}
//				printf("timer 2\r\n");
			}
			else                            
			{
//				printf("timer 1\r\n");
				TIM_SetCounter(ADVANCE_TIMEX, 0);											                  //M1计数器清零
				Startup_Flag = 1;							                                          //标记首次捕获上升沿
				Sampling_Flag = 1;
			}
		}
	 }
	TIM_ClearITPendingBit(ADVANCE_TIMEX, ADVANCE_TIM_IT_CCx);
}

/* 定时器1更新中断服务函数 */
void ADVANCE_TIM_IRQHandler(void)
{
	//printf("ok\r\n");	
	if (TIM_GetITStatus(ADVANCE_TIMEX, TIM_IT_Update) != 0)                                //定时器1产生更新中断
	{
		if (Endup_Flag == 0)                                                        //如果捕获完成标志没有被置位
		{
			Overflow_Count++; //溢出计数
		}		
	}
	TIM_ClearITPendingBit(ADVANCE_TIMEX, TIM_IT_Update);                                   //清除中断标志;
}
/* 定时器7更新中断服务函数 */
void BASIC_TIM_IRQHandler(void)
{
	static u8 time = 0;
	if(TIM_GetITStatus(BASIC_TIMX, TIM_IT_Update) != 0)
	{
		time++;
		if(time >= 8)
		{
			Sampling_Flag = 0;
			Endup_Flag = 1;                                                           //规定周期采样结束标志
			time = 0;
			TIM_Cmd(BASIC_TIMX,DISABLE);                                                  //关闭定时器7计数
		}
	}
	TIM_ClearITPendingBit(BASIC_TIMX, TIM_IT_Update);
}


extern uint8_t Oil_base_dat;
/* 定时器4更新中断服务函数 */
void OIL_NORMAL_TIM_IRQHandler(void)
{
	static uint8_t tim4_count = 0;
	if(TIM_GetITStatus(OIL_NORMAL_TIM, TIM_IT_Update) != 0)
	{
		tim4_count++;
		if(tim4_count >= 120)
		{
			if(Oil_base_dat > 0)
			{
				Oil_base_dat--;
			}
      tim4_count = 0;		
		}
	}
	TIM_ClearITPendingBit(OIL_NORMAL_TIM, TIM_IT_Update);
}


/*串口中断服务函数*/

void DEBUG_USART_IRQHandler(void)
{
	BaseType_t pxHigherPriorityTaskWoken,xResult;
	if(USART_GetITStatus(DEBUG_USARTx, USART_IT_RXNE) != RESET)
	{
		uint8_t rdat = USART_ReceiveData(DEBUG_USARTx);
		USART_ClearITPendingBit(DEBUG_USARTx, USART_IT_RXNE);
		if(rdat == USART_ON)
		{
			printf(" \nSTART \r\n");
			pxHigherPriorityTaskWoken = pdFALSE;
			xResult = xEventGroupSetBitsFromISR(EventGroupHandler,EVENTBIT_1,&pxHigherPriorityTaskWoken);
			if(xResult != pdFAIL)
			{
				portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
			}
		}
		else if(rdat == USART_OFF)
		{
			printf(" \nSTOP \r\n");
			pxHigherPriorityTaskWoken = pdFALSE;
			xResult = xEventGroupClearBitsFromISR( EventGroupHandler, EVENTBIT_1 );
			if(xResult != pdFAIL)
			{
				portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
			}
		}
	}
}


void CPU_NORMAL_TIM_IRQHandler(void)
{
	static u8 time = 0;
	if(TIM_GetITStatus(CPU_NORMAL_TIM, TIM_IT_Update) != 0)
	{
		time++;
		if(time >= 20)
		{
			BKP_WriteBackupRegister(BKP_DR40, 0x18);
			TIM_Cmd(CPU_NORMAL_TIM,DISABLE);                                                  //关闭定时器5计数	
			Standby_mode();
		}
	}
	TIM_ClearITPendingBit(CPU_NORMAL_TIM, TIM_IT_Update);
}

void SDIO_IRQHandler(void)
{
  /* Process All SDIO Interrupt Sources */
  SD_ProcessIRQSrc();  //SDIO中断处理函数
}

/* 用于统计运行时间 */
volatile uint32_t CPU_RunTime = 0UL;

void  CPU_BASIC_TIM_IRQHandler (void)
{
	if ( TIM_GetITStatus( CPU_BASIC_TIM, TIM_IT_Update) != RESET ) 
	{	
    CPU_RunTime++;
		TIM_ClearITPendingBit(CPU_BASIC_TIM , TIM_FLAG_Update);  		 
	}		 	
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
