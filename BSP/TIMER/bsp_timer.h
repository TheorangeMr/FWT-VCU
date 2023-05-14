/*******************************************
	*文件名 ：  bsp_timer.h
	*作   者：  WF
	*修改时间： 2021.04.22
	*版   本：  v1.0
  *说   明：  定时器模块头文件
*******************************************/

#ifndef _BSP_TIMER_H
#define _BSP_TIMER_H

#include "stm32f10x.h"


#define TIM1_INPUT_CAPTURE_MODE 1              //定时器1：输入捕获模式

#define  BASIC_TIM_CLK                    RCC_APB1Periph_TIM7
#define  CPU_BASIC_TIM_CLK                RCC_APB1Periph_TIM6
#define  ADVANCE_TIM_CLK                  RCC_APB2Periph_TIM1
#define  NORMAL_TIM_CLK5									RCC_APB1Periph_TIM5
#define  NORMAL_TIM_CLK4									RCC_APB1Periph_TIM4
#define  BASIC_TIMX                       TIM7 
#define  CPU_BASIC_TIM                    TIM6
#define  ADVANCE_TIMEX                    TIM1
#define  CPU_NORMAL_TIM                   TIM5
#define  OIL_NORMAL_TIM                   TIM4

#define  ADVANCE_TIM_IT_CCx               TIM_IT_CC1  
#define  BASIC_TIM_IRQ                    TIM7_IRQn
#define  CPU_BASIC_TIM_IRQ                TIM6_IRQn
#define  CPU_NORMAL_TIM_IRQ								TIM5_IRQn
#define  OIL_NORMAL_TIM_IRQ								TIM4_IRQn
#define  BASIC_TIM_IRQHandler             TIM7_IRQHandler
#define  CPU_BASIC_TIM_IRQHandler         TIM6_IRQHandler
#define  CPU_NORMAL_TIM_IRQHandler				TIM5_IRQHandler
#define  OIL_NORMAL_TIM_IRQHandler				TIM4_IRQHandler
#define  ADVANCE_TIM_IRQ                  TIM1_UP_IRQn
#define  ADVANCE_TIM_CCx_IRQ              TIM1_CC_IRQn   
#define  ADVANCE_TIM_IRQHandler           TIM1_UP_IRQHandler
#define  ADVANCE_TIM_CCx_IRQHandler       TIM1_CC_IRQHandler


#define  ADVANCE_TIM_CH1_GPIO_CLK         RCC_APB2Periph_GPIOA
#define  ADVANCE_TIM_CH1_GPIO_PORT        GPIOA
#define  ADVANCE_TIM_CH1_PIN              GPIO_Pin_8
#define  ADVANCE_TIM_CHANNEL_x            TIM_Channel_1
#define  ADVANCE_TIM_RepetitionCount      0

/*****************************************************************************************
																定时器函数接口
*****************************************************************************************/
void Timer1_Init(u16 psc, u16 arr);
void Timer7_Init(u16 psc, u16 arr);
void Timer6_Init(u16 psc, u16 arr);
void Timer5_Init(u16 psc, u16 arr);
void Timer4_Init(u16 psc, u16 arr);
#endif /* !_BSP_TIMER_H */
