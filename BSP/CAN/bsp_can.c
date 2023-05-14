/*******************************************
	*文件名 ：  bsp_can.c
	*作   者：  WF
	*修改时间： 2022.10.5
	*版   本：  v1.1
  *说   明：  CAN驱动源文件
*******************************************/

#include "bsp_can.h"


/***********************************************************************
													CAN配置私有函数
***********************************************************************/

/* 
	*函数名：CAN_GPIO_Config()
	*功  能：CAN引脚配置函数
	*作  者：WF
	*参  数：无
	*返回值：无
	*时  间：2021.03.26
*/
static void CAN_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* 使能CAN的TX、RX引脚时钟 */
	RCC_APB2PeriphClockCmd(CAN_RX_CLK|CAN_TX_CLK, ENABLE);
	
	/* CAN_RX引脚配置为浮空输入或带上拉输入 */
	GPIO_InitStructure.GPIO_Pin = CAN_RX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(CAN_RX_PORT, &GPIO_InitStructure);
	
	/* CAN_TX引脚配置为推挽复用输出 */
	GPIO_InitStructure.GPIO_Pin = CAN_TX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(CAN_TX_PORT, &GPIO_InitStructure);
}

/* 
	*函数名：CAN_NVIC_Config()
	*功  能：CAN中断配置函数
	*作  者：WF
	*参  数：无
	*返回值：无
	*时  间：2021.03.26
*/
static void CAN_NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
//	/* 配置为中断优先级分组1 */
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	/* 中断配置 */
	NVIC_InitStructure.NVIC_IRQChannel = CAN_RX_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	
	NVIC_Init(&NVIC_InitStructure);
}

/* 
	*函数名：CAN_Mode_Config()
	*功  能：CAN模式配置函数
	*作  者：WF
	*参  数：无
	*返回值：无
	*时  间：2021.03.26
*/
static void CAN_Mode_Config(void)
{
	CAN_InitTypeDef CAN_InitStructure;
	
	RCC_APB1PeriphClockCmd(CAN_CLK, ENABLE);
	
	/* CAN register init */
  CAN_DeInit(CANx);
  CAN_StructInit(&CAN_InitStructure);

  /* CAN cell init */
  CAN_InitStructure.CAN_TTCM=DISABLE;           //关闭时间触发通信模式
  CAN_InitStructure.CAN_ABOM=ENABLE;           //关闭自动总线关闭管理
  CAN_InitStructure.CAN_AWUM=ENABLE;           //关闭自动唤醒模式
  CAN_InitStructure.CAN_NART=DISABLE;           //关闭禁止自动重传（即使能自动重传）
  CAN_InitStructure.CAN_RFLM=DISABLE;           //在接收溢出时FIFO未被锁定
  CAN_InitStructure.CAN_TXFP=DISABLE;           //优先级由报文的标识符来决定
  CAN_InitStructure.CAN_Mode=CAN_Mode_Normal;   //正常模式
  
  /* Baudrate = 125kbps*/
  CAN_InitStructure.CAN_SJW=CAN_SJW_1tq;
  CAN_InitStructure.CAN_BS1=CAN_BS1_2tq;
  CAN_InitStructure.CAN_BS2=CAN_BS2_3tq;
  CAN_InitStructure.CAN_Prescaler=48;
  CAN_Init(CANx, &CAN_InitStructure);
}

/* 
	*函数名：CAN_Mode_Config()
	*功  能：CAN筛选器配置函数
	*作  者：WF
	*参  数：无
	*返回值：无
	*时  间：2021.03.26
*/
static void CAN_Filter_Config(void)
{
	CAN_FilterInitTypeDef  CAN_FilterInitStructure;
	
	/* CAN filter init */

  CAN_FilterInitStructure.CAN_FilterNumber=0;                   //筛选器组0
  CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask; //屏蔽位模式
  CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;//32位筛选器
  CAN_FilterInitStructure.CAN_FilterIdHigh=0x0000;              //筛选器高16位
  CAN_FilterInitStructure.CAN_FilterIdLow=0x0000;               //筛选器低16位
  CAN_FilterInitStructure.CAN_FilterMaskIdHigh=0x0000;          //筛选器高16位不关心
  CAN_FilterInitStructure.CAN_FilterMaskIdLow=0x0000;           //筛选器低16位不关心
  CAN_FilterInitStructure.CAN_FilterFIFOAssignment=0;           //筛选器关联到FIFO0
  CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;          //使能筛选器
  CAN_FilterInit(&CAN_FilterInitStructure);
	
	/* CAN FIFO0 message pending interrupt enable */ 
  CAN_ITConfig(CANx, CAN_IT_FMP0, ENABLE);
}


/*********************************************************************
												  CAN对外接口
**********************************************************************/

/* 
	*函数名：CAN_Config()
	*功  能：CAN配置函数
	*作  者：WF
	*参  数：无
	*返回值：无
	*时  间：2021.03.26
*/
void CAN_Config(void)
{
	CAN_GPIO_Config();
	CAN_NVIC_Config();
	CAN_Mode_Config();
	CAN_Filter_Config();
}

/* 
	*函数名：CAN_SendMsg()
	*功  能：CAN发送帧函数
	*作  者：WF
  *参  数：u32 Ext_ID: 扩展标识符 ,u8* zdata：数据指针, u8 len：数据长度
	*返回值：PASSED：传输成功，FAILED:失败
	*时  间：2022.10.5 
*/
uint8_t CAN_SendMsg(u32 Ext_ID ,u8* zdata, u8 len)
{
	uint32_t i = 0;
	uint8_t TransmitMailbox = 0;

	CanTxMsg TxMessage;
	TxMessage.StdId=0;
	TxMessage.ExtId=Ext_ID;
	TxMessage.IDE=CAN_ID_EXT;
	TxMessage.RTR=CAN_RTR_DATA;
	TxMessage.DLC=len;
	
	/* 将发送数据写入发送邮箱 */
	for(i = 0; i < len; i++)
	{
		TxMessage.Data[i] = zdata[i];
	}
	
	TransmitMailbox = CAN_Transmit(CANx, &TxMessage);
	
	/* 等待传输完成 */
  while((CAN_TransmitStatus(CANx, TransmitMailbox) != CANTXOK) && (i != 0xFFFF))
  {
    i++;
  }
	
	if(i == 0xFFFF) //传输超时
	{
		return FAILED;
	}
	else //传输成功
	{
		return PASSED;
	}
}

/* 
	*函数名：CAN_ReciveMES()
	*功  能：CAN发送帧函数
	*作  者：WF
	*参  数：u8* zdata：数据指针, u8 len：数据长度
	*返回值：PASSED：传输成功，FAILED:失败
	*时  间：2021.03.26
*/
uint8_t CAN_ReciveMsg(u8 *zdata, u8 len)
{
	uint32_t i = 0;
	CanRxMsg RxMessage;
	
	/* 等待邮箱挂起（接收到信息） */
	i = 0;
  while((CAN_MessagePending(CANx, CAN_FIFO0) < 1) && (i != 0xFFFF))
  {
    i++;
  }

  /* receive */
  RxMessage.StdId=0x00;
  RxMessage.IDE=CAN_ID_STD;
  RxMessage.DLC=0;
  RxMessage.Data[0]=0x00;
  RxMessage.Data[1]=0x00;
	
  CAN_Receive(CANx, CAN_FIFO0, &RxMessage);

  if (RxMessage.StdId!=0x1998)
  {
    return FAILED;  
  }

  if (RxMessage.IDE!=CAN_ID_STD)
  {
    return FAILED;
  }

  if (RxMessage.DLC!=len)
  {
    return FAILED;  
  }
	
  return PASSED; /* CAN_Recive Passed */
}
