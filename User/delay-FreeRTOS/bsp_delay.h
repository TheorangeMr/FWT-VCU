/*******************************************
	*文件名 ：  bsp_delay.h
	*作   者：  罗成
	*修改时间： 2022.06.13
	*版   本：  v1.0
  *说   明：  延时函数配置头文件
*******************************************/

/*
    SysTick定时器的详细资料可查看Cortex-M3权威指南（中文）
    
    SysTick->CTRL控制寄存器主要位说明：
    bit0：SysTick 定时器的使能位
    
    bit1：SysTick 定时器 SysTick 异常（中断）请求使能位（1：ENABLE, 0：DISABLE）。
          ENABLE条件下：SysTick 倒数到 0 时产生 SysTick 异常请求。
    
    bit16：如果在上次读取本寄存器后， SysTick 已经数到了0，则该位为 1。
           通过读取bit16可以判断计数是否结束，如果读取该位，该位将自动清零。
    		    								   


    注意nms的范围
    SysTick->LOAD为24位寄存器,所以,最大延时为:
    nms<=0xffffff*8*1000/SYSCLK
    SYSCLK单位为Hz,nms单位为ms
    对72M条件下,nms<=1864 
    
    注意：延时函数的实现，没有使用SysTick 定时器 SysTick 异常（中断）来实现，而是通过不断读取bit16来判断
          计数是否结束，毕竟us级的中断太消耗CPU了。
*/

#define SYSTEM_SUPPORT_OS		1		//定义系统文件夹是否支持UCOS

#ifndef _BSP_DELAY_H
#define _BSP_DELAY_H 			   
#include "stm32f10x.h"  
#include "FreeRTOS.h"
#include "task.h"
	 
void Delay_Init(u8 SYSCLK);
void delay_ms(u16 nms);
void delay_us(u32 nus);
void delay_xms(u32 nms);
#endif


