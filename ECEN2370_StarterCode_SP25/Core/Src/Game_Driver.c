/*
 * Game_Driver.c
 *
 *  Created on: Apr 17, 2025
 *      Author: wjnar
 */

#include "Game_Driver.h"

GameData gameInfo = {0};

void gameInit(){
	GameData gameInfo = getGameData();

	gameInfo.currentPlayer = RED_PLAYER;

	gameInfo.coinDropped = false;

	gameInfo.tokenLocation.x = 0;
	gameInfo.tokenLocation.y = 0;
	gameInfo.tokenLocation.column = 0;
	gameInfo.tokenLocation.row = 0;

	gameInfo.startTime = 0;
	gameInfo.endTime = 0;

	gameInfo.redWin = false;
	gameInfo.yellowWin = false;

	//clean board
	for(int i = 0; i < NUM_ROWS; i++){
		for(int j = 0; j < NUM_COLUMNS; j++){
			gameInfo.board[i][j] = EMPTY_SQUARE;
		}
	}

	setGameData(gameInfo);
}

void spawnPendingToken(uint16_t tokenColor){
	GameData gameInfo = getGameData();

	if((gameInfo.tokenLocation.x == 0 && gameInfo.tokenLocation.y == 0) || gameInfo.coinDropped == true){
		gameInfo.tokenLocation.x = (LCD_PIXEL_WIDTH * CHIP_OFFSET);
		gameInfo.tokenLocation.y = (LCD_PIXEL_HEIGHT * COLUMN_NUM_DIVISOR) * CHIP_OFFSET;
		gameInfo.tokenLocation.column = CENTER_COL;
		gameInfo.coinDropped = false;
	}
	setGameData(gameInfo);

	if (gameInfo.tokenLocation.x >= LCD_PIXEL_WIDTH || gameInfo.tokenLocation.y >= LCD_PIXEL_HEIGHT){
		return;
	}

	LCD_Draw_Circle_Fill(gameInfo.tokenLocation.x, gameInfo.tokenLocation.y, CHIP_RADIUS, tokenColor);
}


void movePendingToken(uint8_t direction){
	GameData gameInfo = getGameData();
	if(gameInfo.tokenLocation.column >= LEFT_MOST_COL || gameInfo.tokenLocation.column <= RIGHT_MOST_COL){

		if(direction == LEFT && gameInfo.tokenLocation.column != LEFT_MOST_COL){
			gameInfo.tokenLocation.column--;
		}else if(direction == RIGHT && gameInfo.tokenLocation.column != RIGHT_MOST_COL){
			gameInfo.tokenLocation.column++;
		}

		gameInfo.tokenLocation.x = (LCD_PIXEL_WIDTH * COLUMN_NUM_DIVISOR * (gameInfo.tokenLocation.column + CHIP_OFFSET));

		if(gameInfo.tokenLocation.y == 0){
			gameInfo.tokenLocation.y = (LCD_PIXEL_HEIGHT * COLUMN_NUM_DIVISOR) * CHIP_OFFSET;
		}
		setGameData(gameInfo);
	}
	displayGameScreen();
	if(gameInfo.currentPlayer == RED_PLAYER){
		spawnPendingToken(LCD_COLOR_RED);
	}else{
		spawnPendingToken(LCD_COLOR_YELLOW);
	}

}

void dropToken(uint16_t tokenColor){
	GameData gameInfo = getGameData();
	for(int i = 5; i >= 0; i--){
		if(gameInfo.board[i][gameInfo.tokenLocation.column] == EMPTY_SQUARE){
			if(gameInfo.currentPlayer == RED_PLAYER){
				gameInfo.board[i][gameInfo.tokenLocation.column] = RED_TAKEN_SQUARE;
				gameInfo.tokenLocation.y = (LCD_PIXEL_HEIGHT * ROW_NUM_DIVISOR * (i + 1 + CHIP_OFFSET));
				gameInfo.tokenLocation.row = i;
				break;

			}else if(gameInfo.currentPlayer == YELLOW_PLAYER){
				gameInfo.board[i][gameInfo.tokenLocation.column] = YELLOW_TAKEN_SQUARE;
				gameInfo.tokenLocation.y = (LCD_PIXEL_HEIGHT * ROW_NUM_DIVISOR * (i + 1 + CHIP_OFFSET));
				gameInfo.tokenLocation.row = i;
				break;
			}
		}
	}


	gameInfo.tokenLocation.x = (LCD_PIXEL_WIDTH * COLUMN_NUM_DIVISOR * (gameInfo.tokenLocation.column + CHIP_OFFSET));

	if(!isColumnFull(gameInfo.tokenLocation.column)){
		setGameData(gameInfo);
		displayGameScreen();

		if(gameInfo.currentPlayer == RED_PLAYER){
			spawnPendingToken(LCD_COLOR_RED);
		}else{
			spawnPendingToken(LCD_COLOR_YELLOW);
		}

		gameInfo.coinDropped = true;

		if(gameInfo.currentPlayer == RED_PLAYER){
			gameInfo.redWin = check_win_cond();

			if(gameInfo.redWin){
				gameInfo.numRedWins++;
				setGameData(gameInfo);
				return;
			}
		}else{
			gameInfo.yellowWin = check_win_cond();

			if(gameInfo.yellowWin) {
				gameInfo.numYellowWins++;
				setGameData(gameInfo);
				return;
			}
		}

		gameInfo.tokenLocation.x = 0;
		gameInfo.tokenLocation.y = 0;
		gameInfo.tokenLocation.column = 3;

		if(gameInfo.currentPlayer == RED_PLAYER){
			gameInfo.currentPlayer = YELLOW_PLAYER;
		}else{
			gameInfo.currentPlayer = RED_PLAYER;
		}

		setGameData(gameInfo);
	}
}

int countMatchesInEachDirection(uint8_t currPlayer, uint8_t currRow, uint8_t currCol, int rowDirection, int colDirection){
	GameData gameInfo = getGameData();

	int count = 0;
	for(int i = 1; i < 4; i++) {
		int row = currRow + (rowDirection * i);
		int col = currCol + (colDirection * i);


		//clamping bounds of board
		if(row < 0 || row >= NUM_ROWS){
			break;
		}
		if(col < 0 || col >= NUM_COLUMNS){
			break;
		}

		//check if token color matches player
		if(currPlayer == RED_PLAYER){
			if (gameInfo.board[row][col] == RED_TAKEN_SQUARE){
				count++;
			}else{
				break;
			}
		}else if(currPlayer == YELLOW_PLAYER){
			if(gameInfo.board[row][col] == YELLOW_TAKEN_SQUARE){
				count++;
			}else{
				break;
			}
		}

	}
	return count;
}

bool check_win_cond(){
	int directions[4][2] = {
	    {0, 1},   // Horizontal
	    {1, 0},   // Vertical
	    {1, 1},   // Diagonal down-right
	    {-1, 1}   // Diagonal up-right
	};

	GameData gameInfo = getGameData();

	for (int i = 0; i < 4; i++) {
		 int count = 1;

		int row = directions[i][0];
	    int col = directions[i][1];


		//checking in both directions
		count += countMatchesInEachDirection(gameInfo.currentPlayer, gameInfo.tokenLocation.row, gameInfo.tokenLocation.column, row, col);
		count += countMatchesInEachDirection(gameInfo.currentPlayer, gameInfo.tokenLocation.row, gameInfo.tokenLocation.column, -row, -col);

		if (count >= 4){
			return true;
		}
	}
	return false;
}

bool check_possible_win_cond(uint8_t currPlayer, uint8_t currRow, uint8_t currCol){
	int directions[4][2] = {
	    {0, 1},   // Horizontal
	    {1, 0},   // Vertical
	    {1, 1},   // Diagonal down-right
	    {-1, 1}   // Diagonal up-right
	};


	for (int i = 0; i < 4; i++) {
		int count = 1;

		int row = directions[i][0];
	    int col = directions[i][1];


		//checking in both directions
		count += countMatchesInEachDirection(currPlayer, currRow, currCol, row, col);
		count += countMatchesInEachDirection(currPlayer, currRow, currCol, -row, -col);

		if(count >= 4){
			return true;
		}
	}
	return false;
}

bool check_advanced_possible_win_cond(uint8_t currPlayer, uint8_t currRow, uint8_t currCol){
	int directions[4][2] = {
	    {0, 1},   // Horizontal
	    {1, 0},   // Vertical
	    {1, 1},   // Diagonal down-right
	    {-1, 1}   // Diagonal up-right
	};

	for (int i = 0; i < 4; i++) {
		int count = 1;

		int row = directions[i][0];
	    int col = directions[i][1];


		//checking in both directions
		count += countMatchesInEachDirection(currPlayer, currRow, currCol, row, col);
		count += countMatchesInEachDirection(currPlayer, currRow, currCol, -row, -col);

		if(count >= 3){
			return true;
		}
	}

	return false;
}


bool isColumnFull(uint8_t column){
	GameData gameInfo = getGameData();
	for(int i = 5; i >= 0; i--){
		if(gameInfo.board[i][column] == EMPTY_SQUARE){
			return false;
		}
	}
	return true;
}

bool isBoardFull(){
	for(int i = 0; i < NUM_COLUMNS; i++){
		if(!isColumnFull(i)){
			return false;
		}
	}
	return true;
}

int calculateTotalTime(){
	GameData gameInfo = getGameData();
	uint32_t totalTime = (gameInfo.endTime - gameInfo.startTime);
	uint32_t totalTimeInSeconds = roundToSeconds(totalTime);
	return (int) totalTimeInSeconds;
}

uint32_t roundToSeconds(uint32_t value)
{
    if (value < 1000)
        return value;

    return ((value + 500) / 1000);
}

void OpponentPlayRNG(){
	uint32_t rand = getRandomInRange(0, 10);
	for(int i = 0; i < rand; i++){
		HAL_Delay(200);
		uint32_t moveTokenRand = getRandomInRange(0,1);
		if(moveTokenRand == 0){
			movePendingToken(LEFT);
		}else{
			movePendingToken(RIGHT);
		}
	}
	HAL_Delay(200);
	dropToken(LCD_COLOR_YELLOW);
}

void OpponentPlayAI(){
	GameData gameInfo = getGameData();

	for(int c = 0; c < NUM_COLUMNS; c++){
		int r = NUM_ROWS - 1;
		for(r = NUM_ROWS - 1; r >= 0; r--){
			if(gameInfo.board[r][c] == EMPTY_SQUARE){
				if(check_possible_win_cond(YELLOW_PLAYER, r, c)){
					gameInfo.tokenLocation.row = r;
					gameInfo.tokenLocation.column = c;
					setGameData(gameInfo);
					dropToken(LCD_COLOR_YELLOW);
					return;
				}
				break;
			}
		}
	}

	for(int c = 0; c < NUM_COLUMNS; c++){
		int r = NUM_ROWS - 1;
		for(r = NUM_ROWS - 1; r >= 0; r--){
			if(gameInfo.board[r][c] == EMPTY_SQUARE){
				if(check_possible_win_cond(RED_PLAYER, r, c)){
					gameInfo.tokenLocation.row = r;
					gameInfo.tokenLocation.column = c;
					setGameData(gameInfo);
					dropToken(LCD_COLOR_YELLOW);
					return;
				}
				break;
			}
		}
	}
#if(PLAY_AI == 2)
	for(int c = 0; c < NUM_COLUMNS; c++){
		int r = NUM_ROWS - 1;
		for(r = NUM_ROWS - 1; r >= 0; r--){
			if(gameInfo.board[r][c] == EMPTY_SQUARE){
				if(check_advanced_possible_win_cond(YELLOW_PLAYER, r, c)){
					gameInfo.tokenLocation.row = r;
					gameInfo.tokenLocation.column = c;
					setGameData(gameInfo);
					dropToken(LCD_COLOR_YELLOW);
					return;
				}else if(check_advanced_possible_win_cond(RED_PLAYER, r, c)){
					gameInfo.tokenLocation.row = r;
					gameInfo.tokenLocation.column = c;
					setGameData(gameInfo);
					dropToken(LCD_COLOR_YELLOW);
					return;
				}
				break;
			}
		}
	}
#endif

	OpponentPlayRNG();
}

uint32_t getRandomInRange(uint32_t min, uint32_t max)
{
    uint32_t randNum;
    uint32_t bound = (max - min + 1);

    HAL_RNG_GenerateRandomNumber(&hrng, &randNum);


    return min + (randNum % bound);
}

void setGameData(GameData gameData){
	gameInfo = gameData;
}

GameData getGameData(){
	return gameInfo;
}


