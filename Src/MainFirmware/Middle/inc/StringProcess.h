#ifndef __STRINGPROCESS_H_
#define __STRINGPROCESS_H_

/* define the String length limitation */
#define STRING_LENGTH_MAX	50

unsigned int StringMatch(unsigned char *pStr1, unsigned char *pStr2, unsigned int Str1_Len);
unsigned int GetStringLen(unsigned char *pData);

void Hex_ASCII_Conversion(unsigned char *pHexData, unsigned char *pASCIIData, unsigned int Len);
void Hex_ASCII_Conversion_Segment(unsigned char HexData, unsigned char *pData_H, unsigned char *pData_L);
void ASCII_Hex_Conversion(unsigned char *pASCII_Data, unsigned short ASCII_Len, unsigned char *pHexData);

#endif
