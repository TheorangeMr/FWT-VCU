/*******************************************
	*文件名 ：  usart
	*作   者：  罗成
	*修改时间： 2022.10.5
	*版   本：  1.0
*******************************************/
#include "usart.h"

void uart_init(u32 bound)
{
    //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    DEBUG_USART_APBxClkCmd(DEBUG_USART_CLK | DEBUG_USART_GPIO_CLK, ENABLE);    //使能USART1，GPIOA时钟

    //USART1_TX   GPIOA9
    GPIO_InitStructure.GPIO_Pin = DEBUG_USART_TX_GPIO_PIN; //PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;    //复用推挽输出
    GPIO_Init(DEBUG_USART_TX_GPIO_PORT, &GPIO_InitStructure);//初始化GPIOA9

    //USART1_RX      GPIOA10初始化
    GPIO_InitStructure.GPIO_Pin = DEBUG_USART_RX_GPIO_PIN;//PA10
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(DEBUG_USART_RX_GPIO_PORT, &GPIO_InitStructure);//初始化GPIOA10

    //Usart1 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = DEBUG_USART_IRQ;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1 ; //抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;        //子优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;            //IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);    //根据指定的参数初始化VIC寄存器

    //USART 初始化设置

    USART_InitStructure.USART_BaudRate = bound;//串口波特率
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;    //收发模式

    USART_Init(DEBUG_USARTx, &USART_InitStructure); //初始化串口1
    USART_Cmd(DEBUG_USARTx, ENABLE);                    //使能串口1

}

//重定义fputc函数
int fputc(int ch, FILE *f)
{
    while((USART1->SR & 0X40) == 0); //循环发送,直到发送完毕

    USART1->DR = (u8) ch;
    return ch;
}


void usart1_send_data(u8 *data, u32 size)
{
    for(u32 i = 0; i < size; i++)
    {
        while((USART1->SR & 0X40) == 0);

        USART1->DR = data[i];
    }
}
