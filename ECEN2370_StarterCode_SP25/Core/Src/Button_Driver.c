/*
 * Button_Driver.c
 *
 *  Created on: Apr 21, 2025
 *      Author: wjnar
 */


#include "Button_Driver.h"

void ButtonInitInterruptMode(){
	__HAL_RCC_GPIOA_CLK_ENABLE();


	GPIO_InitTypeDef pinConfig = {0};

	pinConfig.Pin = GPIO_PIN_0;
	pinConfig.Mode = GPIO_MODE_IT_RISING;
	pinConfig.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	pinConfig.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &pinConfig);

	HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

bool IsButtonPressed(){
	GPIO_PinState inputPin = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
	if(inputPin & GPIO_PIN_SET) return true;
	else return false;
}
