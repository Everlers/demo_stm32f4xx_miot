#include "led.h"

void HalLedInit(void)
{
	GPIO_InitTypeDef GPIOInitStruct;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);
	
	GPIOInitStruct.GPIO_Pin = HAL_ALL_LED;
	GPIOInitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIOInitStruct.GPIO_OType = GPIO_OType_PP;
	GPIOInitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIOInitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(HAL_LED_PORT,&GPIOInitStruct);
}

void HalLedSwitch(u16 led,FlagStatus sw)
{
	if(sw == SET)
		HAL_LED_PORT->BSRRL |= led;
	else
		HAL_LED_PORT->BSRRH |= led;
}

FlagStatus HalLedSwitchGet(u16 led)
{
	return (HAL_LED_PORT->ODR & led) ? RESET:SET;
}
