#ifndef __LED_H
#define __LED_H	 
#include "stm32f4xx_hal.h"



#define LED_GPIO_PERIPH		RCC_AHB1Periph_GPIOA
#define LED_GPIO			GPIOA
#define LED_GPIO_PIN		GPIO_PIN_1




void LED_toggle();

void LED_init();


void LED_ON();
void LED_OFF();
#endif











