#include "dataAnalyze.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "FreeRTOS.h"
#include "task.h"

//属性值转换
static void analyzeProprttyValue_Psec2Data(property_t *property,char *psecValue);
static void analyzeProprttyValue_Data2Psec(char *psecValue,property_t *property);
//属性转换
static void analyzePsec2SetProperties(miio_data_t *analyzeData,char *psec);
static void analyzePsec2GetProperties(miio_data_t *analyzeData,char *psec);
static void analyzeResult2Psec(char *psec,miio_data_t *analyzeData);
static void analyzeProprttyChanged(char *psec,miio_data_t *analyzeData);
//动作转换
static void analyzeAction(miio_data_t *analyzeData,char *psec);
//事件转换
static void analyzeEvents(char *psec,miio_data_t *analyzeData);
//网络状态转换
static void analyzeNetState(miio_data_t *analyzeData,char *psec);


//PSEC 转 数据
void analyzePsec2Data (miio_data_t *analyzeData,char *psec)
{
	analyzePsec2cmd(&analyzeData->cmd,(char *)psec);//psec字符转枚举命令
	if(analyzeData->cmd != MIIO_CMD_DOWN_NONE)//如果有数据处理
		debug("[MIIO] recv: %s\n",psec);
	switch((u8)analyzeData->cmd)//解析数据参数
	{
		case MIIO_CMD_DOWN_GET_PROPERTIES://查询属性
			analyzePsec2GetProperties(analyzeData,psec);
		break;

		case MIIO_CMD_DOWN_SET_PROPERTIES://设置属性
			analyzePsec2SetProperties(analyzeData,psec);
		break;

		case MIIO_CMD_DOWN_ACTION://执行动作
			analyzeAction(analyzeData,psec);
		break;

		case MIIO_CMD_DOWN_MIIO_NET_CHANGE://网络发生改变
			analyzeNetState(analyzeData,psec);
		break;

		case MIIO_CMD_OTHRT://其他下发数据
			memcpy(analyzeData->modelParam,psec,strlen(psec));//复制数据
		break;

		case MIIO_CMD_ERROR://错误操作
			debug("[MIIO] error\r\n");
		break;
		
		case MIIO_CMD_DOWN_NONE://没有下发
			
		break;
		
		default://未知的下发(未知的错误信息)
			
		break;
	}
}

//数据 转 PSEC
void analyzeData2Psec(char *psec,miio_data_t *analyzeData)
{
	analyzeCmd2psec(psec,analyzeData->cmd);//PSEC指令转换
	switch((u8)analyzeData->cmd)//解析数据参数
	{	
		case MIIO_CMD_RESULT://执行应答
			analyzeResult2Psec(psec,analyzeData);
		break;

		case MIIO_CMD_PROPERTIES_CHANGED://属性更改
			analyzeProprttyChanged(psec,analyzeData);
		break;
		
		case MIIO_CMD_EVENTS://事件上报
			analyzeEvents(psec,analyzeData);
		break;
		
		case MIIO_CMD_MODEL://模组名称
		case MIIO_CMD_MCU_VERSION://MCU版本上报
			if(analyzeData->modelParam[0] != '\0')
			{
				strcat(psec," ");//空格间隔
				strcat(psec,analyzeData->modelParam);//拼接参数
			}
		break;
		
		default://未知(未知的错误信息)
			
		break;
	}
	strcat(psec,"\r");//加入结束符
}

//解析网络状态
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

//解析动作指令 psec 转 data
//action <siid> <aiid> <piid> <value> ... <piid> <value>
static void analyzeAction(miio_data_t *analyzeData,char *psec)
{
	if(analyzeData->cmd != MIIO_CMD_DOWN_ACTION)return;
	strtok(psec," ");//分割字符串
	for(int i=0;i<MIIO_PSEC_MAX_PARAM;i++)//读取参数
	{
		char *p = strtok(NULL," ");
		if(p[0] >= '0' && p[0] <= '9')//如果是数字
		{
			sscanf(p,"%d",&analyzeData->cloudParam[i].siid);//取出<siid>
			p = strtok(NULL," ");//寻找<aiid>
			sscanf(p,"%d",&analyzeData->cloudParam[i].aiid);//取出<aiid>
			p = strtok(NULL," ");//寻找<piid>
			if(p != NULL)//如果有<piid> 注意：有些ACTION是没有值的 没有值就没有属性ID
			{
				sscanf(p,"%d",&analyzeData->cloudParam[i].piid);//取出<piid>
				p = strtok(NULL," ");//寻找<value>
				analyzeProprttyValue_Psec2Data(&analyzeData->cloudParam[i].property,p);//解析value
			}
		}
		else if(p == NULL)//如果已经完成所有iid寻找
			break;
		else//如果不是数字(指令字符)
			i--;
	}
}

//事件上报的格式转换
//event_occured <siid> <eiid> <piid> <value> ... <piid> <value>
static void analyzeEvents(char *psec,miio_data_t *analyzeData)
{
	if(analyzeData->cmd != MIIO_CMD_EVENTS)return;
	for(int i=0;i<MIIO_PSEC_MAX_PARAM;i++)//读取所有服务属性
	{
		if(analyzeData->cloudParam[i].siid != 0)//如果有服务ID
		{
			char *str = pvPortMalloc(MIIO_PSEC_VALUE_MAX_LEN);
			MALLOC_CHECK(str);
			memset(str,0,MIIO_PSEC_VALUE_MAX_LEN);//清除数据
			sprintf(str," %d",analyzeData->cloudParam[i].siid);//添加服务ID
			strcat(psec,str);//siid字符拼接
			sprintf(str," %d",analyzeData->cloudParam[i].eiid);//添加事件ID
			strcat(psec,str);//eiid字符拼接
			sprintf(str," %d",analyzeData->cloudParam[i].piid);//添加属性ID
			strcat(psec,str);//piid字符拼接
			if(analyzeData->cloudParam[i].code == MIIO_CODE_DONE && analyzeData->cloudParam[i].property.format != PROPERTY_FORMAT_UNDEFINED)
      {
				analyzeProprttyValue_Data2Psec(str,&analyzeData->cloudParam[i].property);//解析value数据
        strcat(psec," ");//间隔符号' '
        strcat(psec,str);//value字符拼接
      }
			vPortFree(str);
		}
	}
}

//属性更改上报的格式转换
//properties_changed <siid> <piid> <value> ... <siid> <piid> <value>
static void analyzeProprttyChanged(char *psec,miio_data_t *analyzeData)
{
	if(analyzeData->cmd != MIIO_CMD_PROPERTIES_CHANGED)return;
	for(int i=0;i<MIIO_PSEC_MAX_PARAM;i++)//读取所有服务属性
	{
		if(analyzeData->cloudParam[i].siid != 0)//如果有服务ID
		{
			char *str = pvPortMalloc(MIIO_PSEC_VALUE_MAX_LEN);
			MALLOC_CHECK(str);
			memset(str,0,MIIO_PSEC_VALUE_MAX_LEN);//清除数据
			sprintf(str," %d",analyzeData->cloudParam[i].siid);//添加服务ID
			strcat(psec,str);//siid字符拼接
			sprintf(str," %d",analyzeData->cloudParam[i].piid);//添加属性ID
			strcat(psec,str);//piid字符拼接
			if(analyzeData->cloudParam[i].code == MIIO_CODE_DONE && analyzeData->cloudParam[i].property.format != PROPERTY_FORMAT_UNDEFINED)
      {
				analyzeProprttyValue_Data2Psec(str,&analyzeData->cloudParam[i].property);//解析value数据
        strcat(psec," ");//间隔符号' '
        strcat(psec,str);//value字符拼接
      }
			vPortFree(str);
		}
	}
}

//应答格式转换 data 转 psec
//get_properties <siid> <piid> ... <siid> <piid>
//result <siid> <piid> <code> [value] ... <siid> <piid> <code> [value]
//set_properties <siid> <piid> <value> ... <siid> <piid> <value>
//result <siid> <piid> <code> ... <siid> <piid> <code>
//action <siid> <aiid> <piid> <value> ... <piid> <value>
//result <siid> <aiid> <code> <piid> <value> ... <piid> <value>
static void analyzeResult2Psec(char *psec,miio_data_t *analyzeData)
{
	if(analyzeData->cmd != MIIO_CMD_RESULT)return;
	for(int i=0;i<MIIO_PSEC_MAX_PARAM;i++)//读取所有服务属性
	{
		if(analyzeData->cloudParam[i].siid != 0)//如果有服务ID
		{
			char *str = pvPortMalloc(MIIO_PSEC_VALUE_MAX_LEN);
			MALLOC_CHECK(str);
			memset(str,0,MIIO_PSEC_VALUE_MAX_LEN);//清除数据
			sprintf(str," %d",analyzeData->cloudParam[i].siid);//添加服务ID
			strcat(psec,str);//siid字符拼接
			if(analyzeData->cloudParam[i].aiid != 0)//如果有aiid
			{
				sprintf(str," %d",analyzeData->cloudParam[i].aiid);//添加动作ID
				strcat(psec,str);//aiid字符拼接
        sprintf(str," %d",analyzeData->cloudParam[i].code);//添加错误ID
				strcat(psec,str);//code字符拼接
			}
			if(analyzeData->cloudParam[i].piid != 0)//如果有piid
			{
				sprintf(str," %d",analyzeData->cloudParam[i].piid);//添加动作ID
				strcat(psec,str);//piid字符拼接
        if(analyzeData->cloudParam[i].aiid == 0)
        {
          sprintf(str," %d",analyzeData->cloudParam[i].code);//添加错误ID
          strcat(psec,str);//code字符拼接
        }
			}
			if(analyzeData->cloudParam[i].code == MIIO_CODE_DONE && analyzeData->cloudParam[i].property.format != PROPERTY_FORMAT_UNDEFINED)
      {
				analyzeProprttyValue_Data2Psec(str,&analyzeData->cloudParam[i].property);//解析value数据
        strcat(psec," ");//间隔符号' '
        strcat(psec,str);//value字符拼接
      }
			vPortFree(str);
		}
	}
}

//解析查询属性的psec
//命令格式：get_properties <siid> <piid> ... <siid> <piid>
//应答格式：<siid> <piid> <code> [value] ... <siid> <piid> <code> [value]
//PSEC 转 属性
static void analyzePsec2GetProperties(miio_data_t *analyzeData,char *psec)
{
	strtok(psec," ");//分割字符串
	for(int i=0;i<MIIO_PSEC_MAX_PARAM;i++)//读取参数
	{
		char *p = strtok(NULL," ");
		if(p[0] >= '0' && p[0] <= '9')//如果是数字
		{
			sscanf(p,"%d",&analyzeData->cloudParam[i].siid);//取出<siid>
			p = strtok(NULL," ");//寻找<piid>
			sscanf(p,"%d",&analyzeData->cloudParam[i].piid);//取出<piid>
		}
		else if(p == NULL)//如果已经完成所有iid寻找
			break;
		else//如果不是数字(指令字符)
			i--;
	}
}

//解析设置属性的pesc
//命令格式：set_properties <siid> <piid> <value> ... <siid> <piid> <value>	 		
//应答格式:<siid> <piid> <code> ... <siid> <piid> <code>
static void analyzePsec2SetProperties(miio_data_t *analyzeData,char *psec)
{
	strtok(psec," ");//分割字符串
	for(int i=0;i<MIIO_PSEC_MAX_PARAM;i++)//读取参数
	{
		char *p = strtok(NULL," ");
		if(p[0] >= '0' && p[0] <= '9')//如果是数字
		{
			sscanf(p,"%d",&analyzeData->cloudParam[i].siid);//取出<siid>
			p = strtok(NULL," ");//寻找<piid>
			sscanf(p,"%d",&analyzeData->cloudParam[i].piid);//取出<piid>
			p = strtok(NULL," ");//寻找<value>
			analyzeProprttyValue_Psec2Data(&analyzeData->cloudParam[i].property,p);//解析value
		}
		else if(p == NULL)//如果已经完成所有iid寻找
			break;
		else//如果不是数字(指令字符)
			i--;
	}
}

//属性值的转换 PSEC 转 数据
static void analyzeProprttyValue_Psec2Data(property_t *property,char *psecValue)
{
	if(psecValue[0] >= '0' && psecValue[0] <= '9')//如果是数字
	{
		property->format = PROPERTY_FORMAT_NUMBER;//数字类型数据
		if(strchr(psecValue,'.') != NULL)//如果没有小数点(整数)
		{
			property->data.number.type = DATA_NUMBER_INTEGER;
			sscanf(psecValue,"%ld",&property->data.number.value.integerValue);
		}
		else//(浮点型数据)
		{
			property->data.number.type = DATA_NUMBER_FLOAT;
			sscanf(psecValue,"%f",&property->data.number.value.floatValue);
		}
	}
	else if(strcmp(psecValue,"true") != NULL || strcmp(psecValue,"false") != NULL)//如果是真假
	{
		property->format = PROPERTY_FORMAT_BOOLEAN;//Bool类型数据
		if(psecValue[0] == 't')
			property->data.boolean.value = SET;
		else if(psecValue[0] == 'f')
			property->data.boolean.value = RESET;
	}
	else 
	{
		property->format = PROPERTY_FORMAT_STRING;//string类型数据
		property->data.string.length = strlen(psecValue);
		memcpy(property->data.string.value,psecValue,MIN_VALUE(property->data.string.length,MIIO_PSEC_VALUE_MAX_LEN));
	}
}

//属性值的转换  	   数据 转 PSEC 
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

		default://错误的数据类型
			
		break;
	}
}

//指令的转换 PSEC转CMD
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

//指令的转换 CMD 转 PSEC
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

