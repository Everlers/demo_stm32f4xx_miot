#ifndef _DATAANALYZE_H_
#define _DATAANALYZE_H_
#include "hal_sys.h"

#define MIIO_PSEC_MODEL								"model"									//模组名查询/设置
#define MIIO_PSEC_MCU_VERSION					"mcu_version"						//MCU程序版本上报
#define MIIO_PSEC_ERROR								"error"									//错误
#define MIIO_PSEC_GET_DOWN						"get_down"							//查询下发(MCU主动发送查询服务器下发)
#define MIIO_PSEC_DOWN_NONE						"down none"							//未下发数据
#define MIIO_PSEC_DOWN_GP							"down get_properties"		//服务器下发查询属性
#define MIIO_PSEC_DOWN_SP							"down set_properties"		//服务器下发设定属性
#define MIIO_PSEC_REPORT_PC						"properties_changed"		//上报服务器属性发生更改
#define MIIO_PSEC_NET_CHANGE					"down MIIO_net_change"	//模组网络发生改变
#define MIIO_PSEC_DOWN_ACTION					"down action"						//执行动作
#define MIIO_PSEC_EVENT								"event_occured"					//事件上报
#define MIIO_PSEC_RESULT							"result"								//执行应答
#define MIIO_PSEC_CALL								"call"									//云端方法调用
#define MIIO_PSEC_REBOOT							"reboot"								//重启模块
#define MIIO_PSEC_RESTORE							"restore"								//重置WiFi配置信息
#define MIIO_PSEC_FACTORY							"factory"								//进入工厂测试
#define MIIO_PSEC_NET									"net"										//询问网络状态
#define MIIO_PSEC_TIME								"time"									//查询当前日期时间
#define MIIO_PSEC_MAC									"mac"										//查询模组MAC地址
#define MIIO_PSEC_VERSION							"version"								//模组固件版本
#define MIIO_PSEC_OK									"ok"										//完成

#define MIIO_PSEC_MAX_PARAM						10			//最大的psec参数
#define MIIO_PSEC_VALUE_MAX_LEN				128			//psec值的最大长度

typedef enum{
	MIIO_CODE_DONE 	= 0,			//完成
	MIIO_CODE_RNA 	= 1,			//接收到请求，但操作还没有完成
	MIIO_CODE_PNR		= -4001,	//属性不可读
	MIIO_CODE_PNW 	= -4002,	//属性不可写
	MIIO_CODE_PMEN 	= -4003,	//属性方法事件不存在
	MIIO_CODE_OTHER = -4004,	//其他内部问题
	MIIO_CODE_VALUE = -4005,	//属性value错误
	MIIO_CODE_MINE	= -4006,	//方法in参数错误
	MIIO_CODE_DID 	= -4007,	//did错误
}state_code_t;//状态码

typedef enum{
	MIIO_NET_OFFLINE 	= 0,	//未连接
	MIIO_NET_LOCAL 		= 1,	//连接到WiFi但未连接到服务器
	MIIO_NET_CLOUD		= 2,	//连接上小米云服务器
	MIIO_NET_UPDATING = 3,	//固件升级中
	MIIO_NET_UAP			= 4,	//UAP模式等待连接
	MIIO_NET_UNPROV		=	5,	//关闭WiFi（半小时未快连）
}miio_net_state_t;

typedef enum{
	MIIO_CMD_DOWN_NONE 							= 0,	//没有下发
	MIIO_CMD_MODEL									= 1,	//模组名称
	MIIO_CMD_MCU_VERSION						= 2,	//MCU版本
	MIIO_CMD_GET_DOWN								= 3,	//查询下发
	MIIO_CMD_DOWN_GET_PROPERTIES 		= 4,	//查询属性
	MIIO_CMD_DOWN_SET_PROPERTIES 		= 5,	//设定属性
	MIIO_CMD_DOWN_MIIO_NET_CHANGE		= 6,	//网络改变
	MIIO_CMD_DOWN_ACTION						=	7,	//执行动作
	MIIO_CMD_PROPERTIES_CHANGED			=	8,	//属性更改
	MIIO_CMD_EVENTS									= 9,	//事件上报
	MIIO_CMD_RESULT									= 10,	//执行应答
	MIIO_CMD_CALL										= 11,	//云端方法
	MIIO_CMD_REBOOT									= 12,	//重启模组
	MIIO_CMD_RESTORE								= 13,	//重置模组
	MIIO_CMD_FACTORY								= 14,	//进入工厂
	MIIO_CMD_NET										= 15,	//网络状态
	MIIO_CMD_TIME										=	16,	//查询时间
	MIIO_CMD_MAC										= 17,	//查询MAC
	MIIO_CMD_VERSION								=	18,	//模组版本
	MIIO_CMD_ERROR									=	19,	//错误操作
	MIIO_CMD_OK											=	20,	//完成操作
	MIIO_CMD_OTHRT									= 21, //其他指令
}psec_cmd_t;

/*属性数据类型定义*/
typedef struct _data_boolean
{
  FlagStatus value;
} data_boolean_t;

typedef struct
{
  char          value[MIIO_PSEC_VALUE_MAX_LEN + 1];
  uint32_t      length;
} data_string_t;

typedef enum
{
  DATA_NUMBER_INTEGER  = 0,
  DATA_NUMBER_FLOAT    = 1,
} data_number_type_t;

typedef union
{
  long    integerValue;
  float   floatValue;
} data_number_value_t;

typedef struct
{
	data_number_type_t      type;
  data_number_value_t     value;
} data_number_t;

typedef union
{
  data_boolean_t     boolean;
  data_string_t      string;
  data_number_t      number;
} property_data_t;

typedef enum _property_format
{
  PROPERTY_FORMAT_UNDEFINED      = 0,
  PROPERTY_FORMAT_BOOLEAN        = 1,
  PROPERTY_FORMAT_STRING         = 2,
  PROPERTY_FORMAT_NUMBER         = 3,
} property_format_t;

typedef struct{
	property_format_t format;						//property format
	property_data_t		data;							//property data
}property_t;//属性

typedef struct{
	int 							siid;											//services iid
	int 							piid;											//properties iid
	int 							aiid;											//actions iid
	int 							eiid;											//events iid
	state_code_t 			code;											//state code
	property_t				property;									//property
}psec_param_t;

typedef struct{
	psec_cmd_t			cmd;//指令
	char						modelParam[128];	//用于模块指令的参数
	psec_param_t 		cloudParam[MIIO_PSEC_MAX_PARAM];//用于云参数的数据据
}miio_data_t;

void analyzePsec2Data (miio_data_t *analyzeData,char *psec);
void analyzeData2Psec(char *psec,miio_data_t *analyzeData);

void analyzePsec2cmd(psec_cmd_t *cmd,char *psec);
void analyzeCmd2psec(char *psec,psec_cmd_t cmd);

int speccmp(char *uart,char *spec);
#endif

