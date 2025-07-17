/****************************************************
  * @Name	StringProcess.c
  * @Brief	
  ***************************************************/
  
/*-------------Header Files Include-----------------*/
#include "stringprocess.h"

/*-------------Internal Functions Declaration------*/


/*-------------Module Variables Declaration--------*/


/*---Module Call-Back function pointer Definition---*/


/*-------------Module Functions Definition---------*/
/**
  * @Brief	Check if Str2 is a subString of Str1, if matching fail return 0xFF	
  * @Param	pStr1	: point to the Unknown String
  *			pStr2	: point to the Target String(reference)
  *			Str1_Len: length of Str1
  * @Retval	The starting matching <index> of Str1 if matching succeed
  *			0xFF -> matching fail
  */
unsigned int StringMatch(unsigned char *pStr1, unsigned char *pStr2, unsigned int Str1_Len)
{
	unsigned int i;
	unsigned int Str2_Len;
	unsigned int count;
	unsigned int MatchNum;
	unsigned int Flag;
	
	Str2_Len = GetStringLen(pStr2);
	
	if(Str1_Len >= Str2_Len)
	{
		count = Str1_Len - Str2_Len + 1;	// total compare times
	}
	else
	{
		return 0xFF;
	}
	
	MatchNum = 0;
	
	for(i=0; i<count; i++)
	{
		if(*pStr1 == *pStr2)
		{
			Flag = 1;	// find matching character
			
			while(Flag)
			{
				MatchNum++;
				
				if(MatchNum == Str2_Len)
				{
					return i;	// Matched: Str2 is a subString of Str1. 
								// String start matching index in Str1 is <i>
				}
				
				pStr1++;
				pStr2++;			// ready for comparing next character
				
				if(*pStr1 != *pStr2)
				{
					pStr1 -= MatchNum;
					pStr2 -= MatchNum;
					MatchNum = 0;
					Flag = 0;
					break;		// Match fail, reset all parameters for next starting point
				}
			}
		}
		
		pStr1++;
	}
	
	return 0xFF;
}

/**
  * @Brief	Get the length of target String
  * @Param	pData: point to the target String
  * @Retval	Length of target String(0->STRING_LENGTH_MAX)
  */
unsigned int GetStringLen(unsigned char *pData)
{
	unsigned int Len;
	
	Len = 0;
	
	while(*pData != 0)
	{
		Len++;
		
		if(Len > STRING_LENGTH_MAX)
		{
			break;
		}
		
		pData++;
	}
	return Len;
}

/**
  * @Brief	Data conversion from Hex to ASCII
  * @Param	pHexData   : input hex data
  *			pASCIIData : output ASCII data
  *			Len		   : length of data
  * @Retval	None
  */
void Hex_ASCII_Conversion(unsigned char *pHexData, unsigned char *pASCIIData, unsigned int Len)
{
	unsigned char DataBuff;
	unsigned char Data_H, Data_L;
	
	while(Len)
	{
		Len--;
		DataBuff = *pHexData;
		
		if((DataBuff & 0x0F) < 0x0A)
		{
			Data_L = ('0' + (DataBuff & 0x0F));
		}
		else
		{
			Data_L = ('A' + (DataBuff & 0x0F) - 10);
		}
		
		DataBuff >>= 4;
		
		if((DataBuff & 0x0F) < 0x0A)
		{
			Data_H = ('0' + (DataBuff & 0x0F));
		}
		else
		{
			Data_H = ('A' + (DataBuff & 0x0F) - 10);
		}
		
		*pASCIIData = Data_H;
		pASCIIData++;
		*pASCIIData = Data_L;
		pASCIIData++;
		
		pHexData++;
	}
}

/**
  * @Brief	Data conversion from Hex to ASCII in segments
  * @Param	HexData	: input hex data
  *			pData_H : point to the output high Hex
  *			pData_L : point to the output low Hex
  * @Retval	None
  */
void Hex_ASCII_Conversion_Segment(unsigned char HexData, unsigned char *pData_H, unsigned char *pData_L)
{
	if((HexData & 0x0F) < 0x0A)
	{
		*pData_L = ('0' + (HexData & 0x0F));
	}
	else
	{
		*pData_L = ('A' + (HexData & 0x0F) - 10);
	}
	
	HexData >>= 4;
	
	if((HexData & 0x0F) < 0x0A)
	{
		*pData_H = ('0' + (HexData & 0x0F));
	}
	else
	{
		*pData_H = ('A' + (HexData & 0x0F) - 10);
	}
}

/**
  * @Brief	Data conversion from ASCII to Hex
  * @Param	pASCII_Data: point to the input ASCII data 
  *			ASCII_Len  : input ASCII data length
  *			pHexData   : point to the output HexData 
  * @Retval	None
  */
void ASCII_Hex_Conversion(unsigned char *pASCII_Data, unsigned short ASCII_Len, unsigned char *pHexData)
{
	while(ASCII_Len)
	{
		*pHexData = 0;
		
		if((*pASCII_Data >= '0') && (*pASCII_Data <= '9'))
		{
			*pHexData = (*pASCII_Data - '0') << 4;
		}
		else if((*pASCII_Data >= 'A') && (*pASCII_Data <= 'F'))
		{
			*pHexData = (*pASCII_Data - 'A' + 10) << 4;
		}
		else if((*pASCII_Data >= 'a') && (*pASCII_Data <= 'f'))
		{
			*pHexData = (*pASCII_Data - 'a' + 10) << 4;
		}
		
		pASCII_Data++;
		
		if((*pASCII_Data >= '0') && (*pASCII_Data <= '9'))
		{
			*pHexData += (*pASCII_Data - '0');
		}
		else if((*pASCII_Data >= 'A') && (*pASCII_Data <= 'F'))
		{
			*pHexData += (*pASCII_Data - 'A' + 10);
		}
		else if((*pASCII_Data >= 'a') && (*pASCII_Data <= 'f'))
		{
			*pHexData += (*pASCII_Data - 'a' + 10);
		}
		
		pASCII_Data++;
		pHexData++;
		
		ASCII_Len -=2;
	}
}

/*-------------Internal Functions Definition--------*/


/*-------------Interrupt Functions Definition--------*/


