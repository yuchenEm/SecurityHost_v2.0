#ifndef __TIMESTAMP_H_
#define __TIMESTAMP_H_

/* UTC Timezone offset(in seconds) define: */
#define UTC_OFFSET_4	(3600 * 4)	// UTC-04:00 USA Eastern

unsigned int LeapYear(unsigned int year);
void TimeStamp_DateTime_Conversion(unsigned long TimeStamp, unsigned char *pDateTime);


#endif
