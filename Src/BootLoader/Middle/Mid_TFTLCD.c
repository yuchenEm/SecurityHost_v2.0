/****************************************************
  * @Name	Mid_TFTLCD.c
  * @Brief	TFTLCD operation
  ***************************************************/
  
/*-------------Header Files Include----------------*/
#include "stm32f10x.h"             
#include "mid_tftlcd.h"
#include "hal_gpio.h"
#include "hal_tftlcd.h"
#include "hal_dma.h"
#include "tftlcd_font.h"

/*-------------Internal Functions Declaration-------*/
static void Mid_TFTLCD_DrawPoint(uint16_t x, uint16_t y, uint16_t Color);
static void Mid_TFTLCD_Delay(uint32_t x);

/*-------------Module Variables Declaration---------*/
uint8_t ColorBuff[640];

/*-------------Module Functions Definition----------*/
/**
  * @Brief	Initialize TFTLCD module
  * @Param	None
  * @Retval	None
  */
void Mid_TFTLCD_Init(void)
{
	Hal_TFTLCD_Rest_Low();	// reset
	Mid_TFTLCD_Delay(10000);
	Hal_TFTLCD_Rest_High();
	Mid_TFTLCD_Delay(100);
	
	/* Initial Sequence */
	Hal_TFTLCD_Write_Register(0x11);
	Mid_TFTLCD_Delay(10000);
	Hal_TFTLCD_Write_Register(0x36);	// memory access control
	
	if(DISPLAY_MODE == 0)
	{
		Hal_TFTLCD_Write_Data8(0x00);
	}
	else if(DISPLAY_MODE == 1)
	{
		Hal_TFTLCD_Write_Data8(0xC0);
	}
	else if(DISPLAY_MODE == 2)
	{
		Hal_TFTLCD_Write_Data8(0x70);
	}
	else
	{
		Hal_TFTLCD_Write_Data8(0xA0);
	}
	
	Hal_TFTLCD_Write_Register(0x3A);
//	Hal_TFTLCD_Write_Register(0x03);	// 12bit
	Hal_TFTLCD_Write_Data8(0x05);
	
	/* ST7789S Frame Rate Setting */
	Hal_TFTLCD_Write_Register(0xB2);
	Hal_TFTLCD_Write_Data8(0x0C);
	Hal_TFTLCD_Write_Data8(0x0C);
	Hal_TFTLCD_Write_Data8(0x00);
	Hal_TFTLCD_Write_Data8(0x33);
	Hal_TFTLCD_Write_Data8(0x33);
	Hal_TFTLCD_Write_Register(0xB7);
	Hal_TFTLCD_Write_Data8(0x35);
	
	/* ST7789S Power Setting */
	Hal_TFTLCD_Write_Register(0xBB);
	Hal_TFTLCD_Write_Data8(0x35);

	Hal_TFTLCD_Write_Register(0xC0);
	Hal_TFTLCD_Write_Data8(0x2C);

	Hal_TFTLCD_Write_Register(0xC2);
	Hal_TFTLCD_Write_Data8(0x01);

	Hal_TFTLCD_Write_Register(0xC3);
	Hal_TFTLCD_Write_Data8(0x13);

	Hal_TFTLCD_Write_Register(0xC4);
	Hal_TFTLCD_Write_Data8(0x20);

	Hal_TFTLCD_Write_Register(0xC6);
	Hal_TFTLCD_Write_Data8(0x0F);

	Hal_TFTLCD_Write_Register(0xCA);
	Hal_TFTLCD_Write_Data8(0x0F);

	Hal_TFTLCD_Write_Register(0xC8);
	Hal_TFTLCD_Write_Data8(0x08);

	Hal_TFTLCD_Write_Register(0x55);
	Hal_TFTLCD_Write_Data8(0x90);

	Hal_TFTLCD_Write_Register(0xD0);
	Hal_TFTLCD_Write_Data8(0xA4);
	Hal_TFTLCD_Write_Data8(0xA1);
	
	/* ST7789S Gamma Setting */
	Hal_TFTLCD_Write_Register(0xE0);
	Hal_TFTLCD_Write_Data8(0xD0);
	Hal_TFTLCD_Write_Data8(0x00);
	Hal_TFTLCD_Write_Data8(0x06);
	Hal_TFTLCD_Write_Data8(0x09);
	Hal_TFTLCD_Write_Data8(0x0B);
	Hal_TFTLCD_Write_Data8(0x2A);
	Hal_TFTLCD_Write_Data8(0x3C);
	Hal_TFTLCD_Write_Data8(0x55);
	Hal_TFTLCD_Write_Data8(0x4B);
	Hal_TFTLCD_Write_Data8(0x08);
	Hal_TFTLCD_Write_Data8(0x16);
	Hal_TFTLCD_Write_Data8(0x14);
	Hal_TFTLCD_Write_Data8(0x19);
	Hal_TFTLCD_Write_Data8(0x20);
	
	Hal_TFTLCD_Write_Register(0xE1);
	Hal_TFTLCD_Write_Data8(0xD0);
	Hal_TFTLCD_Write_Data8(0x00);
	Hal_TFTLCD_Write_Data8(0x06);
	Hal_TFTLCD_Write_Data8(0x09);
	Hal_TFTLCD_Write_Data8(0x0B);
	Hal_TFTLCD_Write_Data8(0x29);
	Hal_TFTLCD_Write_Data8(0x36);
	Hal_TFTLCD_Write_Data8(0x54);
	Hal_TFTLCD_Write_Data8(0x4B);
	Hal_TFTLCD_Write_Data8(0x0D);
	Hal_TFTLCD_Write_Data8(0x16);
	Hal_TFTLCD_Write_Data8(0x14);
	Hal_TFTLCD_Write_Data8(0x21);
	Hal_TFTLCD_Write_Data8(0x20);
	
	Hal_TFTLCD_Write_Register(0x29);
	Hal_TFTLCD_Display_On();

	Mid_TFTLCD_ScreenClear();
	Mid_TFTLCD_ShowString(30, 100, "System Initial", LCD_FONT_COLOR, LCD_BACK_COLOR, 32, 0);
	
}

/**
  * @Brief	Set start and end address
  * @Param	Col1, Col2: Column start and end address
  *			Row1, Row2: Row start and end address
  * @Retval	None
  */
void Mid_TFTLCD_AddressSet(uint16_t Col1, uint16_t Row1, uint16_t Col2, uint16_t Row2)
{
	Hal_TFTLCD_Write_Register(0x2A);	// Column address set
	Hal_TFTLCD_Write_Data(Col1);
	Hal_TFTLCD_Write_Data(Col2);
	
	Hal_TFTLCD_Write_Register(0x2B);	// Row address set
	Hal_TFTLCD_Write_Data(Row1);
	Hal_TFTLCD_Write_Data(Row2);
	
	Hal_TFTLCD_Write_Register(0x2C);	// Memory write
}

/**
  * @Brief	Fill color to specified area
  * @Param	xStart, yStart: start address
  *			xEnd, yEnd	  : end address
  * @Retval	None
  */
void Mid_TFTLCD_ColorFill(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint16_t Color)
{
	uint16_t i;
	
	Mid_TFTLCD_AddressSet(xStart, yStart, xEnd-1, yEnd-1);	// Display area
	
	for(i=0; i<xEnd; i++)
	{
		ColorBuff[i++] = Color >> 8;
		ColorBuff[i] = Color;
	}
	
	for(i=yStart; i<yEnd*2; i++)
	{
		Hal_DMA_SPI3Tx_Reg(ColorBuff, xEnd);
	}
}

/**
  * @Brief	Clear screen
  * @Param	None
  * @Retval	None
  */
void Mid_TFTLCD_ScreenClear(void)
{
	Mid_TFTLCD_ColorFill(0, 0, LCD_W, LCD_H, LCD_BACK_COLOR);
}

/**
  * @Brief	Display a Char
  * @Param	x, y	 : Coordinate (320 * 240)
  *			CharData : Char to display
  *			FontColor: Font color
  *			BackColor: Background color
  *			FontSize : Font size
  *			Mode	 : 0 -> no overlay, 1 -> overlay(no BackColor attached)
  * @Retval	None
  */
void Mid_TFTLCD_ShowChar(uint16_t x, uint16_t y, uint8_t CharData, uint16_t FontColor, uint16_t BackColor, uint8_t FontSize, uint8_t Mode)
{
	uint16_t i;
	uint8_t j, k;
	
	uint8_t Size_X, Size_Y;
	uint16_t CharSize;
	uint16_t x_src = x;
	
	uint8_t CharIndex;
	uint8_t Format;
	
	k = 0;
	
	Size_X = FontSize / 2;
	Size_Y = FontSize;
	CharSize = (Size_X/8 + ((Size_X % 8)?1:0)) * Size_Y;
	
	CharIndex = CharData - ' ';	// get Char offset (' ' is the first one in ASCII buff)
	
	Mid_TFTLCD_AddressSet(x, y, x+Size_X-1, y+Size_Y-1);
	
	for(i=0; i<CharSize; i++)
	{
		if(FontSize == FONTSIZE_12)
		{
			Format = ASCII_1206[CharIndex][i];	// 6 * 12 pixel
		}
		else if(FontSize == FONTSIZE_16)
		{
			Format = ASCII_1608[CharIndex][i];	// 8 * 16 pixel
		}
		else if(FontSize == FONTSIZE_24)
		{
			Format = ASCII_2412[CharIndex][i];	// 12 * 24 pixel
		}
		else if(FontSize == FONTSIZE_32)
		{
			Format = ASCII_3216[CharIndex][i];	// 16 * 32 pixel
		}
		else if(FontSize == FONTSIZE_48)
		{
			Format = ASCII_4824[CharIndex][i];	// 24 * 48 pixel
		}
		else
		{
			return;
		}
		
		for(j=0; j<8; j++)
		{
			if(!Mode)	// no overlay mode
			{
				if(Format & (0x01 << j))
				{
					Hal_TFTLCD_Write_Data(FontColor);
				}
				else
				{
					Hal_TFTLCD_Write_Data(BackColor);
				}
				
				k++;
				
				if((k % Size_X) == 0)
				{
					k=0;
					break;
				}
			}
			else		// overlay mode
			{
				if(Format & (0x01 << j))
				{
					Mid_TFTLCD_DrawPoint(x, y, FontColor);
				}
				
				x++;
				
				if((x - x_src) == Size_X)	// start a new row
				{
					x = x_src;
					y++;
					break;
				}
			}
		}
	}
}

/**
  * @Brief	Display String
  * @Param	x, y: Coordinate (320 * 240)
  *			*pString: Target string to display
  *			FontColor: Font color
  *			BackColor: Background color
  *			FontSize: Font size
			Mode: 0 -> no overlay, 1 -> overlay(no BackColor attached)
  * @Retval	None
  */
void Mid_TFTLCD_ShowString(uint16_t x, uint16_t y, const uint8_t *pString, uint16_t FontColor, uint16_t BackColor, uint8_t FontSize, uint8_t Mode)
{
	while(*pString != '\0')
	{
		Mid_TFTLCD_ShowChar(x, y, *pString, FontColor, BackColor, FontSize, Mode);
		x += FontSize / 2;
		pString++;
	}
}


/*-------------Internal Functions Definition--------*/
/**
  * @Brief	Draw point at specified coordinate(x,y)
  * @Param	x: coordinate-x
  *			y: coordinate-y
  *			Color: pixel color
  * @Retval	None
  */
static void Mid_TFTLCD_DrawPoint(uint16_t x, uint16_t y, uint16_t Color)
{
	Mid_TFTLCD_AddressSet(x, y, x, y);
	Hal_TFTLCD_Write_Data(Color);
}

/**
  * @Brief	Delay x cycles
  * @Param	x: number of cycles to delay
  * @Retval	None
  */
static void Mid_TFTLCD_Delay(uint32_t x)
{
	while(x--);
}

/*-------------Interrupt Functions Definition-------*/


