/*******************************************
	*文件名  ：  bsp_dma.h
	*作   者 ：  WF
	*修改时间：  2021.03.13
	*版   本：   v1.0
    *说   明：   dma头文件
*******************************************/


#ifndef BSP_DMA_H
#define BSP_DMA_H

#include "stm32f10x.h"

void DMA_USART1_TX_Configuration(u32 MemAddr, u16 DataNum);
void DMA_USART1_RX_Configuration(u32 MemAddr, u16 DataNum);
void DMA_ADC1_Configuration(u32 MemAddr, u16 DataNum);
void DMA_USART1_TX_Transmission(u16 DataNum);
void DMA_USART1_RX_Transmission(u16 DataNum);


#endif /* !BSP_DMA_H */

