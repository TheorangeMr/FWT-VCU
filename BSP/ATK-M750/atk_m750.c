/*******************************************
	*文件名 ：  VCU4.3
	*作   者：  罗成
	*修改时间： 2022.10.5
	*版   本：  由正点原子基础例程修改而来
*******************************************/


#include "bsp_delay.h"
#include "usart.h"
#include "uart2.h"
#include "atk_m750.h"
#include "RingBuffer.h"
#include "string.h"
#include "stdlib.h"

extern RingBuffer *p_uart2_rxbuf;

ST_Time Timedat;

static uint8_t dtu_rxcmdbuf[DTU_RX_CMD_BUF_SIZE]; /*处理DTU相关数据缓存*/

const char sqpa[4][4] = {"\r\n","/",",",":"};

/**
 * @brief       发送数据到DTU
 * 
 * @param       data:   需要发送数据的缓存地址
 * @param       size:   发送数据大小
 * 
 * @return      无
 * 
*/
void send_data_to_dtu(uint8_t *data, uint32_t size)
{
    usart2_send_data(data, size);
}

/**
 * @brief       发送命令到DTU并进行数据校验
 * 
 * @param       cmd     :   需要发送的AT指令
 * @param       ask     :   需要校验的应答数据
 * @param       timeout :   AT指令校验超时时间，单位：100ms
 * 
 * @return      1  :   校验ask数据成功
 *              0  :   DTU返回OK
 *             -1  :   DTU返回ERROR
 *             -2  :   发送AT指令校验超时
 */
static int send_cmd_to_dtu(char *cmd, char *ask, uint32_t timeout)
{
    uint32_t rx_len = 0;
	
    /*初始化缓存数据*/
    memset(dtu_rxcmdbuf, 0, DTU_RX_CMD_BUF_SIZE);
    RingBuffer_Reset(p_uart2_rxbuf);
    /*发送AT指令到DTU*/
    send_data_to_dtu((uint8_t *)cmd, strlen(cmd));

    /*等待DTU应答AT指令结果*/
    while (1)
    {
        if (strstr((char *)dtu_rxcmdbuf, ask) != NULL)
        {
            return 1;
        }
        else if (strstr((char *)dtu_rxcmdbuf, "OK") != NULL)
        {
            return 0;
        }
        else if (strstr((char *)dtu_rxcmdbuf, "ERROR") != NULL)
        {
            return -1;
        }

        if (RingBuffer_Len(p_uart2_rxbuf) > 0)
        {
            RingBuffer_Out(p_uart2_rxbuf, &dtu_rxcmdbuf[rx_len++], 1); //从串口缓存中读一个字节

            if (rx_len >= DTU_RX_CMD_BUF_SIZE) /*接收应答数据超长，返回ERROR*/
            {
                return -1;
            }
        }
        else
        {
            timeout--;

            if (timeout == 0)
            {
                return -2;
            }
						delay_xms(100);
        }
    }
}

/**
 * @brief       DTU进入配置状态
 * 
 * @param       无
 * 
 * @return      0  :    成功进入配置状态
 *             -1  :    进入配置状态失败
 */
 int dtu_enter_configmode(void)
{
    int res;
    /* 1.发送+++准备进入配置状态 */
    res = send_cmd_to_dtu("+++", "atk", 5);
    if (res == -1) /*返回ERRRO表示DTU已经处于配置状态*/
    {
        return 0;
    }
    /* 2.发送atk确认进入配置状态 */
    res = send_cmd_to_dtu("atk", "OK", 5);
    if (res == -2)
    {
        return -1;
    }
    return 0;
}

/**
 * @brief       DTU进入透传状态
 * 
 * @param       无
 * 
 * @return      0  :    成功进入透传状态
 *             -1  :    进入透传状态失败
 */
int dtu_enter_transfermode(void)
{
    if (send_cmd_to_dtu("ATO\r\n", "OK", 5) >= 0)
    {
        return 0;
    }
    return -1;
}

/**
 * @brief       DTU自动上报URC信息处理函数:处理+ATK ERROR信息
 * 
 * @param       data    :   接收到DTU的URC数据缓存
 * @param       len     :   URC数据长度
 * 
 * @return      无
 */
static void dtu_urc_atk_error(const char *data, uint32_t len)
{
    printf("\r\nURC :   dtu_urc_atk_error\r\n");
}

/**
 * @brief       DTU自动上报URC信息处理函数:处理Please check SIM Card信息
 * 
 * @param       data    :   接收到DTU的URC数据缓存
 * @param       len     :   URC数据长度
 * 
 * @return      无
 */
static void dtu_urc_error_sim(const char *data, uint32_t len)
{
    printf("\r\nURC :   dtu_urc_error_sim\r\n");
}

/**
 * @brief       DTU自动上报URC信息处理函数:处理Please check GPRS信息
 * 
 * @param       data    :   接收到DTU的URC数据缓存
 * @param       len     :   URC数据长度
 * 
 * @return      无
 */
static void dtu_urc_error_gprs(const char *data, uint32_t len)
{
    printf("\r\nURC :   dtu_urc_error_gprs\r\n");
}

/**
 * @brief       DTU自动上报URC信息处理函数:处理Please check CSQ信息
 * 
 * @param       data    :   接收到DTU的URC数据缓存
 * @param       len     :   URC数据长度
 * 
 * @return      无
 */
static void dtu_urc_error_csq(const char *data, uint32_t len)
{
    printf("\r\nURC :   dtu_urc_error_csq\r\n");
}

/**
 * @brief       DTU自动上报URC信息处理函数:处理Please check MQTT Parameter信息
 * 
 * @param       data    :   接收到DTU的URC数据缓存
 * @param       len     :   URC数据长度
 * 
 * @return      无
 */
static void dtu_urc_error_mqtt(const char *data, uint32_t len)
{
    printf("\r\nURC :   dtu_urc_error_mqtt\r\n");
}

typedef struct
{
    const char *urc_info;                         /*DTU自动上报的URC信息*/
    void (*func)(const char *data, uint32_t len); /*回调处理函数*/
} _dtu_urc_st;

#define DTU_ATK_M750_URC_SIZE 5
static _dtu_urc_st DTU_ATK_M750_URC[DTU_ATK_M750_URC_SIZE] =
    {
       
        {"+ATK ERROR:",                         dtu_urc_atk_error},         /*DTU存在问题，需要联系技术支持进行确认*/
        {"Please check SIM Card !!!\r\n",       dtu_urc_error_sim},         /*DTU未检测到手机卡,请检查手机卡是否正确插入*/
        {"Please check GPRS !!!\r\n",           dtu_urc_error_gprs},        /*请检查SIM卡是否欠费*/
        {"Please check CSQ !!!\r\n",            dtu_urc_error_csq},         /*请检查天线是否正确接入，并确保天线位置的正确性*/
        {"Please check MQTT Parameter !!!\r\n", dtu_urc_error_mqtt},        /*MQTT参数有误*/
};

/**
 * @brief       处理DTU主动上报的URC信息数据，注意：串口每接收一个字节数据，都需要通过函数入口传输进来
 * 
 * @param       ch    :   串口接收的一个字节数据
 * 
 * @return      无
 */
void dtu_get_urc_info(uint8_t ch)
{
    static uint8_t ch_last = 0;
    static uint32_t rx_len = 0;
    int i;

    /*接收DTU数据*/
    dtu_rxcmdbuf[rx_len++] = ch;
    if (rx_len >= DTU_RX_CMD_BUF_SIZE)
    { /*超长处理*/
        ch_last = 0;
        rx_len = 0;
        memset(dtu_rxcmdbuf, 0, DTU_RX_CMD_BUF_SIZE);
    }

    /*处理DTU的URC数据*/
    if ((ch_last == '\r') && (ch == '\n'))
    {
        for (i = 0; i < DTU_ATK_M750_URC_SIZE; i++)
        {
            if (strstr((char *)dtu_rxcmdbuf, DTU_ATK_M750_URC[i].urc_info) == (char *)dtu_rxcmdbuf)
            {
                DTU_ATK_M750_URC[i].func((char *)dtu_rxcmdbuf, strlen((char *)dtu_rxcmdbuf));
            }
        }

        ch_last = 0;
        rx_len = 0;
        memset(dtu_rxcmdbuf, 0, DTU_RX_CMD_BUF_SIZE);
    }

    ch_last = ch;
}

static const _dtu_atcmd_st dtu_net_param_info[] = {

    /*1.选择工作模式为：网络透传模式*/
    {5, "AT+WORK\r\n",      "AT+WORK=\"NET\"\r\n"},

    /*2.配置网络透传模式的工作参数*/
    {5, "AT+LINK1EN\r\n",   "AT+LINK1EN=\"ON\"\r\n"},
    {5, "AT+LINK1\r\n",     "AT+LINK1=\"TCP\",\"cloud.alientek.com\",\"59666\"\r\n"},
    {5, "AT+LINK1MD\r\n",   "AT+LINK1MD=\"LONG\"\r\n"},
    {5, "AT+LINK1TM\r\n",   "AT+LINK1TM=\"5\"\r\n"},

    {5, "AT+LINK2EN\r\n",   "AT+LINK2EN=\"OFF\"\r\n"},
    {5, "AT+LINK3EN\r\n",   "AT+LINK3EN=\"OFF\"\r\n"},
    {5, "AT+LINK4EN\r\n",   "AT+LINK4EN=\"OFF\"\r\n"},

    /*3.配置原子云功能：默认打开*/
    {5, "AT+SVREN\r\n",     "AT+SVREN=\"ON\"\r\n"},
    {5, "AT+SVRNUM\r\n",    "AT+SVRNUM=\"12345678901234567890\"\r\n"},
    {5, "AT+SVRKEY\r\n",    "AT+SVRKEY=\"12345678\"\r\n"},

    /*4.配置心跳包功能：默认打开         注意：强烈建议开启心跳包功能！！！*/
    {5, "AT+HRTEN\r\n",     "AT+HRTEN=\"ON\"\r\n"},
    {5, "AT+HRTDT\r\n",     "AT+HRTDT=\"414C49454E54454B2D4852544454\"\r\n"},
    {5, "AT+HRTTM\r\n",     "AT+HRTTM=\"120\"\r\n"},

    /*5.配置注册包功能：默认关闭 */
    {5, "AT+REGEN\r\n",     "AT+REGEN=\"OFF\"\r\n"},
    {5, "AT+REGDT\r\n",     "AT+REGDT=\"414C49454E54454B2D5245474454\"\r\n"},
    {5, "AT+REGMD\r\n",     "AT+REGMD=\"LINK\"\r\n"},
    {5, "AT+REGTP\r\n",     "AT+REGTP=\"IMEI\"\r\n"},

    /*6.其他参数功能配置*/
};

static const _dtu_atcmd_st dtu_http_param_info[] = {

    /*1.选择工作模式为：HTTP透传模式*/
    {5, "AT+WORK\r\n",      "AT+WORK=\"HTTP\"\r\n"},

    /*2.配置HTTP透传模式的工作参数*/
    {5, "AT+HTTPMD\r\n",    "AT+HTTPMD=\"GET\"\r\n"},
    {5, "AT+HTTPURL\r\n",   "AT+HTTPURL=\"https://cloud.alientek.com/testfordtu?data=\"\r\n"},
    {5, "AT+HTTPTM\r\n",    "AT+HTTPTM=\"10\"\r\n"},
    {5, "AT+HTTPHD\r\n",    "AT+HTTPHD=\"Connection:close\"\r\n"},

    /*3.其他参数功能配置*/
};

static const _dtu_atcmd_st dtu_mqtt_param_info[] = {

    /*1.选择工作模式为：MQTT透传模式*/
    {5, "AT+WORK\r\n",      "AT+WORK=\"MQTT\"\r\n"},

    /*2.配置MQTT透传模式的工作参数*/
    {5, "AT+MQTTCD\r\n",    "AT+MQTTCD=\"alientek\"\r\n"},
    {5, "AT+MQTTUN\r\n",    "AT+MQTTUN=\"admin\"\r\n"},
    {5, "AT+MQTTPW\r\n",    "AT+MQTTPW=\"password\"\r\n"},
    {5, "AT+MQTTIP\r\n",    "AT+MQTTIP=\"cloud.alientek.com\",\"1883\"\r\n"},
    {5, "AT+MQTTSUB\r\n",   "AT+MQTTSUB=\"atk/sub\"\r\n"},
    {5, "AT+MQTTPUB\r\n",   "AT+MQTTPUB=\"atk/pub\"\r\n"},
    {5, "AT+MQTTCON\r\n",   "AT+MQTTCON=\"0\",\"0\",\"1\",\"300\"\r\n"},

    /*3.其他参数功能配置*/
};

static const _dtu_atcmd_st dtu_aliyun_param_info[] = {

    /*1.选择工作模式为：阿里云透传模式*/
    {5, "AT+WORK\r\n",      "AT+WORK=\"ALIYUN\"\r\n"},

    /*2.配置MQTT透传模式的工作参数*/
    {5, "AT+ALIPK\r\n",     "AT+ALIPK=\"ProductKey\"\r\n"},
    {5, "AT+ALIDS\r\n",     "AT+ALIDS=\"DeviceSecret\"\r\n"},
    {5, "AT+ALIDN\r\n",     "AT+ALIDN=\"DeviceName\"\r\n"},
    {5, "AT+ALIRI\r\n",     "AT+ALIRI=\"cn-shanghai\"\r\n"},
    {5, "AT+ALISUB\r\n",    "AT+ALISUB=\"get\"\r\n"},
    {5, "AT+ALIPUB\r\n",    "AT+ALIPUB=\"updata\"\r\n"},
    {5, "AT+ALICON\r\n",    "AT+ALICON=\"0\",\"0\",\"1\",\"300\"\r\n"},

    /*3.其他参数功能配置*/
};

static const _dtu_atcmd_st dtu_onenet_param_info[] = {

    /*1.选择工作模式为：OneNET透传模式*/
    {5, "AT+WORK\r\n",      "AT+WORK=\"ONENET\"\r\n"},

    /*2.配置OneNET透传模式的工作参数*/
    {5, "AT+ONEDI\r\n",     "AT+ONEDI=\"998293424\"\r\n"},
    {5, "AT+ONEPI\r\n",     "AT+ONEPI=\"544458\"\r\n"},
    {5, "AT+ONEAI\r\n",     "AT+ONEAI=\"12345678\"\r\n"},
    {5, "AT+ONEIP\r\n",     "AT+ONEIP=\"mqtt.heclouds.com\",\"6002\"\r\n"},
    {5, "AT+ONECON\r\n",    "AT+ONECON=\"3\",\"0\",\"0\",\"1\",\"300\"\r\n"},

    /*3.其他参数功能配置*/
};

static const _dtu_atcmd_st dtu_baiduyun_param_info[] = {

    /*1.选择工作模式为：百度云透传模式*/
    {5, "AT+WORK\r\n",      "AT+WORK=\"BAIDUYUN\"\r\n"},

    /*2.配置百度云透传模式的工作参数*/
    {5, "AT+BAIEP\r\n",     "AT+BAIEP=\"alientek\"\r\n"},
    {5, "AT+BAINM\r\n",     "AT+BAINM=\"name\"\r\n"},
    {5, "AT+BAIKEY\r\n",    "AT+BAIKEY=\"key\"\r\n"},
    {5, "AT+BAIRI\r\n",     "AT+BAIRI=\"gz\"\r\n"},
    {5, "AT+BAISUB\r\n",    "AT+BAISUB=\"sub\"\r\n"},
    {5, "AT+BAIPUB\r\n",    "AT+BAIPUB=\"pub\"\r\n"},
    {5, "AT+BAICON\r\n",    "AT+BAICON=\"0\",\"0\",\"0\",\"1\",\"300\"\r\n"},

    /*3.其他参数功能配置*/
};

/**
 * @brief       配置DTU工作参数
 * 
 * @param       work_param      :   工作模式相关AT指令参数
 * @param       num             :   需要配置的AT指令参数数量
 * 
 * @return      0  :    所有参数配置成功
 *              n  :    第几个参数配置失败：1-n
 */
static int dtu_config_work_param(_dtu_atcmd_st *work_param, uint8_t num)
{
    int i;
    int res = 0;

    for (i = 0; i < num; i++)
    {
        res = send_cmd_to_dtu((work_param + i)->read_cmd,
                              (work_param + i)->write_cmd + strlen((work_param + i)->read_cmd) - 1,
                              work_param[i].timeout);

        if (res == 1) /*如果DTU内部参数和需要配置的参数一致，不需要重复去配置*/
        {
            continue;
        }
        else /*DTU内部参数与配置参数不一致，需要更新DTU内部参数*/
        {
            res = send_cmd_to_dtu((work_param + i)->write_cmd,
                                  "OK",
                                  (work_param + i)->timeout);

            if (res < 0)
            {
                return i+1;
            }
        }
    }

    return 0;
}

/**
 * @brief       初始化DTU的工作状态
 * 
 * @param       work_mode   :   DTU工作模式
 *  @arg        DTU_WORKMODE_NET,       0,  网络透传模式
 *  @arg        DTU_WORKMODE_HTTP,      1,  http透传模式
 *  @arg        DTU_WORKMODE_MQTT,      2,  mqtt透传模式
 *  @arg        DTU_WORKMODE_ALIYUN,    3,  阿里云透传模式
 *  @arg        DTU_WORKMODE_ONENET,    4,  OneNET透传模式
 *  @arg        DTU_WORKMODE_BAIDUYUN,  5,  百度云透传模式
 * 
 * @return      0   :   初始化成功
 *             -1   :   进入配状态失败
 *             -2   :   DTU工作参数配置失败
 *             -3   ：  DTU进入透传状态失败
 */
int dtu_config_init(_dtu_work_mode_eu work_mode)
{
    int res;

    /*1.DTU进入配置状态*/
    res = dtu_enter_configmode();
    if ( res != 0 )
    {
        return -1;
    }
    /*2.配置DTU的工作参数*/
    switch (work_mode)
    {
        case DTU_WORKMODE_NET:
        {
            res = dtu_config_work_param((_dtu_atcmd_st *)&dtu_net_param_info, sizeof(dtu_net_param_info) / sizeof(_dtu_atcmd_st));
            break;
        }
        case DTU_WORKMODE_HTTP:
        {
            res = dtu_config_work_param((_dtu_atcmd_st *)&dtu_http_param_info, sizeof(dtu_http_param_info) / sizeof(_dtu_atcmd_st));
            break;
        }
        case DTU_WORKMODE_MQTT:
        {
            res = dtu_config_work_param((_dtu_atcmd_st *)&dtu_mqtt_param_info, sizeof(dtu_mqtt_param_info) / sizeof(_dtu_atcmd_st));
            break;
        }
        case DTU_WORKMODE_ALIYUN:
        {
            res = dtu_config_work_param((_dtu_atcmd_st *)&dtu_aliyun_param_info, sizeof(dtu_aliyun_param_info) / sizeof(_dtu_atcmd_st));
            break;
        }
        case DTU_WORKMODE_ONENET:
        {
            res = dtu_config_work_param((_dtu_atcmd_st *)&dtu_onenet_param_info, sizeof(dtu_onenet_param_info) / sizeof(_dtu_atcmd_st));
            break;
        }
        case DTU_WORKMODE_BAIDUYUN:
        {
            res = dtu_config_work_param((_dtu_atcmd_st *)&dtu_baiduyun_param_info, sizeof(dtu_baiduyun_param_info) / sizeof(_dtu_atcmd_st));
            break;
        }
        default:
        {
            break;
        }
    }

    if( res != 0 )
    {
        return -2;
    }



    /*3.DTU进入透传状态*/
    res = dtu_enter_transfermode();
    if( res != 0 )
    {
        return -3;
    }
    return 0;
}





/**
 * @brief       发送短信示例
 * @param       phone : 手机号码
 * @param       sms_msg: 短信内容,只能是英文，不支持中文短信
 * @retval       1 : 短信发送OK，0：短信发送错误
 */

int dtu_send_sms(char *phone, char *sms_msg)
{
    #define DTU_SMS_SEND_BUF_MAX    (1024)
    static char dtu_sms_buf[1024];

    int res;
    int ret = 0;

    /*1.DTU进入配置状态*/
    res = dtu_enter_configmode();
    if ( res != 0 )
    {
        return -1;
    }

    snprintf(dtu_sms_buf, DTU_SMS_SEND_BUF_MAX, "AT+SMSEND=\"%s\",\"%s\"\r\n", phone, sms_msg);

    /* 2.DTU发送短信 */
    res = send_cmd_to_dtu(dtu_sms_buf, "SMSEND OK", 100);
    if( res == 1 )
    {
        ret = 1;
    }

    /*3.DTU进入透传状态*/
    res = dtu_enter_transfermode();
    if( res != 0 )
    {
        return -3;
    }

    return ret;
}

/*查询信号强弱指令 AT+CSQ 
查询基站定位指令 AT+LOC
当前网络信息指令 AT+SYSINFO
实时时钟查询指令 AT+CLK   */

/* 
	*函数名：DTU_AT_CLK_DataAnalyze
	*功  能：对DTU发送的实时时间数据解析
	*作  者：罗成
	*参  数：uint8_t *zdata : 接收到的原数据，uint8_t CLKDat[][32]: 解析后的数据保存在二维数组中
	*返回值：无
	*时  间：2022.10.12
*/

void DTU_AT_CLK_DataAnalyze(char CLKDat[][64])
{
	char *pstr,*token;
	char zdat[64] = {0};
	uint8_t i = 4,j = 0;
	pstr = &CLKDat[2][0];
	while(*pstr != '2')
	{
		pstr++;
	}
	while(*pstr != '"')
	{
		zdat[j] = *pstr;
		j++;
		pstr++;
	}
//	printf("time = %s\r\n",zdat);
	token = strtok(zdat, sqpa[1]);
	while( token != NULL ) 
	{
		i++;
		strcpy(CLKDat[i],token);
//		printf( "%s\r\n", CLKDat[i]);
		token = strtok(NULL, sqpa[1]);
	}
	token = strtok(CLKDat[7], sqpa[2]);
	i=7;
		//   CLK_Dat[0] = &token;
	while( token != NULL ) 
	{
		i++;
		strcpy(CLKDat[i],token);
//		printf( "%s\r\n", CLKDat[i]);
		token = strtok(NULL, sqpa[2]);
	}			
	token = strtok(CLKDat[9], sqpa[3]);
	i=9;
		//   CLK_Dat[0] = &token;
	while( token != NULL ) 
	{
		i++;
		strcpy(CLKDat[i],token);
//		printf( "%s\r\n", CLKDat[i]);
		token = strtok(NULL, sqpa[3]);
	}
/*   5:YEAR 6:MONTH 8:DAY 10:HOUR 11:MINUTE 12:SECOND        */                                      	
	Timedat.year = atoi(CLKDat[5]);
	Timedat.month = atoi(CLKDat[6]);
	Timedat.day = atoi(CLKDat[8]);
	Timedat.hour = atoi(CLKDat[10]);
	Timedat.minute = atoi(CLKDat[11]);
	Timedat.second = atoi(CLKDat[12]);
} 

/* 
	*函数名：DTU_AT_CSQ_DataAnalyze
	*功  能：对DTU发送的4G信号数据解析
	*作  者：罗成
	*参  数：uint8_t *zdata : 接收到的原数据，uint8_t CSQDat[][32]: 解析后的数据保存在二维数组中
	*返回值：value
	*时  间：2022.10.12
*/

uint8_t DTU_AT_CSQ_DataAnalyze(char CSQDat[][64])
{
	char signal[3] = {0};
	uint8_t value = 0;
	signal[0] = CSQDat[2][6];
	signal[1] = CSQDat[2][7];					
	value = (signal[0]-48)*10+(signal[1]-48);
  return value;
}


