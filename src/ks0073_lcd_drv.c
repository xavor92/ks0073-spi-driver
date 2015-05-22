/**
  ******************************************************************************
  * @file    ks0073_lcd_drv.c
  * @author  Oliver Westermann
  * @version V1.0
  * @date    04.05.2015
  * @brief   Software for a 20x4 LCD connected via SPI
  ******************************************************************************
*/

#include "ks0073_lcd_drv.h"
/* defines ------------------------------------------------------------------- */

//rows of n set bits


/* local Variables ----------------------------------------------------------- */

GPIO_InitTypeDef GPIO_InitType;
SPI_HandleTypeDef SPI_Handle;

#ifndef KS0073_NO_DMA
DMA_HandleTypeDef hdma_rx, hdma_tx;
KS0073_DataTypeDef dma_tx_buffer[DMA_TX_BUFFER_SIZE];
uint8_t dma_tx_buffer_counter;
char * dma_tx_nextchar;
#endif //KS0073_NO_DMA

/* local Functions, Declarations --------------------------------------------- */

void DataConvert(uint8_t Data, KS0073_RWTypeDef RW, KS0073_RSTypeDef RS, KS0073_DataTypeDef * buffer);
uint8_t readData();
void setDDAddress(uint8_t DDAddress);
uint8_t readBusyFlag();

/* Functions ----------------------------------------------------------------- */

HAL_StatusTypeDef KS0073_puts_dma(char * nextchar)
{
	if(SPI_Handle.State == HAL_SPI_STATE_READY)
	{
		dma_tx_nextchar = nextchar;
		dma_tx_buffer_counter = 0;
		while(*dma_tx_nextchar && dma_tx_buffer_counter < DMA_TX_BUFFER_SIZE)
		{
			DataConvert(*dma_tx_nextchar, KS0073_RW_CLEAR, KS0073_RS_SET, &dma_tx_buffer[dma_tx_buffer_counter]);
			dma_tx_nextchar++;
			dma_tx_buffer_counter++;
		}
		HAL_SPI_Transmit_DMA(&SPI_Handle, (uint8_t * )&dma_tx_buffer, 4 * (dma_tx_buffer_counter) );
		return HAL_OK;
	} else {
		return HAL_BUSY;
	}
}

/**
  * @brief Init routine of KS0073 Controller, accessed over SPI
  * 		SPI init, init of GPIO for backlight & chip select
  * @param Cursor - of Type KS0073_CursorTypeDef, select if Cursor should be visible
  * @param Blink - of Type KS0073_BlinkTypeDef, select if selected address should blink
  * @retval None
 */
void KS0073_Init(KS0073_CursorTypeDef Cursor, KS0073_BlinkTypeDef Blink)
{
	// Initialize & enable backlight
	KS0073_BLE_CLK_ENABLE();
	GPIO_InitType.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitType.Pin = KS0073_BLE_PIN;
	GPIO_InitType.Pull = GPIO_NOPULL;
	GPIO_InitType.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(KS0073_BLE_PORT, &GPIO_InitType);
	KS0073_BL_Enable();

	// SPI Initialization
	SPI_Handle.Instance = KS0073_SPIx;
	SPI_Handle.Init.Mode = KS0073_SPI_MODE;
	SPI_Handle.Init.BaudRatePrescaler = KS0073_SPI_BAUDRATEPRESCALER;
	SPI_Handle.Init.Direction = KS0073_SPI_DIRECTION;
	SPI_Handle.Init.CLKPolarity = KS0073_SPI_CLK_POLARITY;
	SPI_Handle.Init.CLKPhase = KS0073_SPI_CLK_PHASE;
	SPI_Handle.Init.DataSize = KS0073_SPI_DATASIZE;
	SPI_Handle.Init.FirstBit = KS0073_SPI_FIRSTBIT;
	SPI_Handle.Init.TIMode = KS0073_SPI_TIMODE;
	SPI_Handle.Init.CRCCalculation = KS0073_SPI_CRC;
	SPI_Handle.Init.NSS = KS0073_SPI_NSS;
	while(HAL_SPI_Init(&SPI_Handle) != HAL_OK)
	{
		;
	}
	KS0073_SPI_Enable();

	//Enable Chip Select
	HAL_GPIO_WritePin(KS0073_SPI_NCS_PORT, KS0073_SPI_NCS_PIN, GPIO_PIN_RESET);

	// 8 bit data, RE = 1 (Extended Register)
	KS0073_Transmit_Byte(0x3C, KS0073_RW_CLEAR, KS0073_RS_CLEAR, 2);

	// 4 lines?
	if(LINES == 4)
	{
		KS0073_Transmit_Byte(0x09, KS0073_RW_CLEAR, KS0073_RS_CLEAR, 2);
	}

	// 8 bit data, RE = 0 (Standard Register)
	KS0073_Transmit_Byte(0x30, KS0073_RW_CLEAR, KS0073_RS_CLEAR, 2);
	while(readBusyFlag())
				;

	// enable blink, cursor & display
	uint8_t setup = 0x0C;
	if(Cursor == KS0073_CursorOn)
		setup |= 0x02;
	if(Blink == KS0073_BlinkOn)
		setup |= 0x01;
	KS0073_Transmit_Byte(setup, KS0073_RW_CLEAR, KS0073_RS_CLEAR, 2);

	// clear screen, Cursor to home position (address 0)
	KS0073_clearScreen();

	// text direction left to right
	KS0073_Transmit_Byte(0x06, KS0073_RW_CLEAR, KS0073_RS_CLEAR, 2);
}

/**
 * @brief DeInitializes SPI and resets GPIO pins
 */
extern void KS0073_DeInit()
{
	HAL_SPI_DeInit(&SPI_Handle);
	HAL_GPIO_DeInit(KS0073_BLE_PORT, KS0073_BLE_PIN);
	HAL_GPIO_DeInit(KS0073_SPI_NCS_PORT, KS0073_SPI_NCS_PIN);
}

/**
 * @brief converts a byte to fitting format and transmits it to display
 * @param Data - data byte to transmit
 * @param RW - set/reset RW bit (KS0073_RWTypeDef)
 * @param RS - set/reset RS bit (KS0073_RSTypeDef)
 * @param Timeout - Timeout in ms
 */
void KS0073_Transmit_Byte(uint8_t Data, KS0073_RWTypeDef RW, KS0073_RSTypeDef RS, uint32_t Timeout)
{
	KS0073_DataTypeDef buffer;
	DataConvert(Data, RW, RS, &buffer);
	HAL_SPI_Transmit(&SPI_Handle, (uint8_t *)&buffer, 3, Timeout);
}

/**
 *	Clears screen and returns cursor to home position
 */
void KS0073_clearScreen()
{
	KS0073_Transmit_Byte(0x01, KS0073_RW_CLEAR, KS0073_RS_CLEAR, 3);
	while(readBusyFlag())
			;
}

/**
 * Set Cursor to DD Address given by params
 * @param posx - column 0:19
 * @param posy - row 0:3
 */
extern void KS0073_gotoxy(uint8_t posx, uint8_t posy)
{
	uint8_t ddAddress = posy * 0x20;
	ddAddress += posx;
	setDDAddress(ddAddress);
}

/**
 * jumps into next line, rolls around after four lines
 */
extern void KS0073_newLine()
{
	uint8_t address = KS0073_readAddress();//get address
	address /= 0x20;				//get line from address
	KS0073_gotoxy(0, address +1);	//set to line +1
}

/**
 * Puts char to Display
 * Processes '\n' chars and wraps lines if enabled
 * @param newchar - Char to be send
 */
extern void KS0073_putc(char newchar)
{
	if(newchar == '\n')
	{
		KS0073_newLine();
	} else
	{
		#ifdef WRAPLINES
			uint8_t pos = KS0073_readAddress();
			if(pos % 0x20 >= LINE_LENGTH)
			{
				KS0073_newLine();
			}
		#endif
		KS0073_Transmit_Byte(newchar, KS0073_RW_CLEAR, KS0073_RS_SET, 1);
	}
}

/**
 * Puts a string to the display on a char by char base
 * @param nextchar - string / charpointer
 */
extern void KS0073_puts(char * nextchar)
{
	while(*nextchar)
	{
		KS0073_putc(*nextchar);
		nextchar++;
	}
}

extern void KS0073_put_int(uint32_t integer)
{
	//KS0073_putc((integer%100000/10000) + 48);
	KS0073_putc((integer%10000/1000) + 48);
	KS0073_putc((integer%1000/100) + 48);
	KS0073_putc(((integer%100)/10) + 48);
	KS0073_putc((integer%10) + 48);
}

extern HAL_SPI_StateTypeDef KS0073_getSPIState()
{
	return SPI_Handle.State;
}

/**
 * reads BusyFlag & address
 * @return Bit 7 - BusyFlag / Bit 6:0 address
 */
uint8_t KS0073_readAddress()
{
	uint8_t buffer;
	buffer = 0x1F | KS0073_RW_BIT;
	HAL_SPI_Transmit(&SPI_Handle, &buffer, 1, 5);
	buffer = 0;
	HAL_SPI_TransmitReceive(&SPI_Handle, &buffer, &buffer, 1, 5);
	return buffer;
}

/**
 * Clear the Graphic Area
 * @param graphicAreaPnt
 */
extern void KS0073_ClearGraphicArea(KS0073_GraphicAreaSmallTypeDef * graphicAreaPnt)
{
	uint8_t i = 0;
	for(;i < 16; i++)
	{
		graphicAreaPnt->Line[i] = 0;
	}
}

/**
 * Prints graphicArea to position
 * @param graphicAreaPnt
 * @param xpos
 * @param ypos
 */
extern void KS0073_PrintGraphicArea(KS0073_GraphicAreaSmallTypeDef * graphicAreaPnt, uint8_t xpos, uint8_t ypos)
{
	uint8_t address = 0;
	for(; address < 8; address++)
	{
		uint8_t byte, line;
		byte = 0x40;
		byte |= (address << 3);
		KS0073_Transmit_Byte(byte, KS0073_RW_CLEAR, KS0073_RS_CLEAR, 1);
		for(line = 0; line < 8; line++)
		{
			uint8_t shiftCnt = ((7 - address)%4)*5;
			uint8_t data = ( graphicAreaPnt->Line[line + (address/4) * 8] >> shiftCnt ) & 0x1F;
			KS0073_Transmit_Byte(data, KS0073_RW_CLEAR, KS0073_RS_SET, 1);
		}
	}
	KS0073_gotoxy(xpos,ypos);
	KS0073_putc(0);
	KS0073_putc(1);
	KS0073_putc(2);
	KS0073_putc(3);
	KS0073_gotoxy(xpos,ypos+1);
	KS0073_putc(4);
	KS0073_putc(5);
	KS0073_putc(6);
	KS0073_putc(7);
}

/**
 * Merges Area2 into Area1
 * @param Area1
 * @param Area2
 */
extern void KS0073_MergeGraphicAreas(KS0073_GraphicAreaSmallTypeDef * Area1, KS0073_GraphicAreaSmallTypeDef * Area2)
{
	uint8_t i;
	for(i = 0; i < 16; i++)
	{
		Area1->Line[i] |= Area2->Line[i];
	}
}

/**
 * Draws a horizontal Line into graphicAreaPnt at point x1/y1 of length length
 * @param graphicAreaPnt
 * @param x1
 * @param y1
 * @param length
 */
extern void KS0073_DrawHorizontalLine(KS0073_GraphicAreaSmallTypeDef * graphicAreaPnt, uint8_t x1, uint8_t y1, uint8_t length)
{
	//create a line of length x
	uint32_t line = 1;
	uint8_t i;
	for(i = 1; i<length;i++)
	{
		line <<= 1;
		line |= 1;
	}
	graphicAreaPnt->Line[15-y1] |= line << (19  + 1 - x1 - length);
}

/**
 * Draws a vertical Line into graphicAreaPnt at point x1/y1 of length length
 * @param graphicAreaPnt
 * @param x1
 * @param y1
 * @param length
 */
extern void KS0073_DrawVerticalLine(KS0073_GraphicAreaSmallTypeDef * graphicAreaPnt, uint8_t x1, uint8_t y1, uint8_t length)
{
	uint32_t line = 1 << (19 - x1);
	uint8_t i;
	for(i = 16 - length - y1; i < 16 - y1; i++)
	{
		graphicAreaPnt->Line[i] |= line;
	}
}

/**
 * Tests if same bits in two KS0073_GraphicAreaSmallTypeDefs are set
 * @param backgroundArea - the surrundings
 * @param Object - the object
 * @return 1 for collision, 0 for No Collision
 */
extern int KS0073_CollisionTest(KS0073_GraphicAreaSmallTypeDef * backgroundArea, KS0073_GraphicAreaSmallTypeDef * Object)
{
	uint8_t i;
	for(i = 0; i < 16; i++)
	{
		if(backgroundArea->Line[i] & Object->Line[i])
		{
			return 1;
		}
	}
	return 0;
}


/**
 * Draws a square into GraphicArea
 * @param graphicAreaPnt Pointer to GraphicArea
 * @param x1 x lower
 * @param y1 y left
 * @param x2 x upper
 * @param y2 y right
 */
extern void KS0073_DrawSquare(KS0073_GraphicAreaSmallTypeDef * graphicAreaPnt, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
	//draw horizontal Lines
	KS0073_DrawHorizontalLine(graphicAreaPnt, x1, y1, x2 - x1 + 1 );
	KS0073_DrawHorizontalLine(graphicAreaPnt, x1, y2, x2 - x1 +1 );
	KS0073_DrawVerticalLine(graphicAreaPnt, x1, y1, y2-y1+1);
	KS0073_DrawVerticalLine(graphicAreaPnt, x2, y1, y2-y1+1);
}

/**
 * 	Enables backlight
 */
inline void KS0073_BL_Enable()
{
	HAL_GPIO_WritePin(KS0073_BLE_PORT, KS0073_BLE_PIN, GPIO_PIN_RESET);
}

/**
 * @brief Disables Backlight
 */
inline void KS0073_BL_Disable()
{
	HAL_GPIO_WritePin(KS0073_BLE_PORT, KS0073_BLE_PIN, GPIO_PIN_SET);
}

/**
 * SPI enable
 */
inline void KS0073_SPI_Enable()
{
	__HAL_SPI_ENABLE(&SPI_Handle);
}
/**
 * SPI Disable
 */
inline void KS0073_SPI_Disable()
{
	__HAL_SPI_DISABLE(&SPI_Handle);
}

/**
  * @brief SPI MSP Init
  * 		SPI LowLevel Init of all Pin Out/InPuts & Clocks
  * @param  hspi: pointer to a SPI_HandleTypeDef structure that contains
  *               the configuration information for SPI module.
  * @retval None
  */
void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
	GPIO_InitTypeDef InitStruct;
	//Clocks
	KS0073_SPI_CLK_ENABLE();
	KS0073_SPI_SCK_CLK_ENABLE();
	KS0073_SPI_MISO_CLK_ENABLE();
	KS0073_SPI_MOSI_CLK_ENABLE();
	KS0073_SPI_NCS_CLK_ENABLE();

	//Pin Setups
	InitStruct.Mode = GPIO_MODE_AF_PP;
	InitStruct.Pull = GPIO_PULLDOWN;
	InitStruct.Speed = GPIO_SPEED_HIGH;

	InitStruct.Pin = KS0073_SPI_SCK_PIN;
	HAL_GPIO_Init(KS0073_SPI_SCK_PORT, &InitStruct);

	InitStruct.Pin = KS0073_SPI_MOSI_PIN;
	HAL_GPIO_Init(KS0073_SPI_MOSI_PORT, &InitStruct);

	InitStruct.Pin = KS0073_SPI_MISO_PIN;
	HAL_GPIO_Init(KS0073_SPI_MISO_PORT, &InitStruct);

	InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(KS0073_SPI_NCS_PORT, &InitStruct);


#ifndef KS0073_NO_DMA
	KS0073_DMA_CLK_ENABLE();
	hdma_tx.Instance 					= KS0073_DMA_TX_CHANNEL;
	hdma_tx.Init.Direction				= DMA_MEMORY_TO_PERIPH;
	hdma_tx.Init.PeriphInc				= DMA_PINC_DISABLE;
	hdma_tx.Init.MemInc					= DMA_MINC_ENABLE;
	hdma_tx.Init.PeriphDataAlignment 	= DMA_PDATAALIGN_BYTE;
	hdma_tx.Init.MemDataAlignment    	= DMA_MDATAALIGN_BYTE;
	hdma_tx.Init.Mode                	= DMA_NORMAL;
	hdma_tx.Init.Priority            	= DMA_PRIORITY_LOW;

	HAL_DMA_Init(&hdma_tx);

	__HAL_LINKDMA(hspi, hdmatx,hdma_tx );

	hdma_rx.Instance                 = KS0073_DMA_RX_CHANNEL;

	hdma_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
	hdma_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
	hdma_rx.Init.MemInc              = DMA_MINC_ENABLE;
	hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
	hdma_rx.Init.Mode                = DMA_NORMAL;
	hdma_rx.Init.Priority            = DMA_PRIORITY_HIGH;

	HAL_DMA_Init(&hdma_rx);
	__HAL_LINKDMA(hspi, hdmarx, hdma_rx);

	HAL_NVIC_SetPriority(KS0073_DMA_TX_IRQn, 1 , 1);
	HAL_NVIC_EnableIRQ( KS0073_DMA_TX_IRQn);

    //HAL_NVIC_SetPriority(KS0073_DMA_RX_IRQn, 1, 0);
    //HAL_NVIC_EnableIRQ(KS0073_DMA_RX_IRQn);

#endif //KS0073_NO_DMA
}


/**
  * @brief SPI MSP DeInit
  * 		SPI LowLevel DeInit of all Pin Out/InPuts & Clocks
  * @param  hspi: pointer to a SPI_HandleTypeDef structure that contains
  *               the configuration information for SPI module.
  * @retval None
  */
void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{
	HAL_GPIO_DeInit(KS0073_SPI_SCK_PORT, KS0073_SPI_SCK_PIN);
	HAL_GPIO_DeInit(KS0073_SPI_MOSI_PORT, KS0073_SPI_MOSI_PIN);
	HAL_GPIO_DeInit(KS0073_SPI_MISO_PORT, KS0073_SPI_MISO_PIN);
	HAL_GPIO_DeInit(KS0073_SPI_NCS_PORT, KS0073_SPI_NCS_PIN);
	HAL_SPI_DeInit( hspi );

	#ifndef KS0073_NO_DMA
		KS0073_DMA_CLK_ENABLE();

		HAL_DMA_DeInit(&hdma_tx);

		HAL_NVIC_DisableIRQ(KS0073_DMA_TX_IRQn);
	#endif //KS0073_NO_DMA
}


#ifndef KS0073_NO_DMA

	/**
	  * @brief Tx Transfer completed callbacks
	  * @param  hspi: pointer to a SPI_HandleTypeDef structure that contains
	  *                the configuration information for SPI module.
	  * @retval None
	  */
	void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
	{
		dma_tx_buffer_counter = 0;
		while(*dma_tx_nextchar && dma_tx_buffer_counter < DMA_TX_BUFFER_SIZE)
		{
			DataConvert(*dma_tx_nextchar, KS0073_RW_CLEAR, KS0073_RS_SET, &dma_tx_buffer[dma_tx_buffer_counter]);
			dma_tx_nextchar++;
			dma_tx_buffer_counter++;
		}
		HAL_SPI_Transmit_DMA(&SPI_Handle, (uint8_t * )&dma_tx_buffer, 4 * (dma_tx_buffer_counter) );
	}

	/**
	  * @brief  This function handles DMA Rx interrupt request.
	  * @param  None
	  * @retval None
	  */
	void KS0073_DMA_RX_IRQHandler(void)
	{
	  HAL_DMA_IRQHandler( SPI_Handle.hdmarx);
	}

	/**
	  * @brief  This function handles DMA Tx interrupt request.
	  * @param  None
	  * @retval None
	  */
	void KS0073_DMA_TX_IRQHandler(void)
	{
	  HAL_DMA_IRQHandler( SPI_Handle.hdmatx);
	}


#endif //KS0073_NO_DMA

/* local Functions, Definitions ---------------------------------------------- */

/**
 * @brief Builds a KS0073_DataTypeDef from Data, RW & RS
 * @param Data - Input Data, for example a command or a char
 * @param RW - R/W Bit, 1 for Read-Operations, 0 for Write-Operations
 * @param RS - Register Select
 * @param buffer - Pointer to a KS0073_DataTypeDef Element to save the Result
 */
void DataConvert(uint8_t Data, KS0073_RWTypeDef RW, KS0073_RSTypeDef RS, KS0073_DataTypeDef * buffer)
{
	//Five 1s as Start Bit
	buffer->start = 0x1F;
	if(RW == KS0073_RW_SET)
	{
		buffer->start |= KS0073_RW_BIT;
	}
	if(RS == KS0073_RS_SET)
	{
		buffer->start |= KS0073_RS_BIT;
	}
	buffer->data_low = Data & 0xF;
	buffer->data_high = (Data >> 4);
	buffer->delay = 0;
}

/**
 * reads data from present address
 * @return data
 */
uint8_t readData()
{
	uint8_t buffertx, bufferrx;
	buffertx = 0x1F | KS0073_RW_BIT | KS0073_RS_BIT;
	HAL_SPI_TransmitReceive(&SPI_Handle, &buffertx, &bufferrx, 1, 5);
	buffertx = 0x1F;
	HAL_SPI_TransmitReceive(&SPI_Handle, &buffertx, &bufferrx, 1, 5);
	return bufferrx;
}
/**
 * sets Address to DDAdress in DisplayRAM
 * @param DDAddress
 */
void setDDAddress(uint8_t DDAddress)
{
	DDAddress |= 0x80;
	KS0073_Transmit_Byte(DDAddress, KS0073_RW_CLEAR, KS0073_RS_CLEAR, 5);
}

/**
 * Checks BusyFlag
 * @return BusyFlag - 1 = Busy, 0 = NotBusy
 */
uint8_t readBusyFlag()
{
	if(KS0073_readAddress() & 0x80) //if BusyFlag set
	{
		return 1;
	}
	return 0;
}

