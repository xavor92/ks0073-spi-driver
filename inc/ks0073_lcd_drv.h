/**
  ******************************************************************************
  * @file    ks0073_lcd_drv.h
  * @author  Oliver Westermann
  * @version V1.0
  * @date    04.05.2015
  * @brief   Header and Config of KS0073-controlled LCD
  *
  * For Detailed Frame Description read KS0073 datasheet
  *
  * Before transferring real data, start byte has to be transferred. It is
  * composed of succeeding 5 “High” bits, read write control bit (R/W), register
  * selection bit (RS), and end bit that indicates the end of start byte.
  * Whenever succeeding 5 “High” bits are detected by KS0073, it resets the
  * serial transfer counter and prepares to receive next informations.
  *
  * Data has to be transfered in the HighNibble, for example 0x12 would be
  * transfered like this:
  *
  * 1. Byte: StartByte
  * 2. Byte: 0b0200 0000
  * 3. Byte: 0b1000 0000
  ******************************************************************************
*/

#ifndef KS0073_LCD_DRV_H_
#define KS0073_LCD_DRV_H_

/* Includes ------------------------------------------------------------------*/

#include "stm32f1xx_nucleo.h"

/* Defines -------------------------------------------------------------------*/

//Clocks
#define KS0073_SPI_CLK_ENABLE()			__HAL_RCC_SPI1_CLK_ENABLE()
#define KS0073_SPI_CLK_DISABLE()		__HAL_RCC_SPI1_CLK_DISABLE()
#define KS0073_BLE_CLK_ENABLE()			__HAL_RCC_GPIOC_CLK_ENABLE()
#define KS0073_BLE_CLK_DISABLE()		__HAL_RCC_GPIOC_CLK_DISABLE()
#define KS0073_SPI_SCK_CLK_ENABLE()		__HAL_RCC_GPIOA_CLK_ENABLE()
#define KS0073_SPI_SCK_CLK_DISABLE()	__HAL_RCC_GPIOA_CLK_DISABLE()
#define KS0073_SPI_MISO_CLK_ENABLE()	__HAL_RCC_GPIOA_CLK_ENABLE()
#define KS0073_SPI_MISO_CLK_DISABLE()	__HAL_RCC_GPIOA_CLK_DISABLE()
#define KS0073_SPI_MOSI_CLK_ENABLE()	__HAL_RCC_GPIOA_CLK_ENABLE()
#define KS0073_SPI_MOSI_CLK_DISABLE()	__HAL_RCC_GPIOA_CLK_DISABLE()
#define KS0073_SPI_NCS_CLK_ENABLE()		__HAL_RCC_GPIOC_CLK_ENABLE()
#define KS0073_SPI_NCS_CLK_DISABLE()	__HAL_RCC_GPIOC_CLK_DISABLE()

//SPI Config
#define KS0073_SPIx 					SPI1
#define KS0073_SPI_MODE 				SPI_MODE_MASTER
#define KS0073_SPI_BAUDRATEPRESCALER 	SPI_BAUDRATEPRESCALER_256
#define KS0073_SPI_DIRECTION 			SPI_DIRECTION_2LINES
#define KS0073_SPI_CLK_POLARITY 		SPI_POLARITY_HIGH
#define KS0073_SPI_CLK_PHASE 			SPI_PHASE_2EDGE
#define KS0073_SPI_DATASIZE 			SPI_DATASIZE_8BIT
#define KS0073_SPI_FIRSTBIT 			SPI_FIRSTBIT_LSB
#define KS0073_SPI_TIMODE 				SPI_TIMODE_DISABLE //TI-Mode not available on STM32F1xx
#define KS0073_SPI_CRC 					SPI_CRCCALCULATION_DISABLE
#define KS0073_SPI_NSS 					SPI_NSS_SOFT

//BackLigth Enable Pin

#define KS0073_BLE_PIN 					GPIO_PIN_5
#define KS0073_BLE_PORT 				GPIOC

//SCK, MISO, MOSI, NCS Pins
#define KS0073_SPI_SCK_PIN 				GPIO_PIN_5
#define KS0073_SPI_SCK_PORT				GPIOA
#define KS0073_SPI_MISO_PIN 			GPIO_PIN_6
#define KS0073_SPI_MISO_PORT			GPIOA
#define KS0073_SPI_MOSI_PIN 			GPIO_PIN_7
#define KS0073_SPI_MOSI_PORT			GPIOA
#define KS0073_SPI_NCS_PIN				GPIO_PIN_6
#define KS0073_SPI_NCS_PORT				GPIOC

//Bit Positions
#define KS0073_RW_BIT 					0x20
#define KS0073_RS_BIT					0x40

/* Exported types ------------------------------------------------------------*/

typedef struct
{
	uint8_t start;
	uint8_t data_low, data_high;
} KS0073_DataTypeDef;

typedef enum
{
	KS0073_RW_SET = 0x01,
	KS0073_RW_CLEAR = 0x00,
} KS0073_RWTypeDef;

typedef enum
{
	KS0073_RS_SET = 0x01,
	KS0073_RS_CLEAR = 0x00,
} KS0073_RSTypeDef;

typedef enum
{
	KS0073_CursorOff = 0x00,
	KS0073_CursorOn = 0x01
} KS0073_CursorTypeDef;

typedef enum
{
	KS0073_BlinkOff = 0x00,
	KS0073_BlinkOn = 0x00
}KS0073_BlinkTypeDef;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

extern void KS0073_Init(KS0073_CursorTypeDef Cursor, KS0073_BlinkTypeDef Blink);
extern void KS0073_DeInit();
extern void KS0073_Transmit_Byte(uint8_t Data, KS0073_RWTypeDef RW, KS0073_RSTypeDef RS, uint32_t Timeout);
extern void KS0073_clearScreen();

extern inline void KS0073_BL_Enable();
extern inline void KS0073_BL_Disable();
extern inline void KS0073_SPI_Enable();
extern inline void KS0073_SPI_Disable();

extern void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi);
extern void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi);

#endif /* KS0073_LCD_DRV_H_ */

