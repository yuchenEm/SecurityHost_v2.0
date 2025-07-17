#ifndef __MID_TFTLCD_H_
#define __MID_TFTLCD_H_

/** Display Mode Setting: 
  *		0, 1 --> Vertical
  *		2, 3 --> Horizontal
  */
/* Uncomment the target mode below: */
//#define DISPLAY_MODE		0
//#define DISPLAY_MODE		1
//#define DISPLAY_MODE		2
#define DISPLAY_MODE		3

#if DISPLAY_MODE == 0 || DISPLAY_MODE == 1

#define LCD_W	240
#define LCD_H	320

#else
#define LCD_W	320
#define LCD_H	240

#endif

/* RGB565 */ 
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE           	 0x001F  
#define BRED             0XF81F
#define GRED 		         0XFFE0
#define GBLUE	           0X07FF
#define RED              0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			     0XBC40 	
#define BRRED 			     0XFC07 	
#define GRAY  			     0X8430 	
#define DARKBLUE      	 0X01CF		
#define LIGHTBLUE      	 0X7D7C		
#define GRAYBLUE       	 0X5458 	
#define LIGHTGREEN     	 0X841F 	
#define LGRAY 			     0XC618 	
#define LGRAYBLUE        0XA651 	
#define LBBLUE           0X2B12 	

/* LCD Color Setting */
#define LCD_FONT_COLOR			   WHITE
#define LCD_BACK_COLOR			   BLACK
#define LCD_FONT_BACK_COLOR		 LIGHTBLUE	
#define LCD_PARA_BACK_COLOR		 GRAY


void Mid_TFTLCD_Init(void);

void Mid_TFTLCD_AddressSet(uint16_t Col1, uint16_t Row1, uint16_t Col2, uint16_t Row2);
void Mid_TFTLCD_ColorFill(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint16_t Color);
void Mid_TFTLCD_ScreenClear(void);

void Mid_TFTLCD_ShowChar(uint16_t x, uint16_t y, uint8_t CharData, uint16_t FontColor, uint16_t BackColor, uint8_t FontSize, uint8_t Mode);
void Mid_TFTLCD_ShowString(uint16_t x, uint16_t y, const uint8_t *pString, uint16_t FontColor, uint16_t BackColor, uint8_t FontSize, uint8_t Mode);

void Mid_TFTLCD_ShowImage_32x32Pixel(uint16_t x, uint16_t y, uint8_t ImageIndex, uint16_t ImageColor, uint16_t BackColor, uint8_t Mode);
void Mid_TFTLCD_ShowSystemTime(uint8_t *pSystemtime);
void Mid_TFTLCD_DisplayTH(uint16_t Tem_x, uint16_t Tem_y, uint16_t Hum_x, uint16_t Hum_y);
void Mid_TFTLCD_DisplayPowerState(uint16_t AC_x, uint16_t AC_y, uint16_t BAT_x, uint16_t BAT_y);
void Mid_TFTLCD_DisplayWiFiSignal(void);
void Mid_TFTLCD_DisplayMQTTConnection(void);

#endif
