/****************************************************
  * @Name	CRC16.c
  * @Brief	CRC16 sumcheck algorithm
  ***************************************************/
  
/*-------------Header Files Include-----------------*/
#include "crc16.h"


/*-------------Internal Functions Declaration------*/


/*-------------Module Variables Declaration--------*/
const unsigned short wCRCTableAbs[] ={
	0x0000, 0xCC01, 0xD801, 0x1400, 
	0xF001, 0x3C00, 0x2800, 0xE401, 
	0xA001, 0x6C00, 0x7800, 0xB401, 
	0x5000, 0x9C01, 0x8801, 0x4400,
};

/*-------------Module Functions Definition---------*/
/**
  * @Brief	CRC16(Modbus) sumcheck
  * @Param	ptr: point to the data provided
  *			len: data length
  * @Retval	Sumcheck value
  */
unsigned short Mid_CRC16_Modbus(unsigned char *ptr, unsigned int len) 
{
	unsigned short wCRC = 0xFFFF;
	unsigned short i;
	unsigned char chChar;
	unsigned char temp[2];
	
	for (i = 0; i < len; i++)
	{
		chChar = *ptr++;
		wCRC = wCRCTableAbs[(chChar ^ wCRC) & 15] ^ (wCRC >> 4);
		wCRC = wCRCTableAbs[((chChar >> 4) ^ wCRC) & 15] ^ (wCRC >> 4);
	}
	
	temp[0] = wCRC & 0xFF; 
	temp[1] = (wCRC >> 8) & 0xFF;
	wCRC = (temp[0] << 8) | temp[1];
	
	return wCRC;
}

/**
  * @Brief	CRC16(Modbus) sumcheck start with existing CRC16 check value
  * @Param	ptr	 : point to the data provided
  *			len	 : data length
  *			crc16: provided existing CRC16 check value
  * @Retval	Sumcheck value
  */
unsigned short Mid_CRC16_Modbus_Continuous(unsigned char *ptr, unsigned int len, unsigned short crc16)
{
	unsigned short i;
	unsigned char temp[2];
	unsigned char chChar;
	unsigned short wCRC;
	
	wCRC = crc16;
	
	for (i = 0; i < len; i++)
	{
		chChar = *ptr++;
		wCRC = wCRCTableAbs[(chChar ^ wCRC) & 15] ^ (wCRC >> 4);
		wCRC = wCRCTableAbs[((chChar >> 4) ^ wCRC) & 15] ^ (wCRC >> 4);
	}
	
	temp[0] = wCRC&0xFF; 
	temp[1] = (wCRC>>8)&0xFF;
	wCRC = (temp[0]<<8)|temp[1];	
	
	return wCRC;
}


/*-------------Internal Functions Definition--------*/


/*-------------Interrupt Functions Definition--------*/


