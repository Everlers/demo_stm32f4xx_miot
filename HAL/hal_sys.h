#ifndef HAL_SYS_H_
#define HAL_SYS_H_
#include "stm32f4xx_conf.h"

#define HAL_FREERTOS_EN					1											// π”√FreeRTOS

#if HAL_FREERTOS_EN == 1
//#include "FreeRTOS.h"
//#include "task.h"
#endif

#define BV(n)										(1<<n)
#define MIN_VALUE(n,n1)					((n<n1) ? n:n1)
#define MAX_VALUE(n,n1)					((n>n1) ? n:n1)
#define MALLOC_CHECK(x)					if(x == NULL)return;

#define debug			JlinkUSART//((void)0)

//JLINK CDC USART (ITM)
#define ITM_Port8(n)    (*((volatile unsigned char *)(0xE0000000+4*n)))
#define ITM_Port16(n)   (*((volatile unsigned short*)(0xE0000000+4*n)))
#define ITM_Port32(n)   (*((volatile unsigned long *)(0xE0000000+4*n)))
#define DEMCR           (*((volatile unsigned long *)(0xE000EDFC)))
#define TRCENA          0x01000000

void JlinkUSART(char *fmt,...);
#endif
