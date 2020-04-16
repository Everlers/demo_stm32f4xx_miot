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
	usartConfig();//���ô���
	memset(mi.mac,0,sizeof(mi.mac));
	memset(mi.time,0,sizeof(mi.time));
	mi.version = NULL;
	mi.net = MIIO_NET_OFFLINE;
	mi.getDownCount = 0;
	mi.usartFifoQueue = xQueueCreate(MIIO_USART_MAX_BYTE*MIIO_USART_MAX_PACK,sizeof(u8));//�������ն���FIFO
	mi.usartRecvCount = xSemaphoreCreateCounting(MIIO_USART_MAX_PACK,0);//�����������ź���
	mi.semaphoreMutex = xSemaphoreCreateRecursiveMutex();//�����ݹ黥����(�ݹ黥���ź���)
	
	xSemaphoreTakeRecursive(mi.semaphoreMutex,portMAX_DELAY);//��ȡ������
	while(miioModelReboot() != MIIO_RET_OK);//��λģ��
	while(miioUpMCUVersion(MIIO_MCU_VERSION) != MIIO_RET_OK);//�ϱ�MCU�汾��Ϣ
	while(miioSetModelName(MIIO_MODEL_NAME) != MIIO_RET_OK);//�趨ģ������	
	miioGetVersion();//��ѯģ��汾��Ϣ
	miioGetMac();//��ѯģʽMAC��ַ
	miioGetTime();//��ѯģ��ʱ��
	xSemaphoreGiveRecursive(mi.semaphoreMutex);//�ͷŻ�����
}

//ģ�����ݴ���
void miioTask(void)
{
	if(mi.usartRecvCount != NULL && mi.usartRecvCount != NULL)
	{
		xSemaphoreTakeRecursive(mi.semaphoreMutex,portMAX_DELAY);//��ȡ������
		miioGetModel();//��ѯģ������
		if(xSemaphoreTake(mi.usartRecvCount,MIIO_WAIT_TIMEOUT) == pdPASS)//������յ�����
		{
			char *psec = pvPortMalloc(MIIO_USART_MAX_BYTE);
			if(psec != NULL)
			{
				usartDataRead(psec,MIIO_USART_MAX_BYTE);//��ȡ����
				miioRecvHandle(psec);//������յ�������
				vPortFree(psec);
			}
		}
		xSemaphoreGiveRecursive(mi.semaphoreMutex);//�ͷŻ�����
	}
	vTaskDelay(MIIO_GET_DOWN_TIME);//�ͷ�����
}

//ģ�����ݲ�ѯ
void miioGetModel(void)
{
	miioSendPsec(MIIO_PSEC_GET_DOWN);//��ѯ�·�
	mi.getDownCount++;
}

//�·�ָ��Ĵ���
static void miioRecvHandle(void *psed)
{
	miio_data_t *analyzeData = pvPortMalloc(sizeof(miio_data_t));//�����������
	MALLOC_CHECK(analyzeData);
	miioDataDeinit(analyzeData);//��ʼ��ȱʡֵ
	analyzePsec2Data(analyzeData,psed);//��������
	if(analyzeData->cmd != MIIO_CMD_DOWN_NONE && analyzeData->cmd != MIIO_CMD_OTHRT)//��������ݴ���
	{
		if(analyzeData->cmd == MIIO_CMD_DOWN_MIIO_NET_CHANGE)//������緢���ı�
			mi.net = (miio_net_state_t)analyzeData->modelParam[0];//��¼����仯
		if(analyzeData->cmd == MIIO_CMD_ERROR)//���󱨸�
			mi.errorCount ++;//���Ӵ��󱨸�ļ���
		interfaceDataHanle(analyzeData);//ʹ�ýӿں�����������(����Ӧ�ó���)
	}
	vPortFree(analyzeData);
}

//���Ը���֪ͨ
miio_ret miioPropertyChanged(int siid,int piid,property_t *property)
{
	miio_ret ret;
	miio_data_t *miioData = pvPortMalloc(sizeof(miio_data_t));
	MIIO_MALLOC_CHECK(miioData);
	miioDataDeinit(miioData);//��ʼ���ṹ��
	miioData->cmd = MIIO_CMD_PROPERTIES_CHANGED;//���Ը���֪ͨ
	miioData->cloudParam[0].siid = siid;//���Ʒ���ID
	miioData->cloudParam[0].piid = piid;//��������ID
	miioPropertyCopy(&miioData->cloudParam[0].property,property);//��������
	ret = miioSendWait(miioData,MIIO_PSEC_OK);//�������ݲ��ȴ����
	vPortFree(miioData);
	return ret;
}

//�¼��ϱ�
miio_ret miioEvents(int siid,int eiid,int piid,property_t *property)
{
	miio_ret ret;
	miio_data_t *miioData = pvPortMalloc(sizeof(miio_data_t));
	MIIO_MALLOC_CHECK(miioData);
	miioDataDeinit(miioData);//��ʼ���ṹ��
	miioData->cmd = MIIO_CMD_EVENTS;//�¼��ϱ�
	miioData->cloudParam[0].siid = siid;//���Ʒ���ID
	miioData->cloudParam[0].eiid = siid;//�����¼�ID
	miioData->cloudParam[0].piid = piid;//��������ID
	miioPropertyCopy(&miioData->cloudParam[0].property,property);//��������
	ret = miioSendWait(miioData,MIIO_PSEC_OK);//�������ݲ��ȴ����
	vPortFree(miioData);
	return ret;
}

//�趨ģ������
miio_ret miioSetModelName(char *name)
{
	miio_ret ret;
	miio_data_t *md = pvPortMalloc(sizeof(miio_data_t));
	MIIO_MALLOC_CHECK(md);
	miioDataDeinit(md);
	md->cmd = MIIO_CMD_MODEL;
	strcpy(md->modelParam,name);
	ret = miioSendWait(md,MIIO_PSEC_OK);//�趨ģ������
	vPortFree(md);
	return ret;
}

//�ϱ�MCU�汾��Ϣ
miio_ret miioUpMCUVersion(char *version)
{
	miio_ret ret;
	miio_data_t *md = pvPortMalloc(sizeof(miio_data_t));
	MIIO_MALLOC_CHECK(md);
	miioDataDeinit(md);
	md->cmd = MIIO_CMD_MCU_VERSION;
	strcpy(md->modelParam,version);
	ret = miioSendWait(md,MIIO_PSEC_OK);//�ϱ�MCU�汾��Ϣ
	vPortFree(md);
	return ret;
}

//���빤������
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

//����ģ��
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

//����ģ��
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

//��ѯģ��汾��Ϣ
char *miioGetVersion(void)
{
	if(mi.version == NULL)
	{
		char *psec = pvPortMalloc(MIIO_USART_MAX_BYTE);
		if(psec == NULL)return NULL;
		if(mi.semaphoreMutex == NULL)return mi.version;
		xSemaphoreTakeRecursive(mi.semaphoreMutex,portMAX_DELAY);//��ȡ������
		miioSendPsec(MIIO_PSEC_VERSION);//��ѯ�̼��汾��Ϣ
		if(xSemaphoreTake(mi.usartRecvCount,MIIO_WAIT_TIMEOUT) == pdPASS)//�ȴ�����Ӧ��
		{
			memset(psec,0,MIIO_USART_MAX_BYTE);
			if(usartDataRead(psec,MIIO_USART_MAX_BYTE) == MIIO_RET_OK)
			{
				char *p = strstr(psec,"\r");//����\r
				if(p != NULL && psec[0] >= '0' && psec[0] <= '9')
				{
					*p = '\0';//ɾ��\r
					mi.version = pvPortMalloc(strlen(psec)+1);//�����ڴ�
					if(mi.version != NULL)
					{
						memset(mi.version,0,strlen(psec)+1);//����ڴ�
						strcpy(mi.version,psec);//����ģ��汾��Ϣ
					}
				}
			}
		}
		xSemaphoreGiveRecursive(mi.semaphoreMutex);
		vPortFree(psec);
	}
	return mi.version;
}

//��ѯģ��ʱ��
//2020-02-21 12:12:12
char *miioGetTime(void)
{
	char *psec = pvPortMalloc(MIIO_USART_MAX_BYTE);
	if(psec == NULL)return mi.time;
	if(mi.semaphoreMutex == NULL)return mi.time;
	xSemaphoreTakeRecursive(mi.semaphoreMutex,portMAX_DELAY);//��ȡ������
	miioSendPsec(MIIO_PSEC_TIME);//��ѯģ��ʱ��
	if(xSemaphoreTake(mi.usartRecvCount,MIIO_WAIT_TIMEOUT) == pdPASS)//�ȴ�����Ӧ��
	{
		memset(psec,0,MIIO_USART_MAX_BYTE);
		if(usartDataRead(psec,MIIO_USART_MAX_BYTE) == MIIO_RET_OK)
		{
			char *p = strstr(psec,"\r");
			if(p != NULL && psec[0] >= '0' && psec[0] <= '9')
			{
				*p = '\0';//ɾ��\r
				memset(mi.time,0,sizeof(mi.time));//�������
				memcpy(mi.time,psec,MIN_VALUE(strlen(psec),sizeof(mi.time)));//����ʱ��
			}
		}
	}
	xSemaphoreGiveRecursive(mi.semaphoreMutex);
	vPortFree(psec);
	return mi.time;
}

//��ѯģ��MAC��ַ
char *miioGetMac(void)
{
	if(mi.mac[0] == '\0')
	{
		char *psec = pvPortMalloc(MIIO_USART_MAX_BYTE);
		if(psec == NULL)return mi.mac;
		if(mi.semaphoreMutex == NULL)return mi.mac;
		xSemaphoreTakeRecursive(mi.semaphoreMutex,portMAX_DELAY);//��ȡ������
		miioSendPsec(MIIO_PSEC_MAC);//��ѯģ��ʱ��
		if(xSemaphoreTake(mi.usartRecvCount,MIIO_WAIT_TIMEOUT) == pdPASS)//�ȴ�����Ӧ��
		{
			memset(psec,0,MIIO_USART_MAX_BYTE);
			if(usartDataRead(psec,MIIO_USART_MAX_BYTE) == MIIO_RET_OK)
			{
				char *p = strstr(psec,"\r");//����\r
				if(p != NULL)
				{
					*p = '\0';//ɾ��\r
					if(strlen(psec) == 12)
						memcpy(mi.mac,psec,MIN_VALUE(strlen(psec),sizeof(mi.mac)));//����MAC
				}
			}
		}
		vPortFree(psec);
		xSemaphoreGiveRecursive(mi.semaphoreMutex);
	}
	return mi.mac;
}

//���Ͳ��ȴ�Ӧ��
miio_ret miioSendWait(miio_data_t *miioData,char *rsp)
{
	miio_ret ret;
	char *psec = pvPortMalloc(MIIO_USART_MAX_BYTE);
	MIIO_MALLOC_CHECK(psec);
	memset(psec,0,MIIO_USART_MAX_BYTE);//�������
	analyzeData2Psec(psec,miioData);//��������
	xSemaphoreTakeRecursive(mi.semaphoreMutex,portMAX_DELAY);//��ȡ������
	usartDataSend(psec);//�������ݵ�ģ��
	if(miioData->cmd != MIIO_CMD_GET_DOWN)
		debug("[MIIO] send: %s\n",psec);
	if(xSemaphoreTake(mi.usartRecvCount,MIIO_WAIT_TIMEOUT) == pdPASS)//�ȴ�����Ӧ��
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
	vPortFree(psec);//�ͷ��ڴ�
	xSemaphoreGiveRecursive(mi.semaphoreMutex);//�ͷŻ�����
	return ret;
}

void miioSend(miio_data_t *miioData)
{
	char *psec = pvPortMalloc(MIIO_USART_MAX_BYTE);
	MALLOC_CHECK(psec);
	memset(psec,0,MIIO_USART_MAX_BYTE);//�������
	analyzeData2Psec(psec,miioData);//��������
	xSemaphoreTakeRecursive(mi.semaphoreMutex,portMAX_DELAY);//��ȡ������
	usartDataSend(psec);//��ģ�鷢��psecָ��
	xSemaphoreGiveRecursive(mi.semaphoreMutex);//�ͷŻ�����
	if(miioData->cmd != MIIO_CMD_GET_DOWN)
		debug("[MIIO] send: %s\n",psec);
	vPortFree(psec);
}

static void miioSendPsec(char *psec)
{
	u16 len = strlen(psec);
	char *p = pvPortMalloc(len+2);
	MALLOC_CHECK(p);
	vTaskSuspendAll();//���������
	memcpy(p,psec,len);//����ָ��
	p[len] = '\r';
	p[len+1] = '\0';
	usartDataSend(p);
	xTaskResumeAll();//�������
	vPortFree(p);
}

//��ʼ��miio_data_tȱʡֵ
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

//��������
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
	
	USART_InitStructure.USART_BaudRate = 115200;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//�շ�ģʽ
	USART_Init(USART2, &USART_InitStructure);//��ʼ������
	
	USART_ClearFlag(USART2,USART_FLAG_IDLE);//����жϱ�־λ
	USART_ClearFlag(USART2,USART_FLAG_RXNE);
	USART_ClearFlag(USART2,USART_FLAG_TC);
	USART_ITConfig(USART2,USART_IT_TC,DISABLE);//�����ж�
	USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);//�����ж�
	USART_ITConfig(USART2,USART_IT_IDLE,DISABLE);//�����ж�
	
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= configMAX_SYSCALL_INTERRUPT_PRIORITY;//��ռ���ȼ��ܱ�RTOS����
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
		if(xQueueReceive(mi.usartFifoQueue,&p[i],0) == pdTRUE)//�����ȡ������
		{
			if(p[i] == '\r')//���ݰ��Ƿ���\r��β
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
		else //��δ���յ���β��û������
		{
			ret = MIIO_RET_P_FULL;
			break;
		}
	}
	if(i == pLen && p[i-1] != '\r')//������������С����
		ret = MIIO_RET_P_FULL;
	return ret;
}

//����2�жϷ������
void USART2_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken;
	if(USART_GetITStatus(USART2,USART_IT_RXNE) != RESET) //�����ж�
	{
		char data = USART2->DR;
		if(mi.usartFifoQueue != NULL && mi.usartRecvCount != NULL)
		{
			if(xQueueSendFromISR(mi.usartFifoQueue,&data,NULL) == pdTRUE)//�����ݷ��������
			{
				if(data == '\r')//����������
				{
					xSemaphoreGiveFromISR(mi.usartRecvCount,&xHigherPriorityTaskWoken);//�ͷ��ź��� ��������һ֡����
				}
			}
			else//���������
			{
				xSemaphoreGiveFromISR(mi.usartRecvCount,&xHigherPriorityTaskWoken);//�ͷ��ź��� ��������һ֡����
			}
		}
	}
	USART_ClearITPendingBit(USART1,USART_IT_RXNE);//����жϱ�־λ
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);//ǿ���л������˳��жϺ����̿�ʼ���ȣ�
}

