/*
 * LCD_Driver.c
 *
 *  Created on: Sep 28, 2023
 *      Author: Xavion
 */

#include "LCD_Driver.h"

/**
  * @brief LTDC Initialization Function
  * @param None
  * @retval None
  */

static LTDC_HandleTypeDef hltdc;
static RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;
static FONT_t *LCD_Currentfonts;
static uint16_t CurrentTextColor   = 0xFFFF;


/*
 * fb[y*W+x] OR fb[y][x]
 * Alternatively, we can modify the linker script to have an end address of 20013DFB instead of 2002FFFF, so it does not place variables in the same region as the frame buffer. In this case it is safe to just specify the raw address as frame buffer.
 */
//uint32_t frameBuffer[(LCD_PIXEL_WIDTH*LCD_PIXEL_WIDTH)/2] = {0};		//16bpp pixel format. We can size to uint32. this ensures 32 bit alignment


//Someone from STM said it was "often accessed" a 1-dim array, and not a 2d array. However you still access it like a 2dim array,  using fb[y*W+x] instead of fb[y][x].
uint16_t frameBuffer[LCD_PIXEL_WIDTH*LCD_PIXEL_HEIGHT] = {0};			//16bpp pixel format.


void LCD_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable the LTDC clock */
  __HAL_RCC_LTDC_CLK_ENABLE();

  /* Enable GPIO clock */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /* GPIO Config
   *
    LCD pins
   LCD_TFT R2 <-> PC.10
   LCD_TFT G2 <-> PA.06
   LCD_TFT B2 <-> PD.06
   LCD_TFT R3 <-> PB.00
   LCD_TFT G3 <-> PG.10
   LCD_TFT B3 <-> PG.11
   LCD_TFT R4 <-> PA.11
   LCD_TFT G4 <-> PB.10
   LCD_TFT B4 <-> PG.12
   LCD_TFT R5 <-> PA.12
   LCD_TFT G5 <-> PB.11
   LCD_TFT B5 <-> PA.03
   LCD_TFT R6 <-> PB.01
   LCD_TFT G6 <-> PC.07
   LCD_TFT B6 <-> PB.08
   LCD_TFT R7 <-> PG.06
   LCD_TFT G7 <-> PD.03
   LCD_TFT B7 <-> PB.09
   LCD_TFT HSYNC <-> PC.06
   LCDTFT VSYNC <->  PA.04
   LCD_TFT CLK   <-> PG.07
   LCD_TFT DE   <->  PF.10
  */

  /* GPIOA configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_6 |
                           GPIO_PIN_11 | GPIO_PIN_12;
  GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
  GPIO_InitStructure.Alternate= GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

 /* GPIOB configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_8 | \
                           GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

 /* GPIOC configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_10;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

 /* GPIOD configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_3 | GPIO_PIN_6;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);

 /* GPIOF configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_10;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStructure);

 /* GPIOG configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_6 | GPIO_PIN_7 | \
                           GPIO_PIN_11;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);

  /* GPIOB configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1;
  GPIO_InitStructure.Alternate= GPIO_AF9_LTDC;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* GPIOG configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_10 | GPIO_PIN_12;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);
}

void LTCD_Layer_Init(uint8_t LayerIndex)
{
	LTDC_LayerCfgTypeDef  pLayerCfg;

	pLayerCfg.WindowX0 = 0;	//Configures the Window HORZ START Position.
	pLayerCfg.WindowX1 = LCD_PIXEL_WIDTH;	//Configures the Window HORZ Stop Position.
	pLayerCfg.WindowY0 = 0;	//Configures the Window vertical START Position.
	pLayerCfg.WindowY1 = LCD_PIXEL_HEIGHT;	//Configures the Window vertical Stop Position.
	pLayerCfg.PixelFormat = LCD_PIXEL_FORMAT_1;  //INCORRECT PIXEL FORMAT WILL GIVE WEIRD RESULTS!! IT MAY STILL WORK FOR 1/2 THE DISPLAY!!! //This is our buffers pixel format. 2 bytes for each pixel
	pLayerCfg.Alpha = 255;
	pLayerCfg.Alpha0 = 0;
	pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
	pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
	if (LayerIndex == 0){
		pLayerCfg.FBStartAdress = (uintptr_t)frameBuffer;
	}
	pLayerCfg.ImageWidth = LCD_PIXEL_WIDTH;
	pLayerCfg.ImageHeight = LCD_PIXEL_HEIGHT;
	pLayerCfg.Backcolor.Blue = 0;
	pLayerCfg.Backcolor.Green = 0;
	pLayerCfg.Backcolor.Red = 0;
	if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, LayerIndex) != HAL_OK)
	{
		LCD_Error_Handler();
	}
}

void clearScreen(void)
{
  LCD_Clear(0,LCD_COLOR_WHITE);
}

void LTCD__Init(void)
{
	hltdc.Instance = LTDC;
	/* Configure horizontal synchronization width */
	hltdc.Init.HorizontalSync = ILI9341_HSYNC;
	/* Configure vertical synchronization height */
	hltdc.Init.VerticalSync = ILI9341_VSYNC;
	/* Configure accumulated horizontal back porch */
	hltdc.Init.AccumulatedHBP = ILI9341_HBP;
	/* Configure accumulated vertical back porch */
	hltdc.Init.AccumulatedVBP = ILI9341_VBP;
	/* Configure accumulated active width */
	hltdc.Init.AccumulatedActiveW = 269;
	/* Configure accumulated active height */
	hltdc.Init.AccumulatedActiveH = 323;
	/* Configure total width */
	hltdc.Init.TotalWidth = 279;
	/* Configure total height */
	hltdc.Init.TotalHeigh = 327;
	/* Configure R,G,B component values for LCD background color */
	hltdc.Init.Backcolor.Red = 0;
	hltdc.Init.Backcolor.Blue = 0;
	hltdc.Init.Backcolor.Green = 0;

	/* LCD clock configuration */
	/* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 Mhz */
	/* PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAIN = 192 Mhz */
	/* PLLLCDCLK = PLLSAI_VCO Output/PLLSAIR = 192/4 = 48 Mhz */
	/* LTDC clock frequency = PLLLCDCLK / LTDC_PLLSAI_DIVR_8 = 48/4 = 6Mhz */

	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
	PeriphClkInitStruct.PLLSAI.PLLSAIN = 192;
	PeriphClkInitStruct.PLLSAI.PLLSAIR = 4;
	PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_8;
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
	/* Polarity */
	hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
	hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
	hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
	hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;

	LCD_GPIO_Init();

	if (HAL_LTDC_Init(&hltdc) != HAL_OK)
	 {
	   LCD_Error_Handler();
	 }

	ili9341_Init();
}

/* START Draw functions */


/*
 * This is really the only function needed.
 * All drawing consists of is manipulating the array.
 * Adding input sanitation should probably be done.
 */
void LCD_Draw_Pixel(uint16_t x, uint16_t y, uint16_t color)
{
	frameBuffer[y*LCD_PIXEL_WIDTH+x] = color;  //You cannot do x*y to set the pixel.
}

/*
 * These functions are simple examples. Most computer graphics like OpenGl and stm's graphics library use a state machine. Where you first call some function like SetColor(color), SetPosition(x,y), then DrawSqure(size)
 * Instead all of these are explicit where color, size, and position are passed in.
 * There is tons of ways to handle drawing. I dont think it matters too much.
 */
void LCD_Draw_Circle_Fill(uint16_t Xpos, uint16_t Ypos, uint16_t radius, uint16_t color)
{
    for(int16_t y=-radius; y<=radius; y++)
    {
        for(int16_t x=-radius; x<=radius; x++)
        {
            if(x*x+y*y <= radius*radius)
            {
            	LCD_Draw_Pixel(x+Xpos, y+Ypos, color);
            }
        }
    }
}

void LCD_Draw_Vertical_Line(uint16_t x, uint16_t y, uint16_t len, uint16_t color)
{
  for (uint16_t i = 0; i < len; i++){
	  LCD_Draw_Pixel(x, i+y, color);
  }
}

void LCD_Draw_Horizontal_Line(uint16_t x, uint16_t y, uint16_t len, uint16_t color){
	for (uint16_t i = 0; i < len; i++){
	  LCD_Draw_Pixel(i+x, y, color);
	}
}

void LCD_Draw_Box(uint16_t x, uint16_t y, uint16_t height, uint16_t width, uint16_t color)
{
  for (uint16_t i = 0; i < height; i++){
	  for(uint16_t j = 0; j < width; j++){
		  LCD_Draw_Pixel(x+j, i+y, color);
	  }
  }
}

void LCD_Clear(uint8_t LayerIndex, uint16_t Color)
{
	if (LayerIndex == 0){
		for (uint32_t i = 0; i < LCD_PIXEL_WIDTH * LCD_PIXEL_HEIGHT; i++){
			frameBuffer[i] = Color;
		}
	}
  // TODO: Add more Layers if needed
}

//This was taken and adapted from stm32's mcu code
void LCD_SetTextColor(uint16_t Color)
{
  CurrentTextColor = Color;
}

//This was taken and adapted from stm32's mcu code
void LCD_SetFont(FONT_t *fonts)
{
  LCD_Currentfonts = fonts;
}

//This was taken and adapted from stm32's mcu code
void LCD_Draw_Char(uint16_t Xpos, uint16_t Ypos, const uint16_t *c)
{
  uint32_t index = 0, counter = 0;
  for(index = 0; index < LCD_Currentfonts->Height; index++)
  {
    for(counter = 0; counter < LCD_Currentfonts->Width; counter++)
    {
      if((((c[index] & ((0x80 << ((LCD_Currentfonts->Width / 12 ) * 8 ) ) >> counter)) == 0x00) && (LCD_Currentfonts->Width <= 12)) || (((c[index] & (0x1 << counter)) == 0x00)&&(LCD_Currentfonts->Width > 12 )))
      {
         //Background If want to overrite text under then add a set color here
      }
      else
      {
    	  LCD_Draw_Pixel(counter + Xpos,index + Ypos,CurrentTextColor);
      }
    }
  }
}

//This was taken and adapted from stm32's mcu code
void LCD_DisplayChar(uint16_t Xpos, uint16_t Ypos, uint8_t Ascii)
{
  Ascii -= 32;
  LCD_Draw_Char(Xpos, Ypos, &LCD_Currentfonts->table[Ascii * LCD_Currentfonts->Height]);
}

void LCD_DisplayString(uint16_t Xpos, uint16_t Ypos, const char* str){
	while(*str){
		LCD_DisplayChar(Xpos, Ypos, (uint8_t)*str);

		Xpos += LCD_Currentfonts->Width;
		str++;
	}
}


void visualDemo(void)
{
	uint16_t x;
	uint16_t y;
	// This for loop just illustrates how with using logic and for loops, you can create interesting things
	// this may or not be useful ;)
	for(y=0; y<LCD_PIXEL_HEIGHT; y++){
		for(x=0; x < LCD_PIXEL_WIDTH; x++){
			if (x & 32)
				frameBuffer[x*y] = LCD_COLOR_WHITE;
			else
				frameBuffer[x*y] = LCD_COLOR_BLACK;
		}
	}

	HAL_Delay(1500);
	LCD_Clear(0, LCD_COLOR_GREEN);
	HAL_Delay(1500);
	LCD_Clear(0, LCD_COLOR_RED);
	HAL_Delay(1500);
	LCD_Clear(0, LCD_COLOR_WHITE);
	LCD_Draw_Vertical_Line(10,10,250,LCD_COLOR_MAGENTA);
	HAL_Delay(1500);
	LCD_Draw_Vertical_Line(230,10,250,LCD_COLOR_MAGENTA);
	HAL_Delay(1500);

	LCD_Draw_Circle_Fill(125,150,20,LCD_COLOR_BLACK);
	HAL_Delay(2000);

	LCD_Clear(0,LCD_COLOR_BLUE);
	LCD_SetTextColor(LCD_COLOR_BLACK);
	LCD_SetFont(&Font16x24);

	LCD_DisplayChar(100,140,'H');
	LCD_DisplayChar(115,140,'e');
	LCD_DisplayChar(125,140,'l');
	LCD_DisplayChar(130,140,'l');
	LCD_DisplayChar(140,140,'o');

	LCD_DisplayChar(100,160,'W');
	LCD_DisplayChar(115,160,'o');
	LCD_DisplayChar(125,160,'r');
	LCD_DisplayChar(130,160,'l');
	LCD_DisplayChar(140,160,'d');
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void LCD_Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

// Touch Functionality   //

#if COMPILE_TOUCH_FUNCTIONS == 1

void InitializeLCDTouch(void){
  if(STMPE811_Init() != STMPE811_State_Ok)
  {
	 for(;;); // Hang code due to error in initialzation
  }
}

STMPE811_State_t returnTouchStateAndLocation(STMPE811_TouchData * touchStruct){
	return STMPE811_ReadTouch(touchStruct);
}

void DetermineTouchPosition(STMPE811_TouchData* touchStruct){
	STMPE811_DetermineTouchPosition(touchStruct);
}

uint8_t ReadRegisterFromTouchModule(uint8_t RegToRead){
	return STMPE811_Read(RegToRead);
}

void WriteDataToTouchModule(uint8_t RegToWrite, uint8_t writeData){
	STMPE811_Write(RegToWrite, writeData);
}

uint8_t DetermineLeftOrRightTouch(uint16_t xCoord){
	if(xCoord < (LCD_PIXEL_WIDTH/2)){
		return LEFT;
	}else{
		return RIGHT;
	}
}


void displayStartScreen(){
	LCD_Clear(0,LCD_COLOR_BLACK);
	LCD_SetTextColor(LCD_COLOR_WHITE);
	LCD_SetFont(&Font16x24);

	LCD_Draw_Circle_Fill(60,LCD_PIXEL_HEIGHT/2,50,LCD_COLOR_BLUE);

	LCD_DisplayChar(30,(LCD_PIXEL_HEIGHT/2) - 20,'S');
	LCD_DisplayChar(41,(LCD_PIXEL_HEIGHT/2) - 20,'i');
	LCD_DisplayChar(50,(LCD_PIXEL_HEIGHT/2) - 20,'n');
	LCD_DisplayChar(64,(LCD_PIXEL_HEIGHT/2) - 20,'g');
	LCD_DisplayChar(73,(LCD_PIXEL_HEIGHT/2) - 20,'l');
	LCD_DisplayChar(80,(LCD_PIXEL_HEIGHT/2) - 20,'e');

	LCD_DisplayChar(30,(LCD_PIXEL_HEIGHT/2),'P');
	LCD_DisplayChar(41,(LCD_PIXEL_HEIGHT/2),'l');
	LCD_DisplayChar(49,(LCD_PIXEL_HEIGHT/2),'a');
	LCD_DisplayChar(59,(LCD_PIXEL_HEIGHT/2),'y');
	LCD_DisplayChar(72,(LCD_PIXEL_HEIGHT/2),'e');
	LCD_DisplayChar(83,(LCD_PIXEL_HEIGHT/2),'r');

	LCD_Draw_Circle_Fill(180,LCD_PIXEL_HEIGHT/2,50,LCD_COLOR_BLUE);

	LCD_DisplayChar(155,(LCD_PIXEL_HEIGHT/2) - 20,'M');
	LCD_DisplayChar(171,(LCD_PIXEL_HEIGHT/2) - 20,'u');
	LCD_DisplayChar(183,(LCD_PIXEL_HEIGHT/2) - 20,'l');
	LCD_DisplayChar(190,(LCD_PIXEL_HEIGHT/2) - 20,'t');
	LCD_DisplayChar(197,(LCD_PIXEL_HEIGHT/2) - 20,'i');

	LCD_DisplayChar(150,(LCD_PIXEL_HEIGHT/2),'P');
	LCD_DisplayChar(161,(LCD_PIXEL_HEIGHT/2),'l');
	LCD_DisplayChar(169,(LCD_PIXEL_HEIGHT/2),'a');
	LCD_DisplayChar(179,(LCD_PIXEL_HEIGHT/2),'y');
	LCD_DisplayChar(192,(LCD_PIXEL_HEIGHT/2),'e');
	LCD_DisplayChar(203,(LCD_PIXEL_HEIGHT/2),'r');

}

void displayGameScreen(){
	//Screen has 7 columns and 6 rows
	LCD_Clear(0,LCD_COLOR_BLACK);

	//Drawing Horizontal Lines
	for(int i = 1; i < NUM_ROWS + 1; i++){
		LCD_Draw_Horizontal_Line(0, (LCD_PIXEL_HEIGHT * ROW_NUM_DIVISOR) * i, LCD_PIXEL_WIDTH, LCD_COLOR_WHITE);
	}
	LCD_Draw_Horizontal_Line(0, LCD_PIXEL_HEIGHT - 1, LCD_PIXEL_WIDTH, LCD_COLOR_WHITE);

	//Drawing vertical Lines
	for(int i = 0; i < NUM_COLUMNS; i++){
		LCD_Draw_Vertical_Line((LCD_PIXEL_WIDTH * COLUMN_NUM_DIVISOR) * i, LCD_PIXEL_HEIGHT * COLUMN_NUM_DIVISOR, (LCD_PIXEL_HEIGHT * 6) * COLUMN_NUM_DIVISOR, LCD_COLOR_WHITE);
	}
	LCD_Draw_Vertical_Line(LCD_PIXEL_WIDTH - 1, LCD_PIXEL_HEIGHT * 1/7, LCD_PIXEL_HEIGHT * 6/7, LCD_COLOR_WHITE);

	GameData gameInfo = getGameData();
	for(int i = 0; i < NUM_ROWS; i++){
		for(int j = 0; j < NUM_COLUMNS ; j++){
			if(gameInfo.board[i][j] == RED_TAKEN_SQUARE){
				gameInfo.tokenLocation.x = (LCD_PIXEL_WIDTH * COLUMN_NUM_DIVISOR * (j + CHIP_OFFSET));
				gameInfo.tokenLocation.y = (LCD_PIXEL_HEIGHT * ROW_NUM_DIVISOR * (i + 1 + CHIP_OFFSET));
				LCD_Draw_Circle_Fill(gameInfo.tokenLocation.x, gameInfo.tokenLocation.y, CHIP_RADIUS, LCD_COLOR_RED);
			}
			if(gameInfo.board[i][j] == YELLOW_TAKEN_SQUARE){
				gameInfo.tokenLocation.x = (LCD_PIXEL_WIDTH * COLUMN_NUM_DIVISOR * (j + CHIP_OFFSET));
				gameInfo.tokenLocation.y = (LCD_PIXEL_HEIGHT * ROW_NUM_DIVISOR * (i + 1 + CHIP_OFFSET));
				LCD_Draw_Circle_Fill(gameInfo.tokenLocation.x, gameInfo.tokenLocation.y, CHIP_RADIUS, LCD_COLOR_YELLOW);
			}
		}
	}
}

void displayWinner(){
	GameData gameInfo = getGameData();

	LCD_SetTextColor(LCD_COLOR_BLACK);
	LCD_SetFont(&Font16x24);
	if(gameInfo.redWin || gameInfo.yellowWin){
		if(gameInfo.redWin){
			LCD_Draw_Circle_Fill(LCD_PIXEL_WIDTH_CENTER, LCD_PIXEL_HEIGHT_CENTER, 100, LCD_COLOR_RED);

			LCD_DisplayChar(90,(LCD_PIXEL_HEIGHT/2) - 15,'R');
			LCD_DisplayChar(110,(LCD_PIXEL_HEIGHT/2) - 15,'e');
			LCD_DisplayChar(130,(LCD_PIXEL_HEIGHT/2) - 15,'d');
			gameInfo.numRedWins++;
		}
		else if(gameInfo.yellowWin){
			LCD_Draw_Circle_Fill(LCD_PIXEL_WIDTH_CENTER, LCD_PIXEL_HEIGHT_CENTER, 100, LCD_COLOR_YELLOW);

			LCD_DisplayChar(70,(LCD_PIXEL_HEIGHT/2) - 15,'Y');
			LCD_DisplayChar(90,(LCD_PIXEL_HEIGHT/2) - 15,'e');
			LCD_DisplayChar(110,(LCD_PIXEL_HEIGHT/2) - 15,'l');
			LCD_DisplayChar(130,(LCD_PIXEL_HEIGHT/2) - 15,'l');
			LCD_DisplayChar(150,(LCD_PIXEL_HEIGHT/2) - 15,'o');
			LCD_DisplayChar(170,(LCD_PIXEL_HEIGHT/2) - 15,'w');
			gameInfo.numYellowWins++;
		}

		LCD_DisplayChar(85,(LCD_PIXEL_HEIGHT/2) + 5,'W');
		LCD_DisplayChar(105,(LCD_PIXEL_HEIGHT/2) + 5,'i');
		LCD_DisplayChar(125,(LCD_PIXEL_HEIGHT/2) + 5,'n');
		LCD_DisplayChar(145,(LCD_PIXEL_HEIGHT/2) + 5,'s');
	}else{
		LCD_Draw_Circle_Fill(LCD_PIXEL_WIDTH_CENTER, LCD_PIXEL_HEIGHT_CENTER, 100, LCD_COLOR_GREY);

		LCD_DisplayChar(85,(LCD_PIXEL_HEIGHT/2) - 5,'D');
		LCD_DisplayChar(105,(LCD_PIXEL_HEIGHT/2) - 5,'r');
		LCD_DisplayChar(125,(LCD_PIXEL_HEIGHT/2) - 5,'a');
		LCD_DisplayChar(145,(LCD_PIXEL_HEIGHT/2) - 5,'w');
	}

	LCD_Draw_Box(40, 280, 35, 160, LCD_COLOR_BLUE);

	LCD_DisplayChar(60,290,'C');
	LCD_DisplayChar(75,290,'o');
	LCD_DisplayChar(90,290,'n');
	LCD_DisplayChar(105,290,'t');
	LCD_DisplayChar(120,290,'i');
	LCD_DisplayChar(135,290,'n');
	LCD_DisplayChar(150,290,'u');
	LCD_DisplayChar(165,290,'e');
}



void displayEndScreen(){
	LCD_Clear(0,LCD_COLOR_BLACK);
	LCD_SetTextColor(LCD_COLOR_WHITE);
	LCD_SetFont(&Font16x24);

	GameData gameInfo = getGameData();

	LCD_DisplayString(85, LCD_PIXEL_HEIGHT/8, "Wins");
	LCD_Draw_Horizontal_Line(80, (LCD_PIXEL_HEIGHT/8) + 23, 70, LCD_COLOR_WHITE);

	LCD_SetTextColor(LCD_COLOR_RED);
	LCD_DisplayString(20, LCD_PIXEL_HEIGHT/4, "Red:");

	char redWins[6];
	sprintf(redWins, "%d", gameInfo.numRedWins);

	LCD_DisplayString(80, LCD_PIXEL_HEIGHT/4, redWins);


	LCD_SetTextColor(LCD_COLOR_YELLOW);
	LCD_DisplayString(110, LCD_PIXEL_HEIGHT/4, "Yellow:");

	char yellowWins[6];
	sprintf(yellowWins, "%d", gameInfo.numYellowWins);

	LCD_DisplayString(220, LCD_PIXEL_HEIGHT/4, yellowWins);


	LCD_SetTextColor(LCD_COLOR_WHITE);
	LCD_DisplayChar(60,LCD_PIXEL_HEIGHT_CENTER,'T');
	LCD_DisplayChar(75,LCD_PIXEL_HEIGHT_CENTER,'i');
	LCD_DisplayChar(90,LCD_PIXEL_HEIGHT_CENTER,'m');
	LCD_DisplayChar(105,LCD_PIXEL_HEIGHT_CENTER,'e');
	LCD_DisplayChar(115,LCD_PIXEL_HEIGHT_CENTER,':');

	char time[12];
	sprintf(time ,"%ds", calculateTotalTime());

	LCD_DisplayString(130, LCD_PIXEL_HEIGHT_CENTER, time);


	LCD_Draw_Box(40, 280, 35, 160, LCD_COLOR_BLUE);

	LCD_DisplayChar(60,290,'C');
	LCD_DisplayChar(75,290,'o');
	LCD_DisplayChar(90,290,'n');
	LCD_DisplayChar(105,290,'t');
	LCD_DisplayChar(120,290,'i');
	LCD_DisplayChar(135,290,'n');
	LCD_DisplayChar(150,290,'u');
	LCD_DisplayChar(165,290,'e');

}




#endif // COMPILE_TOUCH_FUNCTIONS
