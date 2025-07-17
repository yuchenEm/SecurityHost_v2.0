#ifndef __TFTLCD_ICON_H_
#define __TFTLCD_ICON_H_

/* ICON Coordinate */
#define COOR_ICON_AC_X   1
#define COOR_ICON_AC_Y   1

#define COOR_ICON_BAT_X   33
#define COOR_ICON_BAT_Y   1

#define COOR_ICON_SERVER_X   210
#define COOR_ICON_SERVER_Y    1

#define COOR_ICON_WIFI_X   250
#define COOR_ICON_WIFI_Y    1

#define COOR_ICON_SIM_X   282
#define COOR_ICON_SIM_Y    1

#define COOR_ICON_SYSTEMODE_X   70
#define COOR_ICON_SYSTEMODE_Y   100

/* Coordinates of Temperature image: */
#define COOR_ICON_TEMP_X   70
#define COOR_ICON_TEMP_Y   3

/* Coordinates of Humidity image: */
#define COOR_ICON_HUM_X   135
#define COOR_ICON_HUM_Y   3

/* Coordinates of AC image: */
#define COOR_ICON_AC_X   1
#define COOR_ICON_AC_Y   1

/* Coordinates of Battery image: */
#define COOR_ICON_BAT_X   33
#define COOR_ICON_BAT_Y   1


/* ICON Index (32 * 32 pixel image) */
typedef enum
{
	ICON_32X32_WIFI_S1 = 0,
	ICON_32X32_WIFI_S2,
	ICON_32X32_WIFI_S3,
	ICON_32X32_WIFI_S4,
	
    ICON_32X32_GSM_NOCARD,

	ICON_32X32_GSM_S1,
	ICON_32X32_GSM_S2,
	ICON_32X32_GSM_S3,
	ICON_32X32_GSM_S4,

	ICON_32X32_BAT_LEVEL0,	
	ICON_32X32_BAT_LEVEL1,		
	ICON_32X32_BAT_LEVEL2,		
	ICON_32X32_BAT_LEVEL3,	
	ICON_32X32_BAT_LEVEL4,	
	ICON_32X32_BAT_LEVEL5,	

	ICON_32X32_ACLINK,	
	ICON_32X32_ACBREAK,	

	ICON_32X32_SERVER_CONNECTED,	

	ICON_32X32_CLEAR,	

	ICON_32X32_SUM
}en_ICON_32X32_t;


extern const unsigned char ICON_32x32[ICON_32X32_SUM][128];

#endif
