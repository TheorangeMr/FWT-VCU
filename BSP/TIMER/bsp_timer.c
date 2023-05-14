/*******************************************
	*文件名 ：  bsp_timer.c
	*作   者：  罗成
	*修改时间： 2022.05.2
	*版   本：  v1.0
  *说   明：  BSP/TIMER文件
*******************************************/

#include "bsp_timer.h"

#if TIM1_INPUT_CAPTURE_MODE

/* 
	*函数名：Timer1_Init()
	*功  能：TIMER1初始化函数
	*作  者：罗成
	*参  数：u16 psc:预分频系数, u16 arr：自动重装载值
	*返回值：无
	*时  间：2022.05.2
*/

static void NVIC_Time1_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
		/* Enable the TIM1 global Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = ADVANCE_TIM_CCx_IRQ; 										//定时器1通道1捕获/比较中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* 配置定时器1更新中断 */
	NVIC_InitStructure.NVIC_IRQChannel = ADVANCE_TIM_IRQ; 												//定时器1更新中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;    									//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;           									//子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


void Timer1_Init(u16 psc, u16 arr)
{
	TIM_ICInitTypeDef TIM_ICInitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;


	/* TIM1 clock enable */
	RCC_APB2PeriphClockCmd(ADVANCE_TIM_CLK, ENABLE);
	/* GPIOA clock enable */
	RCC_APB2PeriphClockCmd(ADVANCE_TIM_CH1_GPIO_CLK, ENABLE);

	/* TIM1 channel 1 pin (PA.08) configuration */
	GPIO_InitStructure.GPIO_Pin = ADVANCE_TIM_CH1_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;                         //下降沿触发设置为上拉输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(ADVANCE_TIM_CH1_GPIO_PORT, &GPIO_InitStructure);

	/* 配置定时器1 */
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;               //设置为向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;			              //不分频
	TIM_TimeBaseInitStructure.TIM_Period = arr;											              //自动装载初值
	TIM_TimeBaseInitStructure.TIM_Prescaler = psc;									              //预分频系数
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = ADVANCE_TIM_RepetitionCount;//高级定时器必须配置
	TIM_TimeBaseInit(ADVANCE_TIMEX, &TIM_TimeBaseInitStructure);				          //初始化定时器

	/* 配置定时器1输入捕获 */
	TIM_ICInitStructure.TIM_Channel = ADVANCE_TIM_CHANNEL_x;								      //通道1
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;		                //上升沿触发
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;               //直接映射(TI1-->IC1)
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;						        			//不分频
	TIM_ICInitStructure.TIM_ICFilter = 0x8;													       			  //输入滤波
	TIM_ICInit(ADVANCE_TIMEX, &TIM_ICInitStructure); 

  NVIC_Time1_Config();
	TIM_ClearITPendingBit(ADVANCE_TIMEX, TIM_FLAG_Update | ADVANCE_TIM_IT_CCx); 	/*使能更新中断前清除中断标志，
																																								防止一开始使能就进入中断*/
//	/* TIM enable counter */
//	TIM_Cmd(TIM1, ENABLE);
	/* Enable the CC1 and Update Interrupt Request */
//	TIM_ITConfig(TIM1, TIM_IT_CC1 | TIM_IT_Update, ENABLE);
}



#endif


static void NVIC_Time7_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
		/* 配置定时器7更新中断 */
	NVIC_InitStructure.NVIC_IRQChannel = BASIC_TIM_IRQ; 													//定时器7更新中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

static void NVIC_Time6_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
		/* 配置定时器6更新中断 */
	NVIC_InitStructure.NVIC_IRQChannel = CPU_BASIC_TIM_IRQ; 													//定时器6更新中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

static void NVIC_Time5_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
		/* 配置定时器5更新中断 */
	NVIC_InitStructure.NVIC_IRQChannel = CPU_NORMAL_TIM_IRQ; 													//定时器5更新中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

static void NVIC_Time4_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
		/* 配置定时器5更新中断 */
	NVIC_InitStructure.NVIC_IRQChannel = OIL_NORMAL_TIM_IRQ; 													//定时器5更新中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


/* 
	*函数名：Timer7_Init()
	*功  能：TIMER7初始化函数
	*作  者：罗成
	*参  数：u16 psc:预分频系数, u16 arr：自动重装载值
	*返回值：无
	*时  间：2022.05.2
*/

void Timer7_Init(u16 psc, u16 arr)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;	
	/* TIM7 clock enable */
	RCC_APB1PeriphClockCmd(BASIC_TIM_CLK, ENABLE);

	/* 配置定时器7 */
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; 							//设置为向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;										//不分频
	TIM_TimeBaseInitStructure.TIM_Period = arr;																		//自动装载初值
	TIM_TimeBaseInitStructure.TIM_Prescaler = psc;																//预分频系数
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;													//高级定时器必须配置
	TIM_TimeBaseInit(BASIC_TIMX, &TIM_TimeBaseInitStructure);											//初始化定时器
	
	NVIC_Time7_Config();
	TIM_ITConfig(BASIC_TIMX, TIM_IT_Update, ENABLE);
	TIM_Cmd(BASIC_TIMX, DISABLE);                                       			    //关闭定时器7
}




void Timer6_Init(u16 psc, u16 arr)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;	
	/* TIM6 clock enable */
	RCC_APB1PeriphClockCmd(CPU_BASIC_TIM_CLK, ENABLE);

	/* 配置定时器6 */
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; 							//设置为向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;										//不分频
	TIM_TimeBaseInitStructure.TIM_Period = arr;																		//自动装载初值
	TIM_TimeBaseInitStructure.TIM_Prescaler = psc;																//预分频系数
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;													//高级定时器必须配置
	TIM_TimeBaseInit(CPU_BASIC_TIM, &TIM_TimeBaseInitStructure);											//初始化定时器
	
	NVIC_Time6_Config();
	TIM_TimeBaseInit(CPU_BASIC_TIM, &TIM_TimeBaseInitStructure);											// 初始化定时器
	TIM_ClearFlag(CPU_BASIC_TIM, TIM_FLAG_Update);																		// 清除计数器中断标志位
	TIM_ITConfig(CPU_BASIC_TIM,TIM_IT_Update,ENABLE);																	// 开启计数器中断
   TIM_Cmd(CPU_BASIC_TIM, ENABLE);																									// 使能计数器
}

void Timer5_Init(u16 psc, u16 arr)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;	
	/* TIM5 clock enable */
	RCC_APB1PeriphClockCmd(NORMAL_TIM_CLK5, ENABLE);

	/* 配置定时器5 */
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; 							//设置为向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;										//不分频
	TIM_TimeBaseInitStructure.TIM_Period = arr;																		//自动装载初值
	TIM_TimeBaseInitStructure.TIM_Prescaler = psc;																//预分频系数
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;													//高级定时器必须配置
	TIM_TimeBaseInit(CPU_NORMAL_TIM, &TIM_TimeBaseInitStructure);											//初始化定时器
	
	NVIC_Time5_Config();
	TIM_TimeBaseInit(CPU_NORMAL_TIM, &TIM_TimeBaseInitStructure);											// 初始化定时器
	TIM_ClearFlag(CPU_NORMAL_TIM, TIM_FLAG_Update);																		// 清除计数器中断标志位
	TIM_ITConfig(CPU_NORMAL_TIM,TIM_IT_Update,ENABLE);																// 开启计数器中断
  TIM_Cmd(CPU_NORMAL_TIM, DISABLE);																									// 使能计数器
}

void Timer4_Init(u16 psc, u16 arr)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;	
	/* TIM1 clock enable */
	RCC_APB1PeriphClockCmd(NORMAL_TIM_CLK4, ENABLE);

	/* 配置定时器5 */
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; 							//设置为向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;										//不分频
	TIM_TimeBaseInitStructure.TIM_Period = arr;																		//自动装载初值
	TIM_TimeBaseInitStructure.TIM_Prescaler = psc;																//预分频系数
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;													//高级定时器必须配置
	TIM_TimeBaseInit(OIL_NORMAL_TIM, &TIM_TimeBaseInitStructure);											//初始化定时器
	
	NVIC_Time4_Config();
	TIM_TimeBaseInit(OIL_NORMAL_TIM, &TIM_TimeBaseInitStructure);											// 初始化定时器
	TIM_ClearFlag(OIL_NORMAL_TIM, TIM_FLAG_Update);																		// 清除计数器中断标志位
	TIM_ITConfig(OIL_NORMAL_TIM,TIM_IT_Update,ENABLE);																// 开启计数器中断
  TIM_Cmd(OIL_NORMAL_TIM, DISABLE);																									// 使能计数器
}



