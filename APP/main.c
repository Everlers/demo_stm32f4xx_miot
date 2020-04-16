#include "hal_sys.h"
#include "task_config.h"
#include "miio.h"
#include "lcd_tft.h"
#include "ili9341.h"

int main(void)
{
	//配置0~15级的抢占式优先级 不需要子优先级 
	//注意:(使用RTOS的情况下必须设置NVIC_PriorityGroup_4既16级抢占)
	//因为RTOS的中断是15 受系统管理的最大优先级是configMAX_SYSCALL_INTERRUPT_PRIORITY = 11
	//RTOS临界区只会屏蔽优先级数值大于或等于configMAX_SYSCALL_INTERRUPT_PRIORITY值的中断
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	debug("STM32F4xx FreeRTOS.\r\n");
	
	//创建任务
	xTaskCreate(funOsTask,"fun task",FUN_TASK_STACK_SIZE,NULL,FUN_TASK_PRIORITY,NULL);
	xTaskCreate(miioOsTask,"miio task",MIIO_TASK_STACK_SIZE,NULL,MIIO_TASK_PRIORITY,NULL);
	
	vTaskStartScheduler();//开始调度
}

static void miioOsTask(void *pvParam)
{
	miioInit();
	while(1)
	{
		miioTask();
	}
}

static void funOsTask(void *pvParam)
{
	lcd_init();
	ili_blk_on();
	lcd_show_string(0,0,"STM32F4xx FreeRTOS.\r\n",WHITE,FILL_MODE);
	while(1)
	{
		
	}
}
