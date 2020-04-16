#ifndef _TASK_CONFIG_H_
#define _TASK_CONFIG_H_
#include "hal_sys.h"

#define FUN_TASK_STACK_SIZE				( configMINIMAL_STACK_SIZE + 50 )
#define FUN_TASK_PRIORITY					( tskIDLE_PRIORITY + 0 )

#define LCD_TASK_STACK_SIZE				( configMINIMAL_STACK_SIZE + 50 )
#define LCD_TASK_PRIORITY					( tskIDLE_PRIORITY + 0 )

#define MIIO_TASK_STACK_SIZE			( configMINIMAL_STACK_SIZE + 100 )
#define	MIIO_TASK_PRIORITY				( tskIDLE_PRIORITY + 0 )

static void funOsTask(void *pvParam);
static void miioOsTask(void *pvParam);
#endif
