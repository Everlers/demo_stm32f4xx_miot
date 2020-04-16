#include "dataAnalyze.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "FreeRTOS.h"
#include "task.h"

//����ֵת��
static void analyzeProprttyValue_Psec2Data(property_t *property,char *psecValue);
static void analyzeProprttyValue_Data2Psec(char *psecValue,property_t *property);
//����ת��
static void analyzePsec2SetProperties(miio_data_t *analyzeData,char *psec);
static void analyzePsec2GetProperties(miio_data_t *analyzeData,char *psec);
static void analyzeResult2Psec(char *psec,miio_data_t *analyzeData);
static void analyzeProprttyChanged(char *psec,miio_data_t *analyzeData);
//����ת��
static void analyzeAction(miio_data_t *analyzeData,char *psec);
//�¼�ת��
static void analyzeEvents(char *psec,miio_data_t *analyzeData);
//����״̬ת��
static void analyzeNetState(miio_data_t *analyzeData,char *psec);


//PSEC ת ����
void analyzePsec2Data (miio_data_t *analyzeData,char *psec)
{
	analyzePsec2cmd(&analyzeData->cmd,(char *)psec);//psec�ַ�תö������
	if(analyzeData->cmd != MIIO_CMD_DOWN_NONE)//��������ݴ���
		debug("[MIIO] recv: %s\n",psec);
	switch((u8)analyzeData->cmd)//�������ݲ���
	{
		case MIIO_CMD_DOWN_GET_PROPERTIES://��ѯ����
			analyzePsec2GetProperties(analyzeData,psec);
		break;

		case MIIO_CMD_DOWN_SET_PROPERTIES://��������
			analyzePsec2SetProperties(analyzeData,psec);
		break;

		case MIIO_CMD_DOWN_ACTION://ִ�ж���
			analyzeAction(analyzeData,psec);
		break;

		case MIIO_CMD_DOWN_MIIO_NET_CHANGE://���緢���ı�
			analyzeNetState(analyzeData,psec);
		break;

		case MIIO_CMD_OTHRT://�����·�����
			memcpy(analyzeData->modelParam,psec,strlen(psec));//��������
		break;

		case MIIO_CMD_ERROR://�������
			debug("[MIIO] error\r\n");
		break;
		
		case MIIO_CMD_DOWN_NONE://û���·�
			
		break;
		
		default://δ֪���·�(δ֪�Ĵ�����Ϣ)
			
		break;
	}
}

//���� ת PSEC
void analyzeData2Psec(char *psec,miio_data_t *analyzeData)
{
	analyzeCmd2psec(psec,analyzeData->cmd);//PSECָ��ת��
	switch((u8)analyzeData->cmd)//�������ݲ���
	{	
		case MIIO_CMD_RESULT://ִ��Ӧ��
			analyzeResult2Psec(psec,analyzeData);
		break;

		case MIIO_CMD_PROPERTIES_CHANGED://���Ը���
			analyzeProprttyChanged(psec,analyzeData);
		break;
		
		case MIIO_CMD_EVENTS://�¼��ϱ�
			analyzeEvents(psec,analyzeData);
		break;
		
		case MIIO_CMD_MODEL://ģ������
		case MIIO_CMD_MCU_VERSION://MCU�汾�ϱ�
			if(analyzeData->modelParam[0] != '\0')
			{
				strcat(psec," ");//�ո���
				strcat(psec,analyzeData->modelParam);//ƴ�Ӳ���
			}
		break;
		
		default://δ֪(δ֪�Ĵ�����Ϣ)
			
		break;
	}
	strcat(psec,"\r");//���������
}

//��������״̬
static void analyzeNetState(miio_data_t *analyzeData,char *psec)
{
	if(speccmp(psec,"down MIIO_net_change offline"))
		analyzeData->modelParam[0] = MIIO_NET_OFFLINE;
	else if(speccmp(psec,"down MIIO_net_change local"))
		analyzeData->modelParam[0] = MIIO_NET_LOCAL;
	else if(speccmp(psec,"down MIIO_net_change cloud"))
		analyzeData->modelParam[0] = MIIO_NET_CLOUD;
	else if(speccmp(psec,"down MIIO_net_change updating"))
		analyzeData->modelParam[0] = MIIO_NET_UPDATING;
	else if(speccmp(psec,"down MIIO_net_change uap"))
		analyzeData->modelParam[0] = MIIO_NET_UAP;
	else if(speccmp(psec,"down MIIO_net_change unprov"))
		analyzeData->modelParam[0] = MIIO_NET_UNPROV;
}

//��������ָ�� psec ת data
//action <siid> <aiid> <piid> <value> ... <piid> <value>
static void analyzeAction(miio_data_t *analyzeData,char *psec)
{
	if(analyzeData->cmd != MIIO_CMD_DOWN_ACTION)return;
	strtok(psec," ");//�ָ��ַ���
	for(int i=0;i<MIIO_PSEC_MAX_PARAM;i++)//��ȡ����
	{
		char *p = strtok(NULL," ");
		if(p[0] >= '0' && p[0] <= '9')//���������
		{
			sscanf(p,"%d",&analyzeData->cloudParam[i].siid);//ȡ��<siid>
			p = strtok(NULL," ");//Ѱ��<aiid>
			sscanf(p,"%d",&analyzeData->cloudParam[i].aiid);//ȡ��<aiid>
			p = strtok(NULL," ");//Ѱ��<piid>
			if(p != NULL)//�����<piid> ע�⣺��ЩACTION��û��ֵ�� û��ֵ��û������ID
			{
				sscanf(p,"%d",&analyzeData->cloudParam[i].piid);//ȡ��<piid>
				p = strtok(NULL," ");//Ѱ��<value>
				analyzeProprttyValue_Psec2Data(&analyzeData->cloudParam[i].property,p);//����value
			}
		}
		else if(p == NULL)//����Ѿ��������iidѰ��
			break;
		else//�����������(ָ���ַ�)
			i--;
	}
}

//�¼��ϱ��ĸ�ʽת��
//event_occured <siid> <eiid> <piid> <value> ... <piid> <value>
static void analyzeEvents(char *psec,miio_data_t *analyzeData)
{
	if(analyzeData->cmd != MIIO_CMD_EVENTS)return;
	for(int i=0;i<MIIO_PSEC_MAX_PARAM;i++)//��ȡ���з�������
	{
		if(analyzeData->cloudParam[i].siid != 0)//����з���ID
		{
			char *str = pvPortMalloc(MIIO_PSEC_VALUE_MAX_LEN);
			MALLOC_CHECK(str);
			memset(str,0,MIIO_PSEC_VALUE_MAX_LEN);//�������
			sprintf(str," %d",analyzeData->cloudParam[i].siid);//��ӷ���ID
			strcat(psec,str);//siid�ַ�ƴ��
			sprintf(str," %d",analyzeData->cloudParam[i].eiid);//����¼�ID
			strcat(psec,str);//eiid�ַ�ƴ��
			sprintf(str," %d",analyzeData->cloudParam[i].piid);//�������ID
			strcat(psec,str);//piid�ַ�ƴ��
			if(analyzeData->cloudParam[i].code == MIIO_CODE_DONE && analyzeData->cloudParam[i].property.format != PROPERTY_FORMAT_UNDEFINED)
      {
				analyzeProprttyValue_Data2Psec(str,&analyzeData->cloudParam[i].property);//����value����
        strcat(psec," ");//�������' '
        strcat(psec,str);//value�ַ�ƴ��
      }
			vPortFree(str);
		}
	}
}

//���Ը����ϱ��ĸ�ʽת��
//properties_changed <siid> <piid> <value> ... <siid> <piid> <value>
static void analyzeProprttyChanged(char *psec,miio_data_t *analyzeData)
{
	if(analyzeData->cmd != MIIO_CMD_PROPERTIES_CHANGED)return;
	for(int i=0;i<MIIO_PSEC_MAX_PARAM;i++)//��ȡ���з�������
	{
		if(analyzeData->cloudParam[i].siid != 0)//����з���ID
		{
			char *str = pvPortMalloc(MIIO_PSEC_VALUE_MAX_LEN);
			MALLOC_CHECK(str);
			memset(str,0,MIIO_PSEC_VALUE_MAX_LEN);//�������
			sprintf(str," %d",analyzeData->cloudParam[i].siid);//��ӷ���ID
			strcat(psec,str);//siid�ַ�ƴ��
			sprintf(str," %d",analyzeData->cloudParam[i].piid);//�������ID
			strcat(psec,str);//piid�ַ�ƴ��
			if(analyzeData->cloudParam[i].code == MIIO_CODE_DONE && analyzeData->cloudParam[i].property.format != PROPERTY_FORMAT_UNDEFINED)
      {
				analyzeProprttyValue_Data2Psec(str,&analyzeData->cloudParam[i].property);//����value����
        strcat(psec," ");//�������' '
        strcat(psec,str);//value�ַ�ƴ��
      }
			vPortFree(str);
		}
	}
}

//Ӧ���ʽת�� data ת psec
//get_properties <siid> <piid> ... <siid> <piid>
//result <siid> <piid> <code> [value] ... <siid> <piid> <code> [value]
//set_properties <siid> <piid> <value> ... <siid> <piid> <value>
//result <siid> <piid> <code> ... <siid> <piid> <code>
//action <siid> <aiid> <piid> <value> ... <piid> <value>
//result <siid> <aiid> <code> <piid> <value> ... <piid> <value>
static void analyzeResult2Psec(char *psec,miio_data_t *analyzeData)
{
	if(analyzeData->cmd != MIIO_CMD_RESULT)return;
	for(int i=0;i<MIIO_PSEC_MAX_PARAM;i++)//��ȡ���з�������
	{
		if(analyzeData->cloudParam[i].siid != 0)//����з���ID
		{
			char *str = pvPortMalloc(MIIO_PSEC_VALUE_MAX_LEN);
			MALLOC_CHECK(str);
			memset(str,0,MIIO_PSEC_VALUE_MAX_LEN);//�������
			sprintf(str," %d",analyzeData->cloudParam[i].siid);//��ӷ���ID
			strcat(psec,str);//siid�ַ�ƴ��
			if(analyzeData->cloudParam[i].aiid != 0)//�����aiid
			{
				sprintf(str," %d",analyzeData->cloudParam[i].aiid);//��Ӷ���ID
				strcat(psec,str);//aiid�ַ�ƴ��
        sprintf(str," %d",analyzeData->cloudParam[i].code);//��Ӵ���ID
				strcat(psec,str);//code�ַ�ƴ��
			}
			if(analyzeData->cloudParam[i].piid != 0)//�����piid
			{
				sprintf(str," %d",analyzeData->cloudParam[i].piid);//��Ӷ���ID
				strcat(psec,str);//piid�ַ�ƴ��
        if(analyzeData->cloudParam[i].aiid == 0)
        {
          sprintf(str," %d",analyzeData->cloudParam[i].code);//��Ӵ���ID
          strcat(psec,str);//code�ַ�ƴ��
        }
			}
			if(analyzeData->cloudParam[i].code == MIIO_CODE_DONE && analyzeData->cloudParam[i].property.format != PROPERTY_FORMAT_UNDEFINED)
      {
				analyzeProprttyValue_Data2Psec(str,&analyzeData->cloudParam[i].property);//����value����
        strcat(psec," ");//�������' '
        strcat(psec,str);//value�ַ�ƴ��
      }
			vPortFree(str);
		}
	}
}

//������ѯ���Ե�psec
//�����ʽ��get_properties <siid> <piid> ... <siid> <piid>
//Ӧ���ʽ��<siid> <piid> <code> [value] ... <siid> <piid> <code> [value]
//PSEC ת ����
static void analyzePsec2GetProperties(miio_data_t *analyzeData,char *psec)
{
	strtok(psec," ");//�ָ��ַ���
	for(int i=0;i<MIIO_PSEC_MAX_PARAM;i++)//��ȡ����
	{
		char *p = strtok(NULL," ");
		if(p[0] >= '0' && p[0] <= '9')//���������
		{
			sscanf(p,"%d",&analyzeData->cloudParam[i].siid);//ȡ��<siid>
			p = strtok(NULL," ");//Ѱ��<piid>
			sscanf(p,"%d",&analyzeData->cloudParam[i].piid);//ȡ��<piid>
		}
		else if(p == NULL)//����Ѿ��������iidѰ��
			break;
		else//�����������(ָ���ַ�)
			i--;
	}
}

//�����������Ե�pesc
//�����ʽ��set_properties <siid> <piid> <value> ... <siid> <piid> <value>	 		
//Ӧ���ʽ:<siid> <piid> <code> ... <siid> <piid> <code>
static void analyzePsec2SetProperties(miio_data_t *analyzeData,char *psec)
{
	strtok(psec," ");//�ָ��ַ���
	for(int i=0;i<MIIO_PSEC_MAX_PARAM;i++)//��ȡ����
	{
		char *p = strtok(NULL," ");
		if(p[0] >= '0' && p[0] <= '9')//���������
		{
			sscanf(p,"%d",&analyzeData->cloudParam[i].siid);//ȡ��<siid>
			p = strtok(NULL," ");//Ѱ��<piid>
			sscanf(p,"%d",&analyzeData->cloudParam[i].piid);//ȡ��<piid>
			p = strtok(NULL," ");//Ѱ��<value>
			analyzeProprttyValue_Psec2Data(&analyzeData->cloudParam[i].property,p);//����value
		}
		else if(p == NULL)//����Ѿ��������iidѰ��
			break;
		else//�����������(ָ���ַ�)
			i--;
	}
}

//����ֵ��ת�� PSEC ת ����
static void analyzeProprttyValue_Psec2Data(property_t *property,char *psecValue)
{
	if(psecValue[0] >= '0' && psecValue[0] <= '9')//���������
	{
		property->format = PROPERTY_FORMAT_NUMBER;//������������
		if(strchr(psecValue,'.') != NULL)//���û��С����(����)
		{
			property->data.number.type = DATA_NUMBER_INTEGER;
			sscanf(psecValue,"%ld",&property->data.number.value.integerValue);
		}
		else//(����������)
		{
			property->data.number.type = DATA_NUMBER_FLOAT;
			sscanf(psecValue,"%f",&property->data.number.value.floatValue);
		}
	}
	else if(strcmp(psecValue,"true") != NULL || strcmp(psecValue,"false") != NULL)//��������
	{
		property->format = PROPERTY_FORMAT_BOOLEAN;//Bool��������
		if(psecValue[0] == 't')
			property->data.boolean.value = SET;
		else if(psecValue[0] == 'f')
			property->data.boolean.value = RESET;
	}
	else 
	{
		property->format = PROPERTY_FORMAT_STRING;//string��������
		property->data.string.length = strlen(psecValue);
		memcpy(property->data.string.value,psecValue,MIN_VALUE(property->data.string.length,MIIO_PSEC_VALUE_MAX_LEN));
	}
}

//����ֵ��ת��  	   ���� ת PSEC 
static void analyzeProprttyValue_Data2Psec(char *psecValue,property_t *property)
{
	switch((u8)property->format)
	{
		case PROPERTY_FORMAT_BOOLEAN:
			if(property->data.boolean.value == SET)
				strcpy(psecValue,"true");
			else if(property->data.boolean.value == RESET)
				strcpy(psecValue,"false");
		break;

		case PROPERTY_FORMAT_NUMBER:
			if(property->data.number.type == DATA_NUMBER_INTEGER)
				sprintf(psecValue,"%ld",property->data.number.value.integerValue);
			else
				sprintf(psecValue,"%f",property->data.number.value.floatValue);
		break;

		case PROPERTY_FORMAT_STRING:
			memcpy(psecValue,property->data.string.value,MIN_VALUE(property->data.string.length,MIIO_PSEC_VALUE_MAX_LEN));
		break;

		default://�������������
			
		break;
	}
}

//ָ���ת�� PSECתCMD
void analyzePsec2cmd(psec_cmd_t *cmd,char *psec)
{
	if(speccmp((char *)psec,MIIO_PSEC_DOWN_NONE))//down none
		*cmd = MIIO_CMD_DOWN_NONE;
	else if(speccmp((char *)psec,MIIO_PSEC_DOWN_GP))//down get_properties
		*cmd = MIIO_CMD_DOWN_GET_PROPERTIES;
	else if(speccmp((char *)psec,MIIO_PSEC_DOWN_SP))//down set_properties
		*cmd = MIIO_CMD_DOWN_SET_PROPERTIES;
	else if(speccmp((char *)psec,MIIO_PSEC_NET_CHANGE))//down MIIO_net_change
		*cmd = MIIO_CMD_DOWN_MIIO_NET_CHANGE;
	else if(speccmp((char *)psec,MIIO_PSEC_DOWN_ACTION))//down action
		*cmd = MIIO_CMD_DOWN_ACTION;
	else if(speccmp((char *)psec,MIIO_PSEC_ERROR))//error
		*cmd = MIIO_CMD_ERROR;
	else if(speccmp((char *)psec,MIIO_PSEC_OK))//ok
		*cmd = MIIO_CMD_OK;
	else//Other
		*cmd = MIIO_CMD_OTHRT;
}

//ָ���ת�� CMD ת PSEC
void analyzeCmd2psec(char *psec,psec_cmd_t cmd)
{
	switch((u8)cmd)
	{
		case MIIO_CMD_MODEL:
			memcpy(psec,MIIO_PSEC_MODEL,strlen(MIIO_PSEC_MODEL));
		break;

		case MIIO_CMD_MCU_VERSION:
			memcpy(psec,MIIO_PSEC_MCU_VERSION,strlen(MIIO_PSEC_MCU_VERSION));
		break;

		case MIIO_CMD_GET_DOWN:
			memcpy(psec,MIIO_PSEC_GET_DOWN,strlen(MIIO_PSEC_GET_DOWN));
		break;

		case MIIO_CMD_PROPERTIES_CHANGED:
			memcpy(psec,MIIO_PSEC_REPORT_PC,strlen(MIIO_PSEC_REPORT_PC));
		break;

		case MIIO_CMD_EVENTS:
			memcpy(psec,MIIO_PSEC_EVENT,strlen(MIIO_PSEC_EVENT));
		break;

		case MIIO_CMD_RESULT:
			memcpy(psec,MIIO_PSEC_RESULT,strlen(MIIO_PSEC_RESULT));
		break;

		case MIIO_CMD_CALL:
			memcpy(psec,MIIO_PSEC_CALL,strlen(MIIO_PSEC_CALL));
		break;

		case MIIO_CMD_REBOOT:
			memcpy(psec,MIIO_PSEC_REBOOT,strlen(MIIO_PSEC_REBOOT));
		break;

		case MIIO_CMD_RESTORE:
			memcpy(psec,MIIO_PSEC_RESTORE,strlen(MIIO_PSEC_RESTORE));
		break;

		case MIIO_CMD_NET:
			memcpy(psec,MIIO_PSEC_NET,strlen(MIIO_PSEC_NET));
		break;

		case MIIO_CMD_TIME:
			memcpy(psec,MIIO_PSEC_TIME,strlen(MIIO_PSEC_TIME));
		break;

		case MIIO_CMD_MAC:
			memcpy(psec,MIIO_PSEC_MAC,strlen(MIIO_PSEC_MAC));
		break;

		case MIIO_CMD_VERSION:
			memcpy(psec,MIIO_PSEC_VERSION,strlen(MIIO_PSEC_VERSION));
		break;
	}
}

int speccmp(char *uart,char *spec)
{
	return memcmp(spec,uart,strlen(spec)) ? 0:1;
}

