/*
 * ApplicationCode.h
 *
 *  Created on: Dec 30, 2023
 *      Author: Xavion
 */

#include "stm32f4xx_hal.h"
#include "Game_Driver.h"

#include <stdio.h>
#include "Button_Driver.h"
#include "LCD_Driver.h"
#include "main.h"

#ifndef INC_APPLICATIONCODE_H_
#define INC_APPLICATIONCODE_H_

// 0 for RNG, 1 for normal AI, 2 for advanced AI
#define PLAY_AI 1

void ApplicationInit(void);
void LCD_Visual_Demo(void);

#if (COMPILE_TOUCH_FUNCTIONS == 1)

void startGame();
void Run_Game(void);

#endif // (COMPILE_TOUCH_FUNCTIONS == 1)

#endif /* INC_APPLICATIONCODE_H_ */
