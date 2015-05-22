/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include "stm32f1xx.h"
#include "stm32f1xx_nucleo.h"
#include "ks0073_lcd_drv.h"
#include <stdlib.h>
			


/* Private function prototypes -----------------------------------------------*/

typedef struct {
	uint8_t Step;
	uint8_t yValue;
	uint8_t Heigth;
} HindernisTypeDef;

void SystemClock_Config(void);
void drawCopter(KS0073_GraphicAreaSmallTypeDef * area, uint8_t x, uint8_t y);
void scrollLeft(KS0073_GraphicAreaSmallTypeDef * area);
int8_t getverticalSpeed();
void placeObstacle();

static KS0073_GraphicAreaSmallTypeDef BG, Obj, Screen;
static int8_t button_history[4];
static uint8_t y_copter = 0, level = 1;
static uint32_t StepCnt = 0;
#define MAX_RAND 100
#define SEED 42
#define SPEED 100





/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  */
int main(void)
{

	/* STM32F103xB HAL library initialization:
	   - Configure the Flash prefetch
	   - Systick timer is configured by default as source of time base, but user
		 can eventually implement his proper time base source (a general purpose
		 timer for example or other time source), keeping in mind that Time base
		 duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and
		 handled in milliseconds basis.
	   - Set NVIC Group Priority to 4
	   - Low Level Initialization
	 */
	HAL_Init();

	/* Configure the system clock to 64 MHz */
	SystemClock_Config();

	/* Init Display */
	KS0073_Init(KS0073_CursorOff, KS0073_BlinkOff);
	GPIO_InitTypeDef ButtonInit;
	ButtonInit.Mode = GPIO_MODE_INPUT;
	ButtonInit.Pin = GPIO_PIN_13;
	ButtonInit.Pull = GPIO_NOPULL;
	ButtonInit.Speed = GPIO_SPEED_HIGH;

	HAL_GPIO_Init(GPIOC, &ButtonInit);
	__HAL_RCC_GPIOC_CLK_ENABLE();
	y_copter = 4;
	drawCopter(&Obj, 3, y_copter);
	getverticalSpeed();
	getverticalSpeed();
	getverticalSpeed();
	getverticalSpeed();
	KS0073_gotoxy(4,0);
	KS0073_puts("STM32Copter");
	KS0073_gotoxy(2,2);
	KS0073_puts("Blue Button = ");
	KS0073_putc(0xDE);
	KS0073_gotoxy(1,3);
	KS0073_puts("Hold Blue to Play!");
	//Wait for Press to start
	int8_t i = -4;
	int8_t startlevel = 1, trigger = 1;
	while(i < 4)
	{
		KS0073_gotoxy(1,1);
		KS0073_puts("Startlevel: ");
		KS0073_put_int(startlevel);
		i = getverticalSpeed();
		if(i > -3 && trigger == 0 && HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_13) == GPIO_PIN_SET)
		{
			trigger = 1;
			startlevel++;
		}
		if(i == -4)
			trigger = 0;
		HAL_Delay(125);
		/*KS0073_gotoxy(0,1);
		if(i < 0){
			KS0073_putc('-');
			KS0073_put_int(-i);
		} else {
			KS0073_putc('+');
			KS0073_put_int(i);
		}
		KS0073_gotoxy(8,1);
		KS0073_put_int(trigger);*/
	}
	KS0073_clearScreen();
	KS0073_gotoxy(4,0);
	KS0073_putc(0x14);
	KS0073_gotoxy(4,1);
	KS0073_putc(0x14);
	StepCnt=100 * (startlevel - 1);
	while(1)
	{
		while(HAL_GetTick() % SPEED/level != 0)
			;

		int8_t i = getverticalSpeed()/2;
		if(i > 1) i = 1;
		if(i < -1) i = -1;
		KS0073_gotoxy(0,3);
		/*if(i < 0)
		{

			KS0073_putc('-');
		} else {
			KS0073_gotoxy(0,3);
			KS0073_putc('+');
		}
		if(i == 0)
			KS0073_putc('0');
		if(i == 1 || i == -1)
					KS0073_putc('1');
		if(i == 2 || i == -2)
					KS0073_putc('2');*/
		y_copter += i;
		if(y_copter < 2) y_copter = 2;
		if(y_copter == 255) y_copter = 2;
		if(y_copter > 14) y_copter = 14;
		KS0073_ClearGraphicArea(&Obj);
		drawCopter(&Obj, 2, y_copter);
		KS0073_puts("H");
		KS0073_putc(0x7C);
		KS0073_puts("he:");
		KS0073_put_int(y_copter);
		KS0073_puts(" Level:");
		KS0073_put_int(level);
		placeObstacle();
		KS0073_ClearGraphicArea(&Screen);
		KS0073_MergeGraphicAreas(&Screen, &Obj);
		KS0073_MergeGraphicAreas(&Screen, &BG);
		KS0073_PrintGraphicArea(&Screen, 0, 0);
		if(KS0073_CollisionTest(&BG, &Obj))
		{
			KS0073_gotoxy(0,2);
			KS0073_puts("Collision!!!");
			while(1)
				;

		} else {
			KS0073_gotoxy(0,2);
			KS0073_puts("No Collision");
		}
		scrollLeft(&BG);
		StepCnt++;
		KS0073_gotoxy(9, 0);
		KS0073_puts("Score:");
		KS0073_gotoxy(9, 1);
		KS0073_put_int(StepCnt);
	}
	return 0;
}





/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSI)
  *            SYSCLK(Hz)                     = 64000000
  *            HCLK(Hz)                       = 64000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 2
  *            APB2 Prescaler                 = 1
  *            PLLMUL                         = 16
  *            Flash Latency(WS)              = 2
  */
void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef clkinitstruct = {0};
  RCC_OscInitTypeDef oscinitstruct = {0};

  /* Configure PLL ------------------------------------------------------*/
  /* PLL configuration: PLLCLK = (HSI / 2) * PLLMUL = (8 / 2) * 16 = 64 MHz */
  /* PREDIV1 configuration: PREDIV1CLK = PLLCLK / HSEPredivValue = 64 / 1 = 64 MHz */
  /* Enable HSI and activate PLL with HSi_DIV2 as source */
  oscinitstruct.OscillatorType  = RCC_OSCILLATORTYPE_HSI;
  oscinitstruct.HSEState        = RCC_HSE_OFF;
  oscinitstruct.LSEState        = RCC_LSE_OFF;
  oscinitstruct.HSIState        = RCC_HSI_ON;
  oscinitstruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  oscinitstruct.HSEPredivValue    = RCC_HSE_PREDIV_DIV1;
  oscinitstruct.PLL.PLLState    = RCC_PLL_ON;
  oscinitstruct.PLL.PLLSource   = RCC_PLLSOURCE_HSI_DIV2;
  oscinitstruct.PLL.PLLMUL      = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&oscinitstruct)!= HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  clkinitstruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  clkinitstruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  clkinitstruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  clkinitstruct.APB2CLKDivider = RCC_HCLK_DIV2;
  clkinitstruct.APB1CLKDivider = RCC_HCLK_DIV2;
  if (HAL_RCC_ClockConfig(&clkinitstruct, FLASH_LATENCY_2)!= HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }
}

void assert_failed(uint8_t* file, uint32_t line)
{
	line = *file;
}

void drawCopter(KS0073_GraphicAreaSmallTypeDef * area, uint8_t x, uint8_t y)
{
	KS0073_DrawHorizontalLine(area, x - 2, y + 1, 5);
	KS0073_DrawHorizontalLine(area, x, y, 1);
	KS0073_DrawHorizontalLine(area, x-2, y-1, 1);
	KS0073_DrawHorizontalLine(area, x, y-1, 2);
	KS0073_DrawHorizontalLine(area, x-1, y-2,4);

}


void scrollLeft(KS0073_GraphicAreaSmallTypeDef * area)
{
	uint8_t i;
	for(i = 0; i < 16; i++)
	{
		area->Line[i] <<= 1;
	}
}


int8_t getverticalSpeed()
{
	button_history[3] = button_history[2];
	button_history[2] = button_history[1];
	button_history[1] = button_history[0];
	if(HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_13) == GPIO_PIN_RESET)
	{
		button_history[0] = 1;
	} else {
		button_history[0] = -1;
	}
	return (button_history[0] + button_history[1] + button_history[2] + button_history[3]);
}

void placeObstacle()
{
	uint8_t div = (30/level);
	if(div < 10) div = 10;
	if(StepCnt%div == 0)
	{
		uint8_t heigth = rand() % 5+level;
		uint8_t pos = rand() % (15 - heigth);
		KS0073_DrawVerticalLine(&BG, 19, pos, heigth);
	}
	level = (StepCnt/100)+1;
	if(level > 5) level = 5;
}
