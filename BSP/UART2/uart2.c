//#include "sys.h"
#include "uart2.h"
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

extern RingBuffer *p_uart2_rxbuf;


void uart2_init(u32 bound)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//使能GPIOA时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//使能USART2时钟

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;	//PA2
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; //PA3
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn; //使能串口2中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //先占优先级0级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; //从优先级1级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //使能外部中断通道
    NVIC_Init(&NVIC_InitStructure); //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

    USART_InitStructure.USART_BaudRate = bound;//波特率设置
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//8位数据长度
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;///奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//收发模式
    USART_Init(USART2, &USART_InitStructure); //初始化串口


    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启中断

    USART_Cmd(USART2, ENABLE);                    //使能串口 

}


void USART2_IRQHandler(void)                        //串口3中断服务程序
{
    u8 Res;
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) 
    {
        Res = USART_ReceiveData(USART2);            //读取接收到的数据

        RingBuffer_In(p_uart2_rxbuf, &Res, 1);            //放入缓存
    }
}

void usart2_send_data(u8 *data, u32 size)
{
    for(u32 i = 0; i < size; i++)
    {
        while((USART2->SR & 0X40) == 0);

        USART2->DR = data[i];
    }
}




