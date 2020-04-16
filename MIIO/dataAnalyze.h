#ifndef _DATAANALYZE_H_
#define _DATAANALYZE_H_
#include "hal_sys.h"

#define MIIO_PSEC_MODEL								"model"									//ģ������ѯ/����
#define MIIO_PSEC_MCU_VERSION					"mcu_version"						//MCU����汾�ϱ�
#define MIIO_PSEC_ERROR								"error"									//����
#define MIIO_PSEC_GET_DOWN						"get_down"							//��ѯ�·�(MCU�������Ͳ�ѯ�������·�)
#define MIIO_PSEC_DOWN_NONE						"down none"							//δ�·�����
#define MIIO_PSEC_DOWN_GP							"down get_properties"		//�������·���ѯ����
#define MIIO_PSEC_DOWN_SP							"down set_properties"		//�������·��趨����
#define MIIO_PSEC_REPORT_PC						"properties_changed"		//�ϱ����������Է�������
#define MIIO_PSEC_NET_CHANGE					"down MIIO_net_change"	//ģ�����緢���ı�
#define MIIO_PSEC_DOWN_ACTION					"down action"						//ִ�ж���
#define MIIO_PSEC_EVENT								"event_occured"					//�¼��ϱ�
#define MIIO_PSEC_RESULT							"result"								//ִ��Ӧ��
#define MIIO_PSEC_CALL								"call"									//�ƶ˷�������
#define MIIO_PSEC_REBOOT							"reboot"								//����ģ��
#define MIIO_PSEC_RESTORE							"restore"								//����WiFi������Ϣ
#define MIIO_PSEC_FACTORY							"factory"								//���빤������
#define MIIO_PSEC_NET									"net"										//ѯ������״̬
#define MIIO_PSEC_TIME								"time"									//��ѯ��ǰ����ʱ��
#define MIIO_PSEC_MAC									"mac"										//��ѯģ��MAC��ַ
#define MIIO_PSEC_VERSION							"version"								//ģ��̼��汾
#define MIIO_PSEC_OK									"ok"										//���

#define MIIO_PSEC_MAX_PARAM						10			//����psec����
#define MIIO_PSEC_VALUE_MAX_LEN				128			//psecֵ����󳤶�

typedef enum{
	MIIO_CODE_DONE 	= 0,			//���
	MIIO_CODE_RNA 	= 1,			//���յ����󣬵�������û�����
	MIIO_CODE_PNR		= -4001,	//���Բ��ɶ�
	MIIO_CODE_PNW 	= -4002,	//���Բ���д
	MIIO_CODE_PMEN 	= -4003,	//���Է����¼�������
	MIIO_CODE_OTHER = -4004,	//�����ڲ�����
	MIIO_CODE_VALUE = -4005,	//����value����
	MIIO_CODE_MINE	= -4006,	//����in��������
	MIIO_CODE_DID 	= -4007,	//did����
}state_code_t;//״̬��

typedef enum{
	MIIO_NET_OFFLINE 	= 0,	//δ����
	MIIO_NET_LOCAL 		= 1,	//���ӵ�WiFi��δ���ӵ�������
	MIIO_NET_CLOUD		= 2,	//������С���Ʒ�����
	MIIO_NET_UPDATING = 3,	//�̼�������
	MIIO_NET_UAP			= 4,	//UAPģʽ�ȴ�����
	MIIO_NET_UNPROV		=	5,	//�ر�WiFi����Сʱδ������
}miio_net_state_t;

typedef enum{
	MIIO_CMD_DOWN_NONE 							= 0,	//û���·�
	MIIO_CMD_MODEL									= 1,	//ģ������
	MIIO_CMD_MCU_VERSION						= 2,	//MCU�汾
	MIIO_CMD_GET_DOWN								= 3,	//��ѯ�·�
	MIIO_CMD_DOWN_GET_PROPERTIES 		= 4,	//��ѯ����
	MIIO_CMD_DOWN_SET_PROPERTIES 		= 5,	//�趨����
	MIIO_CMD_DOWN_MIIO_NET_CHANGE		= 6,	//����ı�
	MIIO_CMD_DOWN_ACTION						=	7,	//ִ�ж���
	MIIO_CMD_PROPERTIES_CHANGED			=	8,	//���Ը���
	MIIO_CMD_EVENTS									= 9,	//�¼��ϱ�
	MIIO_CMD_RESULT									= 10,	//ִ��Ӧ��
	MIIO_CMD_CALL										= 11,	//�ƶ˷���
	MIIO_CMD_REBOOT									= 12,	//����ģ��
	MIIO_CMD_RESTORE								= 13,	//����ģ��
	MIIO_CMD_FACTORY								= 14,	//���빤��
	MIIO_CMD_NET										= 15,	//����״̬
	MIIO_CMD_TIME										=	16,	//��ѯʱ��
	MIIO_CMD_MAC										= 17,	//��ѯMAC
	MIIO_CMD_VERSION								=	18,	//ģ��汾
	MIIO_CMD_ERROR									=	19,	//�������
	MIIO_CMD_OK											=	20,	//��ɲ���
	MIIO_CMD_OTHRT									= 21, //����ָ��
}psec_cmd_t;

/*�����������Ͷ���*/
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
}property_t;//����

typedef struct{
	int 							siid;											//services iid
	int 							piid;											//properties iid
	int 							aiid;											//actions iid
	int 							eiid;											//events iid
	state_code_t 			code;											//state code
	property_t				property;									//property
}psec_param_t;

typedef struct{
	psec_cmd_t			cmd;//ָ��
	char						modelParam[128];	//����ģ��ָ��Ĳ���
	psec_param_t 		cloudParam[MIIO_PSEC_MAX_PARAM];//�����Ʋ��������ݾ�
}miio_data_t;

void analyzePsec2Data (miio_data_t *analyzeData,char *psec);
void analyzeData2Psec(char *psec,miio_data_t *analyzeData);

void analyzePsec2cmd(psec_cmd_t *cmd,char *psec);
void analyzeCmd2psec(char *psec,psec_cmd_t cmd);

int speccmp(char *uart,char *spec);
#endif

