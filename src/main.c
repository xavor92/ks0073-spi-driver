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
			


/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* Private functions ---------------------------------------------------------*/


const KS0073_FontTypeDef SpInv1al = {
		.line[0] = 0x01,
		.line[1] = 0x03,
		.line[2] = 0x07,
		.line[3] = 0x0D,
		.line[4] = 0x0F,
		.line[5] = 0x02,
		.line[6] = 0x05,
		.line[7] = 0x0A,
};

const KS0073_FontTypeDef SpInv1ar = {
		.line[0] = 0x10,
		.line[1] = 0x18,
		.line[2] = 0x1C,
		.line[3] = 0x16,
		.line[4] = 0x1E,
		.line[5] = 0x08,
		.line[6] = 0x14,
		.line[7] = 0x0A,
};
const KS0073_FontTypeDef SpInv1bl = {
		.line[0] = 0x01,
		.line[1] = 0x03,
		.line[2] = 0x07,
		.line[3] = 0x0D,
		.line[4] = 0x0F,
		.line[5] = 0x05,
		.line[6] = 0x08,
		.line[7] = 0x04,
};

const KS0073_FontTypeDef SpInv1br = {
		.line[0] = 0x10,
		.line[1] = 0x18,
		.line[2] = 0x1C,
		.line[3] = 0x16,
		.line[4] = 0x1E,
		.line[5] = 0x14,
		.line[6] = 0x02,
		.line[7] = 0x04,
};

const KS0073_FontTypeDef SpInv2al = {
		.line[0] = 0x04,
		.line[1] = 0x12,
		.line[2] = 0x17,
		.line[3] = 0x1D,
		.line[4] = 0x1F,
		.line[5] = 0x0F,
		.line[6] = 0x04,
		.line[7] = 0x08,
};

const KS0073_FontTypeDef SpInv2ar = {
		.line[0] = 0x04,
		.line[1] = 0x09,
		.line[2] = 0x1D,
		.line[3] = 0x17,
		.line[4] = 0x1F,
		.line[5] = 0x1E,
		.line[6] = 0x04,
		.line[7] = 0x02,
};

const KS0073_FontTypeDef SpInv2bl = {
		.line[0] = 0x04,
		.line[1] = 0x02,
		.line[2] = 0x07,
		.line[3] = 0x0D,
		.line[4] = 0x1F,
		.line[5] = 0x17,
		.line[6] = 0x14,
		.line[7] = 0x03,
};

const KS0073_FontTypeDef SpInv2br = {
		.line[0] = 0x04,
		.line[1] = 0x08,
		.line[2] = 0x1C,
		.line[3] = 0x16,
		.line[4] = 0x1F,
		.line[5] = 0x1D,
		.line[6] = 0x05,
		.line[7] = 0x18,
};



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

	/* Sample Text */
	KS0073_puts("KS0073 Display\nusing SPI Inferface\n");
	KS0073_newLine();
	KS0073_puts("for STM32 Family");
	KS0073_gotoxy(0, 2);
	KS0073_puts("Software by Olli W.");

	/* grafik test */
	KS0073_setFont(0, &SpInv1al);
	KS0073_setFont(1, &SpInv1ar);
	KS0073_setFont(2, &SpInv1bl);
	KS0073_setFont(3, &SpInv1br);
	KS0073_setFont(4, &SpInv2al);
	KS0073_setFont(5, &SpInv2ar);
	KS0073_setFont(6, &SpInv2bl);
	KS0073_setFont(7, &SpInv2br);

	KS0073_clearScreen();
	while (1)
	{
		KS0073_gotoxy(7,1);
		KS0073_putc(0);
		KS0073_putc(1);
		KS0073_gotoxy(11,2);
		KS0073_putc(4);
		KS0073_putc(5);
		HAL_Delay(500);
		KS0073_gotoxy(7,1);
		KS0073_putc(2);
		KS0073_putc(3);
		KS0073_gotoxy(11,2);
		KS0073_putc(6);
		KS0073_putc(7);
		HAL_Delay(500);
	}
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
