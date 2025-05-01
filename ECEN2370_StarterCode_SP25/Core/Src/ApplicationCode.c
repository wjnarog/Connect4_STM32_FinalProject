/*
 * ApplicationCode.c
 *
 *  Created on: Dec 30, 2023 (updated 11/12/2024) Thanks Donavon! 
 *      Author: Xavion
 */

#include "ApplicationCode.h"

/* Static variables */


extern void initialise_monitor_handles(void); 

#if COMPILE_TOUCH_FUNCTIONS == 1
static STMPE811_TouchData StaticTouchData;
#endif // COMPILE_TOUCH_FUNCTIONS

volatile bool isButtonPressed = BUTTON_UNPRESSED;


void ApplicationInit(void)
{
	initialise_monitor_handles(); // Allows printf functionality
    LTCD__Init();
    LTCD_Layer_Init(0);
    LCD_Clear(0,LCD_COLOR_WHITE);

    #if COMPILE_TOUCH_FUNCTIONS == 1
	InitializeLCDTouch();

	// This is the orientation for the board to be directly up where the buttons are vertically above the screen
	// Top left would be low x value, high y value. Bottom right would be low x value, low y value.
	StaticTouchData.orientation = STMPE811_Orientation_Portrait_2;

	#endif // COMPILE_TOUCH_FUNCTIONS
	ButtonInitInterruptMode();
}

void LCD_Visual_Demo(void)
{
	visualDemo();
}

#if COMPILE_TOUCH_FUNCTIONS == 1

void startGame(){
	GameData gameInfo = getGameData();
	displayStartScreen();

	while (1) {
		/* If touch pressed */
		if (returnTouchStateAndLocation(&StaticTouchData) == STMPE811_State_Pressed) {
//			/* Touch valid */
			HAL_Delay(500);
			if(DetermineLeftOrRightTouch(StaticTouchData.x) == LEFT){
				gameInfo.mode = SINGLE_PLAYER;
				setGameData(gameInfo);
				break;
			}else{
				gameInfo.mode = MULTI_PLAYER;
				setGameData(gameInfo);
				break;
			}

		}
	}
}

void Run_Game(void){
	startGame();
	GameData gameInfo = getGameData();


	displayGameScreen();

	gameInfo.startTime = HAL_GetTick();
	setGameData(gameInfo);
	isButtonPressed = false;

	while(1){
		HAL_Delay(200);
		gameInfo = getGameData();

		if(gameInfo.redWin || gameInfo.yellowWin || isBoardFull()){
			gameInfo.endTime = HAL_GetTick();
			setGameData(gameInfo);
			//Delay so user can see win/loss
			HAL_Delay(1000);
			displayWinner();
			while(1){
				if(returnTouchStateAndLocation(&StaticTouchData) == STMPE811_State_Pressed){
					displayEndScreen();
					while(1){
						HAL_Delay(200);
						if(returnTouchStateAndLocation(&StaticTouchData) == STMPE811_State_Pressed){
							HAL_Delay(200);
							break;
						}
					}
					break;
				}
			}
			break;
		}

		if(gameInfo.currentPlayer == RED_PLAYER){
			spawnPendingToken(LCD_COLOR_RED);
		}else{
			spawnPendingToken(LCD_COLOR_YELLOW);
		}

		if(gameInfo.mode == SINGLE_PLAYER && gameInfo.currentPlayer == YELLOW_PLAYER){
#if (PLAY_AI == 1 || PLAY_AI == 2)
			OpponentPlayAI();

#else
			OpponentPlayRNG();
#endif
		}else{
			if(returnTouchStateAndLocation(&StaticTouchData) == STMPE811_State_Pressed){
				movePendingToken(DetermineLeftOrRightTouch(StaticTouchData.x));
			}
			if(isButtonPressed){
				dropToken(LCD_COLOR_RED);
				isButtonPressed = BUTTON_UNPRESSED;
			}
		}
	}
}


void EXTI0_IRQHandler(void){

//	if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_0) != RESET)
//	{
//		__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);
//		isButtonPressed = true;
//	}

	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
	return;
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin == GPIO_PIN_0){
		isButtonPressed = BUTTON_PRESSED;
	}
}


#endif // COMPILE_TOUCH_FUNCTIONS

