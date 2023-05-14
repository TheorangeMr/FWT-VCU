#ifndef __UART3_H
#define __UART3_H

//#include "sys.h"
#include "ringbuffer.h"

/************************************************
 ALIENTEK 战舰STM32F103开发板扩展实验
 ATK-GPRS-M26(GPRS DTU)应用实例实验        设备端程序（Slave）
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司
 作者：正点原子 @ALIENTEK
************************************************/


void uart2_init(u32 bound);
void usart2_send_data(u8 *data, u32 size);


#endif




