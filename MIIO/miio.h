#ifndef _MIIOT_H_
#define _MIIOT_H_
#include "hal_sys.h"
#include "iid.h"
#include "dataAnalyze.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

#define MIIO_MODEL_NAME										"evler.light.lx2020"//产品模组名称
#define MIIO_MCU_VERSION									"0001"//固件版本号

#define MIIO_USART_MAX_PACK								1			//最大接收多少包数据
#define MIIO_USART_MAX_BYTE								256		//每包最大节后字节数
#define MIIO_WAIT_TIMEOUT									1000	//等待超时时间设置
#define MIIO_GET_DOWN_TIME								100		//查询下发的时间
#define MIIO_MALLOC_CHECK(x)							if(x == NULL)return MIIO_RET_MEM_ERR;

typedef enum{
	MIIO_RET_OK = 0,				//完成
	MIIO_RET_ERR = 1,				//错误
	MIIO_RET_TIMEOUT = 2,		//超时
	MIIO_RET_MEM_ERR = 3,		//内存错误
	MIIO_RET_DATA_ERR = 4,	//数据错误
	MIIO_RET_P_FULL	 = 5,		//参数指针已满
}miio_ret;

typedef struct{
	miio_net_state_t	net;						//网络状态
	char 							*version;				//模组版本信息
	char 							time[24];				//模组时间
	char 							mac[13];				//模组MAC地址
	QueueHandle_t			usartFifoQueue;	//中断FIFO队列
	QueueHandle_t			usartRecvCount;	//串口接收计数型信号量
	SemaphoreHandle_t	semaphoreMutex;	//互斥信号量(互斥锁)
	u32								getDownCount;		//查询下发的计数
	u32								errorCount;			//错误计数
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
