/*******************************************
	*文件名  ：  bsp_dma.c
	*作   者 ：  WF
	*修改时间：  2021.03.13
	*版   本：   v1.0
    *说   明：   dma源文件
*******************************************/



#include "bsp_dma.h"



/* 
	*函数名：DMA_USART1_TX_Configuration()
	*功  能：USART1_TX DMA配置函数
	*作  者：WF
	*参  数：MemAddr：存储器地址，DataNum：数据个数
	*返回值：无
	*时  间：2021.03.13
*/
void DMA_USART1_TX_Configuration(u32 MemAddr, u16 DataNum)
{
	DMA_InitTypeDef  DMA_InitStructure;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 , ENABLE);                     //使能DMA时钟
	
	DMA_DeInit(DMA1_Channel4);                                              //复位
	
	/* 初始化USART1_TX DMA通道参数 */
	DMA_InitStructure.DMA_PeripheralBaseAddr  = (u32)&USART1->DR;           //外设基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = MemAddr;                         //存储器基地址           
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;                      //传输方向外设作为目标
	DMA_InitStructure.DMA_BufferSize = DataNum;                             //设置传输数据数量
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        //外设指针不自增模式
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 //存储器指针自增模式
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //外设数据宽度为8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         //存储器数据宽度为8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                           //正常模式不循环
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                   //中等优先级
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                            //不使用存储器到存储器模式
	DMA_Init(DMA1_Channel4 , &DMA_InitStructure );
}



/* 
	*函数名：DMA_USART1_RX_Configuration()
	*功  能：USART1_RX DMA配置函数
	*作  者：WF
	*参  数：MemAddr：存储器地址，DataNum：数据个数
	*返回值：无
	*时  间：2021.03.13
*/

void DMA_USART1_RX_Configuration(u32 MemAddr, u16 DataNum)
{
	DMA_InitTypeDef  DMA_InitStructure;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 , ENABLE);                     //使能DMA时钟
	
	DMA_DeInit(DMA1_Channel5);                                              //复位
	
	/* 初始化USART1_RX DMA通道参数 */
	DMA_InitStructure.DMA_PeripheralBaseAddr  = (u32)&USART1->DR;           //外设基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = MemAddr;                         //存储器基地址           
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;                      //传输方向外设-->存储器
	DMA_InitStructure.DMA_BufferSize = DataNum;                             //设置传输数据数量
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        //外设指针不自增模式
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 //存储器指针自增模式
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //外设数据宽度为8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         //存储器数据宽度为8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                           //正常模式不循环
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                   //中等优先级
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                            //不使用存储器到存储器模式
	DMA_Init(DMA1_Channel5 , &DMA_InitStructure );
}




/* 
	*函数名：DMA_ADC1_Configuration()
	*功  能：ADC1 DMA配置函数
	*作  者：WF
	*参  数：MemAddr：存储器地址，DataNum：数据个数
	*返回值：无
	*时  间：2021.03.13
*/
void DMA_ADC1_Configuration(u32 MemAddr, u16 DataNum)
{
	DMA_InitTypeDef  DMA_InitStructure;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 , ENABLE);            //使能DMA时钟
	
	DMA_DeInit(DMA1_Channel1);                                     //复位
	
	/* 初始化ADC1 DMA通道参数 */
	DMA_InitStructure.DMA_PeripheralBaseAddr  = (u32)&ADC1->DR;                 //外设基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = MemAddr;                             //存储器基地址           
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                          //传输方向外设-->内存
	DMA_InitStructure.DMA_BufferSize = DataNum;                                 //设置传输数据数量
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;            //外设指针不自增模式
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                     //存储器指针自增模式
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //外设数据宽度为16位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;         //存储器数据宽度为16位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                             //正常模式不循环
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                       //中等优先级
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                                //不使用存储器到存储器模式
	DMA_Init(DMA1_Channel1 , &DMA_InitStructure );
	
	DMA_Cmd(DMA1_Channel1 , ENABLE);                                            //使能DMA1通道1
}


/* 
	*函数名：DMA_USART1_TX_Transmission()
	*功  能：开启USART1_TX的DMA传输
	*作  者：WF
	*参  数：DataNum：数据个数
	*返回值：无
	*时  间：2021.03.13
*/
void DMA_USART1_TX_Transmission(u16 DataNum)
{
	DMA_Cmd(DMA1_Channel4 , DISABLE);                         //关闭DMA通道
	DMA_SetCurrDataCounter(DMA1_Channel4 ,DataNum);           //设置传送数量
	DMA_Cmd(DMA1_Channel4 , ENABLE);                          //使能DMA1通道4,发起一次传送
}


/* 
	*函数名：DMA_USART1_RX_Transmission()
	*功  能：开启USART1_RX的DMA传输
	*作  者：WF
	*参  数：DataNum：数据个数
	*返回值：无
	*时  间：2021.03.13
*/
void DMA_USART1_RX_Transmission(u16 DataNum)
{
	DMA_Cmd(DMA1_Channel5 , DISABLE);                         //关闭DMA通道
	DMA_SetCurrDataCounter(DMA1_Channel5 ,DataNum);           //设置传送数量
	DMA_Cmd(DMA1_Channel5 , ENABLE);                          //使能DMA1通道5,发起一次传送
}

