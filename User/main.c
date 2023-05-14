/*******************************************
	*文件名 ：  VCU4.4
	*作   者：  罗成
	*修改时间： 2022.10.12
	*版   本：  v1.4
*******************************************/

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "semphr.h"
#include "portmacro.h"

#include "ff.h"
#include "diskio.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

#include "usart.h"
#include "bsp_can.h"
#include "bsp_dma.h"
#include "bsp_adc.h"
#include "bsp_timer.h"
#include "bsp_sdio_sdcard.h"
#include "uart2.h"
#include "RingBuffer.h"
#include "atk_m750.h"
#include "string.h"
#include "bsp_pwr.h"
#include "bsp_delay.h"
#include "bsp_iwdg.h"


#define TOOTH_NUM 55			 //齿数
#define WHEEL_RADIUS 0.257 //轮径(单位:m)
#define PI 3.14						 //圆周率
#define TANK_HEIGHT 210		 //油箱高度
#define GEAR_RATIO  4.05    //传动比

/********************************* 测试打印 ****************************************/

#define TEST_vApp_Hook 1
#define TEST_PRINTF_SIMULATOR 0
#define TEST_PRINTF_CAN   0
#define TEST_PRINTF_SPEED 0
#define TEST_PRINTF_OIL 0
#define TEST_PRINTF_CPU 0
#define TEST_PRINTF_BATTERY 0
#define TEST_PRINTF_SD 0
#define TEST_PRINTF_4GDTU 0
#define TEST_PRINTF_ONENET 0
#define TEST_PRINTF_RINGBUFFER 0
#define TEST_IWDG 0



#if TEST_PRINTF_SIMULATOR
portCHAR flag1;
portCHAR flag2;
portCHAR flag3;
portCHAR flag4;
portCHAR flag5;
portCHAR flag6;
portCHAR flag7;
portCHAR flag8;
portCHAR flag9;
portCHAR flag10;
portCHAR flag11;
#endif

/********************************* 互斥信号量 ****************************************/
SemaphoreHandle_t MuxSem_Handle =NULL;    //互斥信号量句柄


/********************************* 事件 ****************************************/

EventGroupHandle_t EventGroupHandler = NULL;	//事件标志组句柄


#define EVENTBIT_0	(1<<0)                          //测速结束事件
#define EVENTBIT_1	(1<<1)                          //串口打印事件
#define EVENTBIT_2	(1<<2)                          //里程计数事件
#define EVENTBIT_3	(1<<3)                          //喂看门狗事件
#define EVENTBIT_4	(1<<4)                          //ONENET数据发送事假



/********************** 任务句柄初始化 ****************************************/

static TaskHandle_t AppTaskCreate_Handle = NULL; 
static TaskHandle_t SPEED_Task_Handle = NULL;
static TaskHandle_t OIL_Task_Handle = NULL;
static TaskHandle_t CAN_Task_Handle = NULL;
static TaskHandle_t USART_Task_Handle = NULL;
static TaskHandle_t Mileage_count_Task_Handle = NULL;
static TaskHandle_t SD_RWTask_Handle = NULL;
static TaskHandle_t Battery_capacity_Task_Handle = NULL;
static TaskHandle_t CPU_Task_Handle = NULL;
static TaskHandle_t OneNET_4G_DTU_Handle = NULL;
static TaskHandle_t DTU_4G_Task_Handle = NULL;
static TaskHandle_t RingBuffer_Read_Task_Handle = NULL;
static TaskHandle_t IWDG_Task_Handle = NULL;

/**************************定义静态任务堆栈，控制块****************************************/


/* 闲置任务任务堆栈 */
static StackType_t Idle_Task_Stack[configMINIMAL_STACK_SIZE];
/* 定时器任务任务堆栈 */
static StackType_t Timer_Task_Stack[configTIMER_TASK_STACK_DEPTH];

/* 定义任务栈 */
static StackType_t AppTaskCreate_Stack[256];
/* SPEED任务堆栈 */
static StackType_t SPEED_Task_Stack[128];
/* OIL任务堆栈 */
static StackType_t OIL_Task_Stack[64];
/* CAN任务堆栈 */
static StackType_t CAN_Task_Stack[128];
/* OneNET_4G_DTU任务堆栈 */
static StackType_t OneNET_4G_DTU_Task_Stack[256];
/* 4G_DTU任务堆栈 */
static StackType_t DTU_4G_Task_Stack[128];


/* 闲置任务控制块 */
static StaticTask_t Idle_Task_TCB;
/* 定时器任务控制块 */
static StaticTask_t Timer_Task_TCB;
/* 定义任务控制块 */
static StaticTask_t AppTaskCreate_TCB;

static StaticTask_t SPEED_Task_TCB;
static StaticTask_t OIL_Task_TCB;
static StaticTask_t CAN_Task_TCB;
static StaticTask_t OneNET_4G_DTU_TCB;
static StaticTask_t DTU_4G_TCB;



/* 获取空闲任务的内存 */

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
	                                 StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize)
{
	*ppxIdleTaskTCBBuffer = &Idle_Task_TCB;
	*ppxIdleTaskStackBuffer = Idle_Task_Stack;
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

/* 获取定时器任务的内存 */

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
	                                 StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize)
{
	*ppxTimerTaskTCBBuffer = &Timer_Task_TCB;
	*ppxTimerTaskStackBuffer = Timer_Task_Stack;
	*pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}


#if TEST_vApp_Hook
/*栈溢出钩子函数*/
void vApplicationStackOverflowHook(TaskHandle_t xTask,signed char *pcTaskName)
{
	printf("栈溢出的任务名：\r\n%s\r\n",pcTaskName);
}

#endif
/*
*************************************************************************
*                             函数声明
*************************************************************************
*/


static void AppTaskCreate(void);                                                /* 用于创建任务 */
static void BSP_Init(void);
static void SPEED_Task(void* parameter);                                        /*速度测量函数*/
static void OIL_Task(void* parameter);                                          /*油量测试函数*/
static void CAN_Task(void* parameter);                                          /*CAN通讯函数*/
static void USART_Task(void* parameter);                                        /*USART通讯函数*/
static void Mileage_count_Task(void* parameter);                                /*记录里程值*/
static void SD_RWTask(void* parameter);                                          /*sd数据读写*/
static float SD_ReadtoMileage_count(void);                                       /*里程计数*/
static void Battery_capacity_Task(void* parameter);                              /*检测电池电量*/
static void CPU_Task(void* pvParameters);                                        /* CPU_Task任务实现 */
static void OneNET_4G_DTU_Task(void* pvParameters);                              /* 4G-OneNET */
static void DTU_4G_Task(void* pvParameters);                                		 /*DTU实时时间，4G信号*/
static void RingBuffer_Read_Task(void* pvParameters);                            /*接受4G DTU数据*/
static void OneNet_FillBuf(uint8_t buff[][64]);
static void IWDG_Task(void* pvParameters);
void PreSleepProcessing(uint32_t ulExpectedIdleTime);
void PostSleepProcessing(uint32_t ulExpectedIdeTime);



/*
*************************************************************************
*                             全局变量
*************************************************************************
*/

extern vu8 Endup_Flag;                      //规定T结束标志
extern vu32 Total_Time_M1;                  //齿轮脉冲数
extern vu32 Total_Time_M2;                  //高频计时数
extern vu32 Overflow_Count;                 //溢出计数           
extern vu8 Actual_Endflag;                  //实际采样结束标志
extern const char sqpa[4][4];
extern ST_Time Timedat;

//vu8 Detection_update_flag = 0;              //测速函数更新标志
u32 Car_SpeedData = 0;			             			//车速
float Rotate_Speed = 0;                       //转速
vu8 RTC_Time_Flag = 0;				     					//CAN接收数据
u8 SD_Mileage_sum_flag = 0;                   //SD-里程数据读取标志
CanRxMsg RxMessage;							 							//CAN接收信息结构体
CanTxMsg TxMessage;							 							//CAN发送信息结构体
u8 VCU_Data_Send[8];           						    //VCU_Data发送报文数组
u8 VCU_Data2_Send[8];          						    //VCU_Data2发送报文数组
uint8_t CanTx_Buf1[8] = {0};									//CAN发送缓存数组1
uint8_t CanTx_Buf2[8] = {0};				 					//CAN发送缓存数组2
uint8_t Oil = 0;							 							  //油量
uint8_t Oil_count[110]= {0};
uint16_t Oil_dat = 0;                         //ADC油量采集原始数据
uint8_t Oil_base_dat = 97;                     //油量最终基准值
uint16_t High[8] = {0};                       //油量高度数组
float Mileage_count = 0;                      //里程计数
float Mileage_sum = 0;
uint16_t Battery = 0;                         //电量
uint8_t Battery_flag = 0;                     //显示电量格数
uint8_t Network_size = 0;                     //4G信号
static uint8_t Mileagecount_Task = 0,SD_RW_Task = 0;


//4G DTU
#define DTU_ONENETDATE_RX_BUF (512)
#define DTU_ONNETDATE_TX_BUF (64)
static uint8_t DTU_Time_flag = 0;
static uint32_t dtu_rxlen = 0;
static uint8_t dtu_rxbuf[DTU_ONENETDATE_RX_BUF] = {0};
RingBuffer *p_uart2_rxbuf;
uint8_t Sdat[3] = {3,0,0x46};
uint8_t Send_date[10][DTU_ONNETDATE_TX_BUF] = {0};
uint8_t DTUTask1 = 0,DTUTask2 = 0,DTUTask3 = 0;
static uint8_t DTUMode_Switch_flag = 0;
uint8_t DTU_AT_CLKFLAG = 0;
uint8_t ONENET_OFF_FLAG = 0;
const char ONENET_COM_OFF[]="lc0218";
const char ONENET_COM_ON[] = "lc2001";

//文件系统
FATFS fs;
FIL fnew;
FRESULT res_flash;
UINT fnum;
BYTE work[FF_MAX_SS];

char ReadBuffer[512] = {0};
char Fdat[50];

/*****************************************************************
  * @brief  主函数
  * @param  无
  * @retval 无
  * @note   第一步：开发板硬件初始化 
            第二步：创建APP应用任务
            第三步：启动FreeRTOS，开始多任务调度
  ****************************************************************/
int main(void)
{
	BSP_Init();
	
//    printf("飞翼车队电气组整车控制器VCU函数调试!\r\n");
	AppTaskCreate_Handle = xTaskCreateStatic((TaskFunction_t	)AppTaskCreate,		  //任务函数
															(const char* 	)"AppTaskCreate",		                //任务名称
															(uint32_t 		)256,	                              //任务堆栈大小
															(void* 		  	)NULL,				                      //传递给任务函数的参数
															(UBaseType_t 	)1, 	                              //任务优先级
															(StackType_t*   )AppTaskCreate_Stack,	            //任务堆栈
															(StaticTask_t*  )&AppTaskCreate_TCB);	            //任务控制块 
  /* 启动任务调度 */          
	if(NULL != AppTaskCreate_Handle)                                              /* 创建成功 */
    vTaskStartScheduler();    
	else
		printf("创建失败! \r\n");
    while(1);   /* 正常不会执行到这里 */
}

/**********************************************************************
  * @ 函数名  ： BSP_Task
  * @ 功能说明： 外设初始化
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/

void BSP_Init(void)
{
	uint16_t timeout = 0;
	int ret;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  ADC_Configuration();
	uart_init(115200);
	uart2_init(115200);
	Delay_Init(72);
	my_mem_init(SRAMIN);
	p_uart2_rxbuf = RingBuffer_Malloc(1024);        /*从内存池中分配1K的内存给串口3接收DTU数据*/
	CAN_Config();
	Timer6_Init(72-1, 50-1);              /*50us*/
	Timer1_Init(72 - 1, 50000 - 1);      /*50ms*/
	Timer7_Init(7200-1, 500-1);            /*50ms*/
	Timer5_Init(7200-1, 5000-1);          /*500ms*/
	Timer4_Init(7200-1, 5000-1);          /*500ms*/	
	TIM_ITConfig(ADVANCE_TIMEX, TIM_IT_CC1| TIM_IT_Update, ENABLE);           //允许捕获/比较1中断和更新中断
	TIM_Cmd(ADVANCE_TIMEX,ENABLE);                                            //打开定时器1计时
	TIM_Cmd(BASIC_TIMX,ENABLE);                                                  
	
	//挂载文件系统
	res_flash = f_mount(&fs,"1:",1);
  if(res_flash!=FR_OK)
  {
    printf("！！外部SD挂载文件系统失败。(%d)\r\n",res_flash);
    printf("！！可能原因：SDIO_SD初始化不成功。\r\n");
		if(res_flash == FR_NO_FILESYSTEM)
		{
			res_flash = f_mkfs("1:",0,work,sizeof work);
			if(res_flash == FR_OK)
			{
				printf("》SD已成功格式化文件系统。\r\n");
				/* 格式化后，先取消挂载 */
				res_flash = f_mount(NULL,"1:",1);	
				/* 重新挂载	*/			
				res_flash = f_mount(&fs,"1:",1);
				goto	repeat;
			}
			else
			{
				printf("《《格式化失败。》》\r\n");
			}
		}
		SD_RW_Task = 1;Mileagecount_Task = 1;
		goto	repeat;		
    repeat:
		;
  }
  else
  {
    printf("》文件系统挂载成功\r\n");
  }
	//配置4G DTU

	printf("Wait for Cat1 DTU to start, wait 10s.... \r\n");
	while( timeout <= 10 )   /* 等待Cat1 DTU启动，需要等待5-6s才能启动 */
	{
			ret = dtu_config_init(DTU_WORKMODE_ONENET);    /*初始化DTU工作参数*/
			if( ret == 0 )
			{
				printf("Cat1 DTU Init Success \r\n");
				DTUMode_Switch_flag = 1;
				if(BKP_ReadBackupRegister(BKP_DR40) != 0x18)
				{
					WWDG_Config(6,1000);                          //6.4s
				}
				else
				{
					BKP_DeInit();
					BKP_ClearFlag();
					Standby_mode();
				}
				break;				
			}
			timeout++;
			delay_xms(1000);
	}
	while( timeout > 10 )   /* 超时 */
	{
		printf("**************************************************************************\r\n");
		printf("ATK-DTU Init Fail ...\r\n");
		printf("请按照以下步骤进行检查:\r\n");
		printf("1.使用电脑上位机配置软件检查DTU能否单独正常工作\r\n");
		printf("2.检查DTU串口参数与STM32通讯的串口参数是否一致\r\n");
		printf("3.检查DTU与STM32串口的接线是否正确\r\n");
		printf("4.检查DTU供电是否正常，DTU推荐使用12V/1A电源供电，不要使用USB的5V给模块供电！！\r\n");
		printf("**************************************************************************\r\n\r\n");
		DTUTask1 = 1;DTUTask2 = 1;DTUTask3 = 1;
		delay_xms(1000);
		printf("Cat1 DTU Init Fail\r\n");
		break;
	}
}



/***********************************************************************
  * @ 函数名  ： AppTaskCreate
  * @ 功能说明： 为了方便管理，所有的任务创建函数都放在这个函数里面
  * @ 参数    ： 无  
  * @ 返回值  ： 无
  **********************************************************************/
static void AppTaskCreate(void)
{
	BaseType_t xReturn = pdPASS;     /* 定义一个创建信息返回值，默认为pdPASS */	
  taskENTER_CRITICAL();           //进入临界区

	  /* 创建EventGroup */  
	EventGroupHandler = xEventGroupCreate();
	if(NULL != EventGroupHandler)
	printf("EventGroupHandler 事件创建成功！\r\n");

	  /* 创建MuxSem */
  MuxSem_Handle = xSemaphoreCreateMutex();	 
  if(NULL != MuxSem_Handle)
    printf("MuxSem_Handle互斥量创建成功!\r\n");

  xReturn = xSemaphoreGive( MuxSem_Handle );//给出互斥量
  if( xReturn == pdTRUE )
  printf("释放信号量!\r\n");	

#if TEST_PRINTF_CPU
  /* 创建CPU_Task任务 */
  xReturn = xTaskCreate((TaskFunction_t )CPU_Task, /* 任务入口函数 */
                        (const char*    )"CPU_Task",/* 任务名字 */
                        (uint16_t       )256,   /* 任务栈大小 */
                        (void*          )NULL,	/* 任务入口函数参数 */
                        (UBaseType_t    )15,	    /* 任务的优先级 */
                        (TaskHandle_t*  )&CPU_Task_Handle);/* 任务控制块指针 */
  if(pdPASS == xReturn)
    printf("创建CPU_Task任务成功!\r\n");	
	else
		printf("CPU_Task任务创建失败!\r\n");	
#endif
	
  /* 创建SPEED_Task任务 */
	SPEED_Task_Handle = xTaskCreateStatic((TaskFunction_t	)SPEED_Task,		        //任务函数
															(const char* 	)"SPEED_Task",		                  //任务名称
															(uint32_t 		)128,					                      //任务堆栈大小
															(void* 		  	)NULL,				                      //传递给任务函数的参数
															(UBaseType_t 	)14, 				                        //任务优先级
															(StackType_t*   )SPEED_Task_Stack,	              //任务堆栈
															(StaticTask_t*  )&SPEED_Task_TCB);	              //任务控制块													
																														
	if(NULL != SPEED_Task_Handle)                                                 /* 创建成功 */
		printf("SPEED_Task任务创建成功!\r\n");
	else
		printf("SPEED_Task任务创建失败!\r\n");
	
	  /* 创建OIL_Task任务 */
	OIL_Task_Handle = xTaskCreateStatic((TaskFunction_t	)OIL_Task,		            //任务函数
															(const char* 	)"OIL_Task",		                    //任务名称
															(uint32_t 		)64,					                      //任务堆栈大小
															(void* 		  	)NULL,				                      //传递给任务函数的参数
															(UBaseType_t 	)13, 				                        //任务优先级
															(StackType_t*   )OIL_Task_Stack,	                //任务堆栈
															(StaticTask_t*  )&OIL_Task_TCB);	                //任务控制块  
	
	if(NULL != OIL_Task_Handle)                                                   /* 创建成功 */
		printf("OIL_Task任务创建成功!\r\n");
	else
		printf("OIL_Task任务创建失败!\r\n");
	
	  /* 创建CAN_Task任务 */
	CAN_Task_Handle = xTaskCreateStatic((TaskFunction_t	)CAN_Task,		            //任务函数
															(const char* 	)"CAN_Task",		                    //任务名称
															(uint32_t 		)128,					                      //任务堆栈大小
															(void* 		  	)NULL,				                      //传递给任务函数的参数
															(UBaseType_t 	)12, 				                        //任务优先级
															(StackType_t*   )CAN_Task_Stack,	                //任务堆栈
															(StaticTask_t*  )&CAN_Task_TCB);	                //任务控制块  
	
	if(NULL != CAN_Task_Handle)                                                   /* 创建成功 */
		printf("CAN_Task任务创建成功!\r\n");
	else
		printf("CAN_Task任务创建失败!\r\n");
	
	  /* 创建USART_Task任务 */
	xReturn = xTaskCreate((TaskFunction_t	)USART_Task,		                        //任务函数
															(const char* 	)"USART_Task",		                  //任务名称
															(uint16_t 		)128,					                      //任务堆栈大小
															(void* 		  	)NULL,				                      //传递给任务函数的参数
															(UBaseType_t 	)5, 				                        //任务优先级
															(TaskHandle_t*  )&USART_Task_Handle);	            //任务控制块指针   
	
	if(pdPASS == xReturn)/* 创建成功 */
		printf("USART_Task任务创建成功!\r\n");
	else
		printf("USART_Task任务创建失败!\r\n");
	if(Mileagecount_Task == 0)
	{  	
			/* 创建Mileage_count_Task任务 */
		xReturn = xTaskCreate((TaskFunction_t	)Mileage_count_Task,		                 //任务函数
																(const char* 	)"Mileage_count_Task",		           //任务名称
																(uint16_t 		)128,					                       //任务堆栈大小
																(void* 		  	)NULL,				                       //传递给任务函数的参数
																(UBaseType_t 	)4, 				                         //任务优先级
																(TaskHandle_t*  )&Mileage_count_Task_Handle);	             //任务控制块指针   
		
		if(pdPASS == xReturn)/* 创建成功 */
			printf("Mileage_count_Task任务创建成功!\r\n");
		else
			printf("Mileage_count_Task任务创建失败!\r\n");	
	}
	if(SD_RW_Task == 0)
	{  
			/* 创建SD_ReadTask任务 */
		xReturn = xTaskCreate((TaskFunction_t	)SD_RWTask,		                     		 //任务函数
																(const char* 	)"SD_RWTask",		                   //任务名称
																(uint16_t 		)512,					                       //任务堆栈大小
																(void* 		  	)NULL,				                       //传递给任务函数的参数
																(UBaseType_t 	)11, 				                         //任务优先级
																(TaskHandle_t*  )&SD_RWTask_Handle);	           //任务控制块指针   
		
		if(pdPASS == xReturn)/* 创建成功 */
			printf("SD_RWTask任务创建成功!\r\n");
		else
			printf("SD_RWTask任务创建失败!\r\n");		
  }
	  /* 创建Battery_capacity_Task任务 */
	xReturn = xTaskCreate((TaskFunction_t	)Battery_capacity_Task,		               //任务函数
															(const char* 	)"Battery_capacity_Task",		         //任务名称
															(uint16_t 		)128,					                       //任务堆栈大小
															(void* 		  	)NULL,				                       //传递给任务函数的参数
															(UBaseType_t 	)3, 				                         //任务优先级
															(TaskHandle_t*  )&Battery_capacity_Task_Handle);	 //任务控制块指针   
	
	if(pdPASS == xReturn)/* 创建成功 */
		printf("Battery_capacity_Task任务创建成功!\r\n");
	else
		printf("Battery_capacity_Task任务创建失败!\r\n");

	if(DTUTask3 == 0)
	{
		/* 创建RingBuffer_Read_Task任务 */
		xReturn = xTaskCreate((TaskFunction_t	)RingBuffer_Read_Task,		         //任务函数
													(const char* 	)"RingBuffer_Read_Task",		         //任务名称
													(uint16_t 		)1024,					                     //任务堆栈大小
													(void* 		  	)NULL,				                       //传递给任务函数的参数
													(UBaseType_t 	)7, 				                         //任务优先级
													(TaskHandle_t*  )&RingBuffer_Read_Task_Handle);	   //任务控制块指针   
	}
	if(pdPASS == xReturn)   /* 创建成功 */
		printf("RingBuffer_Read_Task任务创建成功!\r\n");
	else
		printf("RingBuffer_Read_Task任务创建失败!\r\n");

	if(DTUTask1 == 0)
	{	
			/* 创建OneNET_4G_DTU_Task任务 */
		OneNET_4G_DTU_Handle = xTaskCreateStatic((TaskFunction_t	)OneNET_4G_DTU_Task,  //任务函数
																(const char* 	)"OneNET_4G_DTU_Task",		            //任务名称
																(uint32_t 		)256,					                        //任务堆栈大小
																(void* 		  	)NULL,				                        //传递给任务函数的参数
																(UBaseType_t 	)10, 				                          //任务优先级
																(StackType_t*   )OneNET_4G_DTU_Task_Stack,	        //任务堆栈
																(StaticTask_t*  )&OneNET_4G_DTU_TCB);	              //任务控制块
		if(NULL != OneNET_4G_DTU_Handle)                                                /* 创建成功 */
			printf("OneNET_4G_DTU任务创建成功!\r\n");
		else
			printf("OneNET_4G_DTU任务创建失败!\r\n");
	}
	if(DTUTask2 == 0)
	{		
		/* 创建4G_DTU_Task任务 */
		DTU_4G_Task_Handle = xTaskCreateStatic((TaskFunction_t	)DTU_4G_Task,  					//任务函数
																(const char* 	)"DTU_4G_Task",		                    //任务名称
																(uint32_t 		)128,					                        //任务堆栈大小
																(void* 		  	)NULL,				                        //传递给任务函数的参数
																(UBaseType_t 	)8, 				                          //任务优先级
																(StackType_t*   )DTU_4G_Task_Stack,	                //任务堆栈
																(StaticTask_t*  )&DTU_4G_TCB);	                    //任务控制块  
		if(NULL != DTU_4G_Task_Handle)                                             			/* 创建成功 */
			printf("DTU_4G_Task任务创建成功!\r\n");
		else
			printf("DTU_4G_Task任务创建失败!\r\n");	
	}
	/* 创建IWDG_Task任务 */
	xReturn = xTaskCreate((TaskFunction_t	)IWDG_Task,		         							//任务函数
												(const char* 	)"IWDG_Task",		         								//任务名称
												(uint16_t 		)128,					                     		//任务堆栈大小
												(void* 		  	)NULL,				                       //传递给任务函数的参数
												(UBaseType_t 	)16, 				                         //任务优先级
												(TaskHandle_t*  )&IWDG_Task_Handle);	   						//任务控制块指针   
	if(pdPASS == xReturn)   /* 创建成功 */
		printf("IWDG_Task任务创建成功!\r\n");
	else
		printf("IWDG_Task任务创建失败!\r\n");	
	
  vTaskDelete(AppTaskCreate_Handle);                                            //删除AppTaskCreate任务
  
  taskEXIT_CRITICAL();                                                          //退出临界区
}




/**********************************************************************
  * @ 函数名  ： CAN_Task
  * @ 功能说明： CAN总线传输
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/
static void CAN_Task(void* parameter)
{
	static uint8_t rtc_dat_time = 0;
	while(1)
	{
#if TEST_PRINTF_SIMULATOR
		flag1 =~ flag1;
#endif		

		if(DTU_AT_CLKFLAG == 1)
		{
			CanTx_Buf2[0] = 0x02;
			CanTx_Buf2[1] = (Timedat.year%100);
			CanTx_Buf2[2] = Timedat.month;
			CanTx_Buf2[3] = Timedat.day;
			CanTx_Buf2[4] = Timedat.hour;
			CanTx_Buf2[5] = Timedat.minute;
			CanTx_Buf2[6] = Timedat.second;
			CanTx_Buf2[7] = 0x18;
			CAN_SendMsg(0x2001 ,CanTx_Buf2, 8);
			if(RTC_Time_Flag == 0xab)
			{
				DTU_AT_CLKFLAG = 0;
			}
      else if(rtc_dat_time >= 20)
			{
				rtc_dat_time = 0;
				DTU_AT_CLKFLAG = 0;
			}
			else
			{
				rtc_dat_time++;	
			}
#if TEST_PRINTF_MILEAGE
			uint8_t i = 0;
			for(i = 0;i < 8; i++)
			{
				printf("CanTx_Buf2:%d \r\n",CanTx_Buf2[i]);
			}			
#endif
		}
		else
		{
			CanTx_Buf1[0] = 0x02;                     
			CanTx_Buf1[1] = Car_SpeedData;
			CanTx_Buf1[2] = (Oil_base_dat % 100);
			CanTx_Buf1[3] = (Mileage_sum/1000);              //最大发送里程数为255km
			CanTx_Buf1[4] = ((int)Mileage_sum%1000/10);
			CanTx_Buf1[5] = Battery_flag;
			CanTx_Buf1[6] = Network_size;		
			CanTx_Buf1[7] = 0x12;
			CAN_SendMsg(0x1998 ,CanTx_Buf1, 8);
		}	

#if TEST_PRINTF_MILEAGE
		printf(" can \r\n ");
		printf(" 里程数 : %.2f \r\n ",Mileage_sum);
		printf(" 信号 : %d \r\n ",CanTx_Buf1[6]);
 #endif	

		vTaskDelay(200);
  }
}


/**********************************************************************
  * @ 函数名  ： OIL_Task
  * @ 功能说明： 测量油量
								算法：超声波采样累加取平均，每半小时更新基准值
                      基准值随定时器4,按1分钟递减1。
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/


static void OIL_Task(void* parameter)
{
	static uint16_t Oil_i = 0;
	uint32_t count_som = 0;
	static uint8_t first_flag = 1;
	static uint8_t oil30_flag = 0;
	while(1)
	{
#if TEST_PRINTF_SIMULATOR
		flag2 =~ flag2;
#endif	
		static u8 High_Count = 0;
		if(first_flag == 1)
		{
			oil_first:
			/* 获取各通道ADC采集值 */
			Oil_dat = ADC_SingleMode_GetValue(ADC_Channel_6);
			/* 油量高度与检测电压的关系 High(mm) = Volt(mv) / 5 */
			if(Oil_dat > 100 && Oil_dat < 1400)//&& Oil_dat
			{
#if TEST_PRINTF_OIL
				printf("adc = %d\r\n",Oil_dat); 
#endif
				High[High_Count] = (Oil_dat * 3300*1.8) / 4095 / 5;
				High_Count++;
				if(High_Count >= 100)
				{
					for(u8 i = 0; i < High_Count-1; i++)
					{
						for(u8 j = i+1; j < High_Count; j++)
						{
							if(High[j] < High[i])
							{
								u16 temp = High[i];
								High[i] = High[j];
								High[j] = temp;
							}
						}
					}
#if TEST_PRINTF_OIL
					printf("High[High_Count / 2] = %d\r\n",High[High_Count / 2]);
#endif
					Oil = (High[High_Count / 2] * 100) / TANK_HEIGHT;           //去中位数
					High_Count = 0;
					printf("Oil = %d",Oil);
					if(Oil <= 99 && Oil >=30)
					{
						Oil_base_dat = Oil;
					}
					if(Oil > 99)
					{
						goto oil_first;
					}
					first_flag = 0;
					oil30_flag = 0;
					TIM_Cmd(OIL_NORMAL_TIM, ENABLE);																									// 使能计数器
				}
			}
		}
		else if(oil30_flag == 0)
		{
				/* 获取各通道ADC采集值 */
			Oil_dat = ADC_SingleMode_GetValue(ADC_Channel_6);
			/* 油量高度与检测电压的关系 High(mm) = Volt(mv) / 5 */
			if(Oil_dat > 350)    /* 没测到液体时小于50mv，所以去350 */
			{
				High[High_Count] = (Oil_dat * 3300*1.8) / 4095 / 5;
				High_Count++;
				if(High_Count >= 5)
				{
					for(u8 i = 0; i < High_Count-1; i++)
					{
						for(u8 j = i+1; j < High_Count; j++)
						{
							if(High[j] < High[i])
							{
								u16 temp = High[i];
								High[i] = High[j];
								High[j] = temp;
							}
						}
					}
					Oil = (High[High_Count / 2] * 100) / TANK_HEIGHT;
					High_Count = 0;
				}
		  }
			if(Oil < Oil_base_dat+5&&Oil > Oil_base_dat-5)
			{
				Oil_count[Oil_i++] = Oil;
			}
			if(Oil_i >= 50)
			{
				for(u8 i = 0; i < 50; i++)
				{
					count_som += Oil_count[i];
				}
				Oil_base_dat = count_som/50;
				Oil_i = 0;
			}
#if TEST_PRINTF_OIL
			printf("volt : %d",(Oil_dat * 3300) / 4095);
			printf(" oil：%d\r\n ",Oil);
#endif
		}
		if(Oil_base_dat <= 30)
		{
			oil30_flag = 1;
		}
		if(first_flag == 1)
		{
			vTaskDelay(1);
		}
		else
		{
			vTaskDelay(1000);
		}
	}
}




/**********************************************************************
  * @ 函数名  ： SPEED_Task
  * @ 功能说明： 测量车辆行驶速度
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/
static void SPEED_Task(void* parameter)
{
	EventBits_t r_event;
	BaseType_t xResult;
	xResult = xEventGroupSetBits(EventGroupHandler,EVENTBIT_4);
  while(1)
	{
#if TEST_PRINTF_SIMULATOR
		flag3 =~ flag3;
#endif
		r_event = xEventGroupWaitBits(EventGroupHandler,EVENTBIT_0,pdTRUE,pdTRUE,portMAX_DELAY);
		if((r_event&EVENTBIT_0) == EVENTBIT_0)
		{
#if TEST_PRINTF_SPEED
			 printf(" speed \r\n ");			
#endif			
			if (Actual_Endflag != 0)                         //轮速信号捕获完成
			{
				if(Total_Time_M1 == 0)
				{
					Car_SpeedData = 0;
					Total_Time_M2 = 0;
					Overflow_Count = 0;
					Actual_Endflag = 0;
//					Detection_update_flag = 0;
				}
				else
				{
					Actual_Endflag = 0;
					if(Total_Time_M1 >= 24)
					{
					  Mileage_count = (Total_Time_M1*1.0/TOOTH_NUM) * 2 * PI * WHEEL_RADIUS/GEAR_RATIO;
					}
					else
					{
						Mileage_count = 0;
					}
					Total_Time_M2 += (50000*Overflow_Count);
					Rotate_Speed=(1000000.0*60*Total_Time_M1)/(TOOTH_NUM*Total_Time_M2);    //计算转速(r/min)
					Car_SpeedData = (Rotate_Speed * 2 * PI * WHEEL_RADIUS /GEAR_RATIO)*3/50;      //转换为车速(km/h)
#if TEST_PRINTF_SPEED
					printf(" SPEED = %d,M1 = %d,M2 =  %d\r\n ",Car_SpeedData,Total_Time_M1,Total_Time_M2);
  				printf(" SPEED = %d \r\n ",Car_SpeedData);
#endif
					Total_Time_M1 = 0;
					Total_Time_M2 = 0;
					Overflow_Count = 0;
//					Detection_update_flag = 0;
					vTaskDelay(1);
				}
			}
//			if(Detection_update_flag == 0)
//			{
//				printf(" 2 \r\n ");
				TIM_ITConfig(ADVANCE_TIMEX, ADVANCE_TIM_IT_CCx| TIM_IT_Update, ENABLE);   //允许捕获/比较1中断和更新中断
				TIM_Cmd(ADVANCE_TIMEX,ENABLE);                                            //打开定时器1计时
				TIM_SetCounter(BASIC_TIMX, 0);                                            //采样周期计数清零				
				TIM_Cmd(BASIC_TIMX,ENABLE);                                               //打开定时器7，开始进行周期采样
//				Detection_update_flag = 1;
//			}
	//		else
	//		printf(" 3 \r\n ");
		}
		else
			printf("事件错误  \r\n");
  }   	
}

/**********************************************************************
  * @ 函数名  ： USART_Task
  * @ 功能说明： 串口打印
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/

static void USART_Task(void* parameter)
{
	EventBits_t r_event;
	while(1)
	{
#if TEST_PRINTF_SIMULATOR
		flag4 =~ flag4;
#endif		
		r_event = xEventGroupWaitBits(EventGroupHandler,EVENTBIT_1,pdFALSE,pdTRUE,portMAX_DELAY);
		if((r_event&EVENTBIT_1) == EVENTBIT_1)
		{
			printf("-------------------------------------");
			printf("Speed : %d km/h \r\n",Car_SpeedData);
			printf("Oil : %d km/h \r\n",Oil);
			printf("Mileage : %.2f mk \r\n",Mileage_sum/1000);
			printf("Battery = %d mv，Battery_flag = %d 格 \r\n",Battery,Battery_flag);
			printf("Network_Signal = %d 格 \r\n",Network_size);		
			printf("-------------------------------------");
			vTaskDelay(10100);
		}
	}
}

/**********************************************************************
  * @ 函数名  ： Mileage_count_Task
  * @ 功能说明： 记录车辆行驶里程
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/

static void Mileage_count_Task(void* parameter)
{
	EventBits_t r_event;
	while(1)
	{
#if TEST_PRINTF_SIMULATOR
		flag5 =~ flag5;		
#endif
		r_event = xEventGroupWaitBits(EventGroupHandler,EVENTBIT_2,pdTRUE,pdTRUE,portMAX_DELAY);
		if((r_event&EVENTBIT_2) == EVENTBIT_2)
		{
			Mileage_sum += Mileage_count;
#if TEST_PRINTF_MILEAGE
			printf("里程计数:%f\r\n",Mileage_count);
			printf("%f\r\n",Mileage_sum);			
#endif
			Mileage_count = 0;
		}
	}
}

/**********************************************************************
  * @ 函数名  ： SD_RWTask
  * @ 功能说明： 读出和写入车辆行驶里程
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/

static void SD_RWTask(void* parameter)
{
	while(1)
	{
#if TEST_PRINTF_SIMULATOR
			flag6 =~ flag6;	
 #endif
		if(SD_Mileage_sum_flag == 1)
		{
//			taskENTER_CRITICAL();																								 //进入临界区
			res_flash = f_open(&fnew, "1:里程数据/U12里程数据.txt",FA_OPEN_EXISTING | FA_WRITE);
			if ( res_flash == FR_OK )
			{
				uint16_t b_size = 0;
#if TEST_PRINTF_SD
				printf("》打开U12里程数据.txt文件成功，向文件写入数据。\r\n");				
#endif
				if (res_flash == FR_OK)
				{
					b_size = 0x22;                                                   //该值通过查看1:里程数据/记录里程数位置.txt获得
					/* 写入里程数据 */
					sprintf(Fdat,"%.1f",Mileage_sum);
					res_flash = f_lseek(&fnew,b_size);
					f_printf(&fnew,"%20s",Fdat);
#if TEST_PRINTF_SD
					printf("数据写入完毕\n");					
#endif
				}
//				taskEXIT_CRITICAL();                                               //退出临界区
			}
			f_close(&fnew);
		}
		else
		{
			taskENTER_CRITICAL();
			Mileage_sum = SD_ReadtoMileage_count();
#if TEST_PRINTF_SD
			printf("Mileage_sum: %.1f\r\n",Mileage_sum);
#endif
			SD_Mileage_sum_flag = 1;
			taskEXIT_CRITICAL(); 
	  }
		vTaskDelay(1010);
  }
}
/**********************************************************************
  * @ 函数名  ： SD_ReadtoMileage_count
  * @ 功能说明： 读出SD卡中的车辆行驶里程
  * @ 参数    ：   
  * @ 返回值  ： Readdat
  ********************************************************************/
static float SD_ReadtoMileage_count(void)
{
	DIR dir;
	float Readdat = 0;
	char floatdat[50];
	uint32_t b_size = 0;
				/* 尝试打开目录 */
	res_flash=f_opendir(&dir,"1:里程数据");
	if(res_flash!=FR_OK)
	{
		float starter = 0.0;
		char BSIZE[10] = {0};
		/* 打开目录失败，就创建目录 */
		res_flash=f_mkdir("1:里程数据");
		if(res_flash != FR_OK)
		{
			printf("创建文件失败(%d)",res_flash);				
		}
		res_flash=f_closedir(&dir);
		res_flash = f_open(&fnew, "1:里程数据/U12里程数据.txt",FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
		if ( res_flash == FR_OK )
		{
#if TEST_PRINTF_SD
			printf("》打开/创建U12里程数据.txt文件成功，向文件写入数据。\n");
#endif
			if (res_flash == FR_OK)
			{
				res_flash = f_lseek(&fnew,0);
				/* 格式化写入，参数格式类似printf函数 */
				f_printf(&fnew,"》飞翼车队U12里程记录\n");
				f_printf(&fnew,"》里程大小：");
				b_size = f_size(&fnew);
#if TEST_PRINTF_SD				
				printf("文字数据大小：0x%x\n",b_size);
#endif				
				sprintf(floatdat,"%.1f",starter);
				res_flash = f_lseek(&fnew,b_size);
				f_printf(&fnew,"%20s",floatdat);
				res_flash = f_lseek(&fnew,0);
				/* 读取文件所有内容到缓存区 */
				res_flash = f_read(&fnew,ReadBuffer,f_size(&fnew),&fnum);
				if(res_flash == FR_OK)
				{
#if TEST_PRINTF_SD					
					printf("》文件内容：\n%s\n",ReadBuffer);
#endif
				}
			}
		}
		f_close(&fnew);
		res_flash = f_open(&fnew, "1:里程数据/记录里程数位置.txt",FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
		sprintf(BSIZE,"0x%x",b_size);
		f_printf(&fnew,"%s",BSIZE);
		res_flash = f_lseek(&fnew,0);
		/* 读取文件所有内容到缓存区 */
		res_flash = f_read(&fnew,ReadBuffer,f_size(&fnew),&fnum);
		if(res_flash == FR_OK)
		{
#if TEST_PRINTF_SD
			printf("》文件内容：\n%s\n",ReadBuffer);
#endif
		}
		f_close(&fnew);
	}
	else
	{
		res_flash=f_closedir(&dir);
		res_flash = f_open(&fnew, "1:里程数据/U12里程数据.txt",FA_OPEN_EXISTING | FA_WRITE | FA_READ);
		if ( res_flash == FR_OK )
		{
#if TEST_PRINTF_SD
			printf("》打开U12里程数据.txt文件成功，向文件读取数据。\n");
#endif
			if (res_flash == FR_OK)
			{
				b_size = 0x22;                                               //该值通过查看1:里程数据/记录里程数位置.txt获得
				res_flash = f_lseek(&fnew,b_size);
				/* 读取里程数据 */
				res_flash = f_read(&fnew,ReadBuffer,f_size(&fnew),&fnum);
				if(res_flash == FR_OK)
				{
#if TEST_PRINTF_SD
					printf("\n%s\n",ReadBuffer);
#endif
					Readdat = atof(ReadBuffer);
#if TEST_PRINTF_SD
					printf("浮点数据：\r\n%.1f\r\n",Readdat);
#endif					
				}
				f_close(&fnew);
			}
		}
		else
		{
			printf("！！打开文件失败。(%d)\r\n",res_flash);
		}
	}
	return Readdat;
}


/**********************************************************************
  * @ 函数名  ： Battery_capacity_Task
  * @ 功能说明： 电池电量检测函数
			满电量       Battery >3270 mv
			5格电量      3190 < Battery < 3270 mv
			4格电量      3100 < Battery < 3190 mv
			3格电量      3020 < Battery < 3100 mv
			2格电量      2930 < Battery < 3020 mv
			1格电量      2870 < Battery < 2930 mv
			0格电量      Battery < 2850 mv
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/

static void Battery_capacity_Task(void* parameter)
{
	while(1)
	{
		Battery = ADC_SingleMode_GetValue(ADC_Channel_4)*3300/4096;
#if TEST_PRINTF_SIMULATOR
		flag7 =~ flag7;
#endif
#if TEST_PRINTF_BATTERY
		printf("Battery = %d mv \r\n",Battery);	
#endif
		if(Battery >3270)
		{Battery_flag = 6;
		}
		else if( Battery > 3190&&Battery < 3270)
		{Battery_flag = 5;
		}
		else if( Battery > 3100&&Battery < 3190)
		{Battery_flag = 4;
		}
		else if( Battery > 3020&&Battery < 3100)
		{Battery_flag = 3;
		}
		else if( Battery > 2930&&Battery < 3020)
		{Battery_flag = 2;
		}
		else if( Battery > 2870&&Battery < 2930)
		{Battery_flag = 1;
		}
		else if(Battery < 2850)
		{Battery_flag = 0;
		}
#if TEST_PRINTF_BATTERY
		printf("Battery_flag = %d 格 \r\n",Battery_flag);		
#endif
    if(Battery_flag <= 1&&DTUTask1 == 0)
		{
			BKP_WriteBackupRegister(BKP_DR40, 0x18);
			__set_FAULTMASK(1);
			NVIC_SystemReset();
		}
		else if(Battery_flag <= 1&&DTUTask1 == 1)
		{
			TIM_Cmd(CPU_NORMAL_TIM, ENABLE);
		}
		vTaskDelay(30050);
	}
}


/**********************************************************************
  * @ 函数名  ： CPU_Task
  * @ 功能说明： CPU利用率统计函数
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/

#if TEST_PRINTF_CPU

static void CPU_Task(void* parameter)
{	
  uint8_t CPU_RunInfo[400];		//保存任务运行时间信息
  
  while (1)
  {
    memset(CPU_RunInfo,0,400);				//信息缓冲区清零
    
    vTaskList((char *)&CPU_RunInfo);  //获取任务运行时间信息
    
    printf("---------------------------------------------\r\n");
    printf("任务名      任务状态 优先级   剩余栈 任务序号\r\n");
    printf("%s", CPU_RunInfo);
    printf("---------------------------------------------\r\n");
    
    memset(CPU_RunInfo,0,400);				//信息缓冲区清零
    
    vTaskGetRunTimeStats((char *)&CPU_RunInfo);
    
    printf("任务名       运行计数         使用率\r\n");
    printf("%s", CPU_RunInfo);
    printf("---------------------------------------------\r\n\n");
    vTaskDelay(1000);   /* 延时500个tick */		
  }
}

#endif
/**********************************************************************
  * @ 函数名  ： OneNET_4G_DTU_Task
  * @ 功能说明： 通过4G DTU向OneNET中国移动物联网平台发送车辆动态数据
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/

static void OneNET_4G_DTU_Task(void* pvParameters)                         
{
	EventBits_t r_event;
	int res = 0;
	uint8_t i = 0;
	static uint8_t k = 0;
	while(1)
	{
		r_event = xEventGroupWaitBits(EventGroupHandler,EVENTBIT_4,pdFALSE,pdTRUE,portMAX_DELAY);
		if((r_event&EVENTBIT_4) == EVENTBIT_4)
		{
	#if TEST_PRINTF_ONENET
			printf("OneNET_4G_DTU_Task\r\n");
	#endif
	#if TEST_PRINTF_SIMULATOR
			flag8 =~ flag8;
	#endif
			if(DTUMode_Switch_flag == 0)             															 //判断是否处于控制阶段 0处于配置状态 1处于透传状态
			{
				/*1.DTU进入配置状态*/
				while(i<10)
				{
					res = dtu_enter_configmode();
					if ( res != 0 )
					{
						printf("进入配置失败\r\n");
						i++;
						vTaskDelay(10);
					}
					else
					{
						i = 0;
						break;
					}
				}
				/*DTU进入透传状态*/
				res = dtu_enter_transfermode();
				if( res != 0 )
				{
						printf("DTU进入透传状态失败\r\n");
				}
				else
				{
					DTUMode_Switch_flag = 1;
					printf("DTU进入透传状态\r\n");
					OneNet_FillBuf(Send_date);   
					send_data_to_dtu(&Sdat[0], sizeof(Sdat[0]));
					send_data_to_dtu(&Sdat[1], sizeof(Sdat[1]));
					send_data_to_dtu(&Sdat[2], sizeof(Sdat[2]));
					if(k<=10)
					{
						send_data_to_dtu(Send_date[0], sizeof(Send_date[0]));
						delay_xms(50);
						send_data_to_dtu(Send_date[1], sizeof(Send_date[1]));
						delay_xms(50);
						k++;					
					}
					else
					{
						k = 0;
						send_data_to_dtu(Send_date[2], sizeof(Send_date[2]));
						delay_xms(50);
						send_data_to_dtu(Send_date[3], sizeof(Send_date[3]));
						delay_xms(50);
						send_data_to_dtu(Send_date[4], sizeof(Send_date[4]));					
					}			
#if TEST_PRINTF_ONENET
#endif
				}
			}
			else
			{
				OneNet_FillBuf(Send_date);
				if(k <= 10)
				{
					send_data_to_dtu(Send_date[0], sizeof(Send_date[0]));
					delay_xms(50);
					send_data_to_dtu(Send_date[1], sizeof(Send_date[1]));
					delay_xms(50);
					k++;					
				}
				else
				{
					k = 0;
					send_data_to_dtu(Send_date[2], sizeof(Send_date[2]));
					delay_xms(50);
					send_data_to_dtu(Send_date[3], sizeof(Send_date[3]));
					delay_xms(50);
					send_data_to_dtu(Send_date[4], sizeof(Send_date[4]));					
				}
			}
			xEventGroupSetBits(EventGroupHandler,EVENTBIT_3);
			vTaskDelay(3510);
		}
	}
}

/**********************************************************************
  * @ 函数名  ： DTU_4G_Task
  * @ 功能说明： 通过4G DTU获取网络信号，通过4G DTU获取实时时钟
  * 	满格信号     CSQvalue >14
			4格信号      9 < CSQvalue <= 14
			3格信号      5 < CSQvalue <= 9
			2格信号      2 < CSQvalue <= 5
			1格信号      1 < CSQvalue <= 2
			0格信号      CSQvalue = 0
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/



static void DTU_4G_Task(void* pvParameters)                              
{
	BaseType_t xReturn = pdPASS;
	uint8_t i = 0,res = 0;
	while(1)
	{
#if TEST_PRINTF_SIMULATOR
		flag9 =~ flag9;
#endif		
		xReturn = xSemaphoreTake(MuxSem_Handle,/* 互斥量句柄 */
															portMAX_DELAY); /* 等待时间 */
#if TEST_PRINTF_4GDTU		
		if(xReturn!=0)
		{printf("DTU_4G_Task_Handle 获得互斥量失败!\r\n");		
		}
		else
		{printf("DTU_4G_Task_Handle 获得互斥量!\r\n");
			printf("DTU_4G_Task_Handle\r\n");
		}
#endif
    if(DTU_Time_flag == 1)
		{
#if 	TEST_PRINTF_4GDTU
			printf("Signal_4G_DTU_Task\r\n");
#endif
				while(i<10)
				{
					/*1.DTU进入配置状态*/
					res = dtu_enter_configmode();
					if ( res != 0 )
					{
						printf("进入配置失败\r\n");
						vTaskDelay(10);
						i++;
					}
					else
					{
						i = 0;
						DTUMode_Switch_flag = 0;
						send_data_to_dtu("AT+CSQ\r\n", strlen("AT+CSQ\r\n"));
#if 		  TEST_PRINTF_4GSIGNAL
						printf("AT+CSQ\r\n");
#endif
						break;
					}
				}
		}		
    else
		{
				while(i<10)
				{
					/*1.DTU进入配置状态*/
					res = dtu_enter_configmode();
					if ( res != 0 )
					{
						printf("进入配置失败\r\n");
						vTaskDelay(10);
						i++;
					}
					else
					{			
						i = 0;
						DTUMode_Switch_flag = 0;
						send_data_to_dtu("AT+CLK\r\n", strlen("AT+CLK\r\n"));
						DTU_Time_flag = 1;
#if TEST_PRINTF_4GDTU				
						printf("AT+CLK\r\n");
#endif	
						break;
					}
				}
		}
	  xReturn = xSemaphoreGive( MuxSem_Handle );//给出互斥量
#if TEST_PRINTF_4GDTU
		if(xReturn != 0)
		{		printf("DTU_4G_Task_Handle 释放互斥量!\r\n");
		}
		else
		{}
#endif	
			vTaskDelay(20050);
  }
}

/**********************************************************************
  * @ 函数名  ： RingBuffer_Read_Task
  * @ 功能说明： 接受4G DTU返回的数据与警告
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/

static void RingBuffer_Read_Task(void* pvParameters)
{
	BaseType_t xReturn = pdPASS;
	uint8_t buf = 0;
	char *token;
	char DTU_Dat[6][64] = {0};
	const char DTU_ATCLK[] = "AT+CLK";
	const char DTU_ATCSQ[] = "AT+CSQ";
	uint8_t i = 0,CSQvalue = 0;
	while(1)
	{
#if TEST_PRINTF_SIMULATOR
	  flag11 =~ flag11;	
#endif	

		xReturn = xSemaphoreTake(MuxSem_Handle,/* 互斥量句柄 */
													portMAX_DELAY); /* 等待时间 */
#if TEST_PRINTF_RINGBUFFER
		if(xReturn != 0 )
		{}
		else
    {printf("RingBuffer_Read_Task 获取互斥量!\r\n");	
		}								
#endif
		if (RingBuffer_Len(p_uart2_rxbuf) > 0)          /*接收到DTU传送过来的服务器数据*/
		{
				RingBuffer_Out(p_uart2_rxbuf, &buf, 1);
				dtu_rxbuf[dtu_rxlen++] = buf;
				dtu_get_urc_info(buf);                      /*解析DTU上报的URC信息*/
				if (dtu_rxlen >= DTU_ONENETDATE_RX_BUF)     /*接收缓存溢出*/
				{
						usart1_send_data(dtu_rxbuf, dtu_rxlen); /*接收到从DTU传过来的网络数据，转发到调试串口1输出*/
						dtu_rxlen = 0;
				}
		}
		else
		{
				if (dtu_rxlen > 0)
				{
#if TEST_PRINTF_RINGBUFFER
						usart1_send_data(dtu_rxbuf, dtu_rxlen); /*接收到从DTU传过来的网络数据，转发到调试串口1输出*/
#endif
					  if(strcmp((char *)dtu_rxbuf,ONENET_COM_OFF)== 0)
						{
#if TEST_PRINTF_RINGBUFFER							
							printf("data = %s\r\n",dtu_rxbuf);
#endif							
							ONENET_OFF_FLAG = 1;
							xEventGroupClearBits(EventGroupHandler,EVENTBIT_4);
						}
						else if(strcmp((char *)dtu_rxbuf,ONENET_COM_ON)== 0)
						{
							ONENET_OFF_FLAG = 0;
							xEventGroupSetBits(EventGroupHandler,EVENTBIT_4);							
						}
						/* 获取第一个子字符串 */
						token = strtok((char *)dtu_rxbuf, sqpa[0]);
						/* 继续获取其他的子字符串 */
						while( token != NULL )
						{
							i++;
							strcpy(DTU_Dat[i],token);
#if TEST_PRINTF_RINGBUFFER
							printf( "%s\r\n", DTU_Dat[i]);					
#endif
							token = strtok(NULL, sqpa[0]);
						}
						if(strcmp(DTU_Dat[1],DTU_ATCSQ) == 0)
						{
							CSQvalue = DTU_AT_CSQ_DataAnalyze(DTU_Dat);
							if(CSQvalue > 14){
							Network_size = 5;}
							else if(CSQvalue > 9&&CSQvalue <= 14){
							Network_size = 4;}
							else if(CSQvalue > 5&&CSQvalue <= 9){
							Network_size = 3;}
							else if(CSQvalue > 2&&CSQvalue <= 5){
							Network_size = 2;}
							else if(CSQvalue > 1&&CSQvalue <= 2){
							Network_size = 1;}
							else if(CSQvalue == 0){
							Network_size = 0;}
#if TEST_PRINTF_RINGBUFFER
						  printf("CSQvalue = %d\r\n",CSQvalue);	
#endif
						}
						else if(strcmp(DTU_Dat[1],DTU_ATCLK) == 0)
						{
							DTU_AT_CLK_DataAnalyze(DTU_Dat);
							DTU_AT_CLKFLAG = 1;
#if TEST_PRINTF_RINGBUFFER
							printf("%d \r\n",Timedat.year);
							printf("%d \r\n",Timedat.month);
							printf("%d \r\n",Timedat.day);
							printf("%d \r\n",Timedat.hour);
							printf("%d \r\n",Timedat.minute);
							printf("%d \r\n",Timedat.second);
						  printf("CLKvalue = %s\r\n",DTU_Dat[1]);
#endif	
						}
						i = 0;
						dtu_rxlen = 0;
				}	
		}
	  xReturn = xSemaphoreGive( MuxSem_Handle );//给出互斥量
#if TEST_PRINTF_RINGBUFFER
		if(xReturn != 0)
		{}
		else
		{printf("RingBuffer_Read_Task 释放互斥量!\r\n");
		}
#endif
		if(ONENET_OFF_FLAG == 1)
		{
			xEventGroupSetBits(EventGroupHandler,EVENTBIT_3);
		}
			vTaskDelay(3);	  
	}
}


/**********************************************************************
  * @ 函数名  ： IWDG_Task
  * @ 功能说明： 防止4G DTU-Onenet函数跑飞
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/
static void IWDG_Task(void* pvParameters)
{
	EventBits_t r_event;
	while(1)
	{
		r_event = xEventGroupWaitBits(EventGroupHandler,EVENTBIT_3,pdTRUE,pdTRUE,portMAX_DELAY);
		if((r_event&EVENTBIT_3) == EVENTBIT_3)
		{
#if TEST_IWDG
			printf("IWDG_ReloadCounter \r\n");
 #endif
			IWDG_ReloadCounter();
		}
	}
}


static void OneNet_FillBuf(uint8_t buff[][64])
{
	char text[64] = {0};
	memset(text, 0, sizeof(text));
	sprintf(text, "{'FWT_Car_Speed':'%d'}", Car_SpeedData); //FWT_Car_Speed是数据流的一个名称，Car_SpeedData是速度值;
#if TEST_PRINTF_ONENET
	printf("Car_SpeedData = %d\r\n",Car_SpeedData);	
#endif
	strcpy((char*)buff[0], text);
	memset(text, 0, sizeof(text));
	sprintf(text, "{'FWT_Car_Oil':'%d'}", Oil);
#if TEST_PRINTF_ONENET
		printf("Oil = %d\r\n",Oil);
#endif
	strcpy((char*)buff[1], text);
	memset(text, 0, sizeof(text));
	sprintf(text, "{'FWT_Car_Mileage':'%.2f'}", Mileage_sum/1000);
#if TEST_PRINTF_ONENET
	printf("Mileage_sum = %.2f\r\n",Mileage_sum/1000);	
#endif
	strcpy((char*)buff[2], text);
	memset(text, 0, sizeof(text));
	sprintf(text, "{'FWT_VCUBattery':'%d'}", Battery_flag);
#if TEST_PRINTF_ONENET
	printf("Battery_flag = %d\r\n",Battery_flag);	
#endif	
	strcpy((char*)buff[3], text);
	memset(text, 0, sizeof(text));
	sprintf(text, "{'FWT_4G_Signal':'%d'}", Network_size);
#if TEST_PRINTF_ONENET
	printf("Network_size = %d\r\n",Network_size);	
#endif	
	strcpy((char*)buff[4], text);	
}
