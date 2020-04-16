#ifndef _MIIOT_H_
#define _MIIOT_H_
#include "hal_sys.h"
#include "iid.h"
#include "dataAnalyze.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

#define MIIO_MODEL_NAME										"evler.light.lx2020"//��Ʒģ������
#define MIIO_MCU_VERSION									"0001"//�̼��汾��

#define MIIO_USART_MAX_PACK								1			//�����ն��ٰ�����
#define MIIO_USART_MAX_BYTE								256		//ÿ�����ں��ֽ���
#define MIIO_WAIT_TIMEOUT									1000	//�ȴ���ʱʱ������
#define MIIO_GET_DOWN_TIME								100		//��ѯ�·���ʱ��
#define MIIO_MALLOC_CHECK(x)							if(x == NULL)return MIIO_RET_MEM_ERR;

typedef enum{
	MIIO_RET_OK = 0,				//���
	MIIO_RET_ERR = 1,				//����
	MIIO_RET_TIMEOUT = 2,		//��ʱ
	MIIO_RET_MEM_ERR = 3,		//�ڴ����
	MIIO_RET_DATA_ERR = 4,	//���ݴ���
	MIIO_RET_P_FULL	 = 5,		//����ָ������
}miio_ret;

typedef struct{
	miio_net_state_t	net;						//����״̬
	char 							*version;				//ģ��汾��Ϣ
	char 							time[24];				//ģ��ʱ��
	char 							mac[13];				//ģ��MAC��ַ
	QueueHandle_t			usartFifoQueue;	//�ж�FIFO����
	QueueHandle_t			usartRecvCount;	//���ڽ��ռ������ź���
	SemaphoreHandle_t	semaphoreMutex;	//�����ź���(������)
	u32								getDownCount;		//��ѯ�·��ļ���
	u32								errorCount;			//�������
}miio_t;

void miioInit(void);
void miioTask(void);
void miioSend(miio_data_t *data);
miio_ret miioSendWait(miio_data_t *miioData,char *rsp);

miio_ret miioEvents(int siid,int eiid,int piid,property_t *property);
miio_ret miioPropertyChanged(int siid,int piid,property_t *property);


miio_ret miioModelReboot(void);
miio_ret miioModelRestore(void);
miio_ret miioModelFactory(void);
miio_ret miioUpMCUVersion(char *version);
miio_ret miioSetModelName(char *name);
char *miioGetTime(void);
char *miioGetMac(void);
char *miioGetVersion(void);

void miioDataDeinit(miio_data_t *miioData);
void miioPropertyDeinit(property_t *property);
void miioIidCopy(psec_param_t *param1,psec_param_t *param2);
void miioPropertyCopy(property_t *proerty1,property_t *proerty2);

#endif
