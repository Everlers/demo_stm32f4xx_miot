#include "hal_sys.h"
#include "task_config.h"
#include "miio.h"
#include "lcd_tft.h"
#include "ili9341.h"

int main(void)
{
	//����0~15������ռʽ���ȼ� ����Ҫ�����ȼ� 
	//ע��:(ʹ��RTOS������±�������NVIC_PriorityGroup_4��16����ռ)
	//��ΪRTOS���ж���15 ��ϵͳ�����������ȼ���configMAX_SYSCALL_INTERRUPT_PRIORITY = 11
	//RTOS�ٽ���ֻ���������ȼ���ֵ���ڻ����configMAX_SYSCALL_INTERRUPT_PRIORITYֵ���ж�
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	debug("STM32F4xx FreeRTOS.\r\n");
	
	//��������
	xTaskCreate(funOsTask,"fun task",FUN_TASK_STACK_SIZE,NULL,FUN_TASK_PRIORITY,NULL);
	xTaskCreate(miioOsTask,"miio task",MIIO_TASK_STACK_SIZE,NULL,MIIO_TASK_PRIORITY,NULL);
	
	vTaskStartScheduler();//��ʼ����
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
