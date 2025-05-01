/*
 * Game_Driver.h
 *
 *  Created on: Apr 17, 2025
 *      Author: wjnar
 */

#ifndef INC_GAME_DRIVER_H_
#define INC_GAME_DRIVER_H_

#include <stdbool.h>
#include "ApplicationCode.h"
#include "LCD_Driver.h"

#define CHIP_RADIUS 15
#define NUM_COLUMNS 7
#define NUM_ROWS 6

#define CENTER_COL 3
#define LEFT_MOST_COL 0
#define RIGHT_MOST_COL 6

//board logisitics
#define EMPTY_SQUARE 0
#define RED_TAKEN_SQUARE 1
#define YELLOW_TAKEN_SQUARE 2

//current player
#define RED_PLAYER 0
#define YELLOW_PLAYER 1

// 0,0 is top left
typedef struct{
    uint16_t x;
    uint16_t y;
    uint8_t column;
    uint8_t row;
} tokenCoords;

typedef struct {
	bool mode; //single or multi player

	uint8_t currentPlayer;

	bool redWin;
	uint8_t numRedWins;
	bool yellowWin;
	uint8_t numYellowWins;

	bool coinDropped;

	tokenCoords tokenLocation;
	uint8_t board[NUM_ROWS][NUM_COLUMNS];

	uint32_t startTime;
	uint32_t endTime;


} GameData;

enum GameMode{
	SINGLE_PLAYER,
	MULTI_PLAYER
};


void gameInit();

void spawnPendingToken(uint16_t tokenColor);

void movePendingToken(uint8_t direction);

void dropToken(uint16_t tokenColor);

int countMatchesInEachDirection(uint8_t currPlayer, uint8_t currRow, uint8_t currCol, int rowDirection, int colDirection);

bool check_win_cond();

bool check_possible_win_cond(uint8_t currPlayer, uint8_t currRow, uint8_t currCol);

bool isColumnFull(uint8_t column);

bool isBoardFull();

void OpponentPlayRNG();

void OpponentPlayAI();

uint32_t getRandomInRange(uint32_t min, uint32_t max);

int calculateTotalTime();

uint32_t roundToSeconds(uint32_t value);

void setGameData(GameData gameData);

GameData getGameData();


#endif /* INC_GAME_DRIVER_H_ */
