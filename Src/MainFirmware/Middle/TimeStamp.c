/****************************************************
  * @Name	TimeStamp.c
  * @Brief	
  *			TimeStamp_DateTime_Conversion: 
  *			Convert the given UNIX Timestamp to UTC datetime
  *	@Note	Define the UTC Timezone offset in the .h file when location changes
  ***************************************************/
  
/*-------------Header Files Include-----------------*/
#include "timestamp.h"

/*-------------Internal Functions Declaration------*/


/*-------------Module Variables Declaration--------*/
const unsigned int Month_Days[] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

/*---Module Call-Back function pointer Definition---*/


/*-------------Module Functions Definition---------*/
/**
  * @Brief	Check if the input year is a leap-year
  * @Param	year: input year in number
  * @Retval	0->not a leap year; 1->leap year
  */
unsigned int LeapYear(unsigned int year)
{
	return ((year % 4 == 0) && (year % 100 !=0)) || (year % 400 == 0);
}

/**
  * @Brief	Convert the TimeStamp to Datetime
  * @Param	TimeStamp: input UNIX TimeStamp
  *			pDateTime: point to the result string
  * @Retval	None
  */
void TimeStamp_DateTime_Conversion(unsigned long TimeStamp, unsigned char *pDateTime)
{
	unsigned int second, minute, hour, day, month, year;
	unsigned int m_days[12];
	unsigned int i;
	
	day = TimeStamp / 86400;
	second = TimeStamp % 86400;
	year = 1970;
	
	/* calculate the year since 1970 */
	while(1)
	{
		unsigned int year_days = LeapYear(year) ? 366 : 365;
		
		if(day >= year_days)
		{
			day -= year_days;
			year++;
		}
		else
		{
			break;
		}
	}
	
	/* calculate the month and date */
	
	for(i=0; i<12; i++)
	{
		m_days[i] = Month_Days[i];
	}
	
	if(LeapYear(year))
	{
		m_days[1] = 29;
	}
	
	for(month=0; month<12; month++)
	{
		if(day < m_days[month])
		{
			break;
		}
		
		day -= m_days[month];
	}
	
	day = day + 1;
	month = month + 1;
	
	/* calculate the hour, minue, second */
	if(second >= UTC_OFFSET_4)
	{
		hour = (second - UTC_OFFSET_4) / 3600;
	}
	else
	{
		hour = (second - UTC_OFFSET_4 + 86400) / 3600;
		day -= 1;
	}
	
	second %= 3600;
	minute = second / 60;
	second = second % 60;
	
	pDateTime[0] = (year / 1000) + '0';
	pDateTime[1] = ((year % 1000) / 100) + '0';
	pDateTime[2] = ((year % 100) / 10) + '0';
	pDateTime[3] = ((year % 10)) + '0';
	pDateTime[4] = '-';
	pDateTime[5] = (month / 10) + '0';
	pDateTime[6] = (month % 10) + '0';
	pDateTime[7] = '-';
	pDateTime[8] = (day / 10) + '0';
	pDateTime[9] = (day % 10) + '0';
	pDateTime[10] = '\0';

	pDateTime[11] = (hour / 10) + '0';
	pDateTime[12] = (hour % 10) + '0';
	pDateTime[13] = ':';
	pDateTime[14] = (minute / 10) + '0';
	pDateTime[15] = (minute % 10) + '0';
	pDateTime[16] = '\0';
}

/*-------------Internal Functions Definition--------*/


/*-------------Interrupt Functions Definition--------*/


