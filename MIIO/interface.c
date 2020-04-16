#include "interface.h"
#include "string.h"
#include "iid.h"
#include "FreeRTOS.h"
#include "task.h"

static void devicePropertyGet(int siid,int piid,property_t *property);
static void devicePropertySet(int siid,int piid,property_t *property);
static state_code_t deviceAction(int siid,int aiid,int piid,property_t *value);

//下发指令接口
void interfaceDataHanle (miio_data_t *miioData)
{
	miio_data_t *report = pvPortMalloc(sizeof(miio_data_t));
	MALLOC_CHECK(report);
	miioDataDeinit(report);
	switch((u8)miioData->cmd)
	{
		case MIIO_CMD_DOWN_GET_PROPERTIES://查询属性
		{
			report->cmd = MIIO_CMD_RESULT;
			for(int i=0;i<MIIO_PSEC_MAX_PARAM;i++)
			{
				if(miioData->cloudParam[i].siid == 0)break;
				miioIidCopy(&report->cloudParam[i],&miioData->cloudParam[i]);//设定IID
				devicePropertyGet(miioData->cloudParam[i].siid,miioData->cloudParam[i].piid,&report->cloudParam[i].property);//读取属性数据
			}
			miioSendWait(report,MIIO_PSEC_OK);
		}
		break;

		case MIIO_CMD_DOWN_SET_PROPERTIES://设置属性
		{
			report->cmd = MIIO_CMD_RESULT;
			for(int i=0;i<MIIO_PSEC_MAX_PARAM;i++)
			{
				if(miioData->cloudParam[i].siid == 0)break;
				miioIidCopy(&report->cloudParam[i],&miioData->cloudParam[i]);
				report->cloudParam[i].code = MIIO_CODE_DONE;
				devicePropertySet(miioData->cloudParam[i].siid,miioData->cloudParam[i].piid,&miioData->cloudParam[i].property);//写入属性数据
			}
			miioSendWait(report,MIIO_PSEC_OK);
		}
		break;
		
		case MIIO_CMD_DOWN_ACTION://动作执行
		{
			report->cmd = MIIO_CMD_RESULT;
			for(int i=0;i<MIIO_PSEC_MAX_PARAM;i++)
			{
				if(miioData->cloudParam[i].siid == 0)break;
				miioIidCopy(&report->cloudParam[i],&miioData->cloudParam[i]);
				miioPropertyCopy(&report->cloudParam[i].property,&miioData->cloudParam[i].property);
				report->cloudParam[i].code = deviceAction(miioData->cloudParam[i].siid,miioData->cloudParam[i].aiid,miioData->cloudParam[i].piid,&miioData->cloudParam[i].property);//执行动作
			}
			miioSendWait(report,MIIO_PSEC_OK);
		}
		break;
	}
	vPortFree(report);
}

FlagStatus sw;
u8 Brightnes = 1;

//执行动作
static state_code_t deviceAction(int siid,int aiid,int piid,property_t *value)
{
	switch(siid)
	{
		case IID_1_DeviceInformation:
			
		break;
		
		case IID_2_Light:
		{
			switch(aiid)
			{
				case IID_2_1_BrightnessDown:
					//ledBrightnesSet(ledBrightnesGet()-10);
					Brightnes -= 10;
				return MIIO_CODE_DONE;
				
				case IID_2_2_BrightnessUp:
					//ledBrightnesSet(ledBrightnesGet()+10);
					Brightnes += 10;
				return MIIO_CODE_DONE;
			}
		}
		break;
	}
	return MIIO_CODE_PMEN;
}

//属性查询
static void devicePropertyGet(int siid,int piid,property_t *property)
{
	switch(siid)
	{
		case IID_1_DeviceInformation:
			
		break;
		
		case IID_2_Light:
		{
			switch(piid)
			{
				case IID_2_1_On:
					property->format = PROPERTY_FORMAT_BOOLEAN;
					property->data.boolean.value = sw;//ledSwitchGet();
				break;

				case IID_2_2_Brightness:
					property->format = PROPERTY_FORMAT_NUMBER;
					property->data.number.type = DATA_NUMBER_INTEGER;
					property->data.number.value.integerValue = Brightnes;
				break;
			}
		}
		break;
	}
}

//属性设置
static void devicePropertySet(int siid,int piid,property_t *property)
{
	switch(siid)
	{
		case IID_1_DeviceInformation:
			
		break;
		
		case IID_2_Light:
		{
			switch(piid)
			{
				case IID_2_1_On:
					if(property->format == PROPERTY_FORMAT_BOOLEAN)
					{
						sw = property->data.boolean.value;
						//ledSwitchSet(property->data.boolean.value);
					}
				break;

				case IID_2_2_Brightness:
					if(property->format == PROPERTY_FORMAT_NUMBER)
					{
						if(property->data.number.type == DATA_NUMBER_FLOAT)
							Brightnes = property->data.number.value.floatValue;
							//ledBrightnesSet(property->data.number.value.floatValue);
					}
				break;
			}
		}
		break;
	}
}

