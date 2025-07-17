#ifndef __TFTLCD_FONT_H_
#define __TFTLCD_FONT_H_

/* Font Size */
enum
{
	FONTSIZE_12 = 12,
	FONTSIZE_16 = 16,
	FONTSIZE_24 = 24,
	FONTSIZE_32 = 32,
	FONTSIZE_48 = 48,
};

extern const unsigned char ASCII_1206[][12];	// 6 * 12 pixel
extern const unsigned char ASCII_1608[][16];	// 8 * 16 pixel
extern const unsigned char ASCII_2412[][48];	// 12 * 24 pixel
extern const unsigned char ASCII_3216[][64];	// 16 * 32 pixel
extern const unsigned char ASCII_4824[][144];	// 24 * 48 pixel

#endif
