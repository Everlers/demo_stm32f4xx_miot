#include "miio.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "interface.h"
#include "FreeRTOS.h"
#include "task.h"

miio_t mi;

static void miioGetModel(void);
static void miioRecvHandle(void *psed);
static void miioSendPsec(char *psec);
static void usartConfig(void);
static void usartDataSend(char *p);
static miio_ret usartDataRead(char *p,u16 pLen);

void miioInit(void)
{
	usartConfig();//配置串口
	memset(mi.mac,0,sizeof(mi.mac));
	memset(mi.time,0,sizeof(mi.time));
	mi.version = NULL;
	mi.net = MIIO_NET_OFFLINE;
	mi.getDownCount = 0;
	mi.usartFifoQueue = xQueueCreate(MIIO_USART_MAX_BYTE*MIIO_USART_MAX_PACK,sizeof(u8));//创建接收队列FIFO
	mi.usartRecvCount = xSemaphoreCreateCounting(MIIO_USART_MAX_PACK,0);//创建计数型信号量
	mi.semaphoreMutex = xSemaphoreCreateRecursiveMutex();//创建递归互斥锁(递归互斥信号量)
	
	xSemaphoreTakeRecursive(mi.semaphoreMutex,portMAX_DELAY);//获取互斥锁
	while(miioModelReboot() != MIIO_RET_OK);//复位模组
	while(miioUpMCUVersion(MIIO_MCU_VERSION) != MIIO_RET_OK);//上报MCU版本信息
	while(miioSetModelName(MIIO_MODEL_NAME) != MIIO_RET_OK);//设定模组名称	
	miioGetVersion();//查询模组版本信息
	miioGetMac();//查询模式MAC地址
	miioGetTime();//查询模组时间
	xSemaphoreGiveRecursive(mi.semaphoreMutex);//释放互斥锁
}

//模组数据处理
void miioTask(void)
{
	if(mi.usartRecvCount != NULL && mi.usartRecvCount != NULL)
	{
		xSemaphoreTakeRecursive(mi.semaphoreMutex,portMAX_DELAY);//获取互斥锁
		miioGetModel();//查询模组数据
		if(xSemaphoreTake(mi.usartRecvCount,MIIO_WAIT_TIMEOUT) == pdPASS)//如果接收到队列
		{
			char *psec = pvPortMalloc(MIIO_USART_MAX_BYTE);
			if(psec != NULL)
			{
				usartDataRead(psec,MIIO_USART_MAX_BYTE);//读取数据
				miioRecvHandle(psec);//处理接收到的数据
				vPortFree(psec);
			}
		}
		xSemaphoreGiveRecursive(mi.semaphoreMutex);//释放互斥锁
	}
	vTaskDelay(MIIO_GET_DOWN_TIME);//释放任务
}

//模组数据查询
void miioGetModel(void)
{
	miioSendPsec(MIIO_PSEC_GET_DOWN);//查询下发
	mi.getDownCount++;
}

//下发指令的处理
static void miioRecvHandle(void *psed)
{
	miio_data_t *analyzeData = pvPortMalloc(sizeof(miio_data_t));//解析后的数据
	MALLOC_CHECK(analyzeData);
	miioDataDeinit(analyzeData);//初始化缺省值
	analyzePsec2Data(analyzeData,psed);//解析数据
	if(analyzeData->cmd != MIIO_CMD_DOWN_NONE && analyzeData->cmd != MIIO_CMD_OTHRT)//如果有数据处理
	{
		if(analyzeData->cmd == MIIO_CMD_DOWN_MIIO_NET_CHANGE)//如果网络发生改变
			mi.net = (miio_net_state_t)analyzeData->modelParam[0];//记录网络变化
		if(analyzeData->cmd == MIIO_CMD_ERROR)//错误报告
			mi.errorCount ++;//增加错误报告的计数
		interfaceDataHanle(analyzeData);//使用接口函数处理数据(处理应用程序)
	}
	vPortFree(analyzeData);
}

//属性更改通知
miio_ret miioPropertyChanged(int siid,int piid,property_t *property)
{
	miio_ret ret;
	miio_data_t *miioData = pvPortMalloc(sizeof(miio_data_t));
	MIIO_MALLOC_CHECK(miioData);
	miioDataDeinit(miioData);//初始化结构体
	miioData->cmd = MIIO_CMD_PROPERTIES_CHANGED;//属性更改通知
	miioData->cloudParam[0].siid = siid;//复制服务ID
	miioData->cloudParam[0].piid = piid;//复制属性ID
	miioPropertyCopy(&miioData->cloudParam[0].property,property);//复制属性
	ret = miioSendWait(miioData,MIIO_PSEC_OK);//发送数据并等待完成
	vPortFree(miioData);
	return ret;
}

//事件上报
miio_ret miioEvents(int siid,int eiid,int piid,property_t *property)
{
	miio_ret ret;
	miio_data_t *miioData = pvPortMalloc(sizeof(miio_data_t));
	MIIO_MALLOC_CHECK(miioData);
	miioDataDeinit(miioData);//初始化结构体
	miioData->cmd = MIIO_CMD_EVENTS;//事件上报
	miioData->cloudParam[0].siid = siid;//复制服务ID
	miioData->cloudParam[0].eiid = siid;//复制事件ID
	miioData->cloudParam[0].piid = piid;//复制属性ID
	miioPropertyCopy(&miioData->cloudParam[0].property,property);//复制属性
	ret = miioSendWait(miioData,MIIO_PSEC_OK);//发送数据并等待完成
	vPortFree(miioData);
	return ret;
}

//设定模组名字
miio_ret miioSetModelName(char *name)
{
	miio_ret ret;
	miio_data_t *md = pvPortMalloc(sizeof(miio_data_t));
	MIIO_MALLOC_CHECK(md);
	miioDataDeinit(md);
	md->cmd = MIIO_CMD_MODEL;
	strcpy(md->modelParam,name);
	ret = miioSendWait(md,MIIO_PSEC_OK);//设定模组名称
	vPortFree(md);
	return ret;
}

//上报MCU版本信息
miio_ret miioUpMCUVersion(char *version)
{
	miio_ret ret;
	miio_data_t *md = pvPortMalloc(sizeof(miio_data_t));
	MIIO_MALLOC_CHECK(md);
	miioDataDeinit(md);
	md->cmd = MIIO_CMD_MCU_VERSION;
	strcpy(md->modelParam,version);
	ret = miioSendWait(md,MIIO_PSEC_OK);//上报MCU版本信息
	vPortFree(md);
	return ret;
}

//进入工厂测试
miio_ret miioModelFactory(void)
{
	miio_ret ret;
	miio_data_t *model_data;
	model_data = pvPortMalloc(sizeof(miio_data_t));
	MIIO_MALLOC_CHECK(model_data);
	miioDataDeinit(model_data);
	model_data->cmd = MIIO_CMD_FACTORY;
	ret = miioSendWait(model_data,MIIO_PSEC_OK);
	vPortFree(model_data);
	return ret;
}

//重置模组
miio_ret miioModelRestore(void)
{
	miio_ret ret;
	miio_data_t *model_data;
	model_data = pvPortMalloc(sizeof(miio_data_t));
	MIIO_MALLOC_CHECK(model_data);
	miioDataDeinit(model_data);
	model_data->cmd = MIIO_CMD_RESTORE;
	ret = miioSendWait(model_data,MIIO_PSEC_OK);
	vPortFree(model_data);
	return ret;
}

//重启模组
miio_ret miioModelReboot(void)
{
	miio_ret ret;
	miio_data_t *model_data;
	model_data = pvPortMalloc(sizeof(miio_data_t));
	MIIO_MALLOC_CHECK(model_data);
	miioDataDeinit(model_data);
	model_data->cmd = MIIO_CMD_REBOOT;
	ret = miioSendWait(model_data,MIIO_PSEC_OK);
	vPortFree(model_data);
	return ret;
}

//查询模组版本信息
char *miioGetVersion(void)
{
	if(mi.version == NULL)
	{
		char *psec = pvPortMalloc(MIIO_USART_MAX_BYTE);
		if(psec == NULL)return NULL;
		if(mi.semaphoreMutex == NULL)return mi.version;
		xSemaphoreTakeRecursive(mi.semaphoreMutex,portMAX_DELAY);//获取互斥锁
		miioSendPsec(MIIO_PSEC_VERSION);//查询固件版本信息
		if(xSemaphoreTake(mi.usartRecvCount,MIIO_WAIT_TIMEOUT) == pdPASS)//等待数据应答
		{
			memset(psec,0,MIIO_USART_MAX_BYTE);
			if(usartDataRead(psec,MIIO_USART_MAX_BYTE) == MIIO_RET_OK)
			{
				char *p = strstr(psec,"\r");//搜索\r
				if(p != NULL && psec[0] >= '0' && psec[0] <= '9')
				{
					*p = '\0';//删除\r
					mi.version = pvPortMalloc(strlen(psec)+1);//分配内存
					if(mi.version != NULL)
					{
						memset(mi.version,0,strlen(psec)+1);//清除内存
						strcpy(mi.version,psec);//保存模组版本信息
					}
				}
			}
		}
		xSemaphoreGiveRecursive(mi.semaphoreMutex);
		vPortFree(psec);
	}
	return mi.version;
}

//查询模组时间
//2020-02-21 12:12:12
char *miioGetTime(void)
{
	char *psec = pvPortMalloc(MIIO_USART_MAX_BYTE);
	if(psec == NULL)return mi.time;
	if(mi.semaphoreMutex == NULL)return mi.time;
	xSemaphoreTakeRecursive(mi.semaphoreMutex,portMAX_DELAY);//获取互斥锁
	miioSendPsec(MIIO_PSEC_TIME);//查询模组时间
	if(xSemaphoreTake(mi.usartRecvCount,MIIO_WAIT_TIMEOUT) == pdPASS)//等待数据应答
	{
		memset(psec,0,MIIO_USART_MAX_BYTE);
		if(usartDataRead(psec,MIIO_USART_MAX_BYTE) == MIIO_RET_OK)
		{
			char *p = strstr(psec,"\r");
			if(p != NULL && psec[0] >= '0' && psec[0] <= '9')
			{
				*p = '\0';//删除\r
				memset(mi.time,0,sizeof(mi.time));//清除数据
				memcpy(mi.time,psec,MIN_VALUE(strlen(psec),sizeof(mi.time)));//保存时间
			}
		}
	}
	xSemaphoreGiveRecursive(mi.semaphoreMutex);
	vPortFree(psec);
	return mi.time;
}

//查询模组MAC地址
char *miioGetMac(void)
{
	if(mi.mac[0] == '\0')
	{
		char *psec = pvPortMalloc(MIIO_USART_MAX_BYTE);
		if(psec == NULL)return mi.mac;
		if(mi.semaphoreMutex == NULL)return mi.mac;
		xSemaphoreTakeRecursive(mi.semaphoreMutex,portMAX_DELAY);//获取互斥锁
		miioSendPsec(MIIO_PSEC_MAC);//查询模组时间
		if(xSemaphoreTake(mi.usartRecvCount,MIIO_WAIT_TIMEOUT) == pdPASS)//等待数据应答
		{
			memset(psec,0,MIIO_USART_MAX_BYTE);
			if(usartDataRead(psec,MIIO_USART_MAX_BYTE) == MIIO_RET_OK)
			{
				char *p = strstr(psec,"\r");//搜索\r
				if(p != NULL)
				{
					*p = '\0';//删除\r
					if(strlen(psec) == 12)
						memcpy(mi.mac,psec,MIN_VALUE(strlen(psec),sizeof(mi.mac)));//保存MAC
				}
			}
		}
		vPortFree(psec);
		xSemaphoreGiveRecursive(mi.semaphoreMutex);
	}
	return mi.mac;
}

//发送并等待应答
miio_ret miioSendWait(miio_data_t *miioData,char *rsp)
{
	miio_ret ret;
	char *psec = pvPortMalloc(MIIO_USART_MAX_BYTE);
	MIIO_MALLOC_CHECK(psec);
	memset(psec,0,MIIO_USART_MAX_BYTE);//清除缓冲
	analyzeData2Psec(psec,miioData);//解析数据
	xSemaphoreTakeRecursive(mi.semaphoreMutex,portMAX_DELAY);//获取互斥锁
	usartDataSend(psec);//发送数据到模组
	if(miioData->cmd != MIIO_CMD_GET_DOWN)
		debug("[MIIO] send: %s\n",psec);
	if(xSemaphoreTake(mi.usartRecvCount,MIIO_WAIT_TIMEOUT) == pdPASS)//等待数据应答
	{
		memset(psec,0,MIIO_USART_MAX_BYTE);
		if(usartDataRead(psec,MIIO_USART_MAX_BYTE) == MIIO_RET_OK)
		{
			if(speccmp(psec,rsp))
				ret = MIIO_RET_OK;
			else
				ret = MIIO_RET_ERR;
		}
	}
	else
		ret = MIIO_RET_TIMEOUT;
	vPortFree(psec);//释放内存
	xSemaphoreGiveRecursive(mi.semaphoreMutex);//释放互斥锁
	return ret;
}

void miioSend(miio_data_t *miioData)
{
	char *psec = pvPortMalloc(MIIO_USART_MAX_BYTE);
	MALLOC_CHECK(psec);
	memset(psec,0,MIIO_USART_MAX_BYTE);//清除缓冲
	analyzeData2Psec(psec,miioData);//解析数据
	xSemaphoreTakeRecursive(mi.semaphoreMutex,portMAX_DELAY);//获取互斥锁
	usartDataSend(psec);//向模块发送psec指令
	xSemaphoreGiveRecursive(mi.semaphoreMutex);//释放互斥锁
	if(miioData->cmd != MIIO_CMD_GET_DOWN)
		debug("[MIIO] send: %s\n",psec);
	vPortFree(psec);
}

static void miioSendPsec(char *psec)
{
	u16 len = strlen(psec);
	char *p = pvPortMalloc(len+2);
	MALLOC_CHECK(p);
	vTaskSuspendAll();//不允许调度
	memcpy(p,psec,len);//复制指令
	p[len] = '\r';
	p[len+1] = '\0';
	usartDataSend(p);
	xTaskResumeAll();//允许调度
	vPortFree(p);
}

//初始化miio_data_t缺省值
void miioDataDeinit(miio_data_t *miioData)
{
	miioData->cmd = MIIO_CMD_OTHRT;
	for(int i=0;i<MIIO_PSEC_MAX_PARAM;i++)
	{
		miioData->cloudParam[i].siid = 0;
		miioData->cloudParam[i].piid = 0;
		miioData->cloudParam[i].aiid = 0;
		miioData->cloudParam[i].eiid = 0;
		miioData->cloudParam[i].code = MIIO_CODE_DONE;
		miioPropertyDeinit(&miioData->cloudParam[i].property);
	}
	memset(miioData->modelParam,0,sizeof(miioData->modelParam));
}

void miioIidCopy(psec_param_t *param1,psec_param_t *param2)
{
	param1->siid = param2->siid;
	param1->aiid = param2->aiid;
	param1->eiid = param2->eiid;
	param1->piid = param2->piid;
}

void miioPropertyDeinit(property_t *property)
{
	property->format = PROPERTY_FORMAT_UNDEFINED;
	property->data.boolean.value = RESET;
	property->data.number.type = DATA_NUMBER_INTEGER;
	property->data.number.value.integerValue = 0;
	property->data.number.value.floatValue = 0.0;
	property->data.string.length = 0;
	memset(property->data.string.value,0,MIIO_PSEC_VALUE_MAX_LEN);
}

void miioPropertyCopy(property_t *proerty1,property_t *proerty2)
{
	if(proerty1 == NULL)return;
	if(proerty2 != NULL)
	{
		proerty1->format = proerty2->format;
		proerty1->data.boolean.value = proerty2->data.boolean.value;
		proerty1->data.number.type = proerty2->data.number.type;
		proerty1->data.number.value.floatValue = proerty2->data.number.value.floatValue;
		proerty1->data.number.value.integerValue = proerty2->data.number.value.integerValue;
		proerty1->data.string.length = proerty2->data.string.length;
		memcpy(proerty1->data.string.value,proerty2->data.string.value,sizeof(proerty2->data.string.value));
	}
	else
		miioPropertyDeinit(proerty1);
}

//串口配置
static void usartConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	USART_InitStructure.USART_BaudRate = 115200;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//收发模式
	USART_Init(USART2, &USART_InitStructure);//初始化串口
	
	USART_ClearFlag(USART2,USART_FLAG_IDLE);//清除中断标志位
	USART_ClearFlag(USART2,USART_FLAG_RXNE);
	USART_ClearFlag(USART2,USART_FLAG_TC);
	USART_ITConfig(USART2,USART_IT_TC,DISABLE);//发送中断
	USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);//接收中断
	USART_ITConfig(USART2,USART_IT_IDLE,DISABLE);//空闲中断
	
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= configMAX_SYSCALL_INTERRUPT_PRIORITY;//抢占优先级能被RTOS屏蔽
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_Cmd(USART2, ENABLE);
}

static void usartDataSend(char *p)
{
	u16 len = strlen(p);
	for(u16 i=0;i<len;i++)
	{
		while(USART_GetFlagStatus(USART2,USART_FLAG_TXE) == RESET);
		USART_SendData(USART2,p[i]);
	}
}

static miio_ret usartDataRead(char *p,u16 pLen)
{
	u16 i;
	miio_ret ret;
	for(i=0;i<pLen;i++)
	{
		if(xQueueReceive(mi.usartFifoQueue,&p[i],0) == pdTRUE)//如果读取到数据
		{
			if(p[i] == '\r')//数据包是否以\r结尾
			{
				if(i+1 < pLen)
				{
					p[i+1] = '\0';
					ret = MIIO_RET_OK;
				}
				else
					ret = MIIO_RET_P_FULL;
				break;
			}
		}
		else //还未接收到结尾就没有数据
		{
			ret = MIIO_RET_P_FULL;
			break;
		}
	}
	if(i == pLen && p[i-1] != '\r')//如果参数缓冲大小不够
		ret = MIIO_RET_P_FULL;
	return ret;
}

//串口2中断服务程序
void USART2_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken;
	if(USART_GetITStatus(USART2,USART_IT_RXNE) != RESET) //接收中断
	{
		char data = USART2->DR;
		if(mi.usartFifoQueue != NULL && mi.usartRecvCount != NULL)
		{
			if(xQueueSendFromISR(mi.usartFifoQueue,&data,NULL) == pdTRUE)//将数据放入队列中
			{
				if(data == '\r')//如果数据完成
				{
					xSemaphoreGiveFromISR(mi.usartRecvCount,&xHigherPriorityTaskWoken);//释放信号量 计数接收一帧数据
				}
			}
			else//如果队列满
			{
				xSemaphoreGiveFromISR(mi.usartRecvCount,&xHigherPriorityTaskWoken);//释放信号量 计数接收一帧数据
			}
		}
	}
	USART_ClearITPendingBit(USART1,USART_IT_RXNE);//清除中断标志位
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);//强制切换任务（退出中断后立刻开始调度）
}

