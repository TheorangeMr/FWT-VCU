/*******************************************
	*文件名 ：  bsp_delay.c
	*作   者：  罗成
	*修改时间： 2022.06.13
	*版   本：  v1.0
  *说   明：  延时函数配置源文件
*******************************************/

#include "bsp_delay.h"

static u8 fac_us = 0;	 //us延时倍乘因子（1us对应systick时钟数）
static u16 fac_ms = 0; //ms延时倍乘因子（1ms对应systick时钟数）


void Delay_Init(u8 SYSCLK)
{
	#if SYSTEM_SUPPORT_OS
	u32 reload;
	#endif	
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK); //选择外部时钟  HCLK
	fac_us = SYSCLK;
	
	#if SYSTEM_SUPPORT_OS  							//如果需要支持OS.		
	reload = SYSCLK;
	reload*=1000000/configTICK_RATE_HZ;
	
	fac_ms = 1000/configTICK_RATE_HZ;
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;          //开启SYSTICK中断
	SysTick->LOAD = reload;
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;          //开启SYSTICK
	
	#else
		fac_ms=(u16)fac_us*1000;					//非OS下,代表每个ms需要的systick时钟数   
	#endif
}
	


#if SYSTEM_SUPPORT_OS  							//如果需要支持OS.

void delay_us(u32 nus)
{
	u32 ticks;
	u32 told,tnow,tcnt=0;
	u32 reload=SysTick->LOAD;					//LOAD的值	    	 
	ticks=nus*fac_us; 							//需要的节拍数
	told=SysTick->VAL;        					//刚进入时的计数器值
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;		//这里注意一下SYSTICK是一个递减的计数器就可以了.
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;				//时间超过/等于要延迟的时间,则退出.
		}
	};
}

//不会被调度器调度
void delay_xms(u32 nms)
{
	u32 i;
	for(i = 0;i<nms;i++)delay_us(1000);
}

//会被调度器调度
void delay_ms(u16 nms)
{	
	if(xTaskGetSchedulerState()!=taskSCHEDULER_NOT_STARTED)	//如果OS已经在跑了,并且不是在中断里面(中断里面不能任务调度)	    
	{		 
		if(nms>=fac_ms)							//延时的时间大于OS的最少时间周期 
		{ 
   			vTaskDelay(nms/fac_ms);		//OS延时
		}
		nms%=fac_ms;							//OS已经无法提供这么小的延时了,采用普通方式延时    
	}
	delay_us((u32)(nms*1000));					//普通方式延时  
}

#else //不用OS时
//延时nus
//nus为要延时的us数.		    								   
void delay_us(u32 nus)
{		
	u32 temp;	    	 
	SysTick->LOAD=nus*fac_us; 					//时间加载	  		 
	SysTick->VAL=0x00;        					//清空计数器
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//开始倒数	  
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));		//等待时间到达   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//关闭计数器
	SysTick->VAL =0X00;      					 //清空计数器	 
}
//延时nms
//注意nms的范围
//SysTick->LOAD为24位寄存器,所以,最大延时为:
//nms<=0xffffff*8*1000/SYSCLK
//SYSCLK单位为Hz,nms单位为ms
//对72M条件下,nms<=1864 
void delay_ms(u16 nms)
{	 		  	  
	u32 temp;		   
	SysTick->LOAD=(u32)nms*fac_ms;				//时间加载(SysTick->LOAD为24bit)
	SysTick->VAL =0x00;							//清空计数器
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//开始倒数  
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));		//等待时间到达   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//关闭计数器
	SysTick->VAL =0X00;       					//清空计数器	  	    
}
#endif

