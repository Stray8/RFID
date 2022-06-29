#include "led.h"

void LED_toggle()
{

	HAL_GPIO_TogglePin(LED_GPIO, LED_GPIO_PIN);
}

void LED_ON()
{

	HAL_GPIO_WritePin(LED_GPIO, LED_GPIO_PIN, GPIO_PIN_SET);
}

void LED_OFF()
{

	HAL_GPIO_WritePin(LED_GPIO, LED_GPIO_PIN, GPIO_PIN_RESET);
}

void LED_init()
{
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;

	__HAL_RCC_GPIOA_CLK_ENABLE();
 

	GPIO_InitStructure.Pin = LED_GPIO_PIN; 
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;//复用功能
	GPIO_InitStructure.Speed = GPIO_SPEED_FAST;	//速度50MHz
	GPIO_InitStructure.Pull = GPIO_PULLUP; //上拉
	HAL_GPIO_Init(LED_GPIO,&GPIO_InitStructure); //初始化PA9，PA10

	HAL_GPIO_WritePin(LED_GPIO, LED_GPIO_PIN, GPIO_PIN_SET);


}