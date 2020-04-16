#include "hal_sys.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include "string.h"
#include "FreeRTOS.h"
#include "task.h"


void JlinkUSART(char *fmt,...)
{
	vTaskSuspendAll();
	char *string = pvPortMalloc(1024);
	if(string != NULL)
	{
		u16 len;
		va_list ap;
		va_start(ap,fmt);  
		vsprintf(string,fmt,ap);  
		len = strlen(string);
		for(u16 i=0;i<len;i++)
		{
			if (DEMCR & TRCENA) {
		    while (ITM_Port32(0) == 0);
		    ITM_Port8(0) = string[i];
		  }
		}
		va_end(ap);  
		vPortFree(string);
	}
	xTaskResumeAll();
}
