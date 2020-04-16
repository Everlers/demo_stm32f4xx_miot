#include "rtosEx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "string.h"
#include "stdio.h"

//����״̬ �鿴�ڴ棺stringTaskList
#if configGENERATE_RUN_TIME_STATS == 1
static u32  tsakRunTimeCount;//ʱ�����
#endif

#if configUSE_TRACE_FACILITY == 1
#if RTOSEX_LCD_SHOW == 1
#include "lcd_tft.h"
#include "ili9341.h"
#endif
char stringTaskList[RTOSEX_MAX_TASK_LIST_NUM+RTOSEX_SHOW_OTHER_INFO_NUM+1][configMAX_TASK_NAME_LEN + RTOSEX_TASK_LIST_PARAM_LEN]={0};
#endif

#if configUSE_TRACE_FACILITY == 1

//��ʾ״̬
void rtosShowStatus(void)
{
	char cStatus;
	static UBaseType_t oldArraySize = 0;
	UBaseType_t arraySize;
	u32 ulTotalTime,ulStatsAsPercentage;
	UBaseType_t alreadyArray[RTOSEX_MAX_TASK_LIST_NUM];//�Ѿ���ʾ�����
	char *string = pvPortMalloc(configMAX_TASK_NAME_LEN+RTOSEX_TASK_LIST_PARAM_LEN+1);
	TaskStatus_t *taskStatus = pvPortMalloc(sizeof(TaskStatus_t) * RTOSEX_MAX_TASK_LIST_NUM);
	if(string == NULL || taskStatus == NULL)return;
	
	arraySize = uxTaskGetSystemState(taskStatus,RTOSEX_MAX_TASK_LIST_NUM,&ulTotalTime);//��ȡ����״̬
	ulTotalTime /= 100UL;//����ٷֱ�ʹ����
	memset(alreadyArray,0,RTOSEX_MAX_TASK_LIST_NUM);//������

	for(UBaseType_t i=0;i<arraySize;i++)
	{
		UBaseType_t minTaskNumber,index;
		minTaskNumber = 0xFFFF;
		for(UBaseType_t x=0;x<arraySize;x++)//Ѱ��δ��ʾ����С��������Ÿ���������
		{
			UBaseType_t y;
			for(y=0;y<i;y++)//����Ƿ��Ѿ���ʾ������
				if(x == alreadyArray[y])
					break;
			if(y == i)//���δ��ʾ������
				minTaskNumber = MIN_VALUE(minTaskNumber,taskStatus[x].xTaskNumber);//ȡ��Сֵ
		}
		for(UBaseType_t x=0;x<arraySize;x++)//Ѱ����С������ŵĽṹ��
		{
			if(taskStatus[x].xTaskNumber == minTaskNumber)
			{
				alreadyArray[i] = x;
				index = x;
				break;
			}
		}
		
		switch( taskStatus[ index ].eCurrentState )//����״̬����ʾת��
		{
			case eRunning:cStatus = tskRUNNING_CHAR;break;
			case eReady:cStatus = tskREADY_CHAR;break;
			case eBlocked:cStatus = tskBLOCKED_CHAR;break;
			case eSuspended:cStatus = tskSUSPENDED_CHAR;break;
			case eDeleted:cStatus = tskDELETED_CHAR;break;
			case eInvalid:		/* Fall through. */
			default:			/* Should not get here, but it is included
								to prevent static checking errors. */
				cStatus = ( char ) 0x00;
			break;
		}
		ulStatsAsPercentage = taskStatus[ index ].ulRunTimeCounter / ulTotalTime;//��������ʹ����
		memset(string,0,configMAX_TASK_NAME_LEN+RTOSEX_TASK_LIST_PARAM_LEN+1);
		vTaskSuspendAll();//�����������񲻿�����
		snprintf(string,configMAX_TASK_NAME_LEN+RTOSEX_TASK_LIST_PARAM_LEN+1, "%c  %u %4u %2u %10u %3u%%"
		,cStatus
		, ( unsigned int ) taskStatus[ index ].uxCurrentPriority
		, ( unsigned int ) taskStatus[ index ].usStackHighWaterMark
		, ( unsigned int ) taskStatus[ index].xTaskNumber
		, ( unsigned int ) taskStatus[ index ].ulRunTimeCounter
		, ( unsigned int ) ulStatsAsPercentage);
		{//�������б���ʾ���ڴ�
			if(oldArraySize != arraySize)//����������������ı�
				memset(stringTaskList,' ',sizeof(stringTaskList));//�����ʾ�ڴ����������
			memset(stringTaskList[0],' ',configMAX_TASK_NAME_LEN/2-2);//���nameǰ����ڴ�
			memcpy(stringTaskList[0]+(configMAX_TASK_NAME_LEN/2)-2,"NAME",4);//д��name�ַ�
			memset(stringTaskList[0]+(configMAX_TASK_NAME_LEN/2)+2,' ',configMAX_TASK_NAME_LEN - (configMAX_TASK_NAME_LEN/2)+2);//���name������ڴ�
			memcpy(stringTaskList[0]+configMAX_TASK_NAME_LEN-1,"STA PI RS  NB     RT      AP ",RTOSEX_TASK_LIST_PARAM_LEN);
			memcpy(stringTaskList[i+1],taskStatus[index].pcTaskName,strlen(taskStatus[index].pcTaskName));//����������
			memset(stringTaskList[i+1]+strlen(taskStatus[index].pcTaskName),' ',configMAX_TASK_NAME_LEN - strlen(taskStatus[index].pcTaskName));//���������油�ո�
			memcpy(&stringTaskList[i+1][configMAX_TASK_NAME_LEN],string,strlen(string));//����������Ϣ
		}
		xTaskResumeAll();//������������
		#if RTOSEX_LCD_SHOW == 1
		{//���ڴ�������б���ʾ��LCD
			if(oldArraySize != arraySize)//����������������ı�
				lcd_clear(0);
			memcpy(string,stringTaskList[0],sizeof(stringTaskList[0]));
			string[sizeof(stringTaskList[0])] = 0;
			lcd_show_string(0,0,string,WHITE,FILL_MODE);
			memcpy(string,stringTaskList[i+1],sizeof(stringTaskList[i+1]));
			string[sizeof(stringTaskList[i+1])] = 0;
			lcd_show_string(0,(i+1)*16,string,WHITE,FILL_MODE);
		}
		#endif
		oldArraySize = arraySize;//ˢ����������
	}
	//��ʾ������Ϣ
	memcpy(stringTaskList[arraySize+1],string,sprintf(string,"Other:"));
	memcpy(stringTaskList[arraySize+2],string,sprintf(string,"FreeHeapSize: %5u",( unsigned int )xPortGetFreeHeapSize()));//��ʾ��ǰʣ��ջ
	memcpy(stringTaskList[arraySize+3],string,sprintf(string,"MinimumEverFreeHeapSize: %5u",( unsigned int )xPortGetMinimumEverFreeHeapSize()));//��ʾʣ��ջ��ʷ��Сֵ
	#if RTOSEX_LCD_SHOW == 1
	sprintf(string,"Other:");
	lcd_show_string(0,(arraySize+1)*16+8,string,WHITE,FILL_MODE);
	sprintf(string,"FreeHeapSize: %5u",( unsigned int )xPortGetFreeHeapSize());
	lcd_show_string(0,(arraySize+2)*16+8,string,WHITE,FILL_MODE);
	sprintf(string,"MinimumEverFreeHeapSize: %5u",( unsigned int )xPortGetMinimumEverFreeHeapSize());
	lcd_show_string(0,(arraySize+3)*16+8,string,WHITE,FILL_MODE);
	#endif
	vPortFree(taskStatus);
	vPortFree(string);
}

#endif

#if configGENERATE_RUN_TIME_STATS == 1

//����ʹ��@vTaskGetRunTimeStats ���� ����һ���߾��ȶ�ʱ��/�������ṩʱ��
void rtosExConfigureRunTime (void)//50us����
{
	TIM_TimeBaseInitTypeDef TIMInitstruct;
	NVIC_InitTypeDef NVICInitStruct;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	
	TIMInitstruct.TIM_Prescaler = 72;//��Ƶ
	TIMInitstruct.TIM_CounterMode = TIM_CounterMode_Up;//���ϼ���
	TIMInitstruct.TIM_Period = 50;//����ֵ
	TIMInitstruct.TIM_ClockDivision = TIM_CKD_DIV1;
	
	TIM_TimeBaseInit(TIM2,&TIMInitstruct);//��ʼ����ʱ��
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);//ʹ�ܶ�ʱ�������ж�
	TIM_Cmd(TIM2,ENABLE);//ʹ�ܶ�ʱ��
	
	NVICInitStruct.NVIC_IRQChannel = TIM2_IRQn;
	NVICInitStruct.NVIC_IRQChannelPreemptionPriority = 0x02;//��ռ���ȼ�
	NVICInitStruct.NVIC_IRQChannelSubPriority = 0x02;//�ж����ȼ�
	NVICInitStruct.NVIC_IRQChannelCmd = ENABLE;//ʹ���ж�
	NVIC_Init(&NVICInitStruct);//��ʼ���ж�������
	tsakRunTimeCount = 0;
}

//����ʹ��@vTaskGetRunTimeStats ���� ��ȡʱ�ڵ�ʱ��ֵ
u32 rtosExGetRunTimeCounterValue (void)
{
	return tsakRunTimeCount;
}

void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2,TIM_IT_Update)==SET) //����ж�
		tsakRunTimeCount++;
	TIM_ClearITPendingBit(TIM2,TIM_IT_Update);  //����жϱ�־λ
}

#endif

//�ڴ����ʧ�ܻص�
void vApplicationMallocFailedHook (void)
{
	RTOSEX_DEBUG("Malloc failed !!! Lack of stack memory!\r\n");
}

//�����ջ����ص�
void vApplicationStackOverflowHook( TaskHandle_t xTask,signed char *pcTaskName )
{
	RTOSEX_DEBUG("Task stack overflow!!!\r\n");
}

