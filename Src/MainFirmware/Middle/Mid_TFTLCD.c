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
#include "hal_al6630.h"
#include "hal_adc.h"
#include "tftlcd_font.h"
#include "tftlcd_icon.h"
#include "mid_wifi.h"

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

/**
  * @Brief	Display 32x32 pixel Image
  * @Param	x, y: Coordinate (320 * 240)
  *			ImageIndex: Target Image to display
  *			ImageColor: Font color
  *			BackColor: Background color
  *			FontSize: Font size
			Mode: 0 -> no overlay, 1 -> overlay(no BackColor attached)
  * @Retval	None
  */
void Mid_TFTLCD_ShowImage_32x32Pixel(uint16_t x, uint16_t y, uint8_t ImageIndex, uint16_t ImageColor, uint16_t BackColor, uint8_t Mode)
{
	uint8_t i, j;
	uint8_t ImageBuff;
	
	uint16_t ImageSize;
	
	ImageSize = 128;	// 128 byte = (32 / 8) * 32
	
	Mid_TFTLCD_AddressSet(x, y, x+31, y+31);
	
	for(i=0; i<ImageSize; i++)
	{
		ImageBuff = ICON_32x32[ImageIndex][i];
		
		for(j=0; j<8; j++)
		{
			if(!Mode)	// no overlay mode(show Image BackColor)
			{
				if(ImageBuff & (0x80 >> j))
				{
					Hal_TFTLCD_Write_Data(ImageColor);
				}
				else
				{
					Hal_TFTLCD_Write_Data(BackColor);
				}
			}
			else		// overlay mode(no Image BackColor)
			{
				if(ImageBuff & (0x80 >> j))
				{
					Hal_TFTLCD_Write_Data(ImageColor);
				}
				else
				{
					Hal_TFTLCD_Write_Data(LCD_BACK_COLOR);
				}
			}
		}
	}
}

/**
  * @Brief	Display System time
  * @Param	pSystemtime: point to the SystemTime provide
  * @Retval	None
  */
void Mid_TFTLCD_ShowSystemTime(uint8_t *pSystemtime)
{	
	Mid_TFTLCD_ShowString(20, 200, &pSystemtime[0], LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
	Mid_TFTLCD_ShowString(160, 200, &pSystemtime[11], LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
}

/**
  * @Brief	Display Temperature and Humidity
  * @Param	Tem_x : Temperature image x-coordinate
  *			Tem_y : Temperature image y-coordinate
  *			Hum_x : Humidity image x-coordinate
  *			Hum_y : Humidity image y-coordinate
  * @Retval	None
  */
void Mid_TFTLCD_DisplayTH(uint16_t Tem_x, uint16_t Tem_y, uint16_t Hum_x, uint16_t Hum_y)
{
	static uint16_t TemValue = 0;
	static uint16_t HumValue = 0;
	
	uint8_t DisplayBuff[6];
	uint8_t Index = 0;
	uint8_t UpdateFlag = 0;
	
	if(TemValue != Hal_AL6630_GetTemperatureValue())
	{
		TemValue = Hal_AL6630_GetTemperatureValue();
		UpdateFlag = 1;
	}
	if(HumValue != Hal_AL6630_GetHumidityValue())
	{
		HumValue = Hal_AL6630_GetHumidityValue();
		UpdateFlag = 1;
	}
	
	if(UpdateFlag == 1)
	{
		Index = 0;
		
		// AL6630 Temperature range: -40C - 80C 
		// only display normal temperature here: 10.0 - 50.0
		if((TemValue > 99) && (TemValue < 500))
		{
			DisplayBuff[Index++] = TemValue / 100 +'0';
		}
		
		DisplayBuff[Index++] = TemValue % 100 / 10 + '0';
		DisplayBuff[Index++] = '.';
		DisplayBuff[Index++] = TemValue % 10 + '0';
		DisplayBuff[Index++] = '!';	// temperature unit
		DisplayBuff[Index] = '\0';
		Mid_TFTLCD_ShowString(Tem_x, Tem_y, DisplayBuff, LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
		
		Index = 0;
		
		// AL6630 Humidity range: 0% - 99.9% 
		// only display normal humidity here: 0.0 - 99.9
		if((HumValue > 0) && (HumValue < 999))
		{
			DisplayBuff[Index++] = HumValue / 100 +'0';
		}
		
		DisplayBuff[Index++] = HumValue % 100 / 10 + '0';
		DisplayBuff[Index++] = '.';
		DisplayBuff[Index++] = HumValue % 10 + '0';
		DisplayBuff[Index++] = '%';	// temperature unit
		DisplayBuff[Index] = '\0';
		Mid_TFTLCD_ShowString(Hum_x, Hum_y, DisplayBuff, LCD_FONT_COLOR, LCD_BACK_COLOR, 24, 0);
	}
}

/**
  * @Brief	Display Battery VoltageLevel and AC ChargingState
  * @Param	AC_x : AC image x-coordinate
  *			AC_y : AC image y-coordinate
  *			BAT_x: BAT image x-coordinate
  *			BAT_y: BAT image y-coordinate
  * @Retval	None
  */
void Mid_TFTLCD_DisplayPowerState(uint16_t AC_x, uint16_t AC_y, uint16_t BAT_x, uint16_t BAT_y)
{
	static uint16_t IntervalCounter = 0; // Charging image flash interval counter
	
	if(Hal_GPIO_ACStateCheck() == STA_AC_LINK)	// AC connected
	{
		Mid_TFTLCD_ShowImage_32x32Pixel(AC_x, AC_y, ICON_32X32_ACLINK, LCD_FONT_COLOR, LCD_BACK_COLOR, 0);
		
		if(Hal_ADC_GetBatteryVoltageLevel() == LEVEL_FULL)
		{
			IntervalCounter = 0;
			
			Mid_TFTLCD_ShowImage_32x32Pixel(BAT_x, BAT_y, (ICON_32X32_BAT_LEVEL5), LCD_FONT_COLOR, LCD_BACK_COLOR, 0);
		}
		else
		{
			IntervalCounter++;
			
			if(IntervalCounter == 50)
			{
				Mid_TFTLCD_ShowImage_32x32Pixel(BAT_x, BAT_y, ICON_32X32_BAT_LEVEL0, LCD_FONT_COLOR, LCD_BACK_COLOR, 0);
			}
			else if(IntervalCounter == 100)
			{
				Mid_TFTLCD_ShowImage_32x32Pixel(BAT_x, BAT_y, ICON_32X32_BAT_LEVEL1, LCD_FONT_COLOR, LCD_BACK_COLOR, 0);
			}
			else if(IntervalCounter == 150)
			{
				Mid_TFTLCD_ShowImage_32x32Pixel(BAT_x, BAT_y, ICON_32X32_BAT_LEVEL2, LCD_FONT_COLOR, LCD_BACK_COLOR, 0);
			}
			else if(IntervalCounter == 200)
			{
				Mid_TFTLCD_ShowImage_32x32Pixel(BAT_x, BAT_y, ICON_32X32_BAT_LEVEL3, LCD_FONT_COLOR, LCD_BACK_COLOR, 0);
			}
			else if(IntervalCounter == 250)
			{
				Mid_TFTLCD_ShowImage_32x32Pixel(BAT_x, BAT_y, ICON_32X32_BAT_LEVEL4, LCD_FONT_COLOR, LCD_BACK_COLOR, 0);
			}
			else if(IntervalCounter == 300)
			{			
				Mid_TFTLCD_ShowImage_32x32Pixel(BAT_x, BAT_y, ICON_32X32_BAT_LEVEL5, LCD_FONT_COLOR, LCD_BACK_COLOR, 0);
			}
			else if(IntervalCounter == 350)
			{
				IntervalCounter = 0;
				
				Mid_TFTLCD_ShowImage_32x32Pixel(BAT_x, BAT_y, ICON_32X32_BAT_LEVEL0, LCD_FONT_COLOR, LCD_BACK_COLOR, 0);
			}
		}
	}
	else	// AC not connected
	{
		Mid_TFTLCD_ShowImage_32x32Pixel(AC_x, AC_y, ICON_32X32_ACBREAK, LCD_FONT_COLOR, LCD_BACK_COLOR, 0);
	
		if(Hal_ADC_GetBatteryVoltageLevel() <= LEVEL_VOLT_1)
		{
			IntervalCounter++;
			
			if(IntervalCounter == 50)
			{
				Mid_TFTLCD_ShowImage_32x32Pixel(BAT_x, BAT_y, (ICON_32X32_BAT_LEVEL0 + Hal_ADC_GetBatteryVoltageLevel()), LCD_FONT_COLOR, LCD_BACK_COLOR, 0);
			}
			else if(IntervalCounter > 100)
			{
				IntervalCounter = 0;
				
				Mid_TFTLCD_ShowImage_32x32Pixel(BAT_x, BAT_y, ICON_32X32_CLEAR, LCD_FONT_COLOR, LCD_BACK_COLOR, 0);
			}
		}
		else
		{
			IntervalCounter = 0;
			
			Mid_TFTLCD_ShowImage_32x32Pixel(BAT_x, BAT_y, (ICON_32X32_BAT_LEVEL0 + Hal_ADC_GetBatteryVoltageLevel()), LCD_FONT_COLOR, LCD_BACK_COLOR, 0);
		}
	}
}

/**
  * @Brief	Display WiFi signal icon
  * @Param	None
  * @Retval	None
  */
void Mid_TFTLCD_DisplayWiFiSignal(void)
{
	static uint8_t WiFiSignal = 0xFF;	// 0xFF->WiFi disconnected
	static uint8_t Counter = 0;
	
	/*check and update WiFiSignal */
	if(WiFiSignal != Mid_WiFi_GetSignalLevel())
	{
		WiFiSignal = Mid_WiFi_GetSignalLevel();
		
		Counter = 0;
	}
	
	/* Flip the WiFi signal icon every 1s, when disconnected */
	if(WiFiSignal == 0xFF)
	{
		Counter++;
		
		if(Counter == 100)
		{
			Mid_TFTLCD_ShowImage_32x32Pixel(COOR_ICON_WIFI_X, COOR_ICON_WIFI_Y, ICON_32X32_WIFI_S4, LCD_FONT_COLOR, LCD_BACK_COLOR, 0);
		}
		else if(Counter == 200)
		{
			Mid_TFTLCD_ShowImage_32x32Pixel(COOR_ICON_WIFI_X, COOR_ICON_WIFI_Y, ICON_32X32_WIFI_S4, LCD_BACK_COLOR, LCD_BACK_COLOR, 0);
		}
	}
	/* Show signal level, when connected */
	else
	{
		Counter++;
		
		if(Counter == 100)
		{
			Mid_TFTLCD_ShowImage_32x32Pixel(COOR_ICON_WIFI_X, COOR_ICON_WIFI_Y, (en_ICON_32X32_t)WiFiSignal, LCD_FONT_COLOR, LCD_BACK_COLOR, 0);
		}
	}
}

/**
  * @Brief	Display MQTT server connection state icon
  * @Param	None
  * @Retval	None
  */
void Mid_TFTLCD_DisplayMQTTConnection(void)
{
	static uint16_t Counter = 0;
	
	/* MQTT server connected: */
	if(Mid_WiFi_GetMQTTState() == MQTT_STA_READY)
	{
		Counter++;
		
		if(Counter == 200)
		{
			Counter = 0;
			
			Mid_TFTLCD_ShowImage_32x32Pixel(COOR_ICON_SERVER_X, COOR_ICON_SERVER_Y, ICON_32X32_SERVER_CONNECTED, LCD_FONT_COLOR, LCD_BACK_COLOR, 0);
		}
	}
	/* MQTT server disconnected: Flip the icon every 1s */
	else
	{
		Counter++;
		
		if(Counter == 100)
		{
			Mid_TFTLCD_ShowImage_32x32Pixel(COOR_ICON_SERVER_X, COOR_ICON_SERVER_Y, ICON_32X32_SERVER_CONNECTED, LCD_BACK_COLOR, LCD_BACK_COLOR, 0);
		}
		else if(Counter == 200)
		{
			Counter = 0;
			
			Mid_TFTLCD_ShowImage_32x32Pixel(COOR_ICON_SERVER_X, COOR_ICON_SERVER_Y, ICON_32X32_SERVER_CONNECTED, LCD_FONT_COLOR, LCD_BACK_COLOR, 0);
		}
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


