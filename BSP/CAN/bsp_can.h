#ifndef _BSP_CAN_H
#define _BSP_CAN_H

#include "stm32f10x.h"

/************************************************************
								CAN相关引脚和时钟宏定义
************************************************************/
#define CANx                       CAN1
#define CAN_CLK                    RCC_APB1Periph_CAN1
#define CAN_RX_IRQ								 USB_LP_CAN1_RX0_IRQn
#define CAN_RX_IRQHandler					 USB_LP_CAN1_RX0_IRQHandler

/* CAN RX引脚*/
#define CAN_RX_PORT                GPIOA
#define CAN_RX_CLK                 RCC_APB2Periph_GPIOA
#define CAN_RX_PIN                 GPIO_Pin_11

/* CAN TX引脚 */
#define CAN_TX_PORT                GPIOA
#define CAN_TX_CLK                 (RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOA)
#define CAN_TX_PIN                 GPIO_Pin_12


/* 传输返回标志 */
#define FAILED (0) //传输失败
#define PASSED (1) //传输成功




/*****************************************************************
								       CAN对外函数接
*****************************************************************/
void CAN_Config(void);
uint8_t CAN_SendMsg(u32 Ext_ID ,u8* zdata, u8 len);
uint8_t CAN_ReciveMsg(u8 *zdata, u8 len);

#endif /* !_BSP_CAN_H */


