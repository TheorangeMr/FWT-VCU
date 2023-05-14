/*******************************************
	*文件名  ：  bsp_adc.c
	*作   者 ：  WF
	*修改时间：  2021.03.13
	*版   本：   v1.0
  *说   明：   adc源文件
*******************************************/


#include "bsp_adc.h"
#include "bsp_dma.h"
#include "FreeRTOS.h"
#include "task.h"


/* 
	如果使能了扫描模式和DMA，则定义一个ADC_Value[][]数组来存储AD转换结果，
	共ADC1_SCAN_CHANNEL_NUM个通道，每个通道保存ADC_SMOOTHING组数据。
*/
#if ADC1_SCAN_ENABLE && ADC1_DMA_ENABLE
vu16 ADC_Value[ADC_SMOOTHING][ADC1_SCAN_CHANNEL_NUM] = {0};
#endif


/* 
	*函数名：ADC_Configuration()
	*功  能：ADC初始化函数
	*作  者：WF
	*参  数：无
	*返回值：无
	*时  间：2021.03.13
*/
void ADC_Configuration(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	ADC_InitTypeDef   ADC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_ADC1 , ENABLE);
	
	/* 设置ADC通道引脚为模拟输入 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN ;
	GPIO_InitStructure.GPIO_Pin = ADC_CH6_GPIO_PIN|ADC_CH2_GPIO_PIN;
	GPIO_Init(ADC_CH1_GPIO_PORT , &GPIO_InitStructure);
	
	/* 设置ADC分频因子 */
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	ADC_DeInit(ADC1);
	
	/* 初始化ADC转换模式 */
	#if ADC1_SCAN_ENABLE
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;                      //工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;                            //启动扫描模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;                      //启动连续转换
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;     //转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;                  //数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = ADC1_SCAN_CHANNEL_NUM;             //通道数量
	ADC_Init(ADC1, &ADC_InitStructure);
	#else
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;                      //工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;                           //不启动扫描模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;                     //不启动连续转换
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;     //转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;                  //数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;                                 //通道数量为1 
	ADC_Init(ADC1, &ADC_InitStructure);
	#endif
	
	
	/* 配置规则组各通道参数 */
	#if ADC1_SCAN_ENABLE
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1 , ADC_SampleTime_239Cycles5);
	#endif
	
	#if ADC1_DMA_ENABLE
	DMA_ADC1_Configuration((u32)ADC_Value, ADC_SMOOTHING*ADC1_SCAN_CHANNEL_NUM);
	ADC_DMACmd(ADC1, ENABLE);
	#endif
	
	ADC_Cmd(ADC1 , ENABLE);                      //使能ADC
	
	ADC_ResetCalibration(ADC1);	                 //使能复位校准  
	 
	while(ADC_GetResetCalibrationStatus(ADC1));	 //等待复位校准结束
	
	ADC_StartCalibration(ADC1);	                 //开启AD校准
 
	while(ADC_GetCalibrationStatus(ADC1));	     //等待校准结束
}


/* 
	*函数名：ADC_SingleMode_GetValue()
	*功  能：获取单次模式ADC值
	*作  者：WF
	*参  数：channel_x:ADC通道号
	*返回值：无
	*时  间：2021.03.13
*/
u16 ADC_SingleMode_GetValue(u8 channel_x)
{
	u32 Sum = 0;
	u16 Max = 0;
	u16 Min = 4095;
	u16 tempdata = 0;
	u16 Average = 0;
	
	/* 配置通道参数 */
	ADC_RegularChannelConfig(ADC1, channel_x, 1 , ADC_SampleTime_239Cycles5);
	
	/* 软件滤波采样 */
	for(u8 i = 0; i < ADC_SMOOTHING; i++)
	{
		/* 开启软件转换 */
		ADC_SoftwareStartConvCmd(ADC1 , ENABLE);
		/* 等待转换完成 */
		while(ADC_GetFlagStatus(ADC1 , ADC_FLAG_EOC) == RESET);
		/* 读取数据 */
		tempdata = ADC_GetConversionValue(ADC1);
		
		Max = (Max>tempdata)?Max:tempdata;
		Min = (Min<tempdata)?Min:tempdata;
		Sum += tempdata;
	}
	
	/* 去掉最大值和最小值再求平均值 */
	Average = (Sum-Max-Min)/(ADC_SMOOTHING-2);
	
	return Average;
}


/* 
	*函数名：ADC_ScanMode_GetVlaue()
	*功  能：获取扫描模式ADC值
	*作  者：WF
	*参  数：*zdata：数据指针
	*返回值：无
	*时  间：2021.03.13
*/

#if ADC1_SCAN_ENABLE

void ADC_ScanMode_GetVlaue(u16 *zdata)
{
	u32 Sum = 0;
	u16 Max = 0;
	u16 Min = 4095;
	

	/* 开启软件转换 */
	ADC_SoftwareStartConvCmd(ADC1 , ENABLE);
	
	/* 等待转换完成 */
	//while(ADC_GetFlagStatus(ADC1 , ADC_FLAG_EOC) == RESET);
	//对于等待转换完成，在使用实时操作系统时，可以使用延时函数交出CPU使用权，
	//从而不会形成阻塞
	vTaskDelay(10);
	
	/* 软件滤波（均值滤波） */ 
	for(u8 i = 0; i < ADC1_SCAN_CHANNEL_NUM; i++)
	{
		Sum = 0;
		Max = 0;
		Min = 4095;
		for(u8 j = 0; j < ADC_SMOOTHING; j++)
		{
			Max = (Max>ADC_Value[j][i])?Max:ADC_Value[j][i];
		  Min = (Min<ADC_Value[j][i])?Min:ADC_Value[j][i];
			Sum += ADC_Value[j][i];
		}
		zdata[i] = (Sum-Min-Max)/(ADC_SMOOTHING-2);
	}
}

#endif


