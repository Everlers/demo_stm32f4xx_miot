#ifndef _LED_H_
#define _LED_H_
#include "hal_sys.h"

#define HAL_LED_PORT				GPIOA
#define HAL_LED1						GPIO_Pin_3
#define HAL_LED2						GPIO_Pin_2
#define HAL_ALL_LED					(HAL_LED1|HAL_LED2)

#endif
